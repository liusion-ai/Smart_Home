#ifndef _LED_H_
#define _LED_H_


typedef struct
{
	_Bool Led_Status;

} Led_INFO;

#define Led_ON		1

#define Led_OFF		0

extern Led_INFO Led_info;


void Led_Init(void);

void Led_Set(int who, _Bool status);

#endif
