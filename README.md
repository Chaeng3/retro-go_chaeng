🎮 retro-go_chaeng（ESP32-S3 复古掌机）
<p align="center">
<img width="4000" height="2252" alt="20260614_092928" src="https://github.com/user-attachments/assets/21fac5f2-6500-4e06-944a-c8fae49e23ee" />
</p>
基于 Retro-Go（ducalex）移植的 ESP32-S3 复古掌机项目<br/>
Retro-Go 中文硬件扩展版本（chaeng 定制）
</p>
---
<p align="center">
![Base](https://img.shields.io/badge/Base-Retro--Go%20fork-orange)
![Platform](https://img.shields.io/badge/ESP32--S3-blue)
![Display](https://img.shields.io/badge/LCD-3.2inch%20TFT-green)
![Audio](https://img.shields.io/badge/I2S-Audio-yellow)
![RGB](https://img.shields.io/badge/WS2812-RGB-red)
![Language](https://img.shields.io/badge/中文化-YES-orange)
![License](https://img.shields.io/badge/GPL-v3.0-blue)
</p>
---
📺 项目演示
👉 https://www.bilibili.com/video/BV1SQJF6KEsp/?vd_source=3ac89604b734514d5955b607a3f43d69
---
📌 硬件开源
👉 https://oshwhub.com/chaeng/project_jofcnupz
---
✨ 项目特性
ESP32-S3 双核高性能平台
3.2 寸 TFT LCD（320×240）
SD 卡游戏加载系统
I2S 数字音频输出
完整按键控制系统
WS2812 RGB 状态灯（GPIO38）
可扩展 WiFi / AI / 联机功能
---
🔌 GPIO 引脚定义
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
🧠 系统架构
<p align="center"><b>硬件架构</b></p>
ESP32-S3  
├── 3.2" TFT LCD  
├── SD Card  
├── I2S Audio  
├── WS2812 RGB LED  
├── Buttons  
└── Battery System
---
<p align="center"><b>软件架构</b></p>
retro-go base  
↓  
retro-go_chaeng port  
├── driver layer  
├── ui system  
├── input system  
├── audio system  
└── game runtime
---
📷 实物展示
<p align="center">
  <img src="assets/cover.jpg" width="80%">
</p>
---
🎯 项目目标
构建 ESP32-S3 复古掌机系统
基于 Retro-Go 实现硬件移植
提供完整开源软硬件方案
支持教学 / DIY / 二次开发
可扩展 AI / IoT / 联机能力
---
📜 开源协议
GNU General Public License v3.0 (GPLv3)
✔ 可使用 / 修改 / 分发  
❗ 必须开源衍生代码  
❗ 必须保留 GPL 协议
---
🚀 Roadmap
UI 系统优化
AI 语音扩展
