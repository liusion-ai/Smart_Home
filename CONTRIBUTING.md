# 贡献指南

感谢你对本项目的关注！欢迎贡献代码、报告问题或提出建议。

## 如何贡献

### 报告 Bug

1. 在 [Issues](https://github.com/liusion-ai/--STM32------------/issues) 页面创建新 Issue
2. 描述问题的复现步骤
3. 注明使用的硬件型号、Keil 版本、固件版本
4. 如有可能，附上截图或日志

### 提交代码

1. Fork 本仓库
2. 创建你的功能分支：`git checkout -b feature/你的功能名`
3. 提交你的修改：`git commit -m '添加了某某功能'`
4. 推送到你的分支：`git push origin feature/你的功能名`
5. 创建 Pull Request

### 代码规范

- **C 代码**：遵循 STM32 标准外设库的编码风格
- **命名**：函数用驼峰命名（如 `ReadDHT11`），宏定义全大写（如 `LED_PIN`）
- **注释**：关键函数必须有注释说明功能、参数和返回值
- **文件组织**：硬件驱动放在 `hardware/`，网络协议放在 `NET/`

### 硬件相关

- 如修改引脚配置，请在代码注释中标明对应原理图位置
- 新增外设驱动请放在 `hardware/src/` 和 `hardware/inc/` 下

## 开发环境

- Keil MDK-ARM v5
- STM32F103C8T6 最小系统板
- ST-Link V2 调试器

## 问题交流

如有疑问，欢迎在 Issues 中讨论。
