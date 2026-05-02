# STM32 Desktop Console V1.5 Hardware

## 1. 硬件简介

STM32 Desktop Console V1.5 是一个基于 STM32F103C8T6 最小系统板的桌面多功能控制台硬件平台。

本项目硬件部分集成了 OLED 显示、MAX7219 点阵显示、AHT20 温湿度传感器、MPU6050 姿态传感器、光敏 ADC 输入、W25Q64 外部 Flash、BT24 蓝牙串口模块、蜂鸣器、EC11 编码器、按键和多路 LED 指示。

PCB 采用模块化插接设计，大多数外设使用 2.54mm 排母连接，方便调试、替换和后期升级。

## 2. PCB 基本参数

| 项目 | 参数 |
|---|---|
| 硬件版本 | V1.5 |
| PCB 尺寸 | 100mm × 100mm |
| 层数 | 2 层 |
| 安装孔 | 4 个 M3 安装孔 |
| 安装孔孔径 | 3.2mm |
| 推荐铜柱 | M3 铜柱 |
| 主控 | STM32F103C8T6 最小系统板 |
| 输入电压 | 5V |
| 板载稳压 | AMS1117-3.3 |
| 主要电压网络 | 5V_IN、5V_PROT、5V_BUS、3V3、GND |

## 3. 电源设计

电源路径：

KF301-5.0-2P 输入 5V -> 自恢复保险丝 F1 -> 电源开关 SW1 -> 5V_BUS -> AMS1117-3.3 -> 3V3

| 网络名 | 说明 |
|---|---|
| 5V_IN | 外部 5V 输入 |
| 5V_PROT | 经过自恢复保险丝后的 5V |
| 5V_BUS | 经过电源开关后的系统 5V |
| 3V3 | AMS1117 输出的 3.3V |
| GND | 系统地 |

注意事项：

- MAX7219 模块使用 5V_BUS 供电。
- STM32 核心板、OLED、AHT20、MPU6050、W25Q64、BT24、光敏模块和蜂鸣器模块使用 3V3。
- 上电前必须用万用表检查 5V_IN、5V_BUS、3V3 与 GND 是否短路。
- PCB 设计中 3V3、5V_BUS 等网络使用 NetLabel，GND 使用真正的 GND 符号，避免电源网络误合并。

## 4. 主要模块

| 模块 | 功能 | 供电 |
|---|---|---|
| STM32F103C8T6 最小系统板 | 主控 | 3V3 |
| OLED | 菜单和状态显示 | 3V3 |
| MAX7219 | 8×8 点阵图标显示 | 5V_BUS |
| AHT20 | 温湿度采集 | 3V3 |
| MPU6050 | 姿态/运动检测 | 3V3 |
| 光敏模块 | 环境光 ADC 输入 | 3V3 |
| W25Q64 | 外部 Flash 存储 | 3V3 |
| BT24 | 蓝牙串口通信 | 3V3 |
| EC11 | 菜单旋转控制 | GPIO |
| 按键 | OK / RESET | GPIO / NRST |
| 蜂鸣器模块 | 操作提示音 | 3V3 |
| LED | 电源、用户状态、蓝牙发送指示 | 3V3 / GPIO |

## 5. STM32 引脚分配

| STM32 引脚 | 功能 |
|---|---|
| PA0 | 光敏 ADC 输入 |
| PA1 | EC11 编码器 A 相 |
| PA2 | EC11 编码器 B 相 |
| PA3 | OK 按键 |
| PA4 | W25Q64 CS |
| PA5 | SPI1 SCK |
| PA6 | SPI1 MISO |
| PA7 | SPI1 MOSI |
| PA9 | USART1 TX，连接 BT24 RXD / UART 调试口 |
| PA10 | USART1 RX，连接 BT24 TXD / UART 调试口 |
| PB6 | I2C1 SCL，连接 OLED / AHT20 / MPU6050 |
| PB7 | I2C1 SDA，连接 OLED / AHT20 / MPU6050 |
| PB8 | MAX7219 DIN |
| PB9 | MAX7219 CS |
| PB10 | MAX7219 CLK |
| PB12 | LED_USER1 |
| PB13 | LED_USER2 |
| PB14 | LED_BT_TX |
| PB15 | BUZZER |
| NRST | RESET 按键 |

## 6. 模块接口定义

### OLED 接口

| 引脚 | 功能 | 网络 |
|---|---|---|
| 1 | VCC | 3V3 |
| 2 | GND | GND |
| 3 | SCL | PB6_I2C1_SCL |
| 4 | SDA | PB7_I2C1_SDA |

### AHT20 接口

| 引脚 | 功能 | 网络 |
|---|---|---|
| 1 | SDA | PB7_I2C1_SDA |
| 2 | SCL | PB6_I2C1_SCL |
| 3 | GND | GND |
| 4 | VIN | 3V3 |

### MPU6050 接口

| 引脚 | 功能 | 网络 |
|---|---|---|
| 1 | INT | NC |
| 2 | AD0 | GND |
| 3 | XCL | NC |
| 4 | XDA | NC |
| 5 | SDA | PB7_I2C1_SDA |
| 6 | SCL | PB6_I2C1_SCL |
| 7 | GND | GND |
| 8 | VCC | 3V3 |

### MAX7219 接口

| 引脚 | 功能 | 网络 |
|---|---|---|
| 1 | VCC | 5V_BUS |
| 2 | GND | GND |
| 3 | DIN | PB8_MAX_DIN |
| 4 | CS | PB9_MAX_CS |
| 5 | CLK | PB10_MAX_CLK |

### W25Q64 接口

| 引脚 | 功能 | 网络 |
|---|---|---|
| 1 | VCC | 3V3 |
| 2 | CS | PA4_FLASH_CS |
| 3 | DO | PA6_SPI1_MISO |
| 4 | GND | GND |
| 5 | CLK | PA5_SPI1_SCK |
| 6 | DI | PA7_SPI1_MOSI |

### BT24 蓝牙接口

| 引脚 | 功能 | 网络 |
|---|---|---|
| 1 | STAT | NC |
| 2 | RXD | PA9_USART1_TX |
| 3 | TXD | PA10_USART1_RX |
| 4 | GND | GND |
| 5 | 5V | NC |
| 6 | 3.3V | 3V3 |

注意：

- BT24 使用 3.3V 供电。
- BT24 的 RXD 接 STM32 的 TX，TXD 接 STM32 的 RX。
- BT24 天线区域应设置禁铜区。

### 光敏模块接口

| 引脚 | 功能 | 网络 |
|---|---|---|
| 1 | AO | PA0_ADC_LIGHT |
| 2 | DO | NC |
| 3 | GND | GND |
| 4 | VCC | 3V3 |

### 蜂鸣器接口

| 引脚 | 功能 | 网络 |
|---|---|---|
| 1 | GND | GND |
| 2 | I/O | PB15_BUZZER |
| 3 | VCC | 3V3 |

说明：

- 使用低电平触发蜂鸣器模块。
- 软件中支持蜂鸣提示和静音模式。

### UART 调试口

| 引脚 | 功能 | 网络 |
|---|---|---|
| 1 | 3V3 | 3V3 |
| 2 | GND | GND |
| 3 | TX | PA9_USART1_TX |
| 4 | RX | PA10_USART1_RX |

注意：UART 调试口和 BT24 共用 USART1。调试时不要同时让多个外部设备驱动串口，避免通信冲突。

## 7. I2C 总线说明

I2C1 总线连接：

- OLED
- AHT20
- MPU6050

| 信号 | STM32 引脚 |
|---|---|
| SCL | PB6 |
| SDA | PB7 |

板上预留 4.7k 上拉电阻：

| 电阻 | 网络 |
|---|---|
| R_SCL_PULLUP | PB6_I2C1_SCL -> 3V3 |
| R_SDA_PULLUP | PB7_I2C1_SDA -> 3V3 |

多数 I2C 模块自带上拉电阻，因此本 PCB 上的 I2C 上拉电阻默认可不焊，丝印标记为 DNP / 默认不焊。

## 8. PCB 布局说明

- STM32 核心板位于 PCB 中心区域。
- OLED 和 MAX7219 放在上方，方便观察显示。
- EC11、按键、LED 放在下方，方便操作。
- 电源模块放在左下角，方便接线和调试。
- BT24 蓝牙模块靠近板边，天线朝外。
- AHT20 远离 AMS1117、电源区、MAX7219 和其他发热器件，降低温度误差。
- 四角预留 M3 安装孔。

## 9. BT24 天线禁铜说明

BT24 蓝牙模块的天线区域下方和前方建议设置禁铜区：

BT24 ANT AREA
NO COPPER

设计要求：

- 天线下方不要铺铜。
- 天线附近尽量不要放大面积金属。
- 天线附近尽量避开安装孔、铜柱和大排针。
- 模块天线端尽量朝 PCB 外侧。

## 10. 焊接和调试顺序

推荐按以下顺序焊接和调试：

1. 焊接电源部分：KF301 输入端子、自恢复保险丝、电源开关、AMS1117-3.3、电源指示 LED。
2. 上电前检查短路：5V_IN 与 GND、5V_BUS 与 GND、3V3 与 GND。
3. 接入 5V 电源，测量 5V_BUS 是否约为 5V，3V3 是否约为 3.3V。
4. 焊接 STM32 核心板插座。
5. 插入 STM32F103C8T6 最小系统板，确认能正常下载程序。
6. 依次插入并测试 OLED、AHT20、光敏模块、MAX7219、BT24、W25Q64、MPU6050、蜂鸣器模块。
7. 每增加一个模块都单独测试，避免一次性插入后难以排查问题。

## 11. 已知注意事项

- MAX7219 使用 5V_BUS 供电。
- BT24 使用 3.3V 供电，不接 5V 引脚。
- OLED、AHT20、MPU6050 共用 I2C1 总线。
- I2C 上拉电阻默认可不焊。
- 光敏模块只使用 AO 模拟输出，DO 未使用。
- MPU6050 的 INT、XCL、XDA 未使用。
- UART 调试口和 BT24 共用 PA9 / PA10。
- 所有模块插入前必须确认引脚顺序。
- 当前 V1.5 为第一版 PCB，主要目标是验证完整功能。

## Gerber

当前版本 Gerber 文件：

- hardware/gerber/gerber_v1.5.zip

该文件可用于 PCB 打样。下单前建议重新检查 Gerber 预览、板框、钻孔、丝印、顶层/底层走线和铺铜。
