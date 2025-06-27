#ifndef E2_H
#define E2_H

#include "i2c.h"

/* 风扇从机信息 */
extern i2c_slave_info e2_fan_info;

/* 风扇函数声明 */
i2c_slave_info e2_fan_init(void);
void e2_fan_speed_set(i2c_slave_info info, unsigned char speed);

#endif /* E2_H */

