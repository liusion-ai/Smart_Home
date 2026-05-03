#ifndef __SERVO_H
#define __SERVO_H

typedef struct
{

	_Bool door_Status;

}door_INFO;

#define door_ON			1

#define door_OFF		0

extern door_INFO door_info;


void Servo_Init(void);
void Servo_SetAngle(float Angle);
void Door_Status(_Bool Status);
#endif
