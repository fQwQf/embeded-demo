#include <stdio.h>
#include <stdbool.h>

#include "u1.h"
#include "e1.h"

volatile unsigned int g_seconds_counter = 0; // 全局秒计数器
volatile bool g_second_has_passed = false;	 // 用于通知主循环时间已到

void hz_timer_init(void)
{
	/* 定义一个用于描述Timer0的结构体变量timer_init_struct */
	timer_parameter_struct timer_init_struct;

	/* 使能Timer0的时钟 */
	rcu_periph_clock_enable(RCU_TIMER0);
	/* 复位Timer0 */
	timer_deinit(TIMER0);
	/* 设置Timer0的时钟分频为20000，Timer0时钟频率=系统主频(200M)/时钟分频(20000)=10000，即Timer0计数器中的值1/10000秒加1 */
	timer_init_struct.prescaler = 20000;
	/* 设置Timer0的对齐模式为边缘对齐 */
	timer_init_struct.alignedmode = TIMER_COUNTER_EDGE;
	/* 设置Timer0的计数方向为递增计数 */
	timer_init_struct.counterdirection = TIMER_COUNTER_UP;
	/* 设置Timer0的时钟分频为1分频 */
	timer_init_struct.clockdivision = TIMER_CKDIV_DIV1;
	/* 设置Timer0的计数周期为10000，即当Timer0计数器中的值达到10000后自动复位 */
	timer_init_struct.period = 10000;
	/* 设置Timer0的重装载值为0，即当Timer0计数器中的值达到5000后自动清0 */
	timer_init_struct.repetitioncounter = 0;
	/* 按照timer_init_struct中的参数值初始化Timer0 */
	timer_init(TIMER0, &timer_init_struct);
	/* 设置Timer0计数器中的初始值为0 */
	timer_counter_value_config(TIMER0, 0);
	/* 使能Timer0开始计数 */
	timer_enable(TIMER0);
	/* 使能Timer0中断，即当Timer0计数器中的值达到5000后自动清0，同时Timer0向NVIC发送中断信号 */
	timer_interrupt_enable(TIMER0, TIMER_INT_UP);
	/* 在NVIC中使能Timer0中断，即Timer0发送的中断信号NVIC会转发给CPU，Timer0的中断优先级为1，中断子优先级为1 */
	nvic_irq_enable(TIMER0_UP_TIMER9_IRQn, 1, 1);
}

void TIMER0_UP_TIMER9_IRQHandler(void)
{

	if (timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_UP) != RESET)
	{
		// 保留来自 u1.c 的原有功能
		u1_led_toggle();

		// 增加全局秒计数器
		g_seconds_counter++;

		// 设置标志位
		g_second_has_passed = true;

		// 清除中断标志位
		// 必须清除，否则中断会不停地触发，导致程序卡死在中断里。
		timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);
	}
}

int main(void)
{
	char display_buf[10];

	e1_tube_info = e1_tube_init();

	// 初始化并启动硬件定时器中断
	hz_timer_init();

	e1_tube_str_set(e1_tube_info, "boot");

	while (1)
	{

		if (g_second_has_passed == true)
		{

			g_second_has_passed = false;

			sprintf(display_buf, "%4d", g_seconds_counter);
			e1_tube_str_set(e1_tube_info, display_buf);
		}
	}
}