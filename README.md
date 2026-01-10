# ESP32-S3 BPC Time Signal Generator

[English](#english) | [中文](#中文)

---

## English

### Overview
This project implements a **BPC (68.5 kHz) time signal generator** using an **ESP32-S3** microcontroller.  
It follows the BPC time code specification as described on Wikipedia and generates a carrier-modulated signal suitable for **testing, education, and experimental reception**.

Time synchronization is achieved via **NTP**, and the BPC frames are generated precisely at **:00, :20, and :40 seconds**.

> ⚠️ This project is for **experimental and educational use only**. Do **not** connect it to large antennas or attempt long‑range transmission.

---

### Features
- ESP32-S3 based implementation
- 68.5 kHz carrier using LEDC PWM
- Accurate timing via NTP (`configTime`)
- Full BPC frame generation (20‑second frames)
- Even parity (P1 / P2) calculation
- Optional LED indicator output

---

### Hardware Requirements
- ESP32-S3 development board
- GPIO output pin for BPC signal (default: GPIO 21)
- Optional LED indicator (default: GPIO 19)
- **Coupling coil antenna** (near-field transmission):
  - 0.3 mm diameter enamelled copper wire (漆包线)
  - ~20 turns
  - Series resistor: **330 Ω**

**Performance note:** A radio-controlled watch placed within **~20 cm** of the transmitter can successfully synchronize its time.

---

### Software Requirements
- **Arduino IDE 2.x** or **PlatformIO**
- **Arduino‑ESP32 core v2.x** (ESP‑IDF 4.4 based)
- Board package: ESP32‑S3

---

### Pin Configuration
```cpp
const int BPC_PIN = 21;          // BPC signal output
const int LED_INDICATOR_PIN = 19; // Indicator LED
```

---

### How It Works
- The ESP32 generates a **68.5 kHz carrier** using the LEDC peripheral
- Each second contains a carrier‑off interval of:
  - 100 ms → symbol 0
  - 200 ms → symbol 1
  - 300 ms → symbol 2
  - 400 ms → symbol 3
- Time information is encoded in **quaternary symbols (0–3)**
- Frames repeat every **20 seconds**

---

### Compilation Target
- Framework: **Arduino**
- Core: **arduino‑esp32 v2.x**
- MCU: **ESP32‑S3**

---

### References
- BPC Time Signal: https://en.wikipedia.org/wiki/BPC_(time_signal)
- Antenna design credit: https://github.com/luigicalligaris/dcfake77

---

## 中文

### 项目简介
本项目基于 **ESP32-S3** 实现 **BPC（68.5 kHz）授时信号发生器**，
按照 BPC 标准时间码格式生成信号，用于 **实验、学习和近场测试**。

设备通过 **NTP 网络时间同步**，并在 **每分钟的 :00、:20、:40 秒** 精确发射时间帧。

> ⚠️ 本项目仅用于 **实验和教学目的**，请勿连接大尺寸天线或进行远距离发射。

---

### 功能特性
- 基于 ESP32-S3
- LEDC 产生 68.5 kHz 载波
- NTP 网络对时
- 完整 BPC 20 秒时间帧
- P1 / P2 偶校验计算
- 可选 LED 指示输出

---

### 硬件需求
- ESP32-S3 开发板
- BPC 信号输出 GPIO（默认 GPIO 21）
- 可选指示 LED（默认 GPIO 19）
- **磁耦合线圈天线（近场发射）**：
  - 线径 **0.3 mm 漆包线**
  - **约 20 圈**
  - 串联电阻：**330 Ω**

**实测效果：** 将电波表放置在发射端 **约 20 cm 以内**，可成功完成自动对时。

---

### 软件环境
- **Arduino IDE 2.x** 或 **PlatformIO**
- **Arduino-ESP32 v2.x** 核心
- ESP32-S3 开发板支持包

---

### 引脚配置
```cpp
const int BPC_PIN = 21;          // BPC 信号输出
const int LED_INDICATOR_PIN = 19; // 指示灯
```

---

### 工作原理
- 使用 LEDC 外设生成 **68.5 kHz 载波**
- 每秒包含不同长度的“无载波”间隔：
  - 100 ms → 符号 0
  - 200 ms → 符号 1
  - 300 ms → 符号 2
  - 400 ms → 符号 3
- 时间信息以 **四进制符号（0–3）** 编码
- 每 20 秒循环一帧

---

### 编译环境
- 框架：Arduino
- 核心：arduino-esp32 v2.x
- 芯片：ESP32-S3

---

### 参考资料
- BPC 时间信号: https://en.wikipedia.org/wiki/BPC_(time_signal)
- 天线设计参考：https://github.com/luigicalligaris/dcfake77

