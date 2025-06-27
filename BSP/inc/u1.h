#ifndef U1_H
#define U1_H

#include "gd32f4xx.h"

/* 延时函数声明 */
void u1_delay_ms(unsigned int ms);

/* LED函数声明 */
void u1_led_init(void);
void u1_led_on(void);
void u1_led_off(void);
void u1_led_toggle(void);

/* Timer函数声明 */
void u1_timer0_init(void);

#endif /* U1_H */
