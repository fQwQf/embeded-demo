#ifndef E1_H
#define E1_H

#include <string.h>
#include "i2c.h"

/* LED灯从机信息 */
extern i2c_slave_info e1_led_info;

/* LED灯函数声明 */
i2c_slave_info e1_led_init(void);
void e1_led_rgb_set(i2c_slave_info info, unsigned char red, unsigned char green, unsigned char blue);

/* 数码管从机信息 */
extern i2c_slave_info e1_tube_info;

/* 数码管函数声明 */
i2c_slave_info e1_tube_init(void);
void e1_tube_str_set(i2c_slave_info info, char * str);

#endif /* E1_H */

