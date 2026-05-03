#ifndef __LIGHT_SENSOR_H
#define __LIGHT_SENSOR_H

typedef struct
{
	_Bool LightSensor_mode;

}LightSensor_INFO;


extern LightSensor_INFO LightSensor_info;

void LightSensor_Init(void);
uint8_t LightSensor_Get(void);

#endif
