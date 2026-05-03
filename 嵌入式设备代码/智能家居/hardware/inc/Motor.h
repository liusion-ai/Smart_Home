#ifndef __MOTOR_H
#define __MOTOR_H

typedef struct
{
	_Bool curtains_Status;

}curtains_INFO;

#define curtains_ON			1

#define curtains_OFF		0

extern curtains_INFO curtains_info;


typedef struct
{
	_Bool fan_Status;

}fan_INFO;

#define fan_ON			1

#define fan_OFF			0

extern fan_INFO fan_info;

#define  	RCC_APB2Periph_GPIOX		RCC_APB2Periph_GPIOB
#define	 	gpioX						GPIOB
#define  	gpio_pin  					GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_11
										//GPIO_Pin_6 | GPIO_Pin_7		 «¥∞¡±
										//GPIO_Pin_10 | GPIO_Pin_11 	 «∑Á…»
									
void Motor_Init(void);
void fan_set(_Bool fan_state, int8_t Speed);
void curtains_set(_Bool curtains_state);

#endif
