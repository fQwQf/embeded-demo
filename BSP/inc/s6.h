#ifndef S6_H
#define S6_H

#include "i2c.h"

/* 超声波传感器从机信息 */
extern i2c_slave_info s6_ultrasonic_info;

/* 超声波传感器函数声明 */
i2c_slave_info s6_ultrasonic_init(void);
unsigned int s6_ultrasonic_distance_get(i2c_slave_info info);

#endif /* S6_H */
