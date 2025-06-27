#ifndef S1_H
#define S1_H

#include "i2c.h"

/* 按键键值定义 */
#define SWN     ( 0 )
#define SW1     ('1')
#define SW2     ('2')
#define SW3     ('3')
#define SW4     ('4')
#define SW5     ('5')
#define SW6     ('6')
#define SW7     ('7')
#define SW8     ('8')
#define SW9     ('9')
#define SW10    ('*')
#define SW11    ('0')
#define SW12    ('#')

/* 按键从机信息 */
extern i2c_slave_info s1_key_info;

/* 按键函数声明 */
i2c_slave_info s1_key_init(void);
char s1_key_value_get(i2c_slave_info info);

#endif /* S1_H */
