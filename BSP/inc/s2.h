#ifndef S2_H
#define S2_H

#include "i2c.h"

/* 光照强度传感器从机信息 */
extern i2c_slave_info s2_illuminance_info;

/* 光照强度传感器函数声明 */
i2c_slave_info s2_illuminance_init(void);
unsigned int s2_illuminance_value_get(i2c_slave_info info);

/* 温湿度传感器测量结果 */
typedef struct
{
	float temp; /* 温度（摄氏度） */
	float humi; /* 湿度（百分比） */
}s2_ths_t;

/* 温湿度传感器从机信息 */
extern i2c_slave_info s2_ths_info;

/* 温湿度传感器函数声明 */
i2c_slave_info s2_ths_init(void);
s2_ths_t s2_ths_value_get(i2c_slave_info info);

/* 加速度&角速度传感器测量结果 */
typedef struct 
{
	float acc_x; /* x轴加速度(m/s2) */
	float acc_y; /* y轴加速度(m/s2) */
	float acc_z; /* z轴加速度(m/s2) */
	float gyr_x; /* x轴角速度(rad/s) */
	float gyr_y; /* y轴角速度(rad/s) */
	float gyr_z; /* z轴角速度(rad/s) */
	float temp;  /* 温度 */
}s2_imu_t;

/* 加速度&角速度传感器从机信息 */
extern i2c_slave_info s2_imu_info;

/* 加速度&角速度传感器函数声明 */
i2c_slave_info s2_imu_init(void);
s2_imu_t s2_imu_value_get(i2c_slave_info info);

#endif /* S2_H */
