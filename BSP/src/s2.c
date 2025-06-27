#include "s2.h"

i2c_slave_info s2_illuminance_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char S2_BH1750_ADDR[] = {0x46, 0xB8};
#else
const static unsigned char S2_BH1750_ADDR[] = {0x23, 0x5C};
#endif

static void s2_bh1750_init(i2c_slave_info info)
{
	i2c_byte_write(info, 0x01);
}

i2c_slave_info s2_illuminance_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(S2_BH1750_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], S2_BH1750_ADDR[j]);
			if(info.flag)
			{
				s2_bh1750_init(info);
				return info;
			}
		}
	}
	return info;
}

unsigned int s2_illuminance_value_get(i2c_slave_info info)
{
	unsigned char buf[2];
	unsigned int illuminance = 0;

	i2c_byte_write(info, 0x01);
	i2c_byte_write(info, 0x10);
	i2c_delay_ms(10);
	i2c_bytes_read(info, buf, 2);
	illuminance = (buf[0]<<8) + buf[1];

	return illuminance;
}

i2c_slave_info s2_ths_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char S2_SHT3X_ADDR[] = {0x88, 0x8A};
#else
const static unsigned char S2_SHT3X_ADDR[] = {0x44, 0x45};
#endif

static void s2_sht3x_init(i2c_slave_info info)
{
	i2c_reg_byte_write(info, 0x30, 0xA2);
}

i2c_slave_info s2_ths_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(S2_SHT3X_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], S2_SHT3X_ADDR[j]);
			if(info.flag)
			{
				s2_sht3x_init(info);
				return info;
			}
		}
	}
	return info;
}

static unsigned char s2_sht3x_crc_cal(unsigned short data)
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

s2_ths_t s2_ths_value_get(i2c_slave_info info)
{
	s2_ths_t ths_value;
	unsigned char buf[6];
	unsigned short tmp;

	i2c_reg_byte_write(info, 0x2C, 0x0D);
	i2c_delay_ms(10);
	i2c_bytes_read(info, buf, 6);

	tmp = (buf[0]<<8) + buf[1];
	if(s2_sht3x_crc_cal(tmp) == buf[2])
	{
		ths_value.temp = (double)tmp/(65536-1)*175.0-45.0;
	}

	tmp = (buf[3]<<8) + buf[4];
	if(s2_sht3x_crc_cal(tmp) == buf[5])
	{
		ths_value.humi = (double)tmp/(65536-1)*100.0;
	}

	return ths_value;
}

i2c_slave_info s2_imu_info;

#if defined (GD32F450) || defined (GD32F470)
const static unsigned char S2_ICM20608_ADDR[] = {0xD0, 0xD2};
#else
const static unsigned char S2_ICM20608_ADDR[] = {0x68, 0x69};
#endif

static void s2_icm20608_init(i2c_slave_info info)
{
	i2c_reg_byte_write(info, 0x6B, 0x80);
	i2c_delay_ms(10);
	i2c_reg_byte_write(info, 0x6B, 0x01);
	i2c_delay_ms(10);
	i2c_reg_byte_write(info, 0x19, 0x00);
	i2c_reg_byte_write(info, 0x1B, 0x18);
	i2c_reg_byte_write(info, 0x1C, 0x18);
	i2c_reg_byte_write(info, 0x1A, 0x04);
	i2c_reg_byte_write(info, 0x1D, 0x04);
	i2c_reg_byte_write(info, 0x6C, 0x00);
	i2c_reg_byte_write(info, 0x1E, 0x00);
	i2c_reg_byte_write(info, 0x23, 0x00);
}

i2c_slave_info s2_imu_init(void)
{
	i2c_slave_info info;

	i2c_init();
	for(int i=0; i<sizeof(I2C_PERIPH_NUM)/sizeof(unsigned int); i++)
	{
		for(int j=0; j<sizeof(S2_ICM20608_ADDR)/sizeof(unsigned char); j++)
		{
			info = i2c_slave_detect(I2C_PERIPH_NUM[i], S2_ICM20608_ADDR[j]);
			if(info.flag)
			{
				s2_icm20608_init(info);
				return info;
			}
		}
	}
	return info;
}

s2_imu_t s2_imu_value_get(i2c_slave_info info)
{
	s2_imu_t imu_value;
	unsigned char buf[14] = {0};
	short acc_x, acc_y, acc_z, gyr_x, gyr_y, gyr_z, temp;

	i2c_reg_bytes_read(info, 0x3B, buf, 14);

	acc_x = (buf[0]  << 8) | buf[1]; 
	acc_y = (buf[2]  << 8) | buf[3]; 
	acc_z = (buf[4]  << 8) | buf[5]; 
	temp  = (buf[6]  << 8) | buf[7]; 
	gyr_x = (buf[8]  << 8) | buf[9]; 
	gyr_y = (buf[10] << 8) | buf[11];
	gyr_z = (buf[12] << 8) | buf[13];

	imu_value.acc_x = (float)(acc_x * 0.00478516);
	imu_value.acc_y = (float)(acc_y * 0.00478516);
	imu_value.acc_z = (float)(acc_z * 0.00478516);

	imu_value.temp  = (float)((temp - 25) * 10 / 3268) + 25;

	imu_value.gyr_x = (float)(gyr_x * 0.061);
	imu_value.gyr_y = (float)(gyr_y * 0.061);
	imu_value.gyr_z = (float)(gyr_z * 0.061);

	return imu_value;
}
