
#include "stm32f10x.h"

#include "esp8266.h"

#include "onenet.h"
#include "mqttkit.h"

#include "base64.h"
#include "hmac_sha1.h"

#include "usart.h"
#include "delay.h"

//====ÓÃÓÚ´«µÝ±äÁ¿¸øÆ½Ì¨=====//
#include "Led.h"
#include "Motor.h"
#include "Servo.h"
#include "buzzer.h"

#include <string.h>
#include <stdio.h>

#include "cJSON.h"

#define PROID			"9NS5J5jSMq"

#define ACCESS_KEY		"bXNuR1NDcGE4MVBTMGIxV28yWXdNVmkyTGlSMVBFQzg="

#define DEVICE_NAME		"d1"


char devid[16];

char key[48];


extern unsigned char esp8266_buf[512];


// ¼òÒ× float ×ª×Ö·û´® (µ¥Æ¬»ú×¨ÓÃ)
void float_to_str(float num, char *buf, int precision) {
    sprintf(buf, "%.*f", precision, num);
}


static unsigned char OTA_UrlEncode(char *sign)
{

	char sign_t[40];
	unsigned char i = 0, j = 0;
	unsigned char sign_len = strlen(sign);
	
	if(sign == (void *)0 || sign_len < 28)
		return 1;
	
	for(; i < sign_len; i++)
	{
		sign_t[i] = sign[i];
		sign[i] = 0;
	}
	sign_t[i] = 0;
	
	for(i = 0, j = 0; i < sign_len; i++)
	{
		switch(sign_t[i])
		{
			case '+':
				strcat(sign + j, "%2B");j += 3;
			break;
			
			case ' ':
				strcat(sign + j, "%20");j += 3;
			break;
			
			case '/':
				strcat(sign + j, "%2F");j += 3;
			break;
			
			case '?':
				strcat(sign + j, "%3F");j += 3;
			break;
			
			case '%':
				strcat(sign + j, "%25");j += 3;
			break;
			
			case '#':
				strcat(sign + j, "%23");j += 3;
			break;
			
			case '&':
				strcat(sign + j, "%26");j += 3;
			break;
			
			case '=':
				strcat(sign + j, "%3D");j += 3;
			break;
			
			default:
				sign[j] = sign_t[i];j++;
			break;
		}
	}
	
	sign[j] = 0;
	
	return 0;

}

#define METHOD		"sha1"
static unsigned char OneNET_Authorization(char *ver, char *res, unsigned int et, char *access_key, char *dev_name,
											char *authorization_buf, unsigned short authorization_buf_len, _Bool flag)
{
	
	size_t olen = 0;
	
	char sign_buf[64];								
	char hmac_sha1_buf[64];
	char access_key_base64[64];						
	char string_for_signature[72];				

	if(ver == (void *)0 || res == (void *)0 || et < 1564562581 || access_key == (void *)0
		|| authorization_buf == (void *)0 || authorization_buf_len < 120)
		return 1;
	
	memset(access_key_base64, 0, sizeof(access_key_base64));
	BASE64_Decode((unsigned char *)access_key_base64, sizeof(access_key_base64), &olen, (unsigned char *)access_key, strlen(access_key));
	//UsartPrintf(USART_DEBUG, "access_key_base64: %s\r\n", access_key_base64);
	
	memset(string_for_signature, 0, sizeof(string_for_signature));
	if(flag)
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s\n%s", et, METHOD, res, ver);
	else
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s/devices/%s\n%s", et, METHOD, res, dev_name, ver);
	//UsartPrintf(USART_DEBUG, "string_for_signature: %s\r\n", string_for_signature);
	
	memset(hmac_sha1_buf, 0, sizeof(hmac_sha1_buf));
	
	hmac_sha1((unsigned char *)access_key_base64, strlen(access_key_base64),
				(unsigned char *)string_for_signature, strlen(string_for_signature),
				(unsigned char *)hmac_sha1_buf);
	
	//UsartPrintf(USART_DEBUG, "hmac_sha1_buf: %s\r\n", hmac_sha1_buf);
	
	olen = 0;
	memset(sign_buf, 0, sizeof(sign_buf));
	BASE64_Encode((unsigned char *)sign_buf, sizeof(sign_buf), &olen, (unsigned char *)hmac_sha1_buf, strlen(hmac_sha1_buf));

	OTA_UrlEncode(sign_buf);
	//UsartPrintf(USART_DEBUG, "sign_buf: %s\r\n", sign_buf);
	
	if(flag)
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s&et=%d&method=%s&sign=%s", ver, res, et, METHOD, sign_buf);
	else
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s%%2Fdevices%%2F%s&et=%d&method=%s&sign=%s", ver, res, dev_name, et, METHOD, sign_buf);
	//UsartPrintf(USART_DEBUG, "Token: %s\r\n", authorization_buf);
	
	return 0;

}

_Bool OneNET_RegisterDevice(void)
{

	_Bool result = 1;
	unsigned short send_len = 11 + strlen(DEVICE_NAME);
	char *send_ptr = NULL, *data_ptr = NULL;
	
	char authorization_buf[144];													
	
	send_ptr = malloc(send_len + 240);
	if(send_ptr == NULL)
		return result;
	
	while(ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n", "CONNECT"))
		DelayXms(500);
	
	OneNET_Authorization("2018-10-31", PROID, 1956499200, ACCESS_KEY, NULL,
							authorization_buf, sizeof(authorization_buf), 1);
	
	snprintf(send_ptr, 240 + send_len, "POST /mqtt/v1/devices/reg HTTP/1.1\r\n"
					"Authorization:%s\r\n"
					"Host:ota.heclouds.com\r\n"
					"Content-Type:application/json\r\n"
					"Content-Length:%d\r\n\r\n"
					"{\"name\":\"%s\"}",
	
					authorization_buf, 11 + strlen(DEVICE_NAME), DEVICE_NAME);
	
	ESP8266_SendData((unsigned char *)send_ptr, strlen(send_ptr));
	
	data_ptr = (char *)ESP8266_GetIPD(250);							
	
	if(data_ptr)
	{
		data_ptr = strstr(data_ptr, "device_id");
	}
	
	if(data_ptr)
	{
		char name[16];
		int pid = 0;
		
		if(sscanf(data_ptr, "device_id\" : \"%[^\"]\",\r\n\"name\" : \"%[^\"]\",\r\n\r\n\"pid\" : %d,\r\n\"key\" : \"%[^\"]\"", devid, name, &pid, key) == 4)
		{
			UsartPrintf(USART_DEBUG, "create device: %s, %s, %d, %s\r\n", devid, name, pid, key);
			result = 0;
		}
	}
	
	free(send_ptr);
	ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK");
	
	return result;

}

_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					

	unsigned char *dataPtr;
	
	char authorization_buf[160];			
	
	_Bool status = 1;
	
	OneNET_Authorization("2018-10-31", PROID, 1956499200, ACCESS_KEY, DEVICE_NAME,
								authorization_buf, sizeof(authorization_buf), 0);
	
	UsartPrintf(USART_DEBUG, "OneNET_DevLink\r\n"
							"NAME: %s,	PROID: %s,	KEY:%s\r\n"
                        , DEVICE_NAME, PROID, authorization_buf);
	
	if(MQTT_PacketConnect(PROID, authorization_buf, DEVICE_NAME, 20, 1, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			
		
		dataPtr = ESP8266_GetIPD(250);									
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:UsartPrintf(USART_DEBUG, "Tips:	è¿žæŽ¥æˆåŠŸ\r\n");status = 0;break;
					
					case 1:UsartPrintf(USART_DEBUG, "WARN:	è¿žæŽ¥å¤±è´¥ï¼šåè®”™è¯¯\r\n");break;
					case 2:UsartPrintf(USART_DEBUG, "WARN:	è¿žæŽ¥å¤±è´¥ï¼šéžæ³•çš„clientid\r\n");break;
					case 3:UsartPrintf(USART_DEBUG, "WARN:	è¿žæŽ¥å¤±è´¥ï¼šæœåŠ¡å™¨å¤±è´¥\r\n");break;
					case 4:UsartPrintf(USART_DEBUG, "WARN:	è¿žæŽ¥å¤±è´¥ï¼šç”¨æˆ·åæˆ–å¯†ç é”™è¯¯\r\n");break;
					case 5:UsartPrintf(USART_DEBUG, "WARN:	è¿žæŽ¥å¤±è´¥ï¼šéžæ³•é“¾ï¿?æ¯”åtokenéžæ³•)\r\n");break;
					
					default:UsartPrintf(USART_DEBUG, "ERR:	è¿žæŽ¥å¤±è´¥ï¼šæœªçŸ¥é”™è¯¯\r\n");break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");
	
	return status;
	
}
extern u8 temp, humi ;	
//extern float ppm;
unsigned char OneNet_FillBuf(char *buf)
{
	
	char text[128];
	
	memset(text, 0, sizeof(text));

	strcpy(buf,"{\"id\":\"123\",\"version\":\"1.0\",\"params\":{");

	memset(text, 0, sizeof(text));
	sprintf(text,"\"temp\":{\"value\":%d},", temp);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text,"\"humi\":{\"value\":%d},", humi);
	strcat(buf, text);

	memset(text, 0, sizeof(text));
	sprintf(text,"\"led\":{\"value\":%s},", 
	Led_info.Led_Status ? "true" : "false");
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text,"\"curtains\":{\"value\":%s},", 
	curtains_info.curtains_Status ? "true" : "false");
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text,"\"door\":{\"value\":%s},", 
	door_info.door_Status ? "true" : "false");
	strcat(buf, text);

	memset(text, 0, sizeof(text));
	sprintf(text,"\"fan\":{\"value\":%s}", 
	fan_info.fan_Status ? "true" : "false");
	strcat(buf, text);
	
	strcat(buf,"}}");
	
	return strlen(buf);

}


void OneNet_SendData(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
	
	char buf[256];
	
	short body_len = 0, i = 0;
	
	//UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-MQTT\r\n");
	
	memset(buf, 0, sizeof(buf));

	body_len = OneNet_FillBuf(buf);			

	if(body_len)
	{
		if(MQTT_PacketSaveData(PROID, DEVICE_NAME, body_len, buf, &mqttPacket) == 0)				
		{
			ESP8266_SendData(mqttPacket._data, mqttPacket._len);					
			UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", mqttPacket._len);
			MQTT_DeleteBuffer(&mqttPacket);															
		}
		else
			UsartPrintf(USART_DEBUG, "WARN:	EDP_NewBuffer Failed\r\n");
	}
	
}


void OneNET_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						
	
	UsartPrintf(USART_DEBUG, "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL0, 0, 1, &mqtt_packet) == 0)
	{
		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					
		
		MQTT_DeleteBuffer(&mqtt_packet);										
	}

}

void OneNET_Subscribe(void)
{
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						
	
	char topic_buf[56];
	const char *topic = topic_buf;
	
	snprintf(topic_buf, sizeof(topic_buf), "$sys/%s/%s/thing/property/set", PROID, DEVICE_NAME);	
	
	UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topic_buf);
	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, &topic, 1, &mqtt_packet) == 0)
	{
		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					
		
		MQTT_DeleteBuffer(&mqtt_packet);										
	}

}


void OneNet_RevPro(unsigned char *cmd)
{
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	
	unsigned char type = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;

	cJSON *raw_json, *params_json, *led_json; 
	cJSON *fan_json, *door_json, *curtains_json;

	type = MQTT_UnPacketRecv(cmd);		
	switch(type)
	{
		case MQTT_PKT_PUBLISH:																
		
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{
				char *data_ptr = NULL;
				
				UsartPrintf(USART_DEBUG, "topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
																	cmdid_topic, topic_len, req_payload, req_len);
				 
			raw_json = cJSON_Parse(req_payload);
			if(raw_json != NULL)
			{
				params_json = cJSON_GetObjectItem(raw_json, "params");
				if(params_json != NULL) // ·ÀÖ¹ params Îª¿Õ
				{
					led_json    = cJSON_GetObjectItem(params_json, "led");
					fan_json    = cJSON_GetObjectItem(params_json, "fan");
					door_json   = cJSON_GetObjectItem(params_json, "door");
					curtains_json = cJSON_GetObjectItem(params_json, "curtains");

					// LED
					if(led_json != NULL)
					{
						if(led_json->type == cJSON_True)  Led_Set(2, Led_ON);
						else if(led_json->type == cJSON_False) Led_Set(2, Led_OFF);
					}

					// FAN
					if(fan_json != NULL)
					{
						if(fan_json->type == cJSON_True)  fan_set(1, 100);
						else if(fan_json->type == cJSON_False) fan_set(0, 0);
					}

					// DOOR
					if(door_json != NULL)
					{
						if(door_json->type == cJSON_True)  Door_Status(1);
						else if(door_json->type == cJSON_False) Door_Status(0);
					}

					if(curtains_json != NULL)
					{
						if(curtains_json->type == cJSON_True)
						{
							curtains_set(1);
						}
						else // ÏÖÔÚÕâ¸ö else Ö»ÊôÓÚ curtains_json
						{
							curtains_set(0);
						}
					}
				}
						cJSON_Delete(raw_json);
			}
	

			}	
			break;
		case MQTT_PKT_PUBACK:														
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
				//UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
			
			break;

		case MQTT_PKT_SUBACK:																
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");
			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");
				
				break;
				
				default:
					result = -1;
				break;
			
	
		ESP8266_Clear();									

		if(result == -1)
		return;
	
		if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
		{
			MQTT_FreeBuffer(cmdid_topic);
			MQTT_FreeBuffer(req_payload);
		}
	}
}






