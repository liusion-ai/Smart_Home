# 🏠 STM32 智能家居系统（OneNET 物联网平台）

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-STM32F103-blue.svg)]()
[![Cloud](https://img.shields.io/badge/Cloud-OneNET-green.svg)]()

基于 STM32F103 + OneNET 云平台的智能家居系统，支持温湿度监测、RFID 门禁、远程控制和手机 APP 交互。

## 系统架构

```
┌─────────────┐  MQTT   ┌──────────────┐  HTTP/MQTT  ┌──────────────┐
│  STM32F103  │ ───────►│  OneNET 平台  │◄────────────│   手机 APP   │
│  (设备端)    │ ◄───────│  (云端)       │             │  (HBuilder)  │
└──────┬──────┘         └──────────────┘             └──────────────┘
       │
┌──────┴──────────────────────────────┐
│  DHT11  OLED  MFRC522  Light       │
│  Motor  LED   KEY                   │
└─────────────────────────────────────┘
```

## 硬件清单

| 模块 | 型号 | 功能 |
|------|------|------|
| 主控 | STM32F103C8T6 | 核心处理器 |
| 温湿度 | DHT11 | 环境温湿度采集 |
| 显示 | 0.96寸 OLED (SSD1306) | 本地信息显示 |
| 门禁 | MFRC522 | RFID 刷卡识别 |
| 光照 | 光敏传感器 | 环境光照检测 |
| 电机 | 直流电机 | 窗帘/风扇控制 |
| LED | 多色 LED | 灯光控制 |
| 按键 | 物理按键 | 本地控制输入 |
| WiFi | ESP8266 | 联网通信 |

## 功能列表

- ✅ 温湿度实时采集与 OLED 显示
- ✅ OneNET 平台 MQTT 数据上报
- ✅ 手机 APP 远程查看传感器数据
- ✅ 手机 APP 远程控制 LED / 电机
- ✅ RFID 刷卡门禁功能
- ✅ 光照传感器自动检测
- ✅ 按键本地控制
- ⬜ OTA 远程固件升级（计划中）
- ⬜ 低功耗模式（计划中）

## 项目结构

```
├── 嵌入式设备代码/           # STM32 固件
│   └── 智能家居/
│       ├── core/             # STM32 核心文件（启动文件、CMSIS）
│       ├── fwlib/            # STM32 标准外设库
│       ├── hardware/         # 硬件驱动层
│       │   ├── src/          # DHT11 / OLED / MFRC522 / Motor / Led / ...
│       │   └── inc/          # 对应头文件
│       ├── NET/              # 网络协议层
│       │   ├── MQTT/         # MQTT 协议实现
│       │   ├── onenet/       # OneNET 平台对接
│       │   ├── CJSON/        # cJSON 解析库
│       │   └── device/       # 设备管理
│       └── user/             # 用户主程序
├── 手机终端代码/             # 手机 APP
│   └── HBuilder X 手机APP代码/   # uni-app 跨平台项目
├── 编码转换软件/             # 辅助工具
├── PCB原理图设计和按键功能展示.docx
└── 成果展示链接
```

## 快速开始

### 环境准备

- **IDE**: Keil MDK-ARM v5
- **硬件**: STM32F103C8T6 最小系统板 + 外设模块
- **云平台**: [OneNET 平台](https://open.iot.10086.cn/) 注册账号并创建产品
- **嘉立创PCB**:https://pro.lceda.cn/editor#id=fc1a4a2bb90f4fb0b3a1e4f4355c3804

### 操作步骤

1. 克隆仓库
   ```bash
   git clone https://github.com/liusion-ai/Smart_Home.git
   ```

2. 登录 OneNET 平台，创建产品（协议选 MQTT），添加设备，获取设备 ID 和鉴权信息

3. 用 Keil 打开 `嵌入式设备代码/智能家居/stm32f103.uvprojx`，在网络配置文件中填入你的 OneNET 设备信息和 WiFi 名称密码

4. 编译（F7），通过 ST-Link 烧录到开发板

5. 手机 APP：用 HBuilder X 打开 `手机终端代码/HBuilder X 手机APP代码/`，修改 OneNET API 地址后运行到手机

## 演示

📹 完整演示视频：[https://www.bilibili.com/video/BV1oF9XBrEmL/?vd_source=ecd48a869e2adaad0265fa57f5a0e836](https://www.bilibili.com/video/BV1oF9XBrEmL/?vd_source=ecd48a869e2adaad0265fa57f5a0e836)

## 开发工具

| 工具 | 用途 |
|------|------|
| Keil MDK-ARM | 嵌入式固件开发 |
| HBuilder X | 手机 APP 开发（uni-app） |
| ST-Link | 程序烧录与调试 |
| OneNET 平台 | 物联网云平台 |

## 许可证

本项目基于 [MIT License](LICENSE) 开源。

## 作者

- GitHub: [@liusion-ai](https://github.com/liusion-ai)
