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

---

### 硬件平台

#### MCU: STM32F103C8T6

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
#### main.c
```c
#include "app_init.h"
int main(void)
{
    App_Init();     // 永不返回，内部包含 while(1) 主循环
}
```
#### bsp_oled.c
```text
应用层调用:
  OLED_ShowString() / OLED_Printf() / OLED_ShowNum()
  OLED_ShowImage() / OLED_DrawLine() / OLED_DrawCircle() ...
       │
       ▼ 操作显存数组
  OLED_DisplayBuf[8][128]           ← 128×64 像素缓冲区（8页 × 128列）
       │
       ▼ OLED_Update() 刷屏
  OLED_WriteData() → OLED_I2C_SendByte() → OLED_W_SDA() / OLED_W_SCL()
       │
       ▼ 软件模拟 I2C
  GPIOB PB6(SCL) / PB7(SDA) 开漏输出
       │
       ▼ I2C 总线
  SSD1306 OLED 控制器
```
#### bsp_dth11.c
```text
主机发送起始信号:
  DHT11_Dout_0    拉低 20ms
  DHT11_Dout_1    拉高 13us
  切换为输入模式  等待 DHT11 响应

DHT11 响应:
  拉低 80us → 拉高 80us → 开始发送 40bit 数据

40bit 数据格式:
  [湿度整数 8bit] [湿度小数 8bit] [温度整数 8bit] [温度小数 8bit] [校验和 8bit]
  校验和 = 前 4 字节之和的低 8 位

每 bit 编码:
  起始: 50us 低电平
  数据 0: 26~28us 高电平
  数据 1: 70us 高电平
  读取方式: 等 40us 后采样 GPIO 电平（0 已结束，1 仍在持续）
```
#### bsp_adc.c+bsp_filter.c
```text
voltage = ADC_Value × 5.0 / 4096
RS      = (5.0 - voltage) / (voltage × 0.5)    // 传感器电阻
R0      = 6.64                                  // 标定电阻 (kΩ)
ratio   = RS / R0
PPM     = FilterValue( 11.5428 × ratio^(-0.6549) )  // 幂函数拟合 + 一阶互补滤波

out(t) = α × LowPass(t) + (1-α) × HighPass(t)
LowPass(t)  = α × in(t) + (1-α) × out(t-1)
HighPass(t) = α × out(t-1) + α × (in(t) - in(t-1))
α = 0.2
```
#### bsp_key.c
```text
                    ┌──────────────────────────────────────────┐
                    │                                          │
       edge_down    ▼    debounce_ok       edge_up + long      │
 IDLE ──────────→ DEBOUNCE_PRESS ────────→ PRESSED ───────────┤
   ▲               │        ▲               │    │             │
   │    timeout    │  jitter│               │    │ time>=600ms │
   │    (350ms)    │  (不满足│消抖时间)       │    ▼             │
   │               ▼        │               │  return LONG_PRESS
   └──── WAIT_DOUBLE ───────┘               │    │ (仅首次)
        (debounce_ok)                        │    │ time>=680ms
          │                                  │    ▼
          │ edge_down                        │  return LONG_HOLD
          │ (第二次按下)                        │    │ (每80ms连发)
          └──────────────────────────────────┘    │
                                                  │ edge_up (非长按)
                                                  ▼
                                            DEBOUNCE_RELEASE
                                                  │
                                    debounce_ok   │
                          ┌───────────────────────┤
                          ▼                       ▼
                    from_double=1?          from_double=0?
                    是 → IDLE                否 → WAIT_DOUBLE
                    返回 DOUBLE_CLICK             │
                                                  │ 等 350ms
                                                  ▼
                                                IDLE
                                                返回 CLICK
```
#### esp8266.c
```text
1. AT                    → 等待 "OK"          测试模块
2. AT+CWMODE=1           → 等待 "OK"          设为 Station 模式
3. AT+CWDHCP=1,1         → 等待 "OK"          开启 DHCP
4. AT+CWJAP="SSID","PWD" → 等待 "GOT IP"      连接 WiFi 路由器
5. AT+CIPSTART="TCP",    → 等待 "CONNECT"     建立 TCP 连接到
   "mqtts.heclouds.com",1883                    OneNET MQTT Broker

USART1 RX 中断 (USART1_IRQHandler)
  每收到 1 字节 → esp8266_buf[esp8266_cnt++] = USART1->DR
  缓冲区大小: 512 字节，溢出回绕 (cnt 归零)

ESP8266_WaitRecive()
  在当前和上一次 cnt 相同时判断接收完毕（简单停顿检测）
  问题: 网络延迟大时可能误判——改进方向: 用帧头尾匹配或超时机制

ESP8266_SendCmd(cmd, expected_response)
  1. Usart_SendString(USART1, cmd, len)  发送 AT 命令
  2. 循环 200 次 × 10ms = 最多等待 2 秒
  3. 每次调用 ESP8266_WaitRecive() 检查是否收到数据
  4. strstr(buf, expected_response) 匹配期望响应
  5. 匹配成功 → 返回 0；超时 → 返回 1

ESP8266_SendData(data, len)
  1. 发送 "AT+CIPSEND=<len>\r\n"
  2. 等待 '>' 提示符
  3. 直接发送 len 字节原始数据（透传）

1. 等待接收完成（每次 5ms × timeout 次 = 最多 10ms timeout）
2. 在 esp8266_buf 中搜索 "IPD," 字符串
3. 找到后定位 ':' 分隔符
4. 返回 ':' 后第一个字节的指针 → 即 MQTT 原始报文
5. 超时 → 返回 NULL
```
#### MqttKit.c 
```text
// Common.h 宏定义
#define MQTT_MallocBuffer  malloc    // 使用 C 标准库堆分配
#define MQTT_FreeBuffer    free

// MQTT_PACKET_STRUCTURE 结构体
typedef struct Buffer {
    uint8  *_data;      // 数据指针
    uint32 _len;        // 当前数据长度
    uint32 _size;       // 总缓冲区大小
    uint8  _memFlag;    // MEM_FLAG_ALLOC (malloc) / MEM_FLAG_STATIC (固定数组)
} MQTT_PACKET_STRUCTURE;

固定头:   [1byte 类型=0x10] [1~4byte 剩余长度]
可变头:   [协议名 "MQTT"] [协议级别 4] [连接标志] [KeepAlive 256s]
Payload:  [ClientID "test"] [Username "XPz90SBcDh"] [Password Token]

固定头:   [1byte 类型=0x30 | QoS | Retain] [1~4byte 剩余长度]
可变头:   [2byte Topic长度] [Topic字符串] [2byte PacketID(仅QoS>0)]
Payload:  [消息体字节流]
```
#### onenet.c
```text
上行 Topic
$sys/XPz90SBcDh/test/thing/property/post
下行 Topic
$sys/XPz90SBcDh/test/thing/property/set
MQTT_UnPacketRecv(cmd) 识别包类型
  │
  ├─ MQTT_PKT_CMD (命令下发)
  │    ├─ MQTT_UnPacketCmd() 解出 cmdid + req_body
  │    └─ MQTT_PacketCmdResp() 组包回复 $crsp/cmdid
  │
  ├─ MQTT_PKT_PUBLISH (属性下发)
  │    ├─ MQTT_UnPacketPublish() 解出 topic + payload
  │    ├─ cJSON_Parse(payload) 解析 JSON
  │    ├─ 提取 "params" → "LED" 字段
  │    │    ├─ type == cJSON_True  → LED_ON
  │    │    └─ type == cJSON_False → LED_OFF
  │    ├─ 提取 "params" → "Alarm" 字段
  │    │    ├─ type == cJSON_True  → Alarm_flag = 1
  │    │    └─ type == cJSON_False → Alarm_flag = 0
  │    └─ cJSON_Delete(json) 释放内存
  │
  ├─ MQTT_PKT_SUBACK → 打印 "Subscribe OK"
  ├─ MQTT_PKT_PUBACK → 打印 "Publish Send OK"
  └─ ...
```


### 软件架构
```text
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
```
---
### 界面与按键
```text
     单击 KEY1 (→)                    单击 KEY2 (←)
     ───────────                      ───────────

  MAIN_MENU ──→ TEMP_SET ──→ HUMI_SET ──→ FLAME_SET ──→ MQ2_SET ──→ MQ135_SET
      ↑                                                                    │
      └────────────────────────────────────────────────────────────────────┘
       ←────────────────────────────────────────────────────────────────────
                           单击 KEY2 (←)

  (24,0)  "Smart Kitchen"         6×8 字体
  (0,10)  "Temp:25.3"             6×8 字体
  (0,20)  "Humi:60%"              6×8 字体
  (0,30)  "Flame_ppm:1234.56"     6×8 字体
  (0,40)  "Mq2_ppm:2345.67"       6×8 字体
  (0,50)  "Mq135_ppm:3456.78"     6×8 字体
```
---
### 传感器数据处理
```text
#### dht11
DHT11_Read_TempAndHumidity(&DHT11_Data)
  ├─ 发送起始脉冲（20ms 低 + 13us 高）
  ├─ 检测 DHT11 响应（80us 低 + 80us 高）
  ├─ 读取 5 字节（40bit）
  └─ 校验: check_sum == humi_int + humi_deci + temp_int + temp_deci ?
      ├─ 是 → 返回 SUCCESS，数据存入 DHT11_Data 全局结构体
      └─ 否 → 返回 ERROR，OLED 不显示温湿度行

#### adc
ADC1 硬件
  ├─ 序列1: CH1 (PA1) → DR → DMA → ADCx_Value[0]  (MQ-2)
  ├─ 序列2: CH2 (PA2) → DR → DMA → ADCx_Value[1]  (火焰)
  └─ 序列3: CH3 (PA3) → DR → DMA → ADCx_Value[2]  (MQ-135)
       │
       ▼ 每个通道约 4.6us × 3 = ~14us 完成一轮
       │ 连续循环（DMA 循环模式）
       │
  OLED_Show() / OLED_Show3/4/5 调用 ADCx_PPM(channel)
       │
       ├─ 读取 volatile ADCx_Value[channel]（最新 DMA 数据）
       ├─ voltage = ADC × 5.0 / 4096
       ├─ RS = (5.0 - V) / (V × 0.5)
       ├─ ratio = RS / 6.64
       ├─ raw_ppm = 11.5428 × ratio^(-0.6549)
       └─ FilterValue(&filter[channel], raw_ppm)  ← 一阶互补滤波
```
---
### 云平台通信
#### stage1 ESP8266 初始化 & WiFi 连接
```text
ESP8266_Init()
  │
  ├─[AT] 测试模块存在 ──→ 收到 "OK" ✓
  ├─[AT+CWMODE=1] 设为 Station ──→ "OK" ✓
  ├─[AT+CWDHCP=1,1] 开启 DHCP ──→ "OK" ✓
  ├─[AT+CWJAP="SSID","PWD"] 连 WiFi ──→ "GOT IP" ✓ (最多等 10s)
  └─[AT+CIPSTART="TCP","mqtts.heclouds.com",1883] ──→ "CONNECT" ✓
```
#### stage2 MQTT 登录鉴权
```text
OneNet_DevLink()
  │
  ├─ MQTT_PacketConnect(PROID, TOKEN, DEVID, 256s KeepAlive, QoS0, ...)
  │    构建 CONNECT 报文（~120 字节）
  │
  ├─ ESP8266_SendData(packet, len)
  │    发送 AT+CIPSEND=120 → 等 '>' → 发送 120 字节 MQTT CONNECT
  │
  ├─ ESP8266_GetIPD(250)  等待服务器响应（超时 250×5ms=1.25s）
  │    收到 ESP8266 返回: "+IPD,4:<4字节 CONNACK>"
  │
  └─ MQTT_UnPacketConnectAck(dataPtr)
       ├─ 返回码 0 → 连接成功，status=0
       ├─ 返回码 1 → 协议错误
       ├─ 返回码 2 → ClientID 非法
       ├─ 返回码 3 → 服务器不可用
       ├─ 返回码 4 → 用户名或密码错误
       └─ 返回码 5 → 未授权（Token 非法）
```
#### stage3 订阅下行主题
```text
OneNet_Subscribe({"$sys/XPz90SBcDh/test/thing/property/set"}, 1)
  │
  ├─ MQTT_PacketSubscribe(20, QoS0, topics, 1, &pkt)
  ├─ ESP8266_SendData(pkt._data, pkt._len)
  └─ MQTT_DeleteBuffer(&pkt)
```
#### stage4 主循环
##### 上行
```text
TimeCount 累加到 100
  │
  ├─ JsonValue()
  │    sprintf(PUBLIS_BUF,
  │      "{\"id\":\"123\",\"params\":{"
  │        "\"Buzzer\":{\"value\":%s},"
  │        "\"Temp\":{\"value\":%d},"
  │        "\"LED\":{\"value\":%s},"
  │        "\"Humi\":{\"value\":%d}"
  │      "}}",
  │      "false",                  // Buzzer 写死为 false
  │      DHT11_Data.temp_int,       // 当前温度整数
  │      "true",                   // LED 写死为 true
  │      DHT11_Data.humi_int);      // 当前湿度整数
  │
  ├─ OneNet_Publish("$sys/.../post", PUBLIS_BUF)
  │    ├─ MQTT_PacketPublish(10, topic, msg, strlen(msg), QoS0, retain=0, own=1)
  │    ├─ ESP8266_SendData(pkt._data, pkt._len)
  │    └─ MQTT_DeleteBuffer(&pkt)
  │
  └─ ESP8266_Clear()  清空接收缓冲
       TimeCount = 0
```
##### 下行
```text
ESP8266_GetIPD(2)  // 超时 10ms
  │
  ├─ 无数据 → 返回 NULL → 跳过
  │
  └─ 有数据 → 返回 MQTT 报文字节指针
       │
       └─ OneNet_RevPro(dataPtr)
            │
            ├─ MQTT_UnPacketRecv() 识别类型
            │
            ├─ PUBLISH 类型 → 解析 JSON
            │    └─ 云平台下发格式示例:
            │       {"params":{"LED":{"value":true}}}
            │       {"params":{"Alarm":{"value":false}}}
            │       {"params":{"LED":{"value":true},"Alarm":{"value":true}}}
            │
            └─ CMD 类型 → 回复 $crsp
```
