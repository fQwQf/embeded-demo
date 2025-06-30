#include "u1.h"

/*!
	\功能       延时
	\参数[输入] ms: 延时时间（毫秒）
	\参数[输出] 无
	\返回       无
*/
void u1_delay_ms(unsigned int ms)
{
	for(unsigned int i=0;i<ms;i++)
		for(unsigned int j=0;j<40000;j++);
}

/*!
	\功能       U1子板LED(RUN)初始化
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
void u1_led_init(void)
{
	/* 如果创建工程时选择的MCU的型号为GD32F450，编译如下代码，否则不编译 */
#if defined (GD32F450)
	/* 使能GPIOF组引脚的时钟 */
	rcu_periph_clock_enable(RCU_GPIOF);
	/* 复位GPIOF组引脚 */
	gpio_deinit(GPIOF);
	/* 设置GPIOF_4引脚为输出模式，无上下拉 */
	gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
	/* 设置GPIOF_4引脚输出模式为推挽模式，最高输出速度为50MHZ */
	gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
#endif

	/* 如果创建工程时选择的MCU的型号为GD32F470，编译如下代码，否则不编译 */
#if defined (GD32F470)
	/* 使能GPIOD组引脚的时钟 */
	rcu_periph_clock_enable(RCU_GPIOD);
	/* 复位GPIOD组引脚 */
	gpio_deinit(GPIOD);
	/* 设置GPIOD_15引脚为输出模式，无上下拉 */
	gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
	/* 设置GPIOD_15引脚输出模式为推挽模式，最高输出速度为50MHZ */
	gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
#endif
}

/*!
	\功能       U1子板LED(RUN)点亮
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
void u1_led_on(void)
{
#if defined (GD32F450)
	/* 设置GPIOF_4引脚输出电平为高电平 */
	gpio_bit_set(GPIOF, GPIO_PIN_4);
#endif

#if defined (GD32F470)
	/* 设置GPIOD_15引脚输出电平为低电平 */
	gpio_bit_reset(GPIOD, GPIO_PIN_15);
#endif
}

/*!
	\功能       U1子板LED(RUN)熄灭
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
void u1_led_off(void)
{
#if defined (GD32F450)
	/* 设置GPIOF_4引脚输出电平为低电平 */
	gpio_bit_reset(GPIOF, GPIO_PIN_4);
#endif

#if defined (GD32F470)
	/* 设置GPIOD_15引脚输出电平为高电平 */
	gpio_bit_set(GPIOD, GPIO_PIN_15);
#endif
}

/*!
	\功能       U1子板LED(RUN)状态翻转
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
void u1_led_toggle(void)
{
#if defined (GD32F450)
	/* 设置GPIOF_4引脚输出电平翻转 */
	gpio_bit_toggle(GPIOF, GPIO_PIN_4);
#endif

#if defined (GD32F470)
	/* 设置GPIOD_15引脚输出电平翻转 */
	gpio_bit_toggle(GPIOD, GPIO_PIN_15);
#endif
}

/*!
	\功能       Timer0初始化
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
void u1_timer0_init(void)
{
	/* 定义一个用于描述Timer0的结构体变量timer_init_struct */
	timer_parameter_struct timer_init_struct;

	/* 使能Timer0的时钟 */
	rcu_periph_clock_enable(RCU_TIMER0);
	/* 复位Timer0 */
	timer_deinit(TIMER0);
	/* 设置Timer0的时钟分频为20000，Timer0时钟频率=系统主频(200M)/时钟分频(20000)=10000，即Timer0计数器中的值1/10000秒加1 */
	timer_init_struct.prescaler         = 20000;
	/* 设置Timer0的对齐模式为边缘对齐 */
	timer_init_struct.alignedmode       = TIMER_COUNTER_EDGE;
	/* 设置Timer0的计数方向为递增计数 */
	timer_init_struct.counterdirection  = TIMER_COUNTER_UP;
	/* 设置Timer0的时钟分频为1分频 */
	timer_init_struct.clockdivision     = TIMER_CKDIV_DIV1;
	/* 设置Timer0的计数周期为5000，即当Timer0计数器中的值达到5000后自动复位 */
	timer_init_struct.period            = 5000;
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

/*!
	\功能       Timer0中断处理程序
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
//void TIMER0_UP_TIMER9_IRQHandler(void)
//{
	/* 确认是Timer0产生的中断 */
//	if(timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_UP))
//	{
		/* LED状态翻转 */
//		u1_led_toggle();
		/* 清除Timer0中断标志位 */
//		timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);
//	}
//}
