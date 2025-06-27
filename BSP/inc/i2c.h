#ifndef I2C_H
#define I2C_H

#include "gd32f4xx.h"

/* I2C接口 */
const static unsigned int I2C_PERIPH_NUM[] = {I2C0, I2C1};

/* I2C参数 */
#define I2C0_SPEED         100000
#define I2C0_SLAVE_ADDR    0xA0

#define I2C1_SPEED         100000
#define I2C1_SLAVE_ADDR    0xA0

/* I2C从机信息 */
typedef struct
{
	unsigned int periph; /* 从机接口 */
	unsigned char addr;  /* 从机地址 */
	unsigned char flag;  /* 从机状态，0表示从机不存在，1表示从机存在 */
}i2c_slave_info;

/* I2C函数声明 */
void i2c_delay_ms(unsigned int ms);
void i2c_init(void);
i2c_slave_info i2c_slave_detect(unsigned int periph, unsigned char addr);
int i2c_byte_write(i2c_slave_info info, unsigned char byte);
int i2c_reg_byte_write(i2c_slave_info info, unsigned char reg, unsigned char byte);
int i2c_reg_bytes_write(i2c_slave_info info, unsigned char reg, unsigned char * pbytes, unsigned char count);
int i2c_bytes_read(i2c_slave_info info, unsigned char * pbytes, unsigned char count);
int i2c_reg_bytes_read(i2c_slave_info info, unsigned char reg, unsigned char * pbytes, unsigned char count);

#endif /* I2C_H */




