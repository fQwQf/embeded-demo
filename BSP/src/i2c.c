#include "i2c.h"

/*!
	\功能       延时
	\参数[输入] ms: 延时时间（毫秒）
	\参数[输出] 无
	\返回       无
*/
void i2c_delay_ms(unsigned int ms)
{
	for(unsigned int i=0;i<ms;i++)
		for(unsigned int j=0;j<40000;j++);
}

/*!
	\功能       I2C0接口配置
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
static void i2c0_gpio_config(void)
{
	/* 使能GPIOB组引脚的时钟 */
	rcu_periph_clock_enable(RCU_GPIOB);

	/* 设置GPIOB_8引脚为复用功能模式，内部上拉 */
	gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_8);
	/* 设置GPIOB_8引脚为开漏输出模式，最高输出速度为50MHZ */
	gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	/* 设置GPIOB_8引脚为I2C0的SCL功能 */
	gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_8);

	/* 设置GPIOB_9引脚为复用功能模式，内部上拉 */
	gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
	/* 设置GPIOB_9引脚为开漏输出模式，最高输出速度为50MHZ */
	gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	/* 设置GPIOB_9引脚为I2C0的SDA功能 */
	gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_9);
}

/*!
	\功能       I2C0参数配置
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
static void i2c0_parm_config(void)
{
	/* 使能I2C0的时钟 */
	rcu_periph_clock_enable(RCU_I2C0);
	/* 设置I2C0的时钟速度为I2C0_SPEED */
	i2c_clock_config(I2C0, I2C0_SPEED, I2C_DTCY_2);
	/* 设置I2C0的从机地址为I2C0_SLAVE_ADDR，地址格式为7bits */
	i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2C0_SLAVE_ADDR);
	/* 使能I2C0应答 */
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
	/* 使能I2C0 */
	i2c_enable(I2C0);
}

/*!
	\功能       I2C1接口配置
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
static void i2c1_gpio_config(void)
{
	/* 使能GPIOF组引脚的时钟 */
	rcu_periph_clock_enable(RCU_GPIOF);

	/* 设置GPIOF_1引脚为复用功能模式，内部上拉 */
	gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_1);
	/* 设置GPIOF_1引脚为开漏输出模式，最高输出速度为50MHZ */
	gpio_output_options_set(GPIOF, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
	/* 设置GPIOF_1引脚为I2C1的SCL功能 */
	gpio_af_set(GPIOF, GPIO_AF_4, GPIO_PIN_1);

	/* 设置GPIOF_0引脚为复用功能模式，内部上拉 */
	gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0);
	/* 设置GPIOF_0引脚为开漏输出模式，最高输出速度为50MHZ */
	gpio_output_options_set(GPIOF, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
	/* 设置GPIOF_0引脚为I2C1的SDA功能 */
	gpio_af_set(GPIOF, GPIO_AF_4, GPIO_PIN_0);
}

/*!
	\功能       I2C1参数配置
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
static void i2c1_parm_config(void)
{
	/* 使能I2C1的时钟 */
	rcu_periph_clock_enable(RCU_I2C1);
	/* 设置I2C1的时钟速度为I2C1_SPEED */
	i2c_clock_config(I2C1, I2C1_SPEED, I2C_DTCY_2);
	/* 设置I2C1的从机地址为I2C1_SLAVE_ADDR，地址格式为7bits */
	i2c_mode_addr_config(I2C1, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2C1_SLAVE_ADDR);
	/* 使能I2C1应答 */
	i2c_ack_config(I2C1, I2C_ACK_ENABLE);
	/* 使能I2C1 */
	i2c_enable(I2C1);
}

/*!
	\功能       I2C初始化（包括I2C0和I2C1）
	\参数[输入] 无
	\参数[输出] 无
	\返回       无
*/
void i2c_init(void)
{
	/* I2C0接口配置 */
	i2c0_gpio_config();
	/* I2C0参数配置 */
	i2c0_parm_config();

	/* I2C1接口配置 */
	i2c1_gpio_config();
	/* I2C1参数配置 */
	i2c1_parm_config();
	/* 延时1ms */
	i2c_delay_ms(1);
}

/*!
	\功能       I2C从机检测
	\参数[输入] periph: I2C从机接口，I2Cx(x=0,1,2)
	\参数[输入] addr  : I2C从机地址
	\参数[输出] 无
	\返回       I2C从机信息
*/
i2c_slave_info i2c_slave_detect(unsigned int periph, unsigned char addr)
{
	/* I2C从机信息 */
	i2c_slave_info info =
	{
		.periph = periph,
		.addr = addr,
		.flag = 0,
	};
	unsigned int i = 0;

	/* 等待I2C总线变为空闲状态 */
	while(i2c_flag_get(periph, I2C_FLAG_I2CBSY));
	/* 向I2C总线上发送起始信号 */
	i2c_start_on_bus(periph);
	/* 等待SBSEND位置位，即等待起始信号发送完成 */
	while(!i2c_flag_get(periph, I2C_FLAG_SBSEND));
	/* 向I2C总线上发送从机地址，指定后续数据为主机发送 */
	i2c_master_addressing(periph, addr, I2C_TRANSMITTER);
	/* 等待ADDSEND位置位，即等待从机地址发送完成 */
	while(!i2c_flag_get(periph, I2C_FLAG_ADDSEND))
	{
		/* 等待超时 */
		if(++i > 100000)
		{
			/* 向I2C总线上发送停止信号 */
			i2c_stop_on_bus(info.periph);
			/* 从机状态清0，即从机不存在 */
			info.flag = 0;
			/* 返回I2C从机信息 */
			return info;
		}
	}
	/* 清除ADDSEND位 */
	i2c_flag_clear(info.periph, I2C_FLAG_ADDSEND);
	/* 向I2C总线上发送停止信号 */
	i2c_stop_on_bus(periph);
	/* 从机状态置1，即从机存在 */
	info.flag = 1;
	/* 返回I2C从机信息 */
	return info;
}

/*!
	\功能       向I2C从机发送一个字节数据
	\参数[输入] info: I2C从机信息
	\参数[输入] byte: 要发送的数据
	\参数[输出] 无
	\返回       执行结果，0表示失败，1表示成功
*/
int i2c_byte_write(i2c_slave_info info, unsigned char byte)
{
	unsigned int i = 0;

	/* 等待I2C总线变为空闲状态 */
	while(i2c_flag_get(info.periph, I2C_FLAG_I2CBSY));
	/* 向I2C总线上发送起始信号 */
	i2c_start_on_bus(info.periph);
	/* 等待SBSEND位置位，即等待起始信号发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_SBSEND));
	/* 向I2C总线上发送从机地址，指定后续数据为主机发送 */
	i2c_master_addressing(info.periph, info.addr, I2C_TRANSMITTER);
	/* 等待ADDSEND位置位，即等待从机地址发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_ADDSEND))
	{
		/* 等待超时 */
		if(++i > 100000)
		{
			/* 向I2C总线上发送停止信号 */
			i2c_stop_on_bus(info.periph);
			/* 返回执行结果 */
			return 0;
		}
	}
	/* 清除ADDSEND位 */
	i2c_flag_clear(info.periph, I2C_FLAG_ADDSEND);
	/* 等待TBE位置位，即等待发送缓冲区为空 */
	while(SET != i2c_flag_get(info.periph, I2C_FLAG_TBE));
	/* 向从机发送一个字节数据 */
	i2c_data_transmit(info.periph, byte);
	/* 等待BTC位置位，即等待数据发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
	/* 向I2C总线上发送停止信号 */
	i2c_stop_on_bus(info.periph);
	/* 返回执行结果 */
	return 1;
}

/*!
	\功能       向I2C从机的寄存器中写入一个字节数据
	\参数[输入] info: I2C从机信息
	\参数[输入] reg : 寄存器的地址
	\参数[输入] byte: 要写入的数据
	\参数[输出] 无
	\返回       执行结果，0表示失败，1表示成功
*/
int i2c_reg_byte_write(i2c_slave_info info, unsigned char reg, unsigned char byte)
{
	unsigned int i = 0;

	/* 等待I2C总线变为空闲状态 */
	while(i2c_flag_get(info.periph, I2C_FLAG_I2CBSY));
	/* 向I2C总线上发送起始信号 */
	i2c_start_on_bus(info.periph);
	/* 等待SBSEND位置位，即等待起始信号发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_SBSEND));
	/* 向I2C总线上发送从机地址，指定后续数据为主机发送 */
	i2c_master_addressing(info.periph, info.addr, I2C_TRANSMITTER);
	/* 等待ADDSEND位置位，即等待从机地址发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_ADDSEND))
	{
		/* 等待超时 */
		if(++i > 100000)
		{
			/* 向I2C总线上发送停止信号 */
			i2c_stop_on_bus(info.periph);
			/* 返回执行结果 */
			return 0;
		}
	}
	/* 清除ADDSEND位 */
	i2c_flag_clear(info.periph, I2C_FLAG_ADDSEND);
	/* 等待TBE位置位，即等待发送缓冲区为空 */
	while(SET != i2c_flag_get(info.periph, I2C_FLAG_TBE));
	/* 向从机发送寄存器地址 */
	i2c_data_transmit(info.periph, reg);
	/* 等待BTC位置位，即等待寄存器地址发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
	/* 延时2ms */
	i2c_delay_ms(2);
	/* 等待TBE位置位，即等待发送缓冲区为空 */
	while(SET != i2c_flag_get(info.periph, I2C_FLAG_TBE));
	/* 向从机发送一个字节数据 */
	i2c_data_transmit(info.periph, byte);
	/* 等待BTC位置位，即等待数据发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
	/* 向I2C总线上发送停止信号 */
	i2c_stop_on_bus(info.periph);
	/* 返回执行结果 */
	return 1;
}

/*!
	\功能       向I2C从机的寄存器中写入多个字节数据
	\参数[输入] info  : I2C从机信息
	\参数[输入] reg   : 寄存器的地址
	\参数[输入] pbytes: 要写入的数据
	\参数[输入] count : 要写入的个数
	\参数[输出] 无
	\返回       执行结果，0表示失败，1表示成功
*/
int i2c_reg_bytes_write(i2c_slave_info info, unsigned char reg, unsigned char * pbytes, unsigned char count)
{
	unsigned int i = 0;

	/* 等待I2C总线变为空闲状态 */
	while(i2c_flag_get(info.periph, I2C_FLAG_I2CBSY));
	/* 向I2C总线上发送起始信号 */
	i2c_start_on_bus(info.periph);
	/* 等待SBSEND位置位，即等待起始信号发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_SBSEND));
	/* 向I2C总线上发送从机地址，指定后续数据为主机发送 */
	i2c_master_addressing(info.periph, info.addr, I2C_TRANSMITTER);
	/* 等待ADDSEND位置位，即等待从机地址发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_ADDSEND))
	{
		/* 等待超时 */
		if(++i > 100000)
		{
			/* 向I2C总线上发送停止信号 */
			i2c_stop_on_bus(info.periph);
			/* 返回执行结果 */
			return 0;
		}
	}
	/* 清除ADDSEND位 */
	i2c_flag_clear(info.periph, I2C_FLAG_ADDSEND);
	/* 等待TBE位置位，即等待发送缓冲区为空 */
	while(SET != i2c_flag_get(info.periph, I2C_FLAG_TBE));
	/* 向从机发送寄存器地址 */
	i2c_data_transmit(info.periph, reg);
	/* 等待BTC位置位，即等待寄存器地址发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
	/* 循环发送所有数据 */
	while(count --)
	{
		/* 等待TBE位置位，即等待发送缓冲区为空 */
		while(SET != i2c_flag_get(info.periph, I2C_FLAG_TBE));
		/* 向从机发送一个字节数据 */
		i2c_data_transmit(info.periph, *pbytes);
		/* 等待BTC位置位，即等待数据发送完成 */
		while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
		/* 数据指针指向下一个字节 */
		pbytes ++;
	}
	/* 向I2C总线上发送停止信号 */
	i2c_stop_on_bus(info.periph);
	/* 返回执行结果 */
	return 1;
}

/*!
	\功能       从I2C从机读取多个字节数据
	\参数[输入] info  : I2C从机信息
	\参数[输入] count : 要读取的个数
	\参数[输出] pbytes: 要读取的数据
	\返回       执行结果，0表示失败，1表示成功
*/
int i2c_bytes_read(i2c_slave_info info, unsigned char * pbytes, unsigned char count)
{
	unsigned int i = 0;

	/* 等待I2C总线变为空闲状态 */
	while(i2c_flag_get(info.periph, I2C_FLAG_I2CBSY));
	/* 向I2C总线上发送起始信号 */
	i2c_start_on_bus(info.periph);
	/* 等待SBSEND位置位，即等待起始信号发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_SBSEND));
	/* 向I2C总线上发送从机地址，指定后续数据为主机接收 */
	i2c_master_addressing(info.periph, info.addr, I2C_RECEIVER);		
	/* 等待ADDSEND位置位，即等待从机地址发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_ADDSEND))
	{
		/* 等待超时 */
		if(++i > 100000)
		{
			/* 向I2C总线上发送停止信号 */
			i2c_stop_on_bus(info.periph);
			/* 返回执行结果 */
			return 0;
		}
	}
	/* 清除ADDSEND位 */
	i2c_flag_clear(info.periph, I2C_FLAG_ADDSEND);
	if(count <= 2)
	{
		/* 禁止I2C应答 */
		i2c_ack_config(info.periph, I2C_ACK_DISABLE);
	}
	if(count == 2)
	{
		/* 下一字节接收结束后发送非应答信号 */
		i2c_ackpos_config(info.periph, I2C_ACKPOS_NEXT);
	}
	if(count == 1)
	{
		/* 向I2C总线上发送停止信号 */
		i2c_stop_on_bus(info.periph);
	}
	/* 循环接收所有数据 */
	while(count)
	{
		if(count == 3)
		{
			/* 等待BTC位置位，即等待数据发送完成 */
			while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
			/* 禁止I2C应答 */
			i2c_ack_config(info.periph, I2C_ACK_DISABLE);
		}
		if(count == 2)
		{
			/* 等待BTC位置位，即等待数据发送完成 */
			while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
			/* 向I2C总线上发送停止信号 */
			i2c_stop_on_bus(info.periph);
		}
		/* 等待RBNE位置位，即等待接收缓冲区不为空 */
		if(i2c_flag_get(info.periph, I2C_FLAG_RBNE))
		{
			/* 从从机接收一个字节数据 */
			*pbytes = i2c_data_receive(info.periph);
			/* 数据指针指向下一个字节 */
			pbytes ++;
			/* 剩余字节计数递减 */
			count --;
		}
	}
	/* 使能I2C应答 */
	i2c_ack_config(info.periph, I2C_ACK_ENABLE);
	/* 当前字节接收结束后发送应答信号 */
	i2c_ackpos_config(info.periph, I2C_ACKPOS_CURRENT);
	/* 返回执行结果 */
	return 1;
}

/*!
	\功能       从I2C从机的寄存器中读取多个字节数据
	\参数[输入] info  : I2C从机信息
	\参数[输入] reg   : 寄存器的地址
	\参数[输入] count : 要读取的个数
	\参数[输出] pbytes: 要读取的数据
	\返回       执行结果，0表示失败，1表示成功
*/
int i2c_reg_bytes_read(i2c_slave_info info, unsigned char reg, unsigned char * pbytes, unsigned char count)
{
	unsigned int i = 0;

	/* 等待I2C总线变为空闲状态 */
	while(i2c_flag_get(info.periph, I2C_FLAG_I2CBSY));
	/* 向I2C总线上发送起始信号 */
	i2c_start_on_bus(info.periph);
	/* 等待SBSEND位置位，即等待起始信号发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_SBSEND));
	/* 向I2C总线上发送从机地址，指定后续数据为主机发送 */
	i2c_master_addressing(info.periph, info.addr, I2C_TRANSMITTER);
	/* 等待ADDSEND位置位，即等待从机地址发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_ADDSEND))
	{
		/* 等待超时 */
		if(++i > 100000)
		{
			/* 向I2C总线上发送停止信号 */
			i2c_stop_on_bus(info.periph);
			/* 返回执行结果 */
			return 0;
		}
	}
	/* 清除ADDSEND位 */
	i2c_flag_clear(info.periph, I2C_FLAG_ADDSEND);
	/* 等待TBE位置位，即等待发送缓冲区为空 */
	while(SET != i2c_flag_get(info.periph, I2C_FLAG_TBE));
	/* 向从机发送寄存器地址 */
	i2c_data_transmit(info.periph, reg);
	/* 等待BTC位置位，即等待寄存器地址发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
	/* 向I2C总线上发送起始信号 */
	i2c_start_on_bus(info.periph);
	/* 等待SBSEND位置位，即等待起始信号发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_SBSEND));
	/* 向I2C总线上发送从机地址，指定后续数据为主机接收 */
	i2c_master_addressing(info.periph, info.addr, I2C_RECEIVER);		
	/* 等待ADDSEND位置位，即等待从机地址发送完成 */
	while(!i2c_flag_get(info.periph, I2C_FLAG_ADDSEND))
	{
		/* 等待超时 */
		if(++i > 100000)
		{
			/* 向I2C总线上发送停止信号 */
			i2c_stop_on_bus(info.periph);
			/* 返回执行结果 */
			return 0;
		}
	}
	/* 清除ADDSEND位 */
	i2c_flag_clear(info.periph, I2C_FLAG_ADDSEND);
	if(count <= 2)
	{
		/* 禁止I2C应答 */
		i2c_ack_config(info.periph, I2C_ACK_DISABLE);
	}
	if(count == 2)
	{
		/* 下一字节接收结束后发送非应答信号 */
		i2c_ackpos_config(info.periph, I2C_ACKPOS_NEXT);
	}
	if(count == 1)
	{
		/* 向I2C总线上发送停止信号 */
		i2c_stop_on_bus(info.periph);
	}
	/* 循环接收所有数据 */
	while(count)
	{
		if(count == 3)
		{
			/* 等待BTC位置位，即等待数据发送完成 */
			while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
			/* 禁止I2C应答 */
			i2c_ack_config(info.periph, I2C_ACK_DISABLE);
		}
		if(count == 2)
		{
			/* 等待BTC位置位，即等待数据发送完成 */
			while(!i2c_flag_get(info.periph, I2C_FLAG_BTC));
			/* 向I2C总线上发送停止信号 */
			i2c_stop_on_bus(info.periph);
		}
		/* 等待RBNE位置位，即等待接收缓冲区不为空 */
		if(i2c_flag_get(info.periph, I2C_FLAG_RBNE))
		{
			/* 从从机接收一个字节数据 */
			*pbytes = i2c_data_receive(info.periph);
			/* 数据指针指向下一个字节 */
			pbytes ++;
			/* 剩余字节计数递减 */
			count --;
		}
	}
	/* 使能I2C应答 */
	i2c_ack_config(info.periph, I2C_ACK_ENABLE);
	/* 当前字节接收结束后发送应答信号 */
	i2c_ackpos_config(info.periph, I2C_ACKPOS_CURRENT);
	/* 返回执行结果 */
	return 1;
}
