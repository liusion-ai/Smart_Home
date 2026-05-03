#ifndef _ESP8266_H_
#define _ESP8266_H_


#define REV_OK		0	
#define REV_WAIT	1


void ESP8266_Init(void);

void ESP8266_Clear(void);

_Bool ESP8266_SendCmd(char *cmd, char *res);

void ESP8266_SendData(unsigned char *data, unsigned short len);

unsigned char *ESP8266_GetIPD(unsigned short timeOut);


#endif

#include "stm32f10x.h"


#include "onenet.h"


#include "esp8266.h"


#include "delay.h"
#include "usart.h"
#include "Key.h"
#include "dht11.h"
#include "Led.h"
#include "OLED.h"
#include "mq2.h"
#include "flame.h"
#include "buzzer.h"
#include "adcx.h"
#include "Motor.h"
#include "Servo.h"
#include "pwm.h"
#include "LightSensor.h"
#include "MFRC522.h"
#include "KEY1.h"
#include "flash.h"


#include <string.h>



#define ESP8266_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"
							

#define MAX_CARDS 10
u8 card_whitelist[MAX_CARDS][4];  // 白名单
u8 card_total = 0;                // 已存卡数量
u8 add_mode = 0;                  // 录卡模式
u8 door_open = 0;
u32 door_time = 0;
u8 card_id[4];  // 读到的卡号
						
void Hardware_Init(void);
void Dispaly_Init(void);
void Refresh_Data(void);

// 刷卡函数声明
void Load_Cards(void);
void Card_Add(u8 *id);
void Door_Open(void);
void Door_Close(void);
u8 Card_Check(u8 *id);

u16 value;
float ppm;
u8 temp, humi;
uint8_t LightSensor;
u8 LightSensor_flag = 0;
u8 curtains_state = 0;  // 0=关闭状态 | 1=打开状态
u8 smoke_percent;
u8 flame_detected;
u8 alarm_state = 1;			
u8 overheat_state = 1;		
u8 smoke_sensor_state = 1;	

//舵机开门计时（非阻塞）
u8 door_open_flag = 0;
uint32_t door_tick = 0;


// ========== 毫秒计时 ==========
static uint32_t sys_ms = 0;

void SysTick_Handler(void)
{
    sys_ms++;
}

uint32_t millis(void)
{
    return sys_ms;
}



			
int main(void)
{
	unsigned short time_count = 0;	
	unsigned char *dataPtr = NULL;
	
	OLED_Init();
	OLED_Clear();
	
	Usart2_Init(115200);
	
	Hardware_Init();				
	
	OneNET_RegisterDevice();

	UsartPrintf(USART_DEBUG, "Connect MQTTs Server...\r\n");
	 OLED_ShowString(0, 9, "Connect MQTTs Server...", OLED_6X8);
	OLED_Update(); 
	
	while(ESP8266_SendCmd(ESP8266_INFO, "CONNECT"))
		DelayXms(500);
	UsartPrintf(USART_DEBUG, "Connect MQTT Server Success\r\n");
	OLED_ShowString(0, 18, "Connect MQTT Success!", OLED_6X8);
	OLED_Update(); 
	
	while(OneNet_DevLink())			
		DelayXms(500);
	    
	UsartPrintf(USART_DEBUG, "Connect OneNET Success!\r\n");
	OLED_ShowString(0, 27, "Connect OneNET Success!", OLED_6X8);
	OLED_Update(); 
	
	Led_Set(1,Led_ON);				
	DelayMs(2500);
	Led_Set(1,Led_OFF);
		
	UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-MQTT\r\n");
	OLED_ShowString(0, 36, "Tips: OneNet_SendData", OLED_6X8);
	OLED_Update(); 
  	
	DelayXms(3000);
	OLED_Clear();
	
	OneNET_Subscribe();
	Dispaly_Init();			
			
	Door_Status(0); // 关门
	
	SysTick_Config(SystemCoreClock / 1000);  // 1ms计时
	while(1)
	{		
		u8 Time =30;
		DHT11_Read_Data(&temp, &humi);
		LightSensor  = LightSensor_Get();
		 
		 KEY_Scan(); 
		 Buzzer_Process();
		
	 
        // === 2. 按键业务逻辑 ===
        // 注意：使用索引 0-4 对应 SW1-SW5
		 if(KEY_GetLong(0)) 
        {
			Buzzer_Single();	//开风扇
			fan_set(1,80);										
			UsartPrintf(USART_DEBUG,"11111111r\n");
         
        }
        
        if(KEY_GetShort(0)) // 关风扇
        {
			Buzzer_Double();				
			fan_set(0,0);
			UsartPrintf(USART_DEBUG,"2222222r\n");
          
        }
		 if(KEY_GetLong(1)) // 开窗帘
        {
			Buzzer_Single();
			 curtains_set(1);
			DelayMs(500);
			PWM_SetCompare1(0);
			
			UsartPrintf(USART_DEBUG,"11111111r\n");
     
        }
        
        if(KEY_GetShort(1)) //关窗帘
        {
			
			Buzzer_Double();
			curtains_set(0);
			DelayMs(500);
			PWM_SetCompare1(0);
			
			UsartPrintf(USART_DEBUG,"2222222r\n");
        }
		 if(KEY_GetLong(2)) // 开门
        {
			Buzzer_Single();
			Door_Status(1);
			UsartPrintf(USART_DEBUG,"11111111r\n");         
        }
        
        if(KEY_GetShort(2)) // 关门
        {
			Buzzer_Double();	
			Door_Status(0);			
			UsartPrintf(USART_DEBUG,"2222222r\n");           
        }
		
		 if(KEY_GetLong(3)) // 开灯
        {
			Buzzer_Single();
			 Led_Set(2,Led_ON);			
			UsartPrintf(USART_DEBUG,"11111111r\n");
            
        }
        
        if(KEY_GetShort(3)) // 关灯
        {
			Buzzer_Double();
			Led_Set(2,Led_OFF);
 
			UsartPrintf(USART_DEBUG,"2222222r\n");
         
        }
        if(KEY_GetLong(4)) // 录入ID卡
		{
		    add_mode = 1;
			Buzzer_Single();	
			 Servo_SetAngle(90);

			UsartPrintf(USART_DEBUG,"11111111r\n");
        }
        
        if(KEY_GetShort(4)) // 自动开关窗帘
        {
			Buzzer_loog();
			LightSensor_flag = !LightSensor_flag;
			
			 Led_Set(2,Led_ON);
				DelayMs(500);
			 Led_Set(2,Led_OFF);
				
        }
		
				
		if(LightSensor_flag == 1)			//开启自动窗帘模式			
			{	
					
				if(LightSensor == 0)    // 开窗帘条件
				{
					// 只有当前是关闭状态，才允许开
					if(curtains_state == 0)
					{
						curtains_state = 1;  // 标记为已打开
						Buzzer_Single();
						curtains_set(1);
						DelayMs(500);
						PWM_SetCompare1(0);
					}
				}

				if(LightSensor == 1)    // 关窗帘条件
				{
					// 只有当前是打开状态，才允许关
					if(curtains_state == 1)
					{
						curtains_state = 0;  // 标记为已关闭
						Buzzer_Double();
						curtains_set(0);
						DelayMs(500);
						PWM_SetCompare1(0);
					}
				}
				
			}

        // 读卡（非阻塞）
        if (MFRC522_Request(PICC_REQIDL, card_id) == MI_OK) {
            if (MFRC522_Anticoll(card_id) == MI_OK) {

                if (add_mode) {
                    // 录卡
                    Card_Add(card_id);
                    Buzzer_Double();
					 Led_Set(1,Led_ON);
					DelayMs(500);
					Led_Set(1,Led_OFF);
                    add_mode = 0;
                } else {
                    // 校验
                    if (Card_Check(card_id)) {
                        // 合法卡
                        Buzzer_Single();
                        Door_Status(1);
                        door_open = 1;
                        door_time = millis();
                    } else {
                        // 非法卡
                        Buzzer_loog();	
                    }
                }

                MFRC522_Halt();
                DelayMs(200);
			}
			UsartPrintf(USART_DEBUG,"%X",card_id);
		}	
	
		if(++time_count >= 50)
        {
            DHT11_Read_Data(&temp, &humi);
		
			OneNet_SendData();
          
            time_count = 0;
            ESP8266_Clear();
      }
	

        dataPtr = ESP8266_GetIPD(0);	
	
        if(dataPtr != NULL)			
		{
			
            OneNet_RevPro(dataPtr);		
			
		}
		Refresh_Data();
		OLED_Update(); 
		DelayXms(10);
				
	}
}


void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	

	Delay_Init();									
	Usart1_Init(115200);							
	
	Usart2_Init(115200);							
	Key_Init();
	PWM_Init();
	Servo_Init();
	Motor_Init();
	LightSensor_Init();
	MFRC522_Init();
	KEY_Init();
	Buzzer_Init();

	SystemInit();
	MFRC522_Init();
    MFRC522_Reset();
    MFRC522_AntennaOn();
	
	
	while (DHT11_Init() )				
	{	
		DelayXms(10);	
	}
	
	Led_Init();							

	MQ2_Init();
	
	ESP8266_Init();					
	


}

void Dispaly_Init(void)
{	
	//一个汉字相当于两个字符，（32Bit）                
	OLED_ShowChinese(0, 0 ,"温度");
	OLED_ShowString(56, 0,"o", OLED_6X8);
	OLED_ShowChinese(66, 0 ,"湿度");
	OLED_ShowString(120, 0 ,"%", OLED_6X8);
	
	OLED_ShowChinese(0, 16 ,"烟雾");
	OLED_ShowString(32, 24,":", OLED_6X8);
	
	OLED_ShowChinese(0, 32 ,"风扇");
	OLED_ShowString(32, 40,":", OLED_6X8);
	//OLED_ShowChinese(42, 32 ,"开");
	
	OLED_ShowChinese(66, 32 ,"窗帘");
	OLED_ShowString(98, 40,":", OLED_6X8);
	//OLED_ShowChinese(108, 32 ,"开");
	
	OLED_ShowChinese(0, 48 ,"模式");
	OLED_ShowString(32, 56,":", OLED_6X8);
	//OLED_ShowChinese(42, 48 ,"手");
	
	OLED_ShowChinese(66, 48 ,"大门");
	OLED_ShowString(98, 56,":", OLED_6X8);
	//OLED_ShowChinese(108, 48 ,"开");
	
	OLED_Update();

}

void Refresh_Data(void)
{
	char buf[30];

	ppm = MQ2_GetData_PPM();	
	LightSensor = LightSensor_Get();	
	_Bool alarm_active = (smoke_sensor_state == 0);
	
	sprintf(buf, "%3d", temp);
	OLED_ShowString(30, 0, buf, OLED_8X16);

	sprintf(buf, "%3d", humi);
	OLED_ShowString(96 , 0, buf, OLED_8X16);

	sprintf(buf, "%.1f ppm",ppm);
	OLED_ShowString(40,16,buf,OLED_8X16); 
	
	if(fan_info.fan_Status == 1)			{OLED_ShowChinese(42, 32 ,"开");}
	if(fan_info.fan_Status == 0)			{OLED_ShowChinese(42, 32 ,"关");}
	
	if(LightSensor_flag == 1)				{OLED_ShowChinese(42, 48 ,"自");}
	if(LightSensor_flag == 0)				{OLED_ShowChinese(42, 48 ,"手");}
	
	if(curtains_info.curtains_Status == 1)  {OLED_ShowChinese(108, 32 ,"开");}
	if(curtains_info.curtains_Status == 0)  {OLED_ShowChinese(108, 32 ,"关");}
	
	if(door_info.door_Status == 1)			{OLED_ShowChinese(108, 48 ,"开");}
	if(door_info.door_Status == 0)			{OLED_ShowChinese(108, 48 ,"关");}
		
}

// 检查卡号是否在白名单
u8 Card_Check(u8 *id) {
    u8 i;
    for (i = 0; i < card_total; i++) {
        if (memcmp(card_whitelist[i], id, 4) == 0)
            return 1;
    }
    return 0;
}

// 添加卡到白名单
void Card_Add(u8 *id) {
    if (card_total >= MAX_CARDS) return;
    if (Card_Check(id)) return;

    memcpy(card_whitelist[card_total], id, 4);
    card_total++;
}