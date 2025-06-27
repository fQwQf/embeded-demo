#ifndef S8_H
#define S8_H

#include "i2c.h"

/* 温湿度传感器测量结果 */
typedef struct
{
	float temp; /* 温度（摄氏度） */
	float humi; /* 湿度（百分比） */
}s8_ths_t;

/* 温湿度传感器从机信息 */
extern i2c_slave_info s8_ths_info;

/* 温湿度传感器函数声明 */
i2c_slave_info s8_ths_init(void);
s8_ths_t s8_ths_value_get(i2c_slave_info info);

#endif /* S8_H */
