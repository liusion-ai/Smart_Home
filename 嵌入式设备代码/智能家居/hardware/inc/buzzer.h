#ifndef __BUZZER_H
#define __BUZZER_H
#include "stm32f10x.h"

typedef struct
{
	_Bool alarm_Status;

}alarm_INFO;

extern alarm_INFO alarm_info;

void Buzzer_Init(void);
void Buzzer_Process(void);   // 必须放主循环

void Buzzer_Single(void);    // 单响
void Buzzer_Double(void);    // 双响
void Buzzer_loog(void);		 //警报
void Buzzer_Stop(void);     // 立刻停止所有声音

#endif