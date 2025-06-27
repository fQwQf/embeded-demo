#include "e2.h"

i2c_slave_info e2_fan_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char E2_PCA9685_ADDR[] = {0xC8, 0xCA, 0xCC, 0xCE};
#else
const static unsigned char E2_PCA9685_ADDR[] = {0x64, 0x65, 0x66, 0x67};
#endif

static void e2_pca9685_init(i2c_slave_info info)
{
	i2c_reg_byte_write(info, 0x00, 0x00);
}

i2c_slave_info e2_fan_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(E2_PCA9685_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], E2_PCA9685_ADDR[j]);
			if(info.flag)
			{
				e2_pca9685_init(info);
				return info;
			}
		}
	}
	return info;
}

static void e2_pca9685_pwm_set(i2c_slave_info info, unsigned char num, unsigned short on, unsigned short off)
{
	i2c_reg_byte_write(info, 0x06+4*num, on);
	i2c_reg_byte_write(info, 0x07+4*num, on>>8);
	i2c_reg_byte_write(info, 0x08+4*num, off);
	i2c_reg_byte_write(info, 0x09+4*num, off>>8);
}

void e2_fan_speed_set(i2c_slave_info info, unsigned char speed)
{	
	unsigned short on, off;

	if(speed > 100)
	{
		speed = 100;
	}
	on = 0x00;
	off = on + 0xfff*speed/100;
	e2_pca9685_pwm_set(info, 0, on, off);
}
