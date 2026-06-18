# 🎮 retro-go_chaeng（ESP32-S3 复古掌机）

<p align="center">
  <img width="4000" height="2252" alt="20260614_092928" src="https://github.com/user-attachments/assets/87ce1f19-227a-4a08-91c1-b1887e7af833" />
</p>

<p align="center">
基于 Retro-Go（ducalex）移植的 ESP32-S3 复古掌机项目<br/>
Retro-Go 中文硬件扩展版本（chaeng 定制）
</p>

---

<p align="center">

![Base](https://img.shields.io/badge/Base-Retro--Go%20fork-orange)
![Platform](https://img.shields.io/badge/MCU-ESP32--S3-blue)
![Display](https://img.shields.io/badge/LCD-3.2inch%20TFT-green)
![Audio](https://img.shields.io/badge/I2S-Audio-yellow)
![RGB](https://img.shields.io/badge/WS2812-RGB-red)
![Language](https://img.shields.io/badge/Code-中文化-orange)
![License](https://img.shields.io/badge/GPL-v3.0-blue)

</p>

---

## 📺 B站演示视频

https://www.bilibili.com/video/BV1SQJF6KEsp/?vd_source=3ac89604b734514d5955b607a3f43d69

---

## 📌 硬件开源地址（已参加“星火计划2026”）

https://oshwhub.com/chaeng/project_jofcnupz

---

## ✨ 项目特性

- 基于 Retro-Go（ducalex）二次开发
- ESP32-S3 双核高性能主控
- 3.2 寸 TFT LCD（SPI，320×240）
- SD 卡游戏加载
- I2S 数字音频输出
- 完整按键控制系统
- 可编程 WS2812 RGB 状态灯（默认蓝色）
- 兼容小智AI等项目

---

## 🔌 GPIO 引脚定义

SD_CMD - GPIO11  
SD_CLK - GPIO13  
SD_DATA - GPIO9  
SD_CD - GPIO10  

SPK_DOUT - GPIO40  
SPK_BCLK - GPIO41  
SPK_LRCK - GPIO42  

KEY_BOOT - GPIO0  
KEY_MENU - GPIO18  
KEY_OPTION - GPIO8  
KEY_SELECT - GPIO16  
KEY_START - GPIO17  
KEY_A - GPIO15  
KEY_B - GPIO5  
KEY_LEFT - GPIO19  
KEY_RIGHT - GPIO6  
KEY_UP - GPIO7  
KEY_DOWN - GPIO20  

BAT - GPIO4  
TFT_BLK - GPIO39  
TFT_MOSI - GPIO12  
TFT_CLK - GPIO48  
TFT_DC - GPIO47  
TFT_RST - GPIO3  
TFT_CS - GPIO14  

MIC_WS - GPIO1  
MIC_SCK - GPIO2  
MIC_DIN - GPIO21  

STATUS_LED - GPIO38  

---

## 🧠 系统架构

### 硬件架构

ESP32-S3 主控  
├── 3.2寸 TFT 屏幕  
├── SD 卡模块  
├── I2S 音频输出  
├── WS2812 RGB 灯  
├── 按键输入系统  
└── 电池供电系统  

---

### 软件架构

retro-go base  
↓  
retro-go_chaeng port  
├── driver（驱动层）  
├── ui（界面系统）  
├── input（输入系统）  
├── audio（音频系统）  
└── game（游戏运行层）  

---

## 📷 实物展示

<p align="center">
  <img width="4000" height="2252" alt="20260614_092733" src="https://github.com/user-attachments/assets/bc83d44b-4eb2-4a13-a96a-00459077935c" />
  <img width="4000" height="2252" alt="20260614_092746" src="https://github.com/user-attachments/assets/4a60d512-9948-491f-9214-b1fae31ed36a" />
  <img width="4000" height="2252" alt="20260614_092850" src="https://github.com/user-attachments/assets/493972e9-6671-43dd-b9ab-dcd01f111113" />
  <img width="4000" height="2252" alt="20260614_092856" src="https://github.com/user-attachments/assets/da6b5e10-d566-4c00-b279-9d6e204d3767" />
  <img width="4000" height="2252" alt="20260614_093534" src="https://github.com/user-attachments/assets/cb81de19-9f16-447e-9382-b8fb90c6e463" />
  <img width="4000" height="2252" alt="20260614_091542" src="https://github.com/user-attachments/assets/de9b2670-cbf5-4292-be99-fa3f13dae714" />
</p>

---

## 🎯 项目目标

- 构建 ESP32-S3 复古掌机系统  
- 基于 Retro-Go 实现硬件移植  
- 提供完整开源软硬件方案  
- 支持教学 / DIY / 二次开发  
- 可扩展 AI / IoT / 联机能力  

---

## 📜 开源协议

GNU General Public License v3.0 (GPLv3)

✔ 允许使用 / 修改 / 分发  
❗ 必须开源衍生代码  
❗ 必须保留原始版权信息  

---
- WiFi 联机功能  
- AI 语音扩展  
- 性能优化  
