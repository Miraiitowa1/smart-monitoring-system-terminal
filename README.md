        1  # Smart Kitchen — 智能厨房环境监控系统
        2
        3 -基于 STM32F103 的智能厨房监测系统，集成多传感器数据采集、OLED 显示、WiFi 连云（OneNET MQTT）、声光报警
          -及继电器控制。
        3 +基于 STM32F103C8 的智能厨房环境监测与控制系统。集成 DHT11 温湿度、MQ-2 烟雾、MQ-135 空气质量、火焰传感
          +器数据采集，0.96 寸 OLED 本地显示，ESP8266 WiFi 连接 OneNET 云平台实现 MQTT 双向通信，支持云端远程 LED
          +/报警控制及本地按键交互设置。
        4
        5 +---
        6 +
        7 +## 目录
        8 +
        9 +1. [硬件平台](#硬件平台)
       10 +2. [项目结构](#项目结构)
       11 +3. [模块详解](#模块详解)
       12 +4. [软件架构](#软件架构)
       13 +5. [界面与按键](#界面与按键)
       14 +6. [传感器数据处理](#传感器数据处理)
       15 +7. [云平台通信](#云平台通信)
       16 +8. [中断与定时器](#中断与定时器)
       17 +9. [开发环境与编译](#开发环境与编译)
       18 +10. [已知限制与改进方向](#已知限制与改进方向)
       19 +
       20 +---
       21 +
       22  ## 硬件平台
       23
        7 -| 模块 | 型号 / 接口 |
        8 -|------|------------|
        9 -| MCU | STM32F103（72MHz, Cortex-M3） |
       10 -| 显示 | 0.96寸 OLED（I2C, PB6-SCL / PB7-SDA） |
       11 -| 温湿度 | DHT11（单总线, PB5） |
       12 -| 火焰传感器 | 火焰检测模块（AO 接 PA2 → ADC1 CH2） |
       13 -| 烟雾传感器 | MQ-2（AO 接 PA1 → ADC1 CH1） |
       14 -| 空气质量 | MQ-135（AO 接 PA3 → ADC1 CH3） |
       15 -| WiFi | ESP8266（USART1: PA9-TX / PA10-RX, 115200bps） |
       16 -| 按键 | KEY1-PB12 / KEY2-PB13（上拉输入） |
       17 -| LED | PC13（低电平点亮） |
       18 -| 蜂鸣器 | PA11（高电平触发） |
       19 -| 继电器 | PA0（高电平吸合） |
       20 -| 步进电机 | PB8/PB9/PB14/PB15（4 相） |
       21 -| 调试串口 | USART3（PB10-TX / PB11-RX, 115200bps） |
       24 +### MCU: STM32F103C8T6
       25
       26 +- 内核: ARM Cortex-M3 @ 72MHz
       27 +- Flash: 64KB / RAM: 20KB
       28 +- 工作电压: 3.3V
       29 +
       30 +### 完整引脚分配
       31 +
       32 +| 引脚 | 功能 | 外设 | 模式 | 说明 |
       33 +|------|------|------|------|------|
       34 +| PA0 | RELAY | GPIO | 推挽输出 | 继电器控制，高电平吸合 |
       35 +| PA1 | MQ-2 AO | ADC1 CH1 | 模拟输入 | 烟雾浓度模拟量 |
       36 +| PA2 | FLAME AO | ADC1 CH2 | 模拟输入 | 火焰传感器模拟量 |
       37 +| PA3 | MQ-135 AO | ADC1 CH3 | 模拟输入 | 空气质量模拟量 |
       38 +| PA9 | ESP8266 RX | USART1 TX | 复用推挽 | STM32 → ESP8266 |
       39 +| PA10 | ESP8266 TX | USART1 RX | 上拉输入 | ESP8266 → STM32 |
       40 +| PA11 | BUZZER | GPIO | 推挽输出 | 蜂鸣器，高电平响 |
       41 +| PB5 | DHT11 DAT | GPIO | 双向(OD+IPU) | 单总线数据线 |
       42 +| PB6 | OLED SCL | GPIO | 开漏输出 | I2C 时钟线（软件模拟） |
       43 +| PB7 | OLED SDA | GPIO | 开漏输出 | I2C 数据线（软件模拟） |
       44 +| PB8 | STEP A | GPIO | 推挽输出 | 步进电机 A 相 |
       45 +| PB9 | STEP B | GPIO | 推挽输出 | 步进电机 B 相 |
       46 +| PB10 | DEBUG TX | USART3 TX | 复用推挽 | 调试串口发送 |
       47 +| PB11 | DEBUG RX | USART3 RX | 上拉输入 | 调试串口接收 |
       48 +| PB12 | KEY1 | GPIO | 上拉输入 | 按键 1，按下低电平 |
       49 +| PB13 | KEY2 | GPIO | 上拉输入 | 按键 2，按下低电平 |
       50 +| PB14 | STEP C | GPIO | 推挽输出 | 步进电机 C 相 |
       51 +| PB15 | STEP D | GPIO | 推挽输出 | 步进电机 D 相 |
       52 +| PC13 | LED | GPIO | 推挽输出 | 板载 LED，低电平点亮 |
       53 +
       54 +### 电源树
       55 +
       56 +```
       57 +5V 外部供电
       58 + ├── 3.3V LDO → STM32, OLED, DHT11, 按键上拉
       59 + ├── 5V → ESP8266（需独立供电，峰值电流 ~300mA）
       60 + ├── 5V → 继电器线圈
       61 + ├── 5V → MQ-2 / MQ-135 加热丝
       62 + └── 5V → 步进电机驱动
       63 +```
       64 +
       65 +---
       66 +
       67  ## 项目结构
       68
       69  ```
       70  project/
       27 -├── User/                       # 用户入口
       28 -│   ├── main.c                  # main() → App_Init()
       29 -│   ├── stm32f10x_it.c          # 中断服务函数（SysTick, HardFault 等）
       30 -│   ├── stm32f10x_it.h
       31 -│   └── stm32f10x_conf.h        # 标准外设库配置文件
       71  │
       33 -├── APP/                        # 应用层
       34 -│   ├── app_init.c / .h         # 系统初始化 & 主循环 & OLED 界面 & 按键逻辑
       35 -│   └── app_alarm.c / .h        # 蜂鸣器/LED 报警控制
       72 +├── User/                            # 用户入口层
       73 +│   ├── main.c                       # main() 入口，直接调用 App_Init()
       74 +│   ├── stm32f10x_it.c               # 中断向量表实现（SysTick, HardFault 等异常处理）
       75 +│   ├── stm32f10x_it.h               # 中断函数声明
       76 +│   └── stm32f10x_conf.h             # 标准外设库配置（include 哪些外设驱动、assert 开关）
       77  │
       37 -├── BSP/                        # 板级驱动（BSP）
       38 -│   ├── bsp_oled.c / .h         # 0.96寸 OLED 驱动（I2C 软件模拟，完整图形库）
       39 -│   ├── bsp_oledfont.c / .h     # OLED 字模（6x8 / 8x16 ASCII + 16x16 中文）
       40 -│   ├── bsp_dht11.c / .h        # DHT11 温湿度传感器驱动
       41 -│   ├── bsp_adc.c / .h          # ADC1 + DMA 三通道扫描（火焰/MQ-2/MQ-135）
       42 -│   ├── bsp_key.c / .h          # 按键驱动（单击/双击/长按状态机）
       43 -│   ├── bsp_led.c / .h          # LED 控制（PC13）
       44 -│   ├── bsp_usart.c / .h        # USART1（ESP8266）+ USART3（调试）驱动
       45 -│   ├── bsp_timer.c / .h        # TIM2 1ms 时基（供按键状态机使用）
       46 -│   ├── bsp_flame.c / .h        # 火焰传感器（预留，未使用）
       47 -│   ├── bsp_mq2.c / .h          # MQ-2 传感器（预留，未使用）
       48 -│   ├── bsp_mq135.c / .h        # MQ-135 传感器（预留，未使用）
       49 -│   ├── bsp_steppermotor.c / .h # 步进电机驱动（4相 4拍/8拍）
       50 -│   ├── relay.c / .h            # 继电器控制
       51 -│   └── bsp_timer.c / .h        # 通用定时器
       78 +├── APP/                             # 应用逻辑层
       79 +│   ├── app_init.c                   # ★核心文件★ 系统初始化、主循环、OLED 6页面渲染、按键事件分发
       80 +│   └── app_init.h                   # 应用层头文件（全局变量 extern 声明、include 汇总）
       81 +│   ├── app_alarm.c                  # 蜂鸣器 GPIO 初始化
       82 +│   └── app_alarm.h                  # 蜂鸣器/LED 宏定义（ALARM_ON / ALARM_OFF）
       83  │
       53 -├── SYSTEM/                     # 系统组件
       54 -│   ├── bsp_delay.c / .h        # SysTick 阻塞延时（us / ms）
       55 -│   ├── bsp_filter.c / .h       # 一阶互补滤波器
       56 -│   └── sys.h                   # 类型别名定义（u8/u16/u32 等）
       84 +├── BSP/                             # 板级驱动层（BSP = Board Support Package）
       85 +│   ├── bsp_oled.c                   # OLED 驱动（1.5K行）— I2C 软件模拟、显存管理、完整 2D 图形库
       86 +│   ├── bsp_oled.h                   # OLED API 声明（ShowChar/String/Num/Image/DrawLine/Circle/...)
       87 +│   ├── bsp_oledfont.c               # 字模数据 — ASCII 6x8/8x16 + 中文 16x16（带 UTF-8/GB2312 双编码
          +检索）
       88 +│   ├── bsp_oledfont.h               # 字模结构体定义
       89 +│   ├── bsp_dht11.c                  # DHT11 单总线驱动 — 复位脉冲 + 40bit 读取 + 校验
       90 +│   ├── bsp_dht11.h                  # DHT11 数据结构体 + 引脚宏
       91 +│   ├── bsp_adc.c                    # ADC1 三通道 DMA 扫描 — 火焰/MQ-2/MQ-135 共用
       92 +│   ├── bsp_adc.h                    # ADC 引脚宏 + 通道号 + ADCx_PPM() 声明
       93 +│   ├── bsp_key.c                    # 按键状态机 — 5状态检测单击/双击/长按（基于 1ms tick）
       94 +│   ├── bsp_key.h                    # KeyEvent 枚举 + Key_GetEvent() API
       95 +│   ├── bsp_led.c                    # LED GPIO 初始化
       96 +│   ├── bsp_led.h                    # LED_ON / LED_OFF 宏定义
       97 +│   ├── bsp_usart.c                  # USART1 (ESP8266) + USART3 (调试) — 初始化/发送/Printf
       98 +│   ├── bsp_usart.h                  # USART 宏 + USART_DEBUG 定义
       99 +│   ├── bsp_timer.c                  # TIM2 初始化 + 1ms 中断 + GetTick_ms() 全局时钟
      100 +│   ├── bsp_timer.h                  # TIM2 宏 + GetTick_ms() 声明
      101 +│   ├── bsp_flame.c / .h             # （预留）火焰传感器独立读取 — 实际未使用
      102 +│   ├── bsp_mq2.c / .h               # （预留）MQ-2 独立读取 — 实际未使用
      103 +│   ├── bsp_mq135.c / .h             # （预留）MQ-135 独立读取 — 实际未使用
      104 +│   ├── bsp_steppermotor.c / .h      # 步进电机驱动 — 4拍/8拍/正反转/角度控制
      105 +│   ├── relay.c / .h                 # 继电器初始化 + RELAY_ON/OFF 宏
      106  │
       58 -├── WIFI/                       # 网络通信层
       59 -│   ├── esp8266.c / .h          # ESP8266 AT 指令驱动
       60 -│   ├── onenet.c / .h           # OneNET 云平台适配（设备登录/订阅/发布/命令处理）
       61 -│   ├── MqttKit.c / .h          # MQTT 3.1.1 协议编解码
       62 -│   ├── cJSON.c / .h            # JSON 解析库
       63 -│   └── Common.h                # MQTT 通用类型定义
      107 +├── SYSTEM/                          # 系统组件层
      108 +│   ├── bsp_delay.c                  # SysTick 阻塞延时 — DelayUs() / DelayMs()
      109 +│   ├── bsp_delay.h                  # 延时函数声明
      110 +│   ├── bsp_filter.c                 # 一阶互补滤波器 — 低通+高通混合
      111 +│   ├── bsp_filter.h                 # Filter 结构体 + Filter_Init() / FilterValue()
      112 +│   └── sys.h                        # STM32 类型别名（u8=uint8_t, u16=uint16_t, u32=uint32_t）
      113  │
       65 -├── Library/                    # STM32 标准外设库（V3.5.0）
       66 -├── Startup/                    # 启动文件 + CMSIS Core
       67 -├── DebugConfig/                # 调试配置
       68 -├── build/                      # 编译输出
       69 -├── Listings/                   # .map / .lst
       70 -└── Objects/                    # .o / .axf / .hex
      114 +├── WIFI/                            # 网络通信层
      115 +│   ├── esp8266.c                    # ESP8266 AT 指令驱动 — 初始化/WiFi连接/TCP透传/接收解析
      116 +│   ├── esp8266.h                    # ESP8266 API 声明 + REV_OK/REV_WAIT 宏
      117 +│   ├── onenet.c                     # OneNET 平台适配 — DevLink登录/Subscribe订阅/Publish发布/RevPro
          +接收
      118 +│   ├── onenet.h                     # OneNET API 声明
      119 +│   ├── MqttKit.c                    # MQTT 3.1.1 协议栈 — CONNECT/SUBSCRIBE/PUBLISH 组包解包 + 内存管
          +理
      120 +│   ├── MqttKit.h                    # MQTT 包类型枚举/QoS等级/标志位 + 全部 API 声明
      121 +│   ├── cJSON.c                      # JSON 解析库（cJSON 标准实现）
      122 +│   ├── cJSON.h                      # cJSON API 声明
      123 +│   └── Common.h                     # MQTT 层通用类型（uint1, uint8, int16, size_t 等）
      124 +│
      125 +├── Library/                         # STM32F10x 标准外设库 V3.5.0
      126 +│   ├── stm32f10x_gpio.c / .h        # GPIO 外设驱动
      127 +│   ├── stm32f10x_rcc.c / .h         # 时钟系统驱动
      128 +│   ├── stm32f10x_usart.c / .h       # 串口外设驱动
      129 +│   ├── stm32f10x_adc.c / .h         # ADC 外设驱动
      130 +│   ├── stm32f10x_dma.c / .h         # DMA 外设驱动
      131 +│   ├── stm32f10x_tim.c / .h         # 定时器外设驱动
      132 +│   ├── stm32f10x_spi.c / .h         # SPI 外设驱动
      133 +│   ├── stm32f10x_i2c.c / .h         # I2C 外设驱动
      134 +│   ├── stm32f10x_exti.c / .h        # 外部中断驱动
      135 +│   ├── stm32f10x_flash.c / .h       # Flash 驱动
      136 +│   ├── misc.c / .h                  # NVIC 中断优先级管理
      137 +│   └── ...（其余外设驱动）
      138 +│
      139 +├── Startup/                         # 启动层
      140 +│   ├── startup_stm32f10x_md.s       # 启动汇编（向量表 + Reset_Handler）
      141 +│   ├── system_stm32f10x.c           # 系统时钟初始化 SystemInit()
      142 +│   ├── system_stm32f10x.h           # 系统时钟声明
      143 +│   ├── stm32f10x.h                  # 芯片顶层头文件（寄存器定义）
      144 +│   └── core_cm3.c / .h              # CMSIS Core（NVIC/SysTick 寄存器）
      145 +│
      146 +├── DebugConfig/                     # Keil 调试器配置
      147 +│   └── Target_1_STM32F103C8_1.0.0.dbgconf
      148 +│
      149 +├── .cmsis/                          # CMSIS Pack（ARM Cortex-A 支持文件，本项目未使用）
      150 +├── .eide/                           # EIDE 插件配置
      151 +├── .vscode/                         # VS Code 配置
      152 +├── build/                           # 编译中间文件
      153 +├── Listings/                        # .map / .lst 链接输出
      154 +├── Objects/                         # .o / .axf / .hex 目标文件
      155 +│
      156 +├── project.uvprojx                  # Keil MDK 工程文件
      157 +├── project.uvoptx                   # Keil 工程选项
      158 +├── project.uvguix.xf                # Keil 用户布局
      159 +├── project.code-workspace           # VS Code 工作区
      160 +├── .clang-format                    # 代码格式化规则
      161 +├── .gitignore                       # Git 忽略规则
      162 +└── README.md                        # 本文件
      163  ```
      164
      165 +---
      166 +
      167 +## 模块详解
      168 +
      169 +### 1. User/main.c — 程序入口
      170 +
      171 +```c
      172 +#include "app_init.h"
      173 +int main(void)
      174 +{
      175 +    App_Init();     // 永不返回，内部包含 while(1) 主循环
      176 +}
      177 +```
      178 +
      179 +极简入口。`App_Init()` 完成所有初始化并进入无限循环。
      180 +
      181 +### 2. APP/app_init.c — 核心应用（~240 行）
      182 +
      183 +**全局状态变量：**
      184 +
      185 +| 变量 | 类型 | 默认值 | 说明 |
      186 +|------|------|--------|------|
      187 +| `currentState` | `DisplayState` 枚举 | `MAIN_MENU` | 当前 OLED 页面（0~5） |
      188 +| `Temp_Thr` | `uint16_t` | 30 | 温度报警阈值 (°C) |
      189 +| `Humi_Thr` | `uint16_t` | 60 | 湿度报警阈值 (%) |
      190 +| `Flame_Thr` | `uint16_t` | 6000 | 火焰 PPM 阈值 |
      191 +| `Mq2_Thr` | `uint16_t` | 6000 | MQ-2 PPM 阈值 |
      192 +| `Mq135_Thr` | `uint16_t` | 6000 | MQ-135 PPM 阈值 |
      193 +| `Alarm_flag` | `uint8_t` | 0 | 报警总开关（云端下发控制） |
      194 +| `TimeCount` | `uint16_t` | 0 | 上传周期计数器（每 100 次 main 循环触发一次上传） |
      195 +| `key_value` | `uint8_t` | 0 | 按键扫描返回值（已弃用，改为 Key_GetEvent 事件驱动） |
      196 +
      197 +**函数职责：**
      198 +
      199 +| 函数 | 调用频率 | 职责 |
      200 +|------|---------|------|
      201 +| `OLED_Show()` | 每循环（MAIN_MENU 页） | 读取 DHT11 + ADC 三通道，显示主菜单 |
      202 +| `OLED_Show1~5()` | 每循环（对应设置页） | 显示阈值设置页面 |
      203 +| `Threshold_Adjust()` | 每循环（设置页内调用） | 处理 KEY1/KEY2 双击±1 和长按快调 |
      204 +| `OLED_Switch()` | 每循环 | 处理单击翻页 + 根据 currentState 分发显示 |
      205 +| `Alarm_Statue()` | 每循环 | 比较当前温湿度与阈值，控制蜂鸣器和 LED |
      206 +| `JsonValue()` | 每 100 循环 | 构造上传 JSON 字符串 |
      207 +| `App_Init()` | 仅启动一次 | 全部初始化 → 连云 → 进入主循环 |
      208 +
      209 +### 3. BSP/bsp_oled.c — OLED 驱动（~1500 行）
      210 +
      211 +这是项目最复杂的单文件模块，来自江协科技 V2.0 版本。
      212 +
      213 +**驱动架构：**
      214 +
      215 +```
      216 +应用层调用:
      217 +  OLED_ShowString() / OLED_Printf() / OLED_ShowNum()
      218 +  OLED_ShowImage() / OLED_DrawLine() / OLED_DrawCircle() ...
      219 +       │
      220 +       ▼ 操作显存数组
      221 +  OLED_DisplayBuf[8][128]           ← 128×64 像素缓冲区（8页 × 128列）
      222 +       │
      223 +       ▼ OLED_Update() 刷屏
      224 +  OLED_WriteData() → OLED_I2C_SendByte() → OLED_W_SDA() / OLED_W_SCL()
      225 +       │
      226 +       ▼ 软件模拟 I2C
      227 +  GPIOB PB6(SCL) / PB7(SDA) 开漏输出
      228 +       │
      229 +       ▼ I2C 总线
      230 +  SSD1306 OLED 控制器
      231 +```
      232 +
      233 +**I2C 软件模拟：**
      234 +- 从机地址: `0x78`（SA0 接地）
      235 +- 写命令: 控制字节 `0x00` → 命令字节
      236 +- 写数据: 控制字节 `0x40` → 数据字节流（最多 128 字节连续写入）
      237 +- 应答位: 不检测（仅发送额外时钟）
      238 +
      239 +**显存组织：**
      240 +- 纵向 8 点为一页（Page 0~7），横向 128 列
      241 +- `OLED_DisplayBuf[page][column]` = 8 个纵向像素的位图（bit0=最上，bit7=最下）
      242 +- 所有显示函数操作显存，最后调用 `OLED_Update()` 一次性刷屏
      243 +
      244 +**图形能力：**
      245 +- ASCII 字符显示（6×8 / 8×16 字体）
      246 +- 中文显示（16×16 点阵，UTF-8 / GB2312 双编码自动识别）
      247 +- 格式化输出 `OLED_Printf()`（内部先 vsprintf 到 256 字节缓冲，再逐字符显示）
      248 +- 基本图元: 点、直线（Bresenham）、矩形、三角形（pnpoly 填充）、圆、椭圆、圆弧/扇形
      249 +- 纯软件渲染，无硬件加速
      250 +
      251 +### 4. BSP/bsp_dht11.c — DHT11 驱动
      252 +
      253 +**单总线协议时序：**
      254 +
      255 +```
      256 +主机发送起始信号:
      257 +  DHT11_Dout_0    拉低 20ms
      258 +  DHT11_Dout_1    拉高 13us
      259 +  切换为输入模式  等待 DHT11 响应
      260 +
      261 +DHT11 响应:
      262 +  拉低 80us → 拉高 80us → 开始发送 40bit 数据
      263 +
      264 +40bit 数据格式:
      265 +  [湿度整数 8bit] [湿度小数 8bit] [温度整数 8bit] [温度小数 8bit] [校验和 8bit]
      266 +  校验和 = 前 4 字节之和的低 8 位
      267 +
      268 +每 bit 编码:
      269 +  起始: 50us 低电平
      270 +  数据 0: 26~28us 高电平
      271 +  数据 1: 70us 高电平
      272 +  读取方式: 等 40us 后采样 GPIO 电平（0 已结束，1 仍在持续）
      273 +```
      274 +
      275 +**读取函数 `DHT11_Read_TempAndHumidity()` 返回值：**
      276 +- `SUCCESS (1)`: 数据有效，校验通过
      277 +- `ERROR (0)`: 通信失败或校验错误
      278 +
      279 +### 5. BSP/bsp_adc.c — ADC + DMA 三通道采集
      280 +
      281 +**硬件配置：**
      282 +
      283 +| 参数 | 值 | 说明 |
      284 +|------|-----|------|
      285 +| ADC 外设 | ADC1 | 独立模式 |
      286 +| 时钟 | PCLK2 ÷ 6 = 12MHz | `RCC_PCLK2_Div6` |
      287 +| 采样时间 | 55.5 Cycles | 每个通道约 4.6us |
      288 +| 扫描模式 | 使能 | 自动扫描规则组序列 |
      289 +| 连续转换 | 使能 | 每轮结束后自动开始下一轮 |
      290 +| 数据对齐 | 右对齐 | 12bit → [0, 4095] |
      291 +| DMA 通道 | DMA1_Channel1 | 外设→内存，半字宽度，循环模式 |
      292 +| 触发方式 | 软件触发一次 → 连续运行 | `ADC_SoftwareStartConvCmd` |
      293 +
      294 +**通道映射：**
      295 +
      296 +| 规则组序列 | ADC 通道 | 引脚 | 传感器 | DMA 目标 |
      297 +|-----------|---------|------|--------|---------|
      298 +| 序列 1 | CH1 | PA1 | MQ-2 | `ADCx_Value[0]` |
      299 +| 序列 2 | CH2 | PA2 | 火焰 | `ADCx_Value[1]` |
      300 +| 序列 3 | CH3 | PA3 | MQ-135 | `ADCx_Value[2]` |
      301 +
      302 +**PPM 计算公式**（`ADCx_PPM(channel)`）：
      303 +
      304 +```
      305 +voltage = ADC_Value × 5.0 / 4096
      306 +RS      = (5.0 - voltage) / (voltage × 0.5)    // 传感器电阻
      307 +R0      = 6.64                                  // 标定电阻 (kΩ)
      308 +ratio   = RS / R0
      309 +PPM     = FilterValue( 11.5428 × ratio^(-0.6549) )  // 幂函数拟合 + 一阶互补滤波
      310 +```
      311 +
      312 +**滤波器**（`bsp_filter.c`，每通道独立静态实例）：
      313 +```
      314 +out(t) = α × LowPass(t) + (1-α) × HighPass(t)
      315 +LowPass(t)  = α × in(t) + (1-α) × out(t-1)
      316 +HighPass(t) = α × out(t-1) + α × (in(t) - in(t-1))
      317 +α = 0.2
      318 +```
      319 +一次调用同时做低通+高通，互补混合输出，兼顾稳定性和响应速度。
      320 +
      321 +### 6. BSP/bsp_key.c — 按键状态机
      322 +
      323 +**5 状态 FSM：**
      324 +
      325 +```
      326 +                    ┌──────────────────────────────────────────┐
      327 +                    │                                          │
      328 +       edge_down    ▼    debounce_ok       edge_up + long      │
      329 + IDLE ──────────→ DEBOUNCE_PRESS ────────→ PRESSED ───────────┤
      330 +   ▲               │        ▲               │    │             │
      331 +   │    timeout    │  jitter│               │    │ time>=600ms │
      332 +   │    (350ms)    │  (不满足│消抖时间)       │    ▼             │
      333 +   │               ▼        │               │  return LONG_PRESS
      334 +   └──── WAIT_DOUBLE ───────┘               │    │ (仅首次)
      335 +        (debounce_ok)                        │    │ time>=680ms
      336 +          │                                  │    ▼
      337 +          │ edge_down                        │  return LONG_HOLD
      338 +          │ (第二次按下)                        │    │ (每80ms连发)
      339 +          └──────────────────────────────────┘    │
      340 +                                                  │ edge_up (非长按)
      341 +                                                  ▼
      342 +                                            DEBOUNCE_RELEASE
      343 +                                                  │
      344 +                                    debounce_ok   │
      345 +                          ┌───────────────────────┤
      346 +                          ▼                       ▼
      347 +                    from_double=1?          from_double=0?
      348 +                    是 → IDLE                否 → WAIT_DOUBLE
      349 +                    返回 DOUBLE_CLICK             │
      350 +                                                  │ 等 350ms
      351 +                                                  ▼
      352 +                                                IDLE
      353 +                                                返回 CLICK
      354 +```
      355 +
      356 +**时间参数**（`bsp_key.c` 中定义）：
      357 +
      358 +| 参数 | 值 | 说明 |
      359 +|------|-----|------|
      360 +| `KEY_DEBOUNCE_MS` | 20ms | 按下/释放消抖时间 |
      361 +| `KEY_LONG_PRESS_MS` | 600ms | 长按触发阈值 |
      362 +| `KEY_DOUBLE_CLICK_MS` | 350ms | 两次单击最大间隔 |
      363 +| `KEY_LONG_REPEAT_MS` | 80ms | 长按保持连发间隔 |
      364 +
      365 +**API：**
      366 +```c
      367 +KeyEvent Key_GetEvent(uint8_t key_id);
      368 +// key_id: KEY_ID_1 (PB12) 或 KEY_ID_2 (PB13)
      369 +// 返回: KEY_EVENT_NONE / CLICK / DOUBLE_CLICK / LONG_PRESS / LONG_HOLD
      370 +// 注意: 每次调用返回的事件仅有效一次（消费型），不缓存
      371 +```
      372 +
      373 +**时间基准：**
      374 +- 依赖 `bsp_timer.c` 提供的 `GetTick_ms()` 全局毫秒计数器
      375 +- TIM2 配置: PCLK1=36MHz, PSC=71, Period=999 → 1ms 中断周期
      376 +
      377 +### 7. WIFI/esp8266.c — ESP8266 AT 驱动
      378 +
      379 +**初始化流程**（`ESP8266_Init()`，每步失败无限重试 + 500ms 延迟）：
      380 +
      381 +```
      382 +1. AT                    → 等待 "OK"          测试模块
      383 +2. AT+CWMODE=1           → 等待 "OK"          设为 Station 模式
      384 +3. AT+CWDHCP=1,1         → 等待 "OK"          开启 DHCP
      385 +4. AT+CWJAP="SSID","PWD" → 等待 "GOT IP"      连接 WiFi 路由器
      386 +5. AT+CIPSTART="TCP",    → 等待 "CONNECT"     建立 TCP 连接到
      387 +   "mqtts.heclouds.com",1883                    OneNET MQTT Broker
      388 +```
      389 +
      390 +**接收机制：**
      391 +
      392 +```
      393 +USART1 RX 中断 (USART1_IRQHandler)
      394 +  每收到 1 字节 → esp8266_buf[esp8266_cnt++] = USART1->DR
      395 +  缓冲区大小: 512 字节，溢出回绕 (cnt 归零)
      396 +
      397 +ESP8266_WaitRecive()
      398 +  在当前和上一次 cnt 相同时判断接收完毕（简单停顿检测）
      399 +  问题: 网络延迟大时可能误判——改进方向: 用帧头尾匹配或超时机制
      400 +
      401 +ESP8266_SendCmd(cmd, expected_response)
      402 +  1. Usart_SendString(USART1, cmd, len)  发送 AT 命令
      403 +  2. 循环 200 次 × 10ms = 最多等待 2 秒
      404 +  3. 每次调用 ESP8266_WaitRecive() 检查是否收到数据
      405 +  4. strstr(buf, expected_response) 匹配期望响应
      406 +  5. 匹配成功 → 返回 0；超时 → 返回 1
      407 +
      408 +ESP8266_SendData(data, len)
      409 +  1. 发送 "AT+CIPSEND=<len>\r\n"
      410 +  2. 等待 '>' 提示符
      411 +  3. 直接发送 len 字节原始数据（透传）
      412 +```
      413 +
      414 +**接收云平台下行数据**（`ESP8266_GetIPD(timeout)`）：
      415 +```
      416 +1. 等待接收完成（每次 5ms × timeout 次 = 最多 10ms timeout）
      417 +2. 在 esp8266_buf 中搜索 "IPD," 字符串
      418 +3. 找到后定位 ':' 分隔符
      419 +4. 返回 ':' 后第一个字节的指针 → 即 MQTT 原始报文
      420 +5. 超时 → 返回 NULL
      421 +```
      422 +
      423 +### 8. WIFI/MqttKit.c — MQTT 协议栈
      424 +
      425 +**支持的 MQTT 包类型：**
      426 +
      427 +| 枚举值 | 方向 | 说明 |
      428 +|--------|------|------|
      429 +| `MQTT_PKT_CONNECT (1)` | 上传 | 连接请求（含产品ID、Token、设备名） |
      430 +| `MQTT_PKT_CONNACK (2)` | 下行 | 连接确认（含返回码 0~5） |
      431 +| `MQTT_PKT_PUBLISH (3)` | 双向 | 数据发布 |
      432 +| `MQTT_PKT_PUBACK (4)` | 下行 | QoS 1 发布确认 |
      433 +| `MQTT_PKT_SUBSCRIBE (8)` | 上传 | 主题订阅 |
      434 +| `MQTT_PKT_SUBACK (9)` | 下行 | 订阅确认 |
      435 +| `MQTT_PKT_PINGREQ (12)` | 上传 | 心跳请求（本项目未启用） |
      436 +| `MQTT_PKT_DISCONNECT (14)` | 上传 | 断开连接 |
      437 +| `MQTT_PKT_CMD` | 下行 | 自定义：识别 `$creq` 前缀的命令下发 |
      438 +
      439 +**内存管理：**
      440 +```c
      441 +// Common.h 宏定义
      442 +#define MQTT_MallocBuffer  malloc    // 使用 C 标准库堆分配
      443 +#define MQTT_FreeBuffer    free
      444 +
      445 +// MQTT_PACKET_STRUCTURE 结构体
      446 +typedef struct Buffer {
      447 +    uint8  *_data;      // 数据指针
      448 +    uint32 _len;        // 当前数据长度
      449 +    uint32 _size;       // 总缓冲区大小
      450 +    uint8  _memFlag;    // MEM_FLAG_ALLOC (malloc) / MEM_FLAG_STATIC (固定数组)
      451 +} MQTT_PACKET_STRUCTURE;
      452 +```
      453 +
      454 +**CONNECT 包结构**（`MQTT_PacketConnect()`）：
      455 +```
      456 +固定头:   [1byte 类型=0x10] [1~4byte 剩余长度]
      457 +可变头:   [协议名 "MQTT"] [协议级别 4] [连接标志] [KeepAlive 256s]
      458 +Payload:  [ClientID "test"] [Username "XPz90SBcDh"] [Password Token]
      459 +```
      460 +
      461 +**PUBLISH 包结构**（`MQTT_PacketPublish()`）：
      462 +```
      463 +固定头:   [1byte 类型=0x30 | QoS | Retain] [1~4byte 剩余长度]
      464 +可变头:   [2byte Topic长度] [Topic字符串] [2byte PacketID(仅QoS>0)]
      465 +Payload:  [消息体字节流]
      466 +```
      467 +
      468 +### 9. WIFI/onenet.c — OneNET 平台适配
      469 +
      470 +**设备凭证（硬编码在源码中）：**
      471 +
      472 +| 参数 | 值 |
      473 +|------|-----|
      474 +| 产品 ID (PROID) | `XPz90SBcDh` |
      475 +| 设备名称 (DEVID) | `test` |
      476 +| 鉴权 Token | MD5 签名: `version=2018-10-31&res=products/XPz90SBcDh/devices/test&et=1872683333&method
          +=md5&sign=jNVeaJ6scz/SiFiACVDDoA==` |
      477 +
      478 +**上行 Topic（设备发布数据）：**
      479 +```
      480 +$sys/XPz90SBcDh/test/thing/property/post
      481 +```
      482 +
      483 +**下行 Topic（设备订阅，接收云平台下发指令）：**
      484 +```
      485 +$sys/XPz90SBcDh/test/thing/property/set
      486 +```
      487 +
      488 +**`OneNet_RevPro(cmd)` 指令处理流程：**
      489 +
      490 +```
      491 +MQTT_UnPacketRecv(cmd) 识别包类型
      492 +  │
      493 +  ├─ MQTT_PKT_CMD (命令下发)
      494 +  │    ├─ MQTT_UnPacketCmd() 解出 cmdid + req_body
      495 +  │    └─ MQTT_PacketCmdResp() 组包回复 $crsp/cmdid
      496 +  │
      497 +  ├─ MQTT_PKT_PUBLISH (属性下发)
      498 +  │    ├─ MQTT_UnPacketPublish() 解出 topic + payload
      499 +  │    ├─ cJSON_Parse(payload) 解析 JSON
      500 +  │    ├─ 提取 "params" → "LED" 字段
      501 +  │    │    ├─ type == cJSON_True  → LED_ON
      502 +  │    │    └─ type == cJSON_False → LED_OFF
      503 +  │    ├─ 提取 "params" → "Alarm" 字段
      504 +  │    │    ├─ type == cJSON_True  → Alarm_flag = 1
      505 +  │    │    └─ type == cJSON_False → Alarm_flag = 0
      506 +  │    └─ cJSON_Delete(json) 释放内存
      507 +  │
      508 +  ├─ MQTT_PKT_SUBACK → 打印 "Subscribe OK"
      509 +  ├─ MQTT_PKT_PUBACK → 打印 "Publish Send OK"
      510 +  └─ ...
      511 +```
      512 +
      513 +---
      514 +
      515  ## 软件架构
      516
      517 +### 初始化流程
      518 +
      519  ```
      520  main()
      521   └── App_Init()
       78 -      ├── Delay_Init()          SysTick 延时初始化
       79 -      ├── GENERAL_TIM_Init()    TIM2 1ms 时基启动
       80 -      ├── OLED_Init()           OLED 初始化 + 清屏
       81 -      ├── DHT11_Init()          DHT11 GPIO 配置
       82 -      ├── Key_Init()            按键 GPIO 配置
       83 -      ├── LED_Init()            LED GPIO 配置
       84 -      ├── ADCx_Init()           ADC1+DMA 三通道连续扫描
       85 -      ├── Alarm_Init()          蜂鸣器 GPIO 配置
       86 -      ├── ESP8266_Init()        ESP8266 AT 初始化 → WiFi → TCP 连云
       87 -      ├── OneNet_DevLink()      MQTT CONNECT 鉴权登录
       88 -      ├── OneNet_Subscribe()    订阅下行控制主题
       89 -      └── while(1)              主循环
       90 -           ├── OLED_Switch()    按键处理 + 界面显示
       91 -           ├── Alarm_Statue()   报警判断
       92 -           └── 数据定时上传     OneNet_Publish() 每 100 轮
      522 +      │
      523 +      ├── Delay_Init()               SysTick 时钟源设为 72MHz÷8=9MHz
      524 +      │                               UsCount=9, MsCount=9000（用于阻塞延时）
      525 +      │
      526 +      ├── GENERAL_TIM_Init()         TIM2 使能，PSC=71, Period=999
      527 +      │                               → 1ms 中断 → tick_ms++
      528 +      │                               GetTick_ms() 供按键状态机使用
      529 +      │
      530 +      ├── OLED_Init()                OLED I2C 引脚初始化 + 30+ 条配置命令
      531 +      │                               → OLED_Clear() → OLED_Update()
      532 +      │
      533 +      ├── DHT11_Init()               DHT11 数据线 PB5 配置为推挽输出，初始拉高
      534 +      │
      535 +      ├── Key_Init()                 KEY1(PB12) + KEY2(PB13) 上拉输入
      536 +      │
      537 +      ├── LED_Init()                 LED(PC13) 推挽输出，初始关
      538 +      │
      539 +      ├── ADCx_Init()                ADC1 三通道 DMA 扫描启动
      540 +      │                               → 连续转换，DMA 循环填充 ADCx_Value[3]
      541 +      │
      542 +      ├── Alarm_Init()               蜂鸣器(PA11) 推挽输出，初始关
      543 +      │
      544 +      ├── ESP8266_Init()             ★ 6步 AT 命令序列 ★
      545 +      │    ├── AT                    → 测试
      546 +      │    ├── AT+CWMODE=1           → Station 模式
      547 +      │    ├── AT+CWDHCP=1,1         → DHCP
      548 +      │    ├── AT+CWJAP="SSID","PWD"  → 连接 WiFi（直到获取 IP）
      549 +      │    └── AT+CIPSTART="TCP","mqtts.heclouds.com",1883 → TCP 连云
      550 +      │
      551 +      ├── OLED_Printf("网络连接中...") → OLED_Update()
      552 +      │
      553 +      ├── while(OneNet_DevLink())    MQTT CONNECT 鉴权登录
      554 +      │    └── DelayMs(500)          失败重试
      555 +      │
      556 +      ├── OLED 显示"连接成功"         持续 3 秒后清屏
      557 +      │
      558 +      ├── OneNet_Subscribe(topics,1)  订阅下行控制主题
      559 +      │
      560 +      └── while(1)  ★ 主循环 ★
      561 +           │
      562 +           ├── OLED_Switch()          按键事件处理 + 当前页渲染
      563 +           │    ├── Key_GetEvent(KEY_ID_1)
      564 +           │    ├── Key_GetEvent(KEY_ID_2)
      565 +           │    ├── 单击 → 翻页
      566 +           │    └── switch(currentState) → OLED_Show / Show1~5
      567 +           │         └── Threshold_Adjust() 处理阈值修改（双击/长按）
      568 +           │
      569 +           ├── Alarm_Statue()         报警判断
      570 +           │    └── DHT11_Data.temp_int > Temp_Thr
      571 +           │        || DHT11_Data.humi_int > Humi_Thr
      572 +           │        → Alarm_flag==1 ? ALARM_ON : ALARM_OFF
      573 +           │
      574 +           ├── 每100轮:
      575 +           │    ├── JsonValue()       构造上传 JSON
      576 +           │    ├── OneNet_Publish()  发布到云平台
      577 +           │    ├── ESP8266_Clear()   清接收缓冲
      578 +           │    └── TimeCount = 0
      579 +           │
      580 +           └── ESP8266_GetIPD(2)     轮询下行数据（10ms 超时）
      581 +                └── OneNet_RevPro()   解析并执行云平台指令
      582  ```
      583
       95 -## OLED 界面 & 按键操作
      584 +### 主循环时序（估算）
      585
       97 -系统共 6 个页面，按键操作方式：
      586 +```
      587 +单次循环耗时 ≈ OLED_Switch() ≈ OLED 渲染 + I2C 传输 + 传感器读取
      588 +             ≈ ADCx_PPM × 3 (滤波+幂运算) + DHT11 × 1 (单总线20ms+) + OLED_Update (I2C 128×8字节)
      589 +             ≈ 50~150ms（取决于 DHT11 通信耗时和浮点运算）
      590
       99 -| 操作 | KEY1（PB12） | KEY2（PB13） |
      100 -|------|-------------|-------------|
      101 -| **单击** | 上翻页 | 下翻页 |
      102 -| **双击** | 当前阈值 +1 | 当前阈值 -1 |
      103 -| **长按** | 快速增加阈值 | 快速减小阈值 |
      591 +上传周期 ≈ 100 次循环 ≈ 5~15 秒
      592 +```
      593
      105 -| 页面 | 显示内容 | 可调阈值 |
      106 -|------|---------|---------|
      107 -| 主菜单 | 温湿度 + 火焰/MQ-2/MQ-135 实时读数 | — |
      108 -| 温度阈值 | 当前温度阈值 | 0 ~ 100，步进 1 / 快进 5 |
      109 -| 湿度阈值 | 当前湿度阈值 | 0 ~ 100，步进 1 / 快进 5 |
      110 -| 火焰阈值 | 火焰传感器 PPM + 阈值 | 0 ~ 10000，步进 1 / 快进 500 |
      111 -| MQ-2 阈值 | MQ-2 传感器 PPM + 阈值 | 0 ~ 10000，步进 1 / 快进 500 |
      112 -| MQ-135 阈值 | MQ-135 传感器 PPM + 阈值 | 0 ~ 10000，步进 1 / 快进 500 |
      594 +### 依赖关系图（#include 树）
      595
      596 +```
      597 +main.c
      598 + └── app_init.h
      599 +      ├── stm32f10x.h          (芯片寄存器定义)
      600 +      ├── bsp_delay.h          (DelayUs / DelayMs)
      601 +      │    └── stm32f10x.h
      602 +      ├── bsp_timer.h          (GENERAL_TIM_Init / GetTick_ms)
      603 +      │    └── stm32f10x.h
      604 +      ├── bsp_key.h            (Key_Init / Key_GetEvent / KeyEvent)
      605 +      │    ├── stm32f10x.h
      606 +      │    └── bsp_timer.h
      607 +      ├── bsp_oled.h           (OLED_Init / OLED_Printf / OLED_Update / ...)
      608 +      │    ├── bsp_oledfont.h  (字模数据)
      609 +      │    └── stm32f10x.h
      610 +      ├── bsp_dht11.h          (DHT11_Init / DHT11_Read_TempAndHumidity)
      611 +      │    └── stm32f10x.h
      612 +      ├── bsp_led.h            (LED_Init / LED_ON / LED_OFF)
      613 +      │    └── stm32f10x.h
      614 +      ├── bsp_adc.h            (ADCx_Init / ADCx_PPM)
      615 +      │    └── stm32f10x.h
      616 +      ├── app_alarm.h          (Alarm_Init / ALARM_ON / ALARM_OFF)
      617 +      │    └── stm32f10x.h
      618 +      ├── esp8266.h            (ESP8266_Init / ESP8266_SendData / ESP8266_GetIPD)
      619 +      │    └── stm32f10x.h
      620 +      └── onenet.h             (OneNet_DevLink / OneNet_Subscribe / OneNet_Publish / OneNet_RevPro)
      621 +           └── stm32f10x.h
      622 +```
      623 +
      624 +---
      625 +
      626 +## 界面与按键
      627 +
      628 +### 页面流转
      629 +
      630 +```
      631 +     单击 KEY1 (→)                    单击 KEY2 (←)
      632 +     ───────────                      ───────────
      633 +
      634 +  MAIN_MENU ──→ TEMP_SET ──→ HUMI_SET ──→ FLAME_SET ──→ MQ2_SET ──→ MQ135_SET
      635 +      ↑                                                                    │
      636 +      └────────────────────────────────────────────────────────────────────┘
      637 +       ←────────────────────────────────────────────────────────────────────
      638 +                                    单击 KEY2 (←)
      639 +```
      640 +
      641 +### 主菜单 (MAIN_MENU)
      642 +
      643 +显示内容（128×64 OLED，坐标原点左上角）：
      644 +
      645 +```
      646 +  (24,0)  "Smart Kitchen"         6×8 字体
      647 +  (0,10)  "Temp:25.3"             6×8 字体
      648 +  (0,20)  "Humi:60%"              6×8 字体
      649 +  (0,30)  "Flame_ppm:1234.56"     6×8 字体
      650 +  (0,40)  "Mq2_ppm:2345.67"       6×8 字体
      651 +  (0,50)  "Mq135_ppm:3456.78"     6×8 字体
      652 +```
      653 +
      654 +### 阈值设置页面 (TEMP_SET ~ MQ135_SET)
      655 +
      656 +显示内容（以 TEMP_SET 为例）：
      657 +
      658 +```
      659 +  (30,0)  "温度阈值"               8×16 字体（中文）
      660 +  (30,4)  "Temp:30"                8×16 字体（第 4 页 ≈ Y=32）
      661 +```
      662 +
      663 +操作效果：
      664 +- **双击 KEY1**: Temp_Thr 从 30 → 31
      665 +- **双击 KEY2**: Temp_Thr 从 31 → 30
      666 +- **长按 KEY1**: Temp_Thr 每 80ms +5，快速递增
      667 +- **长按 KEY2**: Temp_Thr 每 80ms -5，快速递减
      668 +
      669 +各设置页参数：
      670 +
      671 +| 页面 | 阈值变量 | 范围 | 双击步进 | 快进步进 |
      672 +|------|---------|------|---------|---------|
      673 +| TEMP_SET | `Temp_Thr` | 0~100 | ±1 | ±5 |
      674 +| HUMI_SET | `Humi_Thr` | 0~100 | ±1 | ±5 |
      675 +| FLAME_SET | `Flame_Thr` | 0~10000 | ±1 | ±500 |
      676 +| MQ2_SET | `Mq2_Thr` | 0~10000 | ±1 | ±500 |
      677 +| MQ135_SET | `Mq135_Thr` | 0~10000 | ±1 | ±500 |
      678 +
      679 +---
      680 +
      681 +## 传感器数据处理
      682 +
      683 +### DHT11 温湿度
      684 +
      685 +**读取触发：** 每次 `OLED_Show()` 调用时（即每次进入主菜单页面）
      686 +**数据流：**
      687 +```
      688 +DHT11_Read_TempAndHumidity(&DHT11_Data)
      689 +  ├─ 发送起始脉冲（20ms 低 + 13us 高）
      690 +  ├─ 检测 DHT11 响应（80us 低 + 80us 高）
      691 +  ├─ 读取 5 字节（40bit）
      692 +  └─ 校验: check_sum == humi_int + humi_deci + temp_int + temp_deci ?
      693 +      ├─ 是 → 返回 SUCCESS，数据存入 DHT11_Data 全局结构体
      694 +      └─ 否 → 返回 ERROR，OLED 不显示温湿度行
      695 +```
      696 +
      697 +**注意：** DHT11 的读取是阻塞的，单次通信耗时 ~25ms。如果传感器不存在或通信失败，每次尝试都会耗时约 20
          +ms（起始脉冲最低电平时间）。
      698 +
      699 +### ADC 三通道 PPM
      700 +
      701 +**触发方式：** ADC1 配置为连续扫描模式 + DMA 循环传输，**启动后无需软件干预，自动持续采集**。
      702 +
      703 +**数据流：**
      704 +```
      705 +ADC1 硬件
      706 +  ├─ 序列1: CH1 (PA1) → DR → DMA → ADCx_Value[0]  (MQ-2)
      707 +  ├─ 序列2: CH2 (PA2) → DR → DMA → ADCx_Value[1]  (火焰)
      708 +  └─ 序列3: CH3 (PA3) → DR → DMA → ADCx_Value[2]  (MQ-135)
      709 +       │
      710 +       ▼ 每个通道约 4.6us × 3 = ~14us 完成一轮
      711 +       │ 连续循环（DMA 循环模式）
      712 +       │
      713 +  OLED_Show() / OLED_Show3/4/5 调用 ADCx_PPM(channel)
      714 +       │
      715 +       ├─ 读取 volatile ADCx_Value[channel]（最新 DMA 数据）
      716 +       ├─ voltage = ADC × 5.0 / 4096
      717 +       ├─ RS = (5.0 - V) / (V × 0.5)
      718 +       ├─ ratio = RS / 6.64
      719 +       ├─ raw_ppm = 11.5428 × ratio^(-0.6549)
      720 +       └─ FilterValue(&filter[channel], raw_ppm)  ← 一阶互补滤波
      721 +```
      722 +
      723 +**注意：** `ADCx_Value` 声明为 `volatile`，确保编译器每次从内存读取最新 DMA 值，而非寄存器缓存。三个通
          +道各自拥有独立的 `static Filter` 实例。
      724 +
      725 +---
      726 +
      727  ## 云平台通信
      728
      116 -### 连接参数
      729 +### 完整通信流程
      730
      118 -| 参数 | 值 |
      119 -|------|-----|
      120 -| 平台 | OneNET（中国移动物联网开放平台） |
      121 -| 协议 | MQTT 3.1.1 over TCP |
      122 -| 地址 | `mqtts.heclouds.com:1883` |
      123 -| 产品 ID | `XPz90SBcDh` |
      124 -| 设备名 | `test` |
      125 -| 鉴权 | Token（MD5 签名） |
      731 +#### 阶段一：ESP8266 初始化 & WiFi 连接
      732
      127 -### 数据流
      733 +```
      734 +ESP8266_Init()
      735 +  │
      736 +  ├─[AT] 测试模块存在 ──→ 收到 "OK" ✓
      737 +  ├─[AT+CWMODE=1] 设为 Station ──→ "OK" ✓
      738 +  ├─[AT+CWDHCP=1,1] 开启 DHCP ──→ "OK" ✓
      739 +  ├─[AT+CWJAP="SSID","PWD"] 连 WiFi ──→ "GOT IP" ✓ (最多等 10s)
      740 +  └─[AT+CIPSTART="TCP","mqtts.heclouds.com",1883] ──→ "CONNECT" ✓
      741 +```
      742
      129 -**上行**（设备 → 云，每约 1 秒自动上传）：
      130 -- Topic: `$sys/XPz90SBcDh/test/thing/property/post`
      131 -- 格式: `{"id":"123","params":{"Buzzer":{"value":false},"Temp":{"value":25},"LED":{"value":true},"Humi
          -":{"value":60}}}`
      743 +每步失败均无限重试 + 500ms 延迟。WiFi 连接步骤尤其耗时，视路由器响应速度需 3~15 秒。
      744
      133 -**下行**（云 → 设备，实时响应）：
      134 -- Topic: `$sys/XPz90SBcDh/test/thing/property/set`
      135 -- 支持指令:
      136 -  - `"LED": true/false` → 控制板载 LED 亮灭
      137 -  - `"Alarm": true/false` → 开关报警功能（蜂鸣器 + LED）
      745 +#### 阶段二：MQTT 登录鉴权
      746
      139 -### 通信协议栈
      747 +```
      748 +OneNet_DevLink()
      749 +  │
      750 +  ├─ MQTT_PacketConnect(PROID, TOKEN, DEVID, 256s KeepAlive, QoS0, ...)
      751 +  │    构建 CONNECT 报文（~120 字节）
      752 +  │
      753 +  ├─ ESP8266_SendData(packet, len)
      754 +  │    发送 AT+CIPSEND=120 → 等 '>' → 发送 120 字节 MQTT CONNECT
      755 +  │
      756 +  ├─ ESP8266_GetIPD(250)  等待服务器响应（超时 250×5ms=1.25s）
      757 +  │    收到 ESP8266 返回: "+IPD,4:<4字节 CONNACK>"
      758 +  │
      759 +  └─ MQTT_UnPacketConnectAck(dataPtr)
      760 +       ├─ 返回码 0 → 连接成功，status=0
      761 +       ├─ 返回码 1 → 协议错误
      762 +       ├─ 返回码 2 → ClientID 非法
      763 +       ├─ 返回码 3 → 服务器不可用
      764 +       ├─ 返回码 4 → 用户名或密码错误
      765 +       └─ 返回码 5 → 未授权（Token 非法）
      766 +```
      767
      768 +#### 阶段三：订阅下行主题
      769 +
      770  ```
      142 -OneNET 云平台
      143 -    ↑↓ MQTT (TCP)
      144 -ESP8266 WiFi 模块
      145 -    ↑↓ AT 指令 (USART1 115200bps)
      146 -esp8266.c (AT 指令驱动)
      147 -    ↑↓ 原始 MQTT 字节流
      148 -MqttKit.c (MQTT 编解码)
      149 -    ↑↓ MQTT 操作接口
      150 -onenet.c (OneNET 适配层)
      151 -    ↑↓ 应用调用
      152 -app_init.c (应用逻辑)
      771 +OneNet_Subscribe({"$sys/XPz90SBcDh/test/thing/property/set"}, 1)
      772 +  │
      773 +  ├─ MQTT_PacketSubscribe(20, QoS0, topics, 1, &pkt)
      774 +  ├─ ESP8266_SendData(pkt._data, pkt._len)
      775 +  └─ MQTT_DeleteBuffer(&pkt)
      776  ```
      777
      155 -## 报警逻辑
      778 +#### 阶段四：正常运行（主循环）
      779
      157 -当 `Alarm_flag = 1`（由云端下发）时：
      158 -- 温度超过阈值 **或** 湿度超过阈值 → 蜂鸣器响 + LED 亮
      159 -- 温度湿度均在阈值内 → 蜂鸣器关 + LED 灭
      780 +**上行（每约 5~15 秒）：**
      781
      161 -## 开发环境
      782 +```
      783 +TimeCount 累加到 100
      784 +  │
      785 +  ├─ JsonValue()
      786 +  │    sprintf(PUBLIS_BUF,
      787 +  │      "{\"id\":\"123\",\"params\":{"
      788 +  │        "\"Buzzer\":{\"value\":%s},"
      789 +  │        "\"Temp\":{\"value\":%d},"
      790 +  │        "\"LED\":{\"value\":%s},"
      791 +  │        "\"Humi\":{\"value\":%d}"
      792 +  │      "}}",
      793 +  │      "false",                  // Buzzer 写死为 false
      794 +  │      DHT11_Data.temp_int,       // 当前温度整数
      795 +  │      "true",                   // LED 写死为 true
      796 +  │      DHT11_Data.humi_int);      // 当前湿度整数
      797 +  │
      798 +  ├─ OneNet_Publish("$sys/.../post", PUBLIS_BUF)
      799 +  │    ├─ MQTT_PacketPublish(10, topic, msg, strlen(msg), QoS0, retain=0, own=1)
      800 +  │    ├─ ESP8266_SendData(pkt._data, pkt._len)
      801 +  │    └─ MQTT_DeleteBuffer(&pkt)
      802 +  │
      803 +  └─ ESP8266_Clear()  清空接收缓冲
      804 +       TimeCount = 0
      805 +```
      806
      163 -- **IDE**: Keil MDK-ARM（`project.uvprojx`）
      164 -- **编译器**: ARMCC V5 / V6
      165 -- **标准库**: STM32F10x Standard Peripheral Library V3.5.0
      166 -- **下载器**: ST-Link / J-Link（SWD 接口）
      167 -- **代码格式化**: `.clang-format`（LLVM 风格）
      807 +**下行（每循环检查）：**
      808
      169 -## 编译 & 下载
      809 +```
      810 +ESP8266_GetIPD(2)  // 超时 10ms
      811 +  │
      812 +  ├─ 无数据 → 返回 NULL → 跳过
      813 +  │
      814 +  └─ 有数据 → 返回 MQTT 报文字节指针
      815 +       │
      816 +       └─ OneNet_RevPro(dataPtr)
      817 +            │
      818 +            ├─ MQTT_UnPacketRecv() 识别类型
      819 +            │
      820 +            ├─ PUBLISH 类型 → 解析 JSON
      821 +            │    └─ 云平台下发格式示例:
      822 +            │       {"params":{"LED":{"value":true}}}
      823 +            │       {"params":{"Alarm":{"value":false}}}
      824 +            │       {"params":{"LED":{"value":true},"Alarm":{"value":true}}}
      825 +            │
      826 +            └─ CMD 类型 → 回复 $crsp
      827 +```
      828
      171 -1. 用 Keil MDK 打开 `project.uvprojx`
      172 -2. 选择 Target（如 `Target_1_STM32F103C8_1.0.0`）
      173 -3. 编译（F7）
      174 -4. 通过 SWD 下载到开发板（F8）
      829 +### OneNET 云平台下发指令格式
      830
      176 -## 注意事项
      831 +**LED 控制：**
      832 +```json
      833 +{
      834 +  "params": {
      835 +    "LED": {"value": true}
      836 +  }
      837 +}
      838 +```
      839
      178 -- ESP8266 未连接时，`ESP8266_Init()` 会死循环等待，测试 OLED 等本地功能需先注释该调用
      179 -- WiFi 密码和 OneNET Token 当前硬编码在源码中，正式产品需移至配置文件
      180 -- 火焰/MQ-2/MQ-135 三个文件（`bsp_flame.c`, `bsp_mq2.c`, `bsp_mq135.c`）为早期预留代码，实际传感器读取
          -统一由 `bsp_adc.c` 的 `ADCx_PPM(channel)` 完成
      181 -- `USART_DEBUG` 宏指向 USART3（PB10/PB11），调试信息输出至此串口，与 ESP8266 使用的 USART1 分离
      840 +**报警开关：**
      841 +```json
      842 +{
      843 +  "params": {
      844 +    "Alarm": {"value": false}
      845 +  }
      846 +}
      847 +```
      848 +
      849 +**组合指令：**
      850 +```json
      851 +{
      852 +  "params": {
      853 +    "LED": {"value": true},
      854 +    "Alarm": {"value": true}
      855 +  }
      856 +}
      857 +```
      858 +
      859 +---
      860 +
      861 +## 中断与定时器
      862 +
      863 +### 中断向量表
      864 +
      865 +| 中断 | 优先级（抢占/子） | 函数 | 功能 |
      866 +|------|------------------|------|------|
      867 +| SysTick | 默认 | `SysTick_Handler()` | 空函数，仅用于阻塞延时（在 `bsp_delay.c` 中通过轮询 COUNTFLAG
          + 实现） |
      868 +| HardFault | -1 | `HardFault_Handler()` | 死循环 `while(1)`，方便调试时定位崩溃 |
      869 +| TIM2 | 1/2 | `GENERAL_TIM_IRQHandler()` | `tick_ms++`，提供全局 1ms 时钟 |
      870 +| USART1 | 0/2 | `USART1_IRQHandler()` | 接收 ESP8266 数据，填入 `esp8266_buf[512]` |
      871 +| USART3 | 0/0 | `USART3_IRQHandler()` | 调试串口接收（当前仅清标志） |
      872 +
      873 +### TIM2 时基配置
      874 +
      875 +```
      876 +PCLK1 = 36MHz (APB1 最大时钟)
      877 +TIM2_CLK = PCLK1 × 2 = 72MHz (APB1 预分频 ≠ 1 时加倍)
      878 +PSC = 72 - 1 = 71 → TIM2 计数时钟 = 72MHz / 72 = 1MHz
      879 +Period = 1000 - 1 = 999 → 溢出周期 = 1000 × 1us = 1ms
      880 +```
      881 +
      882 +每 1ms 触发 `TIM2_IRQHandler`，`tick_ms++`，32 位计数器约 49.7 天溢出。
      883 +
      884 +### SysTick 延时
      885 +
      886 +```
      887 +SysTick_CLK = HCLK / 8 = 72MHz / 8 = 9MHz
      888 +UsCount = 9    → DelayUs(N) = SysTick_Delay(N × 9)
      889 +MsCount = 9000  → DelayMs(N) = SysTick_Delay(N × 9000)
      890 +```
      891 +
      892 +`SysTick_Delay()` 是阻塞实现：写 LOAD → 写 VAL=0 → 使能 → 轮询 COUNTFLAG → 关闭。非 RTOS 环境，延时期
          +间完全占用 CPU。
      893 +
      894 +---
      895 +
      896 +## 开发环境与编译
      897 +
      898 +### 工具链
      899 +
      900 +| 工具 | 版本/说明 |
      901 +|------|---------|
      902 +| IDE | Keil MDK-ARM 5.x |
      903 +| 编译器 | ARMCC V5.06 或 ARMCLANG V6 |
      904 +| 标准库 | STM32F10x StdPeriph Lib V3.5.0 |
      905 +| 调试器 | ST-Link V2 (SWD) 或 J-Link |
      906 +| 串口工具 | 调试串口 USART3 (115200-8N1)，可用任意串口助手 |
      907 +
      908 +### 编译步骤
      909 +
      910 +1. 打开 `project.uvprojx`
      911 +2. 确认 Target 选择正确（`Target_1_STM32F103C8_1.0.0`）
      912 +3. 菜单: Project → Build Target (F7)
      913 +4. 输出文件: `Objects/project.axf` (调试), `Objects/project.hex` (烧录)
      914 +
      915 +### VS Code 支持
      916 +
      917 +项目包含 `.vscode/` 和 `project.code-workspace`，可用 VS Code + EIDE 插件打开编辑，编译仍需 Keil。
      918 +
      919 +---
      920 +
      921 +## 已知限制与改进方向
      922 +
      923 +### 严重/功能缺陷
      924 +
      925 +| 问题 | 影响 | 建议修复 |
      926 +|------|------|---------|
      927 +| 无 MQTT PINGREQ 心跳 | OneNET 可能在 KeepAlive (256s) 超时后断开连接，设备无重连机制 | 主循环中定时
          +调用 `MQTT_PacketPing()` |
      928 +| `ESP8266_WaitRecive()` 使用停顿检测 | 网络延迟大时可能提前判定接收完成，导致数据截断 | 改为帧头尾匹
          +配或固定超时 |
      929 +| `ESP8266_GetIPD(2)` 超时仅 10ms | 云平台响应慢时可能漏收数据 | 增大超时或改为非阻塞队列 |
      930 +| OneNET Token 硬编码 + 过期时间 `et=1872683333` | Token 过期后鉴权失败 | 实现 Token 动态刷新或使用一
          +机一密 |
      931 +| WiFi 密码硬编码 | 换网络需重新编译烧录 | 使用 AT+SmartConfig 或串口配网 |
      932 +| 上传 JSON 中 Buzzer/LED 字段值写死 | 云平台看到的状态不反映实际硬件状态 | 根据 GPIO 实际状态动态填充
          + |
      933 +| TCP 断连无检测和重连 | ESP8266 掉线后设备静默失效 | 定时发送 AT 指令检测 + 断线重连状态机 |
      934 +| MQTT QoS 0 | 网络丢包时数据直接丢失，无重传 | 关键指令使用 QoS 1 |
      935 +
      936 +### 代码质量
      937 +
      938 +| 问题 | 位置 | 建议 |
      939 +|------|------|------|
      940 +| 6 个 OLED_Show 函数高度相似 | `app_init.c` | 用函数指针或参数化合并 |
      941 +| ADCx_PPM 使用了浮点 `pow()` 运算 | `bsp_adc.c:98` | 查表法或定点运算提升速度（当前每调用约 ~0.5ms）
          +|
      942 +| `OLED_DisplayBuf` 为全局变量 1KB | `bsp_oled.c:83` | 对于 20KB RAM 的 STM32F103C8 可接受（约 5%） |
