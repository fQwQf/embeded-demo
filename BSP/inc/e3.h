#ifndef E3_H
#define E3_H

#include "i2c.h"

/* 窗帘从机信息 */
extern i2c_slave_info e3_curtain_info;

/* 窗帘函数声明 */
i2c_slave_info e3_curtain_init(void);
void e3_curtain_position_set(i2c_slave_info info, unsigned char position);
unsigned char e3_curtain_position_get(i2c_slave_info info);
unsigned char e3_curtain_status_get(i2c_slave_info info);

#endif /* E3_H */
