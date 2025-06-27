#include "s5.h"

i2c_slave_info s5_nfc_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char S5_MS523_ADDR[] = {0x50, 0x52, 0x54, 0x56};
#else
const static unsigned char S5_MS523_ADDR[] = {0x28, 0x29, 0x2a, 0x2b};
#endif

/*!
	\brief      clear register bit
	\param[in]  reg:register
	\param[in]  mask:bit mask
	\param[out] none
	\retval     none
*/
static void s5_ms523_bit_clear(i2c_slave_info info, unsigned char reg, unsigned char mask)
{
	unsigned char temp = 0x00;

	i2c_reg_bytes_read(info, reg, &temp, 1);
	i2c_reg_byte_write(info, reg, temp & ~mask);
}

/*!
	\brief      set register bit
	\param[in]  reg:register
	\param[in]  mask:bit mask
	\param[out] none
	\retval     none
*/
static void s5_ms523_bit_set(i2c_slave_info info, unsigned char reg, unsigned char mask)
{
	unsigned char temp = 0x00;

	i2c_reg_bytes_read(info, reg, &temp, 1);
	i2c_reg_byte_write(info, reg, temp | mask);
}

/*!
	\brief      turn on the antenna
	\param[in]  none
	\param[out] none
	\retval     none
*/
static void s5_ms523_antenna_on(i2c_slave_info info)
{
	unsigned char i;

	i2c_reg_bytes_read(info, TxControlReg, &i, 1);

	if (!(i & 0x03))
	{
		s5_ms523_bit_set(info, TxControlReg, 0x03);
	}
}

/*!
	\brief      turn off the antenna
	\param[in]  none
	\param[out] none
	\retval     none
*/
static void s5_ms523_antenna_off(i2c_slave_info info)
{
	s5_ms523_bit_clear(info, TxControlReg, 0x03);
}

/*!
	\brief      reset MS523
	\param[in]  none
	\param[out] none
	\retval     none
*/
static void s5_ms523_reset(i2c_slave_info info)
{
	i2c_reg_byte_write(info, CommandReg, PCD_RESETPHASE);
	i2c_delay_ms(10);

	i2c_reg_byte_write(info, ModeReg, 0x3D);
	i2c_reg_byte_write(info, TReloadRegL, 30);
	i2c_reg_byte_write(info, TReloadRegH, 0);
	i2c_reg_byte_write(info, TModeReg, 0x8D);
	i2c_reg_byte_write(info, TPrescalerReg, 0x3E);
	i2c_reg_byte_write(info, TxAutoReg, 0x40);
	s5_ms523_bit_clear(info, TestPinEnReg, 0x80);
	i2c_reg_byte_write(info, TxAutoReg, 0x40);
}

/*!
	\brief      Communication with iso 14443 card
	\param[in]  Command:Command
	\param[in]  pInData:send data		
	\param[in]  InLenByte:Send data length
	\param[out] pOutData:receive data
	\param[out] pOutLenBit:receive data bit lenth
	\retval     Communication status
*/
#define MAXRLEN		18
static char s5_ms523_comm(i2c_slave_info info, unsigned char Command, unsigned char *pInData, unsigned char InLenByte, unsigned char *pOutData, unsigned short  *pOutLenBit)
{
	char status = MI_ERR;
	unsigned char irqEn = 0x00;
	unsigned char waitFor = 0x00;
	unsigned char lastBits;
	unsigned char n;
	unsigned short i;

	switch(Command)
	{
		case PCD_AUTHENT:
			irqEn = 0x12;
			waitFor = 0x10;
			break;
		case PCD_TRANSCEIVE:
			irqEn = 0x77;
			waitFor = 0x30;
			break;
		default:
			break;
	}

	i2c_reg_byte_write(info, ComIEnReg, irqEn | 0x80);
	s5_ms523_bit_clear(info, ComIrqReg, 0x80);
	i2c_reg_byte_write(info, CommandReg, PCD_IDLE);
	s5_ms523_bit_set(info, FIFOLevelReg, 0x80);

	i2c_reg_bytes_write(info, FIFODataReg, pInData, InLenByte);
	i2c_reg_byte_write(info, CommandReg, Command);
	if (Command == PCD_TRANSCEIVE)
	{
		s5_ms523_bit_set(info, BitFramingReg, 0x80);
	}

	i = 1000;
	do
	{
		i2c_reg_bytes_read(info, ComIrqReg, &n, 1);
		i--;
	}while ((i != 0) && !(n & 0x01) && !(n & waitFor));

	s5_ms523_bit_clear(info, BitFramingReg, 0x80);

	if (i != 0)
	{
		i2c_reg_bytes_read(info, ErrorReg, &n, 1);
		if (!(n & 0x1B))
		{
			status = MI_OK;
			if (n & irqEn & 0x01)
			{
				status = MI_NOTAGERR;
			}
			if (Command == PCD_TRANSCEIVE)
			{
				i2c_reg_bytes_read(info, FIFOLevelReg, &n, 1);
				i2c_reg_bytes_read(info, ControlReg, &lastBits, 1);
				lastBits &= 0x07;
				if (lastBits)
				{
					*pOutLenBit = (n - 1) * 8 + lastBits;
				}
				else
				{
					*pOutLenBit = n * 8;
				}
				if (n == 0)
				{
					n = 1;
				}
				if (n > MAXRLEN)
				{
					n = MAXRLEN;
				}
				i2c_reg_bytes_read(info, FIFODataReg, pOutData, n);
			}
		}
		else
		{
			status = MI_ERR;
		}
	}

	s5_ms523_bit_set(info, ControlReg, 0x80);
	i2c_reg_byte_write(info, CommandReg, PCD_IDLE);

	return status;
}

/*!
	\brief      Calulate CRC16
	\param[in]  pInData:send data		
	\param[in]  InLenByte:Send data length
	\param[out] pOutData:receive data
	\retval     none
*/
static void s5_ms523_crc_calc(i2c_slave_info info, unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
	unsigned char i, n;

	s5_ms523_bit_clear(info, DivIrqReg, 0x04);
	i2c_reg_byte_write(info, CommandReg, PCD_IDLE);
	s5_ms523_bit_set(info, FIFOLevelReg, 0x80);

	i2c_reg_bytes_write(info, FIFODataReg, pIndata, len);
	i2c_reg_byte_write(info, CommandReg, PCD_CALCCRC);

	i = 0xFF;
	do
	{
		i2c_reg_bytes_read(info, DivIrqReg, &n, 1);
		i--;
	}while((i!=0) && !(n&0x04));

	i2c_reg_bytes_read(info, CRCResultRegL, &pOutData[0], 1);
	i2c_reg_bytes_read(info, CRCResultRegM, &pOutData[1], 1);
}

/*!
	\brief      Set the mode of MS523
	\param[in]  type:mode
	\param[out] none
	\retval     status
*/
static char s5_ms523_type_config(i2c_slave_info info, unsigned char type)
{
	if('A' == type)
	{
		i2c_reg_byte_write(info, Status2Reg, 0x08);
		i2c_reg_byte_write(info, ModeReg, 0x3D);
		i2c_reg_byte_write(info, RxSelReg, 0x86);
		i2c_reg_byte_write(info, RFCfgReg, 0x7F);
		i2c_reg_byte_write(info, TReloadRegL, 30);
		i2c_reg_byte_write(info, TReloadRegH, 0);
		i2c_reg_byte_write(info, TModeReg, 0x8D);
		i2c_reg_byte_write(info, TPrescalerReg, 0x3E);
		i2c_delay_ms(10);
		s5_ms523_antenna_on(info);
	}
	else
	{
		return MI_ERR;
	}

	return MI_OK;
}

static void s5_ms523_init(i2c_slave_info info)
{
	s5_ms523_reset(info);
	s5_ms523_antenna_off(info);
	i2c_delay_ms(10);
	s5_ms523_antenna_on(info);
	s5_ms523_type_config(info, 'A');
}

i2c_slave_info s5_nfc_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(S5_MS523_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], S5_MS523_ADDR[j]);
			if(info.flag)
			{
				s5_ms523_init(info);
				return info;
			}
		}
	}
	return info;
}

/*!
	\brief			Card search
	\param[in]	type:Card searching mode
	\param[in]	pTagType:Card type
							0x4400 = Mifare_UltraLight
							0x0400 = Mifare_One(S50)
							0x0200 = Mifare_One(S70)
							0x0800 = Mifare_Pro(X)
							0x4403 = Mifare_DESFire
	\param[out]	none
	\retval			status
*/
char s5_nfc_request(i2c_slave_info info, unsigned char RequestType, unsigned char *pCardType)
{
	char status;
	unsigned short unLen;
	unsigned char ucComMS523Buf[MAXRLEN];

	s5_ms523_bit_clear(info, Status2Reg, 0x08);
	i2c_reg_byte_write(info, BitFramingReg, 0x07);
	s5_ms523_bit_set(info, TxControlReg, 0x03);

	ucComMS523Buf[0] = RequestType;
	status = s5_ms523_comm(info, PCD_TRANSCEIVE, ucComMS523Buf, 1, ucComMS523Buf, &unLen);
	if ((status == MI_OK) && (unLen == 0x10))
	{
		*pCardType = ucComMS523Buf[0];
		*(pCardType + 1) = ucComMS523Buf[1];
	}
	else
	{
		status = MI_ERR;
	}

	return status;
}

/*!
	\brief			card anti-collision
	\param[out]	pSnr:serial number,4bytes
	\param[in]	none
	\retval			status
*/
char s5_nfc_anticoll(i2c_slave_info info, unsigned char *pCardID)
{
	char status;
	unsigned char i, snr_check = 0;
	unsigned short unLen;
	unsigned char ucComMS523Buf[MAXRLEN];

	s5_ms523_bit_clear(info, Status2Reg, 0x08);
	i2c_reg_byte_write(info, BitFramingReg, 0x00);
	s5_ms523_bit_clear(info, CollReg, 0x80);

	ucComMS523Buf[0] = PICC_ANTICOLL1;
	ucComMS523Buf[1] = 0x20;
	status = s5_ms523_comm(info, PCD_TRANSCEIVE, ucComMS523Buf, 2, ucComMS523Buf, &unLen);
	if (status == MI_OK)
	{
		for (i=0; i<4; i++)
		{
			*(pCardID+i)  = ucComMS523Buf[i];
			snr_check ^= ucComMS523Buf[i];
		}
		if (snr_check != ucComMS523Buf[i])
		{
			status = MI_ERR;    
		}
	}

	s5_ms523_bit_set(info, CollReg, 0x80);

	return status;
}

/*!
	\brief      Selected card
	\param[in]  pSnr:serial number,4bytes
	\param[out] none
	\retval     status
*/
char s5_nfc_select(i2c_slave_info info, unsigned char *pCardID)
{
	char status;
	unsigned char i;
	unsigned short unLen;
	unsigned char ucComMS523Buf[MAXRLEN];

	ucComMS523Buf[0] = PICC_ANTICOLL1;
	ucComMS523Buf[1] = 0x70;
	ucComMS523Buf[6] = 0;
	for(i=0;i<4;i++)
	{
		ucComMS523Buf[i+2] = *(pCardID+i);
		ucComMS523Buf[6]  ^= *(pCardID+i);
	}
	s5_ms523_crc_calc(info, ucComMS523Buf, 7, &ucComMS523Buf[7]);
	s5_ms523_bit_clear(info, Status2Reg, 0x08);

	status = s5_ms523_comm(info, PCD_TRANSCEIVE, ucComMS523Buf, 9, ucComMS523Buf, &unLen);
	if ((status == MI_OK) && (unLen == 0x18))
	{
		status = MI_OK;
	}
	else
	{
		status = MI_ERR;
	}

	return status;
}

/*!
	\brief      Verify card password
	\param[in]  auth_mode:Password authentication mode
	\param[in]  addr:Block address
	\param[in]  pKey:Password
	\param[in]  pSnr:serial number,4bytes
	\param[out] none
	\retval     status
*/
char s5_nfc_auth(i2c_slave_info info, unsigned char AuthMode, unsigned char addr, unsigned char *pKey, unsigned char *pCardID)
{
	char status;
	unsigned short unLen;
	unsigned char i, ucComMS523Buf[MAXRLEN];
	unsigned char temp;

	ucComMS523Buf[0] = AuthMode;
	ucComMS523Buf[1] = addr;
	for (i=0;i<6;i++)
	{    
		ucComMS523Buf[i+2] = *(pKey+i);
	}
	for (i=0;i<4;i++)
	{    
		ucComMS523Buf[i+8] = *(pCardID+i);
	}
	status = s5_ms523_comm(info, PCD_AUTHENT, ucComMS523Buf, 12, ucComMS523Buf, &unLen);
	i2c_reg_bytes_read(info, Status2Reg, &temp, 1);
	if((status != MI_OK) || (!(temp & 0x08)))
	{   
		status = MI_ERR;   
	}

	return status;
}

/*!
	\brief      read M1 card data of block
	\param[in]  addr:Block address
	\param[out] pData: data
	\retval     status
*/
char s5_nfc_read(i2c_slave_info info, unsigned char addr, unsigned char *pData)
{
	char status;
	unsigned short unLen;
	unsigned char i, ucComMS523Buf[MAXRLEN]; 

	ucComMS523Buf[0] = PICC_READ;
	ucComMS523Buf[1] = addr;
	s5_ms523_crc_calc(info, ucComMS523Buf, 2, &ucComMS523Buf[2]);
	status = s5_ms523_comm(info, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);
	if((status == MI_OK) && (unLen == 0x90))
	{
		for (i=0; i<16; i++)
		{    
			*(pData+i) = ucComMS523Buf[i];   
		}
	}
	else
	{
		status = MI_ERR;   
	}

	return status;
}

/*!
	\brief      write data of block to M1 card
	\param[in]  addr:Block address
	\param[in] 	pData: data
	\retval     status
*/                
char s5_nfc_write(i2c_slave_info info, unsigned char addr, unsigned char *pData)
{
	char status;
	unsigned short unLen;
	unsigned char i, ucComMS523Buf[MAXRLEN]; 

	ucComMS523Buf[0] = PICC_WRITE;
	ucComMS523Buf[1] = addr;
	s5_ms523_crc_calc(info, ucComMS523Buf, 2, &ucComMS523Buf[2]);
	status = s5_ms523_comm(info, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);
	if ((status != MI_OK) || (unLen != 4) || ((ucComMS523Buf[0] & 0x0F) != 0x0A))
	{
		status = MI_ERR;   
	}
	if (status == MI_OK)
	{
		for (i=0; i<16; i++)
		{
			ucComMS523Buf[i] = *(pData+i);
		}
		s5_ms523_crc_calc(info, ucComMS523Buf, 16, &ucComMS523Buf[16]);
		status = s5_ms523_comm(info,PCD_TRANSCEIVE, ucComMS523Buf, 18, ucComMS523Buf, &unLen);
		if ((status != MI_OK) || (unLen != 4) || ((ucComMS523Buf[0] & 0x0F) != 0x0A))
		{
			status = MI_ERR;
		}
	}

	return status;
}

/*!
	\brief      Enter sleep mode
	\param[in]  none
	\param[out] none
	\retval     status
*/
char s5_nfc_halt(i2c_slave_info info)
{
	unsigned short unLen;
	unsigned char ucComMS523Buf[MAXRLEN]; 

	ucComMS523Buf[0] = PICC_HALT;
	ucComMS523Buf[1] = 0;
	s5_ms523_crc_calc(info, ucComMS523Buf, 2, &ucComMS523Buf[2]);
	s5_ms523_comm(info, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);

	return MI_OK;
}
