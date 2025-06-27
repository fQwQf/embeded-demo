#include "s1.h"

i2c_slave_info s1_key_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char S1_HT16K33_ADDR[] = {0xE8, 0xEA, 0xEC, 0xEE};
#else
const static unsigned char S1_HT16K33_ADDR[] = {0x74, 0x75, 0x76, 0x77};
#endif

static void s1_ht16k33_init(i2c_slave_info info)
{
	i2c_byte_write(info, 0x21);
}

i2c_slave_info s1_key_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(S1_HT16K33_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], S1_HT16K33_ADDR[j]);
			if(info.flag)
			{
				s1_ht16k33_init(info);
				return info;
			}
		}
	}
	return info;
}

char s1_key_value_get(i2c_slave_info info)
{
	unsigned char buf[6] = {0};

	i2c_reg_bytes_read(info, 0x40, buf, sizeof(buf));

	if(buf[0] & 0x01)
	{
		return SW1;
	}
	else if(buf[2] & 0x01)
	{
		return SW2;
	}
	else if(buf[4] & 0x01)
	{
		return SW3;
	}
	else if(buf[0] & 0x02)
	{
		return SW4;
	}
	else if(buf[2] & 0x02)
	{
		return SW5;
	}
	else if(buf[4] & 0x02)
	{
		return SW6;
	}
	else if(buf[0] & 0x04)
	{
		return SW7;
	}
	else if(buf[2] & 0x04)
	{
		return SW8;
	}
	else if(buf[4] & 0x04)
	{
		return SW9;
	}
	else if(buf[0] & 0x08)
	{
		return SW10;
	}
	else if(buf[2] & 0x08)
	{
		return SW11;
	}
	else if(buf[4] & 0x08)
	{
		return SW12;
	}
	else
	{
		return SWN;
	}
}
