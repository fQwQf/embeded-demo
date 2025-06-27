#ifndef S7_H
#define S7_H

#include "i2c.h"

/* 人体红外传感器从机信息 */
extern i2c_slave_info s7_ir_info;

/* 人体红外传感器函数声明 */
i2c_slave_info s7_ir_init(void);
unsigned char s7_ir_status_get(i2c_slave_info info);

#endif /* S7_H */
