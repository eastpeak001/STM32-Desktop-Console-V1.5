# 立创开源广场项目内容填写稿

## 项目名称

基于 STM32F103C8T6 的多传感器蓝牙桌面控制终端

## 一句话简介

基于 STM32F103C8T6 的 V1.4 面包板验证版桌面控制终端，集成 OLED 菜单、传感器采集、蓝牙控制、点阵图标和 W25Q64 历史记录功能。

## 项目简介

这是一个基于 STM32F103C8T6 的多功能桌面控制终端，当前版本为 V1.4 面包板验证版。

项目使用 STM32CubeMX 生成 HAL 工程，通过模块化代码实现 OLED 菜单显示、旋转编码器/按键交互、AHT20 温湿度检测、光敏 ADC 采集、MPU6050 姿态/震动检测、LED 控制、操作提示音/静音模式、MAX7219 8x8 点阵状态图标、DX-BT24 蓝牙串口通信、手机蓝牙命令控制、W25Q64 历史数据记录与蓝牙导出。

本项目主要面向 STM32 初学者，适合作为 HAL 库、GPIO、ADC、I2C、SPI、USART、中断接收、非阻塞任务调度和简单 UI 菜单系统的综合练习。

## 项目特点

- STM32F103C8T6 最小系统板主控。
- OLED SSD1306 显示主菜单和各功能页面。
- 旋转编码器切换菜单，按键进入/返回页面。
- AHT20 采集温湿度。
- 光敏模块通过 ADC 读取环境光强。
- MPU6050 读取三轴加速度，并实现简单震动检测。
- PB12/PB13 两路 LED 支持 OFF、ON、BLINK。
- PB14 作为蓝牙发送指示灯，蓝牙发送数据时短暂点亮。
- PB15 低电平触发有源蜂鸣器，支持操作提示音和静音模式。
- MAX7219 8x8 点阵显示状态图标。
- DX-BT24 蓝牙串口支持手机发送英文/拼音 ASCII 命令。
- W25Q64 SPI Flash 保存最近 100 条历史记录，并支持蓝牙导出。
- 主循环使用 HAL_GetTick() 进行非阻塞刷新，不使用 RTOS。

## 硬件清单

| 名称 | 型号/说明 | 数量 |
| --- | --- | --- |
| 主控板 | STM32F103C8T6 最小系统板 | 1 |
| OLED 显示屏 | SSD1306，0.96 寸，128x64，I2C | 1 |
| 温湿度传感器 | AHT20，I2C | 1 |
| 光敏模块 | AO 模拟输出 | 1 |
| 姿态传感器 | MPU6050，I2C | 1 |
| 蓝牙模块 | DX-BT24 串口蓝牙 | 1 |
| 点阵模块 | MAX7219 8x8 点阵 | 1 |
| SPI Flash | W25Q64 | 1 |
| 旋转编码器 | A/B/SW 输出 | 1 |
| LED | PB12/PB13 普通 LED，PB14 蓝牙发送指示灯 | 3 |
| 蜂鸣器 | 有源蜂鸣器，低电平触发 | 1 |
| 面包板和杜邦线 | 用于 V1.4 验证 | 若干 |

## 引脚分配

| 模块 | 信号 | STM32 引脚 |
| --- | --- | --- |
| OLED SSD1306 | SCL/SDA | PB6/PB7 |
| AHT20 | SCL/SDA | PB6/PB7 |
| MPU6050 | SCL/SDA | PB6/PB7 |
| DX-BT24 | TX/RX | PA10/PA9 |
| 光敏模块 | AO | PA0 |
| 编码器 | A/B/SW | PA1/PA2/PA3 |
| W25Q64 | CS/SCK/MISO/MOSI | PA4/PA5/PA6/PA7 |
| MAX7219 | DIN/CS/CLK | PB8/PB9/PB10 |
| LED1 | 控制脚 | PB12 |
| LED2 | 控制脚 | PB13 |
| 蓝牙 TX 指示灯 | 控制脚 | PB14 |
| 蜂鸣器 | 控制脚 | PB15 |

## 软件说明

工程使用 STM32CubeIDE + STM32CubeMX + HAL 库开发。

主要模块：

- app.c / app.h：应用状态机、菜单切换和任务调度。
- ui.c / ui.h：OLED 页面显示。
- oled.c / oled.h：SSD1306 OLED 驱动。
- encoder.c / encoder.h：编码器和按键轮询扫描。
- aht20.c / aht20.h：AHT20 驱动。
- sensors.c / sensors.h：温湿度和光敏数据管理。
- mpu6050.c / mpu6050.h：MPU6050 驱动。
- buzzer.c / buzzer.h：蜂鸣器和操作提示音开关。
- bluetooth.c / bluetooth.h：蓝牙命令解析、状态发送、PB14 发送指示灯。
- max7219.c / max7219.h：MAX7219 点阵图标显示。
- w25q64.c / w25q64.h：W25Q64 底层驱动。
- history.c / history.h：历史记录保存、读取和清空。

## 蓝牙命令

蓝牙命令使用英文或拼音 ASCII，避免手机串口 App 中文编码不一致导致乱码。命令不区分大小写，支持 CR、LF 或 CRLF 结束。

常用命令：

| 命令 | 拼音别名 | 功能 |
| --- | --- | --- |
| HELP | BANGZHU | 查看命令列表 |
| STATUS | ZHUANGTAI | 获取状态 |
| LED ON | KAIDENG | 打开 PB12/PB13 |
| LED OFF | GUANDENG | 关闭 PB12/PB13 |
| LED BLINK | SHANDENG | LED 闪烁 |
| BUZZER | FENGMING | 测试蜂鸣器 |
| BEEP ON | FENGMING ON | 开启操作提示音 |
| BEEP OFF | FENGMING OFF | 关闭操作提示音 |
| BEEP TOGGLE | JINGYIN | 切换提示音状态 |
| GET MOTION | ZITAI | 获取 MPU6050 数据 |
| SAVE NOW | BAOCUN | 立即保存记录 |
| HISTORY STATUS | JILUZHUANGTAI | 查看记录状态 |
| GET HISTORY | DAOCHUJILU | 导出历史记录 |
| CLEAR HISTORY | QINGKONGJILU | 清空历史记录 |

STATUS 回复示例：

```text
TEMP=25.6,HUMI=60.2,LIGHT=1234,LED=ON,BEEP=ON
OK
```

历史记录导出示例：

```text
REC 1,T=25.6,H=60.2,L=1234,AX=12,AY=-35,AZ=16384,SHAKE=0
OK
```

## 调试说明

建议按模块逐步调试：

1. 先确认 STM32F103C8T6 可以正常下载程序。
2. 调试 OLED 显示，确认主菜单显示正常。
3. 调试编码器和按键，确认菜单可以切换和进入页面。
4. 调试 AHT20 和光敏 ADC 页面。
5. 调试 LED、蜂鸣器和 Beep Mode 页面。
6. 调试 DX-BT24 蓝牙串口，确认命令收发正常。
7. 调试 MAX7219 点阵图标。
8. 调试 MPU6050 Motion 页面。
9. 调试 W25Q64 历史记录保存和导出。

## 当前版本

V1.4 面包板验证版。

本版本已经完成功能验证，适合学习、演示和后续 PCB 化改造。由于当前仍是验证版，硬件连接和结构还有继续优化空间。

## 后续计划

- 将面包板验证版整理为 PCB 版本。
- 优化 W25Q64 历史记录格式，增加 CRC 校验。
- 优化菜单 UI。
- 增加手机端更友好的控制界面。
- 增加低功耗或屏幕休眠功能。

## 开源协议建议

建议使用 MIT License。

本项目主要用于学习和交流，允许他人自由使用、修改和二次开发。若后续用于商业产品或教学资料，可根据实际情况重新选择许可证。

## 标签建议

STM32、STM32F103C8T6、HAL、OLED、蓝牙、AHT20、MPU6050、W25Q64、MAX7219、桌面控制台、传感器、立创开源
