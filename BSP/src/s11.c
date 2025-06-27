#include "s11.h"

i2c_slave_info s11_scale_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char S11_GD32_ADDR[] = {0x60, 0x62, 0x64, 0x66};
#else
const static unsigned char S11_GD32_ADDR[] = {0x30, 0x31, 0x32, 0x33};
#endif

i2c_slave_info s11_scale_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(S11_GD32_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], S11_GD32_ADDR[j]);
			if(info.flag)
			{
				return info;
			}
		}
	}
	return info;
}

unsigned int s11_scale_weight_get(i2c_slave_info info)
{
	unsigned char buf[2] = {0};
	unsigned int weight = 0;

	if( i2c_reg_bytes_read(info, 0x01, buf, 2) )
	{
		weight = (buf[0]<<8) | buf[1];
		weight /= 10;
	}

	return weight;
}
