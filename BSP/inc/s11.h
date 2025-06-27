#ifndef S11_H
#define S11_H

#include "i2c.h"

/* 称重传感器从机信息 */
extern i2c_slave_info s11_scale_info;

/* 称重传感器函数声明 */
i2c_slave_info s11_scale_init(void);
unsigned int s11_scale_weight_get(i2c_slave_info info);

#endif /* S11_H */
