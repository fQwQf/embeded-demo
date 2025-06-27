#include "delay.h"

/*!
	\功能       延时
	\参数[输入] ms: 延时时间（毫秒）
	\参数[输出] 无
	\返回       无
*/
void delay_ms(unsigned int ms)
{
	for(unsigned int i=0;i<ms;i++)
		for(unsigned int j=0;j<40000;j++);
}
