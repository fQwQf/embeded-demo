#include "s6.h"

i2c_slave_info s6_ultrasonic_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char S6_GD32_ADDR[] = {0x58, 0x5A, 0x5C, 0x5E};
#else
const static unsigned char S6_GD32_ADDR[] = {0x2C, 0x2D, 0x2E, 0x2F};
#endif

i2c_slave_info s6_ultrasonic_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(S6_GD32_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], S6_GD32_ADDR[j]);
			if(info.flag)
			{
				return info;
			}
		}
	}
	return info;
}

unsigned int s6_ultrasonic_distance_get(i2c_slave_info info)
{
	unsigned char buf[2] = {0};
	unsigned int distance = 0;

	if( i2c_reg_bytes_read(info, 0xAA, buf, 2) )
	{
		distance = (buf[0]<<8) | buf[1];
	}

	return distance;
}
