
# STM32F103ZET6 智能WiFi天气站 (RT-Thread)

## 项目概述

本项目基于 **正点原子战舰V3开发板** (STM32F103ZET6)，运行 **RT-Thread** 实时操作系统，实现了一款集**天气预报、温湿度采集、语音播报、LCD显示**于一体的智能物联网终端。

### 核心功能

| 功能 | 描述 |
|------|------|
| 🌐 **WiFi联网** | 通过 ESP8266 模块连接 WiFi，访问心知天气 API |
| 🌤 **天气预报** | 获取 3 天天气预报（温度、湿度、天气状况）并 LCD 显示 |
| 🌡 **温湿度采集** | DHT11 传感器采集室内温湿度数据 |
| 🔊 **语音播报** | MY1680 语音模块播报天气和温湿度信息 |
| 🖥 **LCD 显示** | 240×320 TFTLCD 显示 UI 菜单和天气数据 |
| 💡 **LED 指示** | 4 个 LED 流水灯效果 + RGB LED 氛围灯 |
| 🔊 **蜂鸣器** | 按键操作短鸣反馈 |
| ⌨ **按键交互** | 4 个按键 + 长短按识别，支持多级菜单切换 |

### UI 界面

- **主菜单**: 显示功能按键说明
- **KEY1**: 室内温湿度检测 (DHT11 实时采集 + 语音播报)
- **KEY2**: 今日天气预报 (心知 API + 语音播报)
- **KEY3**: 明日天气预报
- **KEY4**: 后日天气预报
- **KEY5**: 返回主菜单
- **KEY6**: 音乐播放模式 (RGB 灯效 + 背景音乐)

---

## MCU 配置

| 参数 | 值 |
|------|-----|
| **MCU** | STM32F103ZET6 (Cortex-M3) |
| **主频** | 72MHz (HSE 12MHz × 9 PLL) |
| **Flash** | 512 KB |
| **SRAM** | 64 KB |
| **RTOS** | RT-Thread v4.0.2 |
| **RTOS Tick** | 1000 Hz |
| **控制台** | UART1 (115200-8-N-1) |

### 时钟树

```
HSE (12MHz) → PLL (×9) → SYSCLK (72MHz)
                            ├── AHB (72MHz)
                            │   ├── APB1 (36MHz) → UART2, UART3
                            │   └── APB2 (72MHz) → UART1
                            └── Cortex-M3 (72MHz)
```

### RT-Thread 内核配置

- 线程优先级: 32 级
- 线程间通信: 信号量 / 互斥量 / 事件 / 邮箱 / 消息队列
- 内存管理: 小内存管理算法 (heap + mempool)
- FinSH 控制台: MSH 模式 (UART1)
- 设备框架: PIN / SERIAL / RTC / SENSOR

---

## 引脚定义

### GPIO 引脚分配

#### LED 指示灯 (低电平点亮)

| 引脚 | 功能 | 方向 |
|------|------|------|
| PE2 | LED1 (DS0) | 推挽输出 |
| PE3 | LED2 (DS1) | 推挽输出 |
| PE4 | LED3 | 推挽输出 |
| PE5 | LED4 | 推挽输出 |

#### RGB LED (低电平点亮)

| 引脚 | 功能 | 方向 |
|------|------|------|
| PA8 | RGB-R (红色) | 推挽输出 |
| PA7 | RGB-G (绿色) | 推挽输出 |
| PA6 | RGB-B (蓝色) | 推挽输出 |

#### 按键

| 引脚 | 功能 | 模式 | 电平逻辑 |
|------|------|------|----------|
| PA0 | KEY1 | 上拉输入 | 按下→高电平 |
| PC4 | KEY2 | 下拉输入 | 按下→低电平 |
| PC5 | KEY3 | 下拉输入 | 按下→低电平 |
| PC6 | KEY4 | 下拉输入 | 按下→低电平 |

> **注意**: KEY1 (PA0) 使用上拉输入，其他按键使用下拉输入，按键扫描支持短按和长按(>10个tick≈1秒)区分。

#### 蜂鸣器

| 引脚 | 功能 | 方向 |
|------|------|------|
| PC0 | 蜂鸣器 | 推挽输出 (高电平有效) |

#### ESP8266 WiFi

| 引脚 | 功能 | 方向 |
|------|------|------|
| PE6 | ESP8266 使能 (EN) | 推挽输出 |
| PB10 | USART3_TX → ESP8266 RX | 复用推挽 |
| PB11 | USART3_RX ← ESP8266 TX | 浮空输入 |

#### MY1680 语音模块

| 引脚 | 功能 | 方向 |
|------|------|------|
| PA2 | MY1680_BUSY_INP (TX) | 推挽输出 |
| PA3 | MY1680_BUSY_OUT (RX) | 浮空输入 |
| PA2 | USART2_TX → MY1680 RX | 复用推挽 |
| PA3 | USART2_RX ← MY1680 TX | 浮空输入 |

#### DHT11 温湿度传感器

| 引脚 | 功能 | 方向 |
|------|------|------|
| PC1 | DHT11_DATA | 开漏/输入 (单总线协议) |

*(DHT11 引脚定义在 sensor_dallas_dht11 驱动包中配置)*

#### LCD (FSMC 接口)

| 引脚 | 功能 |
|------|------|
| PB0 | LCD 背光 (BL) |
| FSMC 总线 | 8080 并口通信 |

---

## 外设配置

### 串口 (UART)

| 外设 | 用途 | 波特率 | 引脚 |
|------|------|--------|------|
| UART1 | 控制台/调试 (FinSH) | 115200 | PA9(TX), PA10(RX) |
| UART2 | MY1680 语音模块 | 9600 | PA2(TX), PA3(RX) |
| UART3 | ESP8266 WiFi 模块 | 115200 | PB10(TX), PB11(RX) |

### WiFi + 网络

- **模块**: ESP8266 (AT 固件)
- **通信**: UART3 → AT 指令 → TCP 透传
- **协议栈**: SAL (Socket Abstraction Layer) + AT Socket
- **连接流程**:
  1. `AT` — 测试连接
  2. `AT+CWMODE=1` — 设为 STA 模式
  3. `AT+CWJAP="SSID","PASSWORD"` — 连接 WiFi
  4. `AT+CIPSTART="TCP","api.seniverse.com",80` — TCP 连接
  5. `AT+CIPMODE=1` — 透传模式
  6. `AT+CIPSEND` — 开始发送

### 天气数据

- **API**: 心知天气 (seniverse.com)
- **城市**: 郑州
- **数据格式**: JSON (使用 cJSON 解析)
- **数据结构**:

```c
typedef struct {
    char data[20];       // 日期
    char name[30];       // 城市名
    char weatherday[20]; // 白天天气
    char weathernight[20];// 夜间天气
    char codeday[10];    // 白天天气代码
    char codenight[10];  // 夜间天气代码
    char temhigh[10];    // 最高温
    char temlow[10];     // 最低温
    char humidity[10];   // 湿度
} Weather_DataStruct;
```

### LCD 显示

- **型号**: 正点原子 2.8/3.5寸 TFTLCD
- **接口**: FSMC (8080 并口)
- **分辨率**: 240×320
- **驱动**: RT-Thread LCD 驱动框架 (rt_device_graphic_ops)
- **字库**: 内置 ASCII 8×16 / 12×24 点阵字库 + 中文字库 (GB2312)

### 语音模块 (MY1680)

- **型号**: MY1680 (MP3 解码芯片)
- **通信协议**: UART 指令帧
- **帧格式**: `0x7E + 长度 + 命令 + 参数 + XOR校验 + 0xEF`
- **功能**: 目录选曲播放 / 数字拼读 / 停止 / 切换

---

## 线程设计

| 线程名 | 优先级 | 栈大小 | 功能 |
|--------|--------|--------|------|
| `LED` | 7 | 512 | LED 流水灯效果 |
| `KEY` | 6 | 2048 | 按键扫描 + UI 菜单切换 |
| `WIFIThread` | 2 | 4096 | WiFi 数据获取 + JSON 解析 |
| `tshell` | 20 | 4096 | FinSH 控制台 (系统自带) |
| `main` | 10 | 2048 | 主线程 (初始化所有模块) |

---

## 项目目录

```
stm32f103zet6/
├── applications/              # 用户应用层代码
│   ├── main.c                 # 主函数 (初始化入口)
│   ├── led.c / led.h          # LED 流水灯驱动
│   ├── beep.c / beep.h        # 蜂鸣器驱动
│   ├── key.c / key.h          # 按键扫描 + UI 菜单
│   ├── rgb_led.c / rgb_led.h  # RGB LED 控制
│   ├── bsp_lcd.c / bsp_lcd.h  # LCD 显示驱动 (画点/字符串/图片)
│   ├── font.c / font.h        # ASCII & 中文字库
│   ├── wifi.c / wifi.h        # ESP8266 AT驱动 + 天气API JSON解析
│   ├── my1680.c / my1680.h    # MY1680 语音模块驱动
│   ├── cJSON.c / cJSON.h      # JSON 解析库
│   └── SConscript             # 编译脚本
│
├── board/                     # 板级支持包
│   ├── board.c / board.h      # 系统时钟配置 (72MHz)
│   ├── linker_scripts/        # 链接脚本
│   ├── CubeMX_Config/         # STM32CubeMX 配置文件
│   │   ├── CubeMX_Config.ioc  # CubeMX 图形化配置
│   │   ├── Inc/               # HAL 头文件
│   │   ├── Src/               # HAL 源文件 (HAL_MSP, 中断等)
│   │   ├── MDK-ARM/           # MDK 调试配置
│   │   └── Drivers/           # CMSIS + STM32F1xx_HAL_Driver
│   └── ... (HAL_Drivers, 驱动文件)
│
├── packages/                  # RT-Thread 软件包
│   ├── at_device-latest/      # AT 设备驱动 (ESP8266)
│   ├── dht11-latest/          # DHT11 温湿度传感器驱动
│   ├── mbedtls-v2.28.1/       # mbedTLS 加密库 (HTTPS)
│   └── webclient-v2.0.0/      # HTTP/HTTPS 客户端
│
├── figures/                   # 图片资源
│   └── board.jpg              # 开发板图片
│
├── build/                     # 编译输出目录
├── DebugConfig/               # 调试配置
├── project.uvprojx            # MDK5 工程文件 (Keil)
├── project.uvproj             # MDK4 工程文件
├── project.ewp                # IAR 工程文件
├── rtconfig.h / rtconfig.py   # RT-Thread 配置文件
├── SConstruct                 # Scons 构建脚本
├── Kconfig                    # 配置菜单
└── README.md                  # 本文件
```

---

## 注意事项

### 硬件

1. **外部晶振**: 本开发板外部高速晶振为 **12MHz**，代码中 HSE_PREDIV=1, PLL_MUL=9 → 72MHz
2. **KEY1 按键**: PA0 引脚上拉，与正点原子官方引脚定义不同（官方为 PA0 唤醒按键），请以实际接线为准
3. **LCD 接口**: 使用 FSMC 并行接口，不可同时使用其他占用 FSMC 总线的外设
4. **ESP8266**: 模块使能引脚 PE6 需输出高电平，否则模块不工作
5. **DHT11**: 单总线协议，时序要求严格，建议使用 RT-Thread 的 DHT11 软件包

### 软件

1. **WiFi 密码硬编码**: `MY_SSID` 和 `MY_PASSWORD` 在 `wifi.h` 中硬编码，上传 GitHub 时**注意不要泄露个人 WiFi 密码**
   - 建议: 使用 `menuconfig` 配置或读取配置文件
2. **心知天气 API Key**: `MY_KEY` 在 `wifi.h` 中硬编码，上传前请**务必删除或替换**
3. **ESP8266 初始化时序**: 初始化中使用了多出 `rt_thread_mdelay/rt_thread_delay` 阻塞延时（约 30 秒总延时），在正式产品中建议优化为状态机 + 超时机制
4. **cJSON**: 项目在 `applications/` 下直接包含了 `cJSON.c/h`，但 `.config` 中未启用 `PKG_USING_CJSON`。当前可用，但与 RT-Thread 的 cJSON 包可能存在冲突风险
5. **字体编码**: 中文字库使用 GB2312 编码，在 UTF-8 的编辑器中查看字符串字面量时中文会显示为乱码，建议保持源文件编码一致
6. **DHT11 读取**: 在 key.c 中直接 `#include "sensor_dallas_dht11.h"` 并读取全局变量 `u_temp`/`u_humi`，实际读取由 DHT11 软件包后台完成

### 构建

- 支持 **Keil MDK4/5**、**IAR** 和 **GCC** 编译
- 使用 `scons --target=mdk5` 重新生成 Keil 工程
- 使用 `menuconfig` 进行 RT-Thread 配置


## License

- 本项目基础部分 (RT-Thread BSP) 遵循 Apache-2.0 License
- 用户应用层代码默认为开源学习用途

### 构建环境步骤
-之后在新电脑上使用这个项目，需要：


# 1. 将 BSP 放入 RT-Thread 源码树的对应位置
# rt-thread/bsp/stm32/stm32f103zet6/

# 2. 更新软件包
cd rt-thread/bsp/stm32/stm32f103zet6
pkgs --update

# 3. 重新生成工程（以 Keil MDK5 为例）
scons --target=mdk5

# 4. 双击 project.uvprojx 编译下载
