# 🏠 STM32 智能家居物联网系统

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-STM32F103C8T6-blue.svg)]()
[![Cloud](https://img.shields.io/badge/Cloud-OneNET(MQTT)-green.svg)]()
[![Code Size](https://img.shields.io/badge/Code-11700%20Lines-orange.svg)]()

> 基于 STM32F103 + ESP826 + OneNET 云平台的全栈智能家居系统，涵盖嵌入式固件开发、物联网通信协议、云端对接、移动端 APP 开发完整链路。

📹 **演示视频**：[B站完整演示](https://www.bilibili.com/video/BV1oF9XBrEmL/?vd_source=ecd48a869e2adad0265fa57f5a0e836)

---

## 📐 系统架构

### 整体架构图

```mermaid
graph TB
    subgraph "移动端 APP（uni-app / HBuilder X）"
        APP["📱 手机 APP<br/>数据可视化 + 远程控制"]
    end

    subgraph "OneNET 云平台"
        MQTT_BROKER["☁️ MQTT Broker<br/>mqts.heclouds.com:183"]
        HTTP_API["🌐 HTTP API<br/>iot-api.heclouds.com"]
        THING_MODEL["📦 物模型<br/>temp / humi / led / fan / door / curtains"]
    end

    subgraph "嵌入式设备端（STM32F103C8T6）"
        subgraph "应用层 user/"
            MAIN["main.c<br/>主循环 + 业务逻辑"]
        end
        subgraph "网络协议层 NET/"
            MQTT["MQTT 协议栈<br/>MqttKit.c"]
            ONENET["OneNET 对接<br/>onenet.c<br/>HMAC-SHA1 鉴权"]
            ESP_DRV["ESP8266 驱动<br/>AT 指令集"]
            CJSON["cJSON 解析器"]
        end
        subgraph "硬件驱动层 hardware/"
            DHT11["DHT11<br/>温湿度"]
            OLED["OLED SSD1306<br/>0.96寸显示"]
            RFID["MFRC522<br/>RFID 门禁"]
            MOTOR["直流电机<br/>窗帘/风扇"]
            SERVO["舵机<br/>门锁"]
            LED["LED 灯光"]
            BUZZER["蜂鸣器"]
            MQ2["MQ-2<br/>烟雾检测"]
            LIGHT["光敏传感器<br/>光照检测"]
            FLAME["火焰传感器"]
        end
    end

    APP -->|HTTP GET/POST| HTTP_API
    APP -->|HTTP| THING_MODEL
    MQTT_BROKER -->|MQTT Publish| ESP_DRV
    ESP_DRV -->|MQTT Subscribe| MQTT_BROKER
    MQTT --> ONENET
    ONENET --> CJSON
    MAIN --> MQTT
    MAIN --> DHT1
    MAIN --> OLED
    MAIN --> RFID
    MAIN --> MOTOR
    MAIN --> SERVO
    MAIN --> LED
    MAIN --> BUZZER
    MAIN --> MQ2
    MAIN --> LIGHT
    MAIN --> FLAME

sequenceDiagram
    participant Sensor as 🔧 传感器
    participant STM32 as 💻 STM32
    participant ESP as 📡 ESP8266
    participant Cloud as ☁️ OneNET
    participant APP as 📱 手机APP

    Note over Sensor,APP: === 数据上报链路（上行）===
    Sensor->>STM32: DHT11/MQ-2 采集数据
    STM32->STM32: cJSON 构建物模型 JSON
    STM32->>ESP: USART2 发送 MQTT Publish
    ESP->>Cloud: TCP 1883 推送数据点
    Cloud->>APP: HTTP API 查询返回

    Note over Sensor,APP: === 远程控制链路（下行）===
    APP->>Cloud: HTTP POST 设置属性
    Cloud->>ESP: MQTT Publish 下发指令
    ESP->>STM32: USART2 +IPD 接收
    STM32->STM32: cJSON 解析 → 控制外设
    STM32->>Sensor: LED/电机/舵机动作

graph LR
    subgraph "Layer 3 — 应用层"
        L3["main.c<br/>业务逻辑 / 状态管理"]
    end
    subgraph "Layer 2 — 协议层"
        L2A["MQTT 协议栈"]
        L2B["OneNET 鉴权<br/>HMAC-SHA1 + Base64"]
        L2C["cJSON 解析"]
    end
    subgraph "Layer 1 — 驱动层"
        L1A["ESP8266 AT驱动"]
        L1B["DHT11 / OLED / MFRC522"]
        L1C["Motor / Servo / LED / Buzzer"]
        L1D["ADC / GPIO / SPI / I2C / USART"]
    end
    subgraph "Layer 0 — 硬件抽象"
        L0["STM32 标准外设库 (fwlib)<br/>CMSIS / NVIC / SysTick"]
    end

    L3 --> L2A
    L3 --> L2C
    L3 --> L1B
    L3 --> L1C
    L2A --> L2B
    L2A --> L1A
    L1A --> L1D
    L1B --> L1D
    L1C --> L1D
    L1D --> L0
🔧 技术选型及理由

1. 主控芯片：STM32F103C8T6

对比项	STM32F103C8T6	ESP32	STC89C52
主频	72 MHz	240 MHz	12 MHz
Flash / RAM	64KB / 20KB	4MB / 520KB	8KB / 512B
外设丰富度	SPI×2, I2C×2, USART×3, ADC×2, TIM×4	WiFi/BT 内置	极少
生态	CubeMX + HAL + 巨大社区	Arduino/IDF	过时
成本	¥6-8	¥12-15	¥3-5

选型理由：

STM32F103 是嵌入式行业入门标杆，面试官最熟悉的芯片，简历可读性高
外设资源足够覆盖本项目所有传感器（SPI 接 RFID、I2C 接 OLED、USART 接 ESP8266）
72MHz Cortex-M3 内核对于传感器采集 + 网络通信绰有余
价格低廉，适合学习和原型验证

2. 通信方案：ESP8266 + MQTT 协议

为什么选 ESP8266 而不是内置 WiFi 的 ESP32？

体现 MCU + 独立 WiFi 模块 的经典架构，更贴近工业产品实际方案（模块化设计，便于替换升级）
学习 AT 指令集交互方式，理解串口通信底层机制
STM32 负责业务逻辑，ESP826 仅负责网络透传，职责分离

为什么选 MQTT 而不是 HTTP？


| 对比项 | MQTT | HTTP |
|--------|------|
| 连接方式 | 长连接 | 短连接（每次新建） |
| 协议开销 | 2 字节最小包头 | 几百字节头部 |
| 实时性 | 毫秒级推送 | 需轮询 |
| 功耗 | 低（心跳保活） | 高（频繁建连） |
| 适用场景 | IoT 双向通信 | Web API |


选型理由：

MQTT 是 IoT 事实标准，OneNET 平台原生支持
发布/订阅模型天然支持一对多（一个设备 → 多个 APP 端）
QoS 机制保证消息可靠性，适合控制指令场景
协议轻量，适合 STM32 这种 RAM 仅 20KB 的 MCU

3. 云平台：OneNET

中国移动运营，国内访问稳定，无需翻墙
免费套餐足够开发测试
提供物模型（Thing Model）标准数据格式，方便 APP 对接
HTTP API 文档完善，支持设备影子、数据流、命令下发

4. 移动端：uni-app (HBuilder X)

一套代码同时编译 Android / iOS / H5
Vue.js 语法，前端开发者零门槛
通过 OneNET HTTP API 与设备交互，无需自建服务器
深色/浅色主题切换，UI 体验优于原生 IoT 平台自带 APP
Smart_Home/
├── 嵌入式设备代码/                    # STM32 固件（Keil MDK-ARM v5 工程）
│   └── 智能家居/
│       ├── core/                     # CMSIS + 启动文件 + 中断向量
│       │   ├── stm32f10x.h          #   寄存器定义
│       │   ├── core_cm3.h           #   Cortex-M3 内核
│       │   ├── system_stm32f10x.c   #   时钟树配置 (72MHz HSE+PLL)
│       │   └── stm32f10x_it.c       #   中断服务函数
│       ├── fwlib/                    # STM32 标准外设库 v3.5
│       │   ├── src/                  #   GPIO / USART / SPI / I2C / TIM / ADC / NVIC
│       │   └── inc/                  #   对应头文件
│       ├── hardware/                 # 硬件驱动层（自研）
│       │   ├── src/
│       │   │   ├── dht1.c          #   温湿度传感器 (1-Wire / PA0)
│       │   ├── OLED.c           #   0.96" OLED 显示 (I2C / SSD1306)
│       │   │   ├── MFRC522.c        #   RFID 射频模块 (SPI / PB12-PB15)
│       │   │   ├── Motor.c          #   直流电机驱动 (PWM / TIM3)
│       │   │   ├── Servo.c          #   舵机控制 (PWM / TIM2)
│       │   │   ├── Led.c            #   LED 控制 (GPIO)
│       │   ├── buzer.c         #   蜂鸣器 (GPIO)
│       │   ├── mq2.c            #   烟雾传感器 (ADC)
│       │   │   ├── LightSensor.c    #   光敏传感器 (GPIO)
│       │   │   ├── flame.c          #   火焰传感器 (GPIO)
│       │   │   ├── key.c            #   矩阵按键扫描
│       │   ├── KEY1.c           #   独立按键（长按/短按检测）
│       │   │   ├── PWM.c            #   PWM 输出配置
│       │   │   ├── adcx.c           #   ADC 采集封装
│       │   ├── flash.c          #   内部 Flash 读写
│       │   ├── delay.c          #   SysTick 精确延时
│       │   │   ├── usart.c          #   USART1(调试) + USART2(ESP826)
│       │   │   └── sys.c            #   系统配置
│       │   └── inc/                  #   对应头文件
│       ├── NET/                      # 网络协议层
│       │   ├── MQTT/
│       │   │   ├── MqttKit.c/.h     #   MQTT 协议栈（CONNECT/PUBLISH/SUBSCRIBE/PING）
│       │   │   └── Common.h         #   通用类型定义
│       │   ├── onenet/
│       │   ├── onenet.c/.h      #   OneNET 平台对接（鉴权/注册/数据上报/命令接收）
│       │   │   ├── base64.c/.h      #   Base64 编解码
│       │   │   └── hmac_sha1.c/.h   #   HMAC-SHA1 签名
│       │   ├── CJSON/
│       │   └── cJSON.c/.h       #   JSON 解析库
│       │   └── device/
│       │       └── esp826.c/.h     #   ESP8266 AT 指令驱动
│       └── user/
│           └── main.c               #   主程序（初始化 + 主循环 + 业务逻辑）
│
├── 手机终端代码/                     # 移动端 APP
│   └── HBuilder X 手机APP代码/      # uni-app 工程
│       ├── pages/index/index.vue    #   主页面（设备卡片 + 控制开关）
│       ├── key.js                   #   OneNET Token 生成（HMAC-SHA1）
│       ├── App.vue                  #   应用入口
│       └── static/                  #   图标资源
│
├── 编码转换软件/                     # 辅助工具
├── PCB原理图设计和按键功能展示.docx  # 硬件设计文档
└── 成果展示链接                      # 演示视频
``

---

## ⚡ 功能清单

| 功能 | 状态 | 实现方式 | 关键技术点 |
|------|-----------|
| 温湿度实时采集 | ✅ | DHT1 → OLED 显示 + 云端上报 | 1-Wire 时序、数据校验 |
| 烟雾/火焰检测 | ✅ | MQ-2 ADC 采集 + 阈值报警 | ADC 多通道、DMA 采集 |
| RFID 刷卡门禁 | ✅ | MFRC522 SPI 通信 + 白名单 | SPI 协议、防碰撞算法 |
| 舵机门锁控制 | ✅ | PWM 角度控制 | TIM PWM 输出 |
| 窗帘电机控制 | ✅ | 直流电机正反转 | H 桥驱动、PWM 调速 |
| 灯光控制 | ✅ | LED GPIO 开关 | GPIO 输出 |
| 蜂鸣器报警 | ✅ | 单次/双重/长鸣模式 | GPIO + 软件定时 |
| 光照自动窗帘 | ✅ | 光敏传感器 → 自动开合窗帘 | 中断/轮询混合模式 |
| 按键本地控制 | ✅ | 5 路独立按键（长按/短按） | 消抖算法、状态机 |
| OLED 信息显示 | ✅ | 0.96" SSD1306 中英文显示 | I2C 驱动、字库管理 |
| MQTT 数据上报 | ✅ | 5 秒周期上报温湿度+设备状态 | cJSON 构建、MQTT Publish |
| 远程控制 | ✅ | OneNET → MQTT 下发 → 设备执行 | 订阅/发布模型、JSON 解析 |
| 手机 APP | ✅ | uni-app 跨平台 + OneNET HTTP API | Vue.js、REST API、深色模式 |
| 设备自动注册 | ✅ | 首次上电通过 HTTP API 自动注册设备 | HMAC-SHA1 鉴权、Base64 |
| WiFi 联网 | ✅ | ESP8266 AT 指令连接路由器 | AT 指令集、中断接收 |

### 🔮 计划中功能

- [ ] OTA 远程固件升级
- [ ] 低功耗模式（STOP + WKUP）
- [ ] 看门狗（IWDG）防死机
- [ ] Flash 参数持久化（阈值/白名单）
- [ ] 多设备联动规则引擎

---

## 🔌 硬件接线

| 模块 | 引脚 | STM32 引脚 | 通信接口 |
|------|-----------|---------|
| ESP8266 | TX/RX | PA3/PA2 (USART2) | UART 115200bps |
| DHT1 | DATA | PA0 | 1-Wire |
| OLED SSD1306 | SCL/SDA | PB6/PB7 (I2C1) | I2C 40KHz |
| MFRC522 | SDA/SCK/MISO/MOSI/RST | PB12/PB13/PB14/PB15/PB0 | SPI（软件模拟） |
| 直流电机 | IN1/IN2 | PA6/PA7 (TIM3 CH1/CH2) | PWM |
| 舵机 | PWM | PA1 (TIM2 CH2) | PWM 50Hz |
| LED | GPIO | PB1/PB2 | GPIO |
| 蜂鸣器 | GPIO | PB8 | GPIO |
| MQ-2 | AO | PA4 (ADC1 CH4) | ADC |
| 光敏传感器 | DO | PB3 | GPIO |
| 火焰传感器 | DO | PB4 | GPIO |
| 按键 ×5 | GPIO | PA4-PA8 | GPIO Input |

---

## 📊 性能数据

| 指标 | 数据 |
|------|
| 固件代码量 | ~11,700 行（含库） |
| 自研代码量 | ~3,500 行（hardware/ + NET/ + user/） |
| Flash 占用 | ~48KB / 64KB（75%） |
| RAM 占用 | ~12KB / 20KB（60%） |
| 主循环周期 | 10ms |
| 传感器采集周期 | 5s（DHT11 最小间隔 2s） |
| MQTT 数据上报周期 | 5s |
| WiFi 连接建立时间 | ~8s（AT → 连接 → 获取IP → MQTT握手） |
| 控制指令响应延迟 | <500ms（APP → Cloud → 设备） |
| 连续运行稳定性 | 72 小时+ 零断连（含自动重连机制） |
| MQTT 包大小 | ~120 bytes/次（JSON payload） |
| OLED 刷新率 | ~10fps（I2C 400KHz） |

---

## 🚀 快速开始

### 环境准备

| 工具 | 版本 | 用途 |
|------|------|
| Keil MDK-ARM | v5.38+ | STM32 固件编译 |
| ST-Link V2 | — | 程序烧录 & 在线调试 |
| HBuilder X | 最新版 | uni-app APP 开发 |
| OneNET 平台 | — | [注册账号](https://open.iot.10086.cn/) |

### 硬件准备

1. STM32F103C8T6 最小系统板 ×1
2. ESP826-01S WiFi 模块 ×1
3. DHT11 温湿度模块 ×1
4. 0.96寸 OLED 显示屏（SSD1306 I2C）×1
5. MFRC522 RFID 模块 + S50 卡/钥匙扣 ×1
6. L298N 电机驱动模块 ×1
7. SG90 舵机 ×1
8. MQ-2 烟雾传感器 ×1
9. 光敏传感器模块 ×1
10. 有源蜂鸣器 ×1
1. LED、按键、杜邦线若干

### 操作步骤

```bash
# 1. 克隆仓库
git clone https://github.com/liusion-ai/Smart_Home.git
设备端：


2.登录 
OneNET 平台
，创建产品（协议选 MQTT，数据协议选 物模型）
3.添加设备，获取 产品ID、设备名称、AccessKey
4.在 onenet.c 中修改以下常量：
#define PROID       "你的产品ID"
#define ACCESS_KEY  "你的AccessKey"
#define DEVICE_NAME "你的设备名称"
5.在 esp8266.c 中修改 WiFi 信息：
#define ESP8266_WIFI_INFO  "AT+CWJAP=\"你的WiFi名\",\"你的WiFi密码\"\r\n"
6.用 Keil 打开 嵌入式设备代码/智能家居/stm32f103.uvprojx，编译（F7），通过 ST-Link 烧录
手机 APP：


7.在 key.js 中填入 OneNET 设备信息：
javascript
javascript
productId: '你的产品ID',
deviceName: '你的设备名称',
accessKey: '你的AccessKey',
8.用 HBuilder X 打开 手机终端代码/HBuilder X 手机APP代码/，运行到手机或模拟器
🛠️ 开发工具链
graph LR
    subgraph "开发环境"
        KEIL["Keil MDK-ARM v5<br/>固件编译"]
        HBUILDER["HBuilder X<br/>APP 开发"]
    end
    subgraph "烧录调试"
        STLINK["ST-Link V2<br/>SWD 烧录"]
        SERIAL["串口调试助手<br/>USART1 1520"]
    end
    subgraph "云平台"
        ONENET["OneNET 平台<br/>设备管理"]
        LEDA["嘉立创 EDA<br/>PCB 设计"]
    end

    KEIL --> STLINK
    STLINK --> MCU["STM32F103"]
    MCU --> SERIAL
    HBUILDER --> APP_PHONE["手机 APP"]
    APP_PHONE --> ONENET
    ONENET --> MCU
    LEDA --> PCB["PCB 打板"]
🤔 设计决策记录

Q1: 为什么用标准外设库而不用 HAL 库？

本项目作为学习项目，使用标准外设库（StdPeriph Lib）可以让开发者更接近寄存器操作，理解底层原理。标准外设库代码可读性好，不会像 HAL 那样隐藏大量细节。在实际工业项目中，建议迁移到 HAL + CubeMX 以提高开发效率。


Q2: 为什么没用 FreeRTOS？

对于本项目的复杂度（5 个传感器 + 网络通信），裸机轮询架构（10ms 主循环）完全够用。引入 RTOS 会增加学习曲线和 RAM 开销（FreeRTOS 最少需要 ~2KB RAM）。但在生产环境中，建议引入 FreeRTOS 来管理多任务，特别是当任务数量增加、需要优先级调度时。


Q3: ESP826 为什么用中断接收而不是 DMA？

ESP8266 的 AT 指令交互是异步的、不定长的，使用中断逐字节接收 + 超时判断（ESP826_WaitRecive）是最简单可靠的方案。DMA 适合定长数据块传输，对于 AT 指令场景配置复杂且收益不大。


Q4: cJSON 在 20KB RAM 的 MCU 上会不会爆内存？

有风险。cJSON 使用 malloc 动态分配内存，如果 JSON 包过大或内存碎片化严重可能导致分配失败。生产环境建议使用更轻量的 JSON 解析方案（如 JSMN），或限制 JSON 深度和字段数量。



📄 许可证

本项目基于 
MIT License
 开源。


👤 作者

GitHub: 
@liusion-ai
项目开发周期：2024.09 - 2025.01
READMEOF
