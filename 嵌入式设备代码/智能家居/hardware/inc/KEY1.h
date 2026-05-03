#ifndef __KEY1_H
#define __KEY1_H
#include "stm32f10x.h"

#define KEY_NUM 5

// 按键状态枚举
typedef enum {
    KEY_IDLE = 0,    // 空闲
    KEY_DOWN_DEBOUNCE, // 按下防抖
    KEY_HOLD_CHECK,    // 长按判定中
    KEY_WAIT_RELEASE,  // 等待松手
    KEY_SHORT_TRIG,    // 短按触发
    KEY_LONG_TRIG      // 长按触发
} KeyState;

// 按键结构体
typedef struct {
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
    KeyState state;
    float debounce_cnt; // 防抖/计时计数器
    uint8_t short_flag;    // 短按标志
    uint8_t long_flag;     // 长按标志
} KeyTypeDef;

// 全局按键数组（外部可访问）
extern KeyTypeDef Key[KEY_NUM];

// 函数声明
void KEY_Init(void);
void KEY_Scan(void);
uint8_t KEY_GetShort(uint8_t index); // 获取短按
uint8_t KEY_GetLong(uint8_t index);  // 获取长按

#endif