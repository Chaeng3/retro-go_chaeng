# 🎮 retro-go_chaeng（ESP32-S3 复古掌机）

<p align="center">
  <img width="4000" height="2252" alt="20260614_092928" src="https://github.com/user-attachments/assets/4de8d9e6-f720-4a8c-8276-cfdc19873c3e" />
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

## 📌 硬件开源地址（该项目已加入立创开源硬件平台“星火计划2026”）

https://oshwhub.com/chaeng/project_jofcnupz

---

## ✨ 项目特性

- 基于 Retro-Go（ducalex）二次开发
- ESP32-S3 双核高性能主控
- 3.2 寸 TFT LCD（SPI，320×240）
- SD 卡游戏加载
- I2S 数字音频输出
- 完整按键控制系统
- 可编程WS2812 RGB 状态灯（默认为蓝色）
- 可扩展小智AI等项目

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
  <img width="4000" height="2252" alt="20260614_092733" src="https://github.com/user-attachments/assets/fc061ba3-564f-49fc-a8e4-8b62e2d24a83" />
  <img width="4000" height="2252" alt="20260614_092746" src="https://github.com/user-attachments/assets/cc8ba6ae-2ef8-4769-ac38-57009eeaa69a" />
  <img width="4000" height="2252" alt="20260614_092850" src="https://github.com/user-attachments/assets/7c8fed1e-9dde-47c7-90db-e8696597fdfa" />
  <img width="4000" height="2252" alt="20260614_093550" src="https://github.com/user-attachments/assets/89ea2591-b8ef-4413-b5ae-eb959fe85d78" />
  <img width="4000" height="2252" alt="20260614_091806" src="https://github.com/user-attachments/assets/317503aa-1960-4bc0-93d7-76a28625c5a6" />
  <img width="4000" height="2252" alt="20260614_091542" src="https://github.com/user-attachments/assets/49615952-6e7b-415b-ad30-2102002bfdc2" />
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
