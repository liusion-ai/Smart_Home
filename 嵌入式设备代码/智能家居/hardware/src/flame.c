#include "stm32f10x.h"
#include "flame.h"

void Flame_Init(void)
{
	GPIO_InitTypeDef gpio_initstruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	gpio_initstruct.GPIO_Mode = GPIO_Mode_IPU;
	gpio_initstruct.GPIO_Pin = GPIO_Pin_12;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_initstruct);
}

u8 Flame_IsDetected(void)
{
	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_RESET ? 1 : 0;
}
