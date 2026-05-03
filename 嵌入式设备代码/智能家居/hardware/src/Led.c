/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	led.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		Led初始化、控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "Led.h"


Led_INFO Led_info = {0};


/*
************************************************************
*	函数名称：	Led_Init
*
*	函数功能：	led
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Led_Init(void)
{

	GPIO_InitTypeDef gpio_initstruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);		//打开GPIOB的时钟
	
	gpio_initstruct.GPIO_Mode = GPIO_Mode_Out_PP;				//设置为输出
	gpio_initstruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;		//将初始化的Pin脚
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;				//可承载的最大频率
	
	GPIO_Init(GPIOC, &gpio_initstruct);							//初始化GPIO
	
	Led_Set(1, Led_OFF);										
	Led_Set(2, Led_OFF);

}

/*
************************************************************
*	函数名称：	Led_Set
*
*	函数功能：	led控制
*
*	入口参数：	status：开关led
*
*	返回参数：	无
*
*	说明：		开-Led_ON		关-Led_OFF
************************************************************
*/
void Led_Set(int who,_Bool status)
{
	if(who == 1)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, status == Led_ON ?  Bit_SET:  Bit_RESET );		//如果status等于Led_ON，则返回Bit_SET，否则返回Bit_RESET
	
		Led_info.Led_Status = status;
	}
	
	if(who == 2)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_14, status == Led_ON ?  Bit_SET:  Bit_RESET  );		//如果status等于Led_ON，则返回Bit_SET，否则返回Bit_RESET
	
		Led_info.Led_Status = status;
	}
}
