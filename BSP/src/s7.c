#include "s7.h"

i2c_slave_info s7_ir_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char S7_PCA9557_ADDR[] = {0x30, 0x32, 0x34, 0x36};
#else
const static unsigned char S7_PCA9557_ADDR[] = {0x18, 0x19, 0x1A, 0x1B};
#endif

static void s7_pca9557_init(i2c_slave_info info)
{
	i2c_reg_byte_write(info, 0x02, 0x00);
	i2c_reg_byte_write(info, 0x03, 0xFF);
}

i2c_slave_info s7_ir_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(S7_PCA9557_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], S7_PCA9557_ADDR[j]);
			if(info.flag)
			{
				s7_pca9557_init(info);
				return info;
			}
		}
	}
	return info;
}

unsigned char s7_ir_status_get(i2c_slave_info info)
{
	unsigned char status = 0;

	i2c_reg_bytes_read(info, 0x00, &status, 1);
	status &= 0x01;

	return status;
}
