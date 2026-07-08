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

### 项目结构

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

