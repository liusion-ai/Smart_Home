#include "KEY1.h"
#include "delay.h" // 如果你的工程有delay驱动，若没有则需删除或替换

// 引脚配置：严格对应图片定义
// SW1-PB5, SW2-PB4, SW3-PB3, SW4-PA15, SW5-PA12
KeyTypeDef Key[KEY_NUM] = {
    {GPIOB, GPIO_Pin_5,  KEY_IDLE, 0, 0, 0}, // SWITCH 1 (索引 0)
    {GPIOB, GPIO_Pin_4,  KEY_IDLE, 0, 0, 0}, // SWITCH 2 (索引 1)
    {GPIOB, GPIO_Pin_3,  KEY_IDLE, 0, 0, 0}, // SWITCH 3 (索引 2)
    {GPIOA, GPIO_Pin_15, KEY_IDLE, 0, 0, 0}, // SWITCH 4 (索引 3)
    {GPIOA, GPIO_Pin_12, KEY_IDLE, 0, 0, 0}  // SWITCH 5 (索引 4)
};

/**
 * @brief  按键初始化
 * @note   配置为上拉输入 (Input Pull-up)
 */
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 1. 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    
    // 2. 解除PA15的JTAG锁定 (必须！否则PA15无法使用)
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    // 3. 配置IO模式
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    for(uint8_t i = 0; i < KEY_NUM; i++)
    {
        GPIO_InitStructure.GPIO_Pin = Key[i].GPIO_Pin;
        GPIO_Init(Key[i].GPIOx, &GPIO_InitStructure);
    }
}

/**
 * @brief  非阻塞按键扫描
 * @note   建议放在主循环 while(1) 中高频调用，或置于1ms定时器中
 */
void KEY_Scan(void)
{
    uint8_t pin_level;
    
    for(uint8_t i = 0; i < KEY_NUM; i++)
    {
        pin_level = GPIO_ReadInputDataBit(Key[i].GPIOx, Key[i].GPIO_Pin);
        
        switch(Key[i].state)
        {
            case KEY_IDLE:
                if(pin_level == 0) // 检测到按下
                {
                    Key[i].debounce_cnt = 0.1; // 20ms防抖
                    Key[i].state = KEY_DOWN_DEBOUNCE;
                }
                break;
                
            case KEY_DOWN_DEBOUNCE:
                if(Key[i].debounce_cnt > 0) Key[i].debounce_cnt--;
                else
                {
                    if(pin_level == 0) 
                    {
                        // 防抖完成，开始计时长按
                        Key[i].debounce_cnt = 3; // 设置长按阈值为2秒(200*10ms)
                        Key[i].state = KEY_HOLD_CHECK;
                    }
                    else 
                    {
                        // 抖动干扰，回归空闲
                        Key[i].state = KEY_IDLE;
                    }
                }
                break;
                
            case KEY_HOLD_CHECK:
                if(pin_level == 1) 
                {
                    // 手松开了，判定为短按
                    Key[i].state = KEY_WAIT_RELEASE;
                }
                else
                {
                    if(Key[i].debounce_cnt > 0) Key[i].debounce_cnt--;
                    else 
                    {
                        // 时间到，判定为长按
                        Key[i].long_flag = 1;
                        Key[i].state = KEY_IDLE;
                    }
                }
                break;
                
            case KEY_WAIT_RELEASE:
                if(pin_level == 1)
                {
                    // 等待松手完成，触发短按
                    Key[i].short_flag = 1;
                    Key[i].state = KEY_IDLE;
                }
                break;
                
            default:
                Key[i].state = KEY_IDLE;
                break;
        }
    }
}

/**
 * @brief  获取短按状态
 * @param  index: 按键索引
 * @retval 1表示触发，0表示未触发
 */
uint8_t KEY_GetShort(uint8_t index)
{
    if(index >= KEY_NUM) return 0;
    uint8_t result = Key[index].short_flag;
    Key[index].short_flag = 0; // 清零
    return result;
}

/**
 * @brief  获取长按状态
 * @param  index: 按键索引
 * @retval 1表示触发，0表示未触发
 */
uint8_t KEY_GetLong(uint8_t index)
{
    if(index >= KEY_NUM) return 0;
    uint8_t result = Key[index].long_flag;
    Key[index].long_flag = 0; // 清零
    return result;
}