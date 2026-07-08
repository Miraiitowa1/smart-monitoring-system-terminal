# Smart Kitchen — 智能厨房环境监控系统

基于 STM32F103C8 的智能厨房环境监测与控制系统。集成 DHT11 温湿度、MQ-2 烟雾、MQ-135 空气质量、火焰传感器数据采集，0.96 寸 OLED 本地显示，ESP8266 WiFi 连接 OneNET 云平台实现 MQTT 双向通信，支持云端远程 LED/报警控制及本地按键交互设置。

---

## 目录

1. [硬件平台](#硬件平台)
2. [项目结构](#项目结构)
3. [模块详解](#模块详解)
4. [软件架构](#软件架构)
5. [界面与按键](#界面与按键)
6. [传感器数据处理](#传感器数据处理)
7. [云平台通信](#云平台通信)
8. [中断与定时器](#中断与定时器)
9. [开发环境与编译](#开发环境与编译)
10. [已知限制与改进方向](#已知限制与改进方向)

---

## 硬件平台

### MCU: STM32F103C8T6

- 内核: ARM Cortex-M3 @ 72MHz
- Flash: 64KB / RAM: 20KB
- 工作电压: 3.3V

---

### 完整引脚分配

| 引脚 | 功能 | 外设 | 模式 | 说明 |
|------|------|------|------|------|
| PA0 | RELAY | GPIO | 推挽输出 | 继电器控制，高电平吸合 |
| PA1 | MQ-2 AO | ADC1 CH1 | 模拟输入 | 烟雾浓度模拟量 |
| PA2 | FLAME AO | ADC1 CH2 | 模拟输入 | 火焰传感器模拟量 |
| PA3 | MQ-135 AO | ADC1 CH3 | 模拟输入 | 空气质量模拟量 |
| PA9 | ESP8266 RX | USART1 TX | 复用推挽 | STM32 → ESP8266 |
| PA10 | ESP8266 TX | USART1 RX | 上拉输入 | ESP8266 → STM32 |
| PA11 | BUZZER | GPIO | 推挽输出 | 蜂鸣器，高电平响 |
| PB5 | DHT11 DAT | GPIO | 双向(OD+IPU) | 单总线数据线 |
| PB6 | OLED SCL | GPIO | 开漏输出 | I2C 时钟线（软件模拟） |
| PB7 | OLED SDA | GPIO | 开漏输出 | I2C 数据线（软件模拟） |
| PB8 | STEP A | GPIO | 推挽输出 | 步进电机 A 相 |
| PB9 | STEP B | GPIO | 推挽输出 | 步进电机 B 相 |
| PB10 | DEBUG TX | USART3 TX | 复用推挽 | 调试串口发送 |
| PB11 | DEBUG RX | USART3 RX | 上拉输入 | 调试串口接收 |
| PB12 | KEY1 | GPIO | 上拉输入 | 按键 1，按下低电平 |
| PB13 | KEY2 | GPIO | 上拉输入 | 按键 2，按下低电平 |
| PB14 | STEP C | GPIO | 推挽输出 | 步进电机 C 相 |
| PB15 | STEP D | GPIO | 推挽输出 | 步进电机 D 相 |
| PC13 | LED | GPIO | 推挽输出 | 板载 LED，低电平点亮 |

### 电源树
```text
5V 外部供电
 ├── 3.3V LDO → STM32, OLED, DHT11, 按键上拉
 ├── 5V → ESP8266（需独立供电，峰值电流 ~300mA）
 ├── 5V → 继电器线圈
 ├── 5V → MQ-2 / MQ-135 加热丝
 └── 5V → 步进电机驱动
```

---

### 项目结构
```text
project/
├── User/                                 # 用户入口层
│   ├── main.c                            # main() 入口，直接调用 App_Init()
│   ├── stm32f10x_it.c                    # 中断向量表实现（SysTick, HardFault 等异常处理）
│   ├── stm32f10x_it.h                    # 中断函数声明
│   └── stm32f10x_conf.h                  # 标准外设库配置（include 哪些外设驱动、assert 开关）
│
├── APP/                                  # 应用逻辑层
│   ├── app_init.c                        # ★核心文件★ 系统初始化、主循环、OLED 6页面渲染、按键事件分发
│   ├── app_irq.irq.h                     # 应用层头文件（全局变量 extern 声明、include 汇总）
│   ├── app_alarm.c                       # 蜂鸣器 GPIO 初始化
│   └── app_alarm.h                       # 蜂鸣器/LED 宏定义（ALARM_ON / ALARM_OFF）
│
├── BSP/                                  # 板级驱动层（BSP = Board Support Package）
│   ├── bsp_oled.c                        # OLED 驱动（1.5K行）— 12C 软件模块、显存管理、完整 2D 图形库
│   ├── bsp_oled.h                        # OLED API 声明
│   ├── bsp_oledfont.c                    # 字模数据 — ASCII 6x8/8x16 + 中文 16x16（带 UTF-8/GB2312 双编码格式）
│   ├── bsp_oledfont.h                    # 字模结构体定义
│   ├── bsp_dht11.c                       # DHT11 单总线驱动 — 复位脉冲 + 40bit 读取 + 校验
│   ├── bsp_dht11.h                       # DHT11 数据结构体 + 引脚宏
│   ├── bsp_adv.c                         # ADC1 三通道 DMA 扫描 — 火焰/MQ-2/MQ-135 共用
│   ├── bsp_adv.c                         # ADC 引脚宏 + 通道号 + ADCx_PPM() 声明
│   ├── bsp_key.c                         # 按键状态机 — 5 状态检测单击/双击/长按（基于 1ms tick）
│   ├── bsp_key.h                         # KeyEvent 枚举 + Key_GetEvent() API
│   ├── bsp_led.c                         # LED GPIO 初始化
│   ├── bsp_led.h                         # LED_ON / LED_OFF 宏定义
│   ├── bsp_usart.c                       # USART1 (ESP8266) + USART3 (调试) — 初始化/发送/Printf
│   ├── bsp_usart.h                       # USART 宏 + USART_DEBUG 宏定义
│   ├── bsp_timer.c                       # TIM2 初始化 + 1ms 中断 + GetTick_ms() 全局时钟
│   ├── bsp_timer.h                       # TIM2 宏 + GetTick_ms() 声明
│   ├── bsp_flame.c / h                   # （预留）火焰传感器独立读取 — 实际未使用
│   ├── bsp_mq2.c / h                     # （预留）MQ-2 独立读取 — 实际未使用
│   ├── bsp_mq135.c / h                   # （预留）MQ-135 独立读取 — 实际未使用
│   ├── bsp_steppermotor.c / h            # 步进电机驱动 — 4拍/8拍/正反转/角度控制
│   └── relay.c / h                       # 继电器 初始化 + RELAY_ON/OFF 宏
│
├── SYSTEM/                               # 系统组件层
│   ├── bsp_delay.c                       # SysTick 阻塞延时 — DelayUs() / DelayMs()
│   ├── bsp_delay.h                       # 延时函数声明
│   ├── bsp_filter.c                      # 一阶互补滤波器 — 低通 + 高通混合
│   ├── bsp_filter.h                      # Filter 结构体 + Filter_Init() / FilterValue()
│   └── sys.h                             # STM32 类型别名（u8=uint8_t, u16=uint16_t, u32=uint32_t）
│
├── WIFI/                                 # 网络通信层
│   ├── esp8266.c                         # ESP8266 AT 指令驱动 — 初始化/WiFi连接/TCP透传/接收解析
│   ├── esp8266.h                         # ESP8266 API 声明 + REV_OK/REV_WAIT 宏
│   ├── onenet.c                          # OneNET 平台适配 — DevLink登录/Subscribe订阅/Published发布/RevPro接收
│   ├── onenet.h                          # OneNET API 声明
│   ├── MqttKit.c                         # MQTT 3.1.1 协议栈 — CONNECT/SUBSCRIBE/PUBLISH 组包解包 + 内存管理
│   ├── MqttKit.h                         # MQTT 包类型枚举/QsO等级/标志位 + 全部 API 声明
│   ├── cJSON.c                           # JSON 解析库（cJSON 标准实现）
│   ├── cJSON.h                           # cJSON API 声明
│   └── Common.h                          # MQTT 层通用类型（uint1, uint8, int16, size_t 等）
│
├── Library/                              # STM32F10x 标准外设库 V3.5.0
│   ├── stm32f10x_qpio.c / h              # GPIO 外设驱动
│   ├── stm32f10x_rcc.c / h               # 时钟系统驱动
│   ├── stm32f10x_usart.c / h             # 串口外设驱动
│   ├── stm32f10x_adc.c / h               # ADC 外设驱动
│   ├── stm32f10x_dma.c / h               # DMA 外设驱动
│   ├── stm32f10x_tim.c / h               # 定时器外设驱动
│   ├── stm32f10x_spi.c / h               # SPI 外设驱动
│   ├── stm32f10x_i2c.c / h               # I2C 外设驱动
│   ├── stm32f10x_exti.c / h              # 外部中断驱动
│   ├── stm32f10x_flash.c / h             # Flash 驱动
│   ├── misc.c / h                        # NVIC 中断优先级管理
│   └── ...（其余外设驱动）
│
├── Startup/                              # 启动层
│   ├── startup_stm32f10x_md.s            # 启动汇编（向量表 + Reset_Handler）
│   ├── system_stm32f10x.c                # 系统时钟初始化 SystemInit()
│   ├── system_stm32f10x.h                # 系统时钟声明
│   ├── stm32f10x.h                       # 芯片顶层头文件（寄存器定义）
│   └── core_cm3.c / .h                   # CMSIS Core（NVIC/SysTick 寄存器）
│
├── DebugConfig/                          # Keil 调试器配置
│   └── Target_1_STM32F103C8_1.0.0.dbgconf
│
├── .cmsis/                               # CMSIS Pack（ARM Cortex-A 支持文件，本项目未使用）
├── .eide/                                # EIDE 插件配置
├── .vscode/                              # VS Code 配置
├── build/                                # 编译中间文件
├── Listings/                             # .map / .lst 链接输出
├── Objects/                              # .o / .axf / .hex 目标文件
│
├── project.uvprojx                       # Keil MDK 工程文件
├── project.uvoptx                        # Keil 工程选项
├── project.uvguix.xf                     # Keil 用户布局
├── project.code-workspace                # VS Code 工作区
├── .clang-format                         # 代码格式化规则
├── .gitignore                            # Git 忽略规则
└── README.md                             # 本文件
```
---

### 模块详解
```c
#include "app_init.h"
int main(void)
{
    App_Init();     // 永不返回，内部包含 while(1) 主循环
}
```
---
### 软件架构
main()
 └── App_Init()
      │
      ├── Delay_Init()               SysTick 时钟源设为 72MHz÷8=9MHz
      │                               UsCount=9, MsCount=9000（用于阻塞延时）
      │
      ├── GENERAL_TIM_Init()         TIM2 使能，PSC=71, Period=999
      │                               → 1ms 中断 → tick_ms++
      │                               GetTick_ms() 供按键状态机使用
      │
      ├── OLED_Init()                OLED I2C 引脚初始化 + 30+ 条配置命令
      │                               → OLED_Clear() → OLED_Update()
      │
      ├── DHT11_Init()               DHT11 数据线 PB5 配置为推挽输出，初始拉高
      │
      ├── Key_Init()                 KEY1(PB12) + KEY2(PB13) 上拉输入
      │
      ├── LED_Init()                 LED(PC13) 推挽输出，初始关
      │
      ├── ADCx_Init()                ADC1 三通道 DMA 扫描启动
      │                               → 连续转换，DMA 循环填充 ADCx_Value[3]
      │
      ├── Alarm_Init()               蜂鸣器(PA11) 推挽输出，初始关
      │
      ├── ESP8266_Init()             ★ 6步 AT 命令序列 ★
      │    ├── AT                    → 测试
      │    ├── AT+CWMODE=1           → Station 模式
      │    ├── AT+CWDHCP=1,1         → DHCP
      │    ├── AT+CWJAP="SSID","PWD"  → 连接 WiFi（直到获取 IP）
      │    └── AT+CIPSTART="TCP","mqtts.heclouds.com",1883 → TCP 连云
      │
      ├── OLED_Printf("网络连接中...") → OLED_Update()
      │
      ├── while(OneNet_DevLink())    MQTT CONNECT 鉴权登录
      │    └── DelayMs(500)          失败重试
      │
      ├── OLED 显示"连接成功"         持续 3 秒后清屏
      │
      ├── OneNet_Subscribe(topics,1)  订阅下行控制主题
      │
      └── while(1)  ★ 主循环 ★
           │
           ├── OLED_Switch()          按键事件处理 + 当前页渲染
           │    ├── Key_GetEvent(KEY_ID_1)
           │    ├── Key_GetEvent(KEY_ID_2)
           │    ├── 单击 → 翻页
           │    └── switch(currentState) → OLED_Show / Show1~5
           │         └── Threshold_Adjust() 处理阈值修改（双击/长按）
           │
           ├── Alarm_Statue()         报警判断
           │    └── DHT11_Data.temp_int > Temp_Thr
           │        || DHT11_Data.humi_int > Humi_Thr
           │        → Alarm_flag==1 ? ALARM_ON : ALARM_OFF
           │
           ├── 每100轮:
           │    ├── JsonValue()       构造上传 JSON
           │    ├── OneNet_Publish()  发布到云平台
           │    ├── ESP8266_Clear()   清接收缓冲
           │    └── TimeCount = 0
           │
           └── ESP8266_GetIPD(2)     轮询下行数据（10ms 超时）
                └── OneNet_RevPro()   解析并执行云平台指令

