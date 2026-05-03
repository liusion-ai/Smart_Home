
//单片机头文件
#include "stm32f10x.h"
#include "delay.h"
#include "led.h"

//硬件驱动
#include "key.h"


/*
************************************************************
*	函数名称：	Led_Init
*
*	函数功能：	蜂鸣器初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Key_Init(void)
{
	GPIO_InitTypeDef gpio_initstruct;
	EXTI_InitTypeDef EXTI_InitStrcture;
	NVIC_InitTypeDef NVIC_InitStrcture;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//打开GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	
	gpio_initstruct.GPIO_Mode = GPIO_Mode_IPU;				//设置为输出
	gpio_initstruct.GPIO_Pin = GPIO_Pin_1;						//将初始化的Pin脚
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;				//可承载的最大频率
	GPIO_Init(GPIOA, &gpio_initstruct);							//初始化GPIO

	
	EXTI_InitStrcture.EXTI_Line = EXTI_Line1;
	EXTI_InitStrcture.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStrcture.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStrcture.EXTI_LineCmd =  ENABLE;
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);	//指定中断通道
	
	EXTI_Init(&EXTI_InitStrcture);
	
	
	NVIC_InitStrcture.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStrcture.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStrcture.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStrcture.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStrcture);
	
}

void EXTI1_IRQHandler(void)
{
	
	DelayMs(10);
	while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)==SET);
	if(Led_info.Led_Status==Led_ON)  Led_Set(1, Led_OFF );
	else Led_Set(1,Led_ON);
	EXTI_ClearITPendingBit(EXTI_Line1);
}
