#include "buzzer.h"

#define BUZZER_PIN  GPIO_Pin_5
#define BUZZER_PORT GPIOA

// 低电平触发
#define BUZZER_ON()  GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN)
#define BUZZER_OFF() GPIO_SetBits(BUZZER_PORT, BUZZER_PIN)

alarm_INFO alarm_info = {0};

typedef enum
{
    BUZZER_STOP = 0,
    BUZZER_SINGLE,	
    BUZZER_DOUBLE,
	BUZZER_LOOG
} Buzzer_State;

static Buzzer_State state = BUZZER_STOP;
static uint16_t cnt = 0;

void Buzzer_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    gpio.GPIO_Pin = BUZZER_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT, &gpio);

    BUZZER_OFF();
}

void Buzzer_Stop(void)
{
    state = BUZZER_STOP;
    BUZZER_OFF();
}

void Buzzer_Single(void)
{
    state = BUZZER_SINGLE;
    cnt = 0;
}

void Buzzer_Double(void)
{
    state = BUZZER_DOUBLE;
    cnt = 0;
}

void Buzzer_loog(void)
{
    state = BUZZER_LOOG;
    cnt = 0;
}

// 主循环里跑，1ms调用一次最准
void Buzzer_Process(void)
{
    if(state == BUZZER_STOP)
    {
        BUZZER_OFF();
        return;
    }

    cnt++;

    // ========== 响 1 次 ==========
    if(state == BUZZER_SINGLE)
    {
        if(cnt <= 3)       BUZZER_ON();
        else
        {
            BUZZER_OFF();
            state = BUZZER_STOP;
        }
    }

    // ========== 响 2 次 ==========
    if(state == BUZZER_DOUBLE)
    {
        if(cnt <= 2)       BUZZER_ON();
        else if(cnt <= 4) BUZZER_OFF();
        else if(cnt <= 5) BUZZER_ON();
        else
        {
            BUZZER_OFF();
            state = BUZZER_STOP;
        }
    }
	
	// ========== 响 多 次 ==========
    if(state == BUZZER_LOOG)
    {
        if(cnt <= 1)       BUZZER_ON();
        else if(cnt <= 2) BUZZER_OFF();
        else if(cnt <= 4) BUZZER_ON();
		else if(cnt <= 5) BUZZER_OFF();
        else if(cnt <= 6) BUZZER_ON();
        else
        {
            BUZZER_OFF();
            state = BUZZER_STOP;
        }
    }
}