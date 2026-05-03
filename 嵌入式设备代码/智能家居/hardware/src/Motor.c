#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "Motor.h"
#include "delay.h"

fan_INFO fan_info = {0};
curtains_INFO curtains_info = {0};

/**
  * 函    数：直流电机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOX, ENABLE);		//开启GPIOA的时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = gpio_pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpioX, &GPIO_InitStructure);						//将PA4和PA5引脚初始化为推挽输出	
	
	PWM_Init();													//初始化直流电机的底层PWM
}

/**
  * 函    数：窗帘开关
  * 参    数：curtains_stare 要设置的状态，1：开，0：关
  * 返 回 值：无
  */
void curtains_set(_Bool curtains_state)
{
	Delay_Init();
	if(curtains_state == 1)				//开（正转）
	{
		PWM_SetCompare1(50);		
		GPIO_SetBits(GPIOB, GPIO_Pin_6);	//PA4置高电平
		GPIO_ResetBits(GPIOB, GPIO_Pin_7);	//PA5置低电平，设置方向为正转
		DelayXms(1000);
		PWM_SetCompare1(0);	
		curtains_info.curtains_Status = 1;
	}
	
	if(curtains_state == 0)			 //关（反转）
	{
		PWM_SetCompare1(50);
		GPIO_ResetBits(GPIOB, GPIO_Pin_6);	//PA4置高电平
		GPIO_SetBits(GPIOB, GPIO_Pin_7);	//PA5置低电平，设置方向为正转
		DelayXms(1000);
		PWM_SetCompare1(0);	
		curtains_info.curtains_Status = 0;
	}
}

/**
  * 函    数：风扇
  * 参    数：fan_state 	 要设置的状态，0：关，1：开
  * 参    数：Speed  	 要设置的速度，范围：-100~100
  * 返 回 值：无
  */

void fan_set(_Bool fan_state, int8_t Speed)
{
	
	if(fan_state == 0)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_10);	
		GPIO_ResetBits(GPIOB, GPIO_Pin_11);	
		PWM_SetCompare2(0);	
		fan_info.fan_Status = 0;		
	}
	
	if(fan_state == 1 && Speed >= 0)		//正转
	{	
		GPIO_SetBits(GPIOB, GPIO_Pin_10);	
		GPIO_ResetBits(GPIOB, GPIO_Pin_11);	
		PWM_SetCompare2(Speed);	
		fan_info.fan_Status = 1;	
	}
	
	if(fan_state == 1 && Speed < 0)			//反转						
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_10);	
		GPIO_SetBits(GPIOB, GPIO_Pin_11);	
		PWM_SetCompare2(-Speed);	
		fan_info.fan_Status = 1;			
	}	
	
}
