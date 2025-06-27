#include "e3.h"

i2c_slave_info e3_curtain_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char E3_GD32_ADDR[] = {0x38, 0x3A, 0x3C, 0x3E};
#else
const static unsigned char E3_GD32_ADDR[] = {0x1C, 0x1D, 0x1E, 0x1F};
#endif

i2c_slave_info e3_curtain_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(E3_GD32_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], E3_GD32_ADDR[j]);
			if(info.flag)
			{
				return info;
			}
		}
	}
	return info;
}

void e3_curtain_position_set(i2c_slave_info info, unsigned char position)
{
	i2c_reg_byte_write(info, 0x03, position);
}

unsigned char e3_curtain_position_get(i2c_slave_info info)
{
	unsigned char position;

	i2c_reg_bytes_read(info, 0x02, &position, 1);

	return position;
}

unsigned char e3_curtain_status_get(i2c_slave_info info)
{
	unsigned char status;

	i2c_reg_bytes_read(info, 0x01, &status, 1);

	return status;
}
