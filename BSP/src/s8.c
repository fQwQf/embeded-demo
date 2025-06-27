#include "s8.h"

i2c_slave_info s8_ths_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char S8_SHT3X_ADDR[] = {0x88, 0x8A};
#else
const static unsigned char S8_SHT3X_ADDR[] = {0x44, 0x45};
#endif

static void s8_sht3x_init(i2c_slave_info info)
{
	i2c_reg_byte_write(info, 0x30, 0xA2);
}

i2c_slave_info s8_ths_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(S8_SHT3X_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], S8_SHT3X_ADDR[j]);
			if(info.flag)
			{
				s8_sht3x_init(info);
				return info;
			}
		}
	}
	return info;
}

static unsigned char s8_sht3x_crc_cal(unsigned short data)
{
	unsigned char crc_byte, temp;

	crc_byte = 0xFF;
	temp = (data>>8) & 0xFF;
	for(int t=0; t<2; t++)
	{
		crc_byte ^= temp;
		for(int i=0; i<8; i++)
		{
			if(crc_byte & 0x80)
			{
				crc_byte <<= 1;
				crc_byte ^= 0x31;
			}
			else
			{
				crc_byte <<= 1;
			}
		}
		if(t == 0)
		{
			temp = data & 0xFF;
		}
	}
	return crc_byte;
}

s8_ths_t s8_ths_value_get(i2c_slave_info info)
{
	s8_ths_t ths_value;
	unsigned char buf[6];
	unsigned short tmp;

	i2c_reg_byte_write(info, 0x2C, 0x0D);
	i2c_delay_ms(10);
	i2c_bytes_read(info, buf, 6);

	tmp = (buf[0]<<8) + buf[1];
	if(s8_sht3x_crc_cal(tmp) == buf[2])
	{
		ths_value.temp = (double)tmp/(65536-1)*175.0-45.0;
	}

	tmp = (buf[3]<<8) + buf[4];
	if(s8_sht3x_crc_cal(tmp) == buf[5])
	{
		ths_value.humi = (double)tmp/(65536-1)*100.0;
	}

	return ths_value;
}
