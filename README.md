<<<<<<< HEAD
# Table of contents
- [Description](#description)
- [Installation](#installation)
- [Usage](#usage)
- [Issues](#issues)
- [Development](#development)
- [Acknowledgements](#acknowledgements)
- [License](#license)

# Description
Retro-Go is a firmware to play retro games on ESP32-based devices (officially supported are
ODROID-GO and MRGC-G32, check [this list for other devices](components/retro-go/README.md)).
The project consists of a launcher and half a dozen applications that have been heavily
optimized to reduce their cpu, memory, and flash needs without reducing compatibility!

### Supported systems:
- Nintendo: **NES, SNES (slow), Gameboy, Gameboy Color, Game & Watch**
- Sega: **SG-1000, Master System, Mega Drive / Genesis, Game Gear**
- Coleco: **Colecovision**
- NEC: **PC Engine**
- Atari: **Lynx**
- Others: **DOOM** (including mods!)

### Retro-Go features:
- In-game menu
- Favorites and recently played
- GB color palettes, RTC adjust and save
- NES color palettes, PAL roms, NSF support
- More emulators and applications
- Scaling and filtering options
- Better performance and compatibility
- Turbo Speed/Fast forward
- Customizable launcher
- Cover art and save state previews
- Multiple save slots per game
- Wifi file manager
- And more!

### Screenshots
![Preview](assets/retro-go-preview.jpg)


# Installation

### ODROID-GO
  1. Download `retro-go_1.x_odroid-go.fw` from the [release page](https://github.com/ducalex/retro-go/releases/) and copy it to `/odroid/firmware` on your sdcard.
  2. Power up the device while holding down B.
  3. Select retro-go in the files list and flash it.

### MyRetroGameCase G32 (GBC)
  1. Download `retro-go_1.x_mrgc-g32.fw` from the [release page](https://github.com/ducalex/retro-go/releases/) and copy it to `/espgbc/firmware` on your sdcard.
  2. Power up the device while holding down MENU (the volume knob).
  3. Select retro-go in the files list and flash it.

### Other devices
  1. Download the .img for your device from the [release page](https://github.com/ducalex/retro-go/releases/).
  2. Connect your device to a computer with a USB cable.
  3. Flash the image with esptool:
     - [Command line](https://github.com/espressif/esptool/releases/): Run `esptool.py write_flash --flash_size detect 0x0 retro-go_*.img`
     - [Web version](https://espressif.github.io/esptool-js/): Connect your device, click Erase Flash, then select your .img file and set address to 0x0, finally click Program)

Your particular device may require extra steps (like holding a button during power up) or different esptool flags or a special cable. If the above steps fail, you might need to ask the manufacturer for instructions on how to flash new firmware!

If your device is not already supported or if a prebuilt version isn't available for it you can check the [development section](#Development) for more information on how to build for your device.


# Usage

## Game covers / artwork
Game covers should be placed in the `romart` folder at the base of your sd card. You can obtain a pre-made pack [here](https://github.com/ducalex/retro-go-covers). Retro-Go is also compatible with the older Go-Play romart pack.

You can add missing cover art by creating a PNG image (160x168, 8bit). Two naming schemes are supported:
- Filename-based: `/romart/nes/Super Mario.png` (notice the rom extension is *not* included)
- CRC32-based: `/romart/nes/A/ABCDE123.png` where `nes` is the same as the rom folder, and `ABCDE123` is the CRC32 of the game (press A -> Properties in the launcher to find it), and `A` is the first character of the CRC32

_Note: CRC32-based, which is what is used in the pre-made pack, is much slower than name-based! This type is useful because filenames vary greatly despite having identical CRCs, but if you generate your own art I suggest you use filename-based format and delete all CRC-based art from your SD Card to improve responsiveness._


## BIOS files
Some emulators support loading a BIOS. The files should be placed as follows:
- GB: `/retro-go/bios/gb_bios.bin`
- GBC: `/retro-go/bios/gbc_bios.bin`
- FDS: `/retro-go/bios/fds_bios.bin`
- MSX: In folder `/retro-go/bios/msx/` put: `MSX.ROM` `MSX2.ROM` `MSX2EXT.ROM` `MSX2P.ROM` `MSX2PEXT.ROM` `FMPAC.ROM` `DISK.ROM` `MSXDOS2.ROM` `PAINTER.ROM` `KANJI.ROM`


## Game & Watch
The roms must be packed with [LCD-Game-Shrinker](https://github.com/bzhxx/LCD-Game-Shrinker) and a tutorial can be [found here](https://gist.github.com/DNA64/16fed499d6bd4664b78b4c0a9638e4ef).


## Wifi
To use wifi you will need to create a `/retro-go/config/wifi.json` config file. You can define up to 4 different networks, then selectable in the menu. Its content should look like this:

````json
{
  "ssid0": "my-network",
  "password0": "my-password",
  "ssid1": "my-other-network",
  "password1": "my-password",
  "ssid2": "my-third-network",
  "password2": "my-password",
  "ssid3": "my-last-network",
  "password3": "my-password"
}
````

### Time synchronization
Time synchronization happens in the launcher immediately after a successful connection to the network.
This is done via NTP by contacting `pool.ntp.org` and cannot be disabled at this time.
Timezone can be configured in the launcher's options menu.

### File manager
You can find the IP of your device in the *about* menu of retro-go. Then on your PC navigate to
http://192.168.x.x/ to access the file manager.


## External DAC (headphones)

Retro-Go supports [the external DAC mod for the ODROID-GO](https://github.com/backofficeshow/odroid-go-audio-hat)
which allows high quality audio through headphones. You can switch to it in the menu `Audio Out: Ext DAC`.

<details>
  <summary>Pinout</summary>

  | GO PIN | PCM5102A PIN |
  |--------|---------|
  | 1 | GND |
  | 2 | - |
  | 3 | LCK |
  | 4 | DIN |
  | 5 | BCK |
  | 6 | VIN |
  | 7 | - |
  | 8 | - |
  | 9 | - |
  | 10 | - |
</details>


# Issues

### Black screen / Boot loops
Retro-Go typically detects and resolves application crashes and freezes automatically. However, if you do
get stuck in a boot loop, you can hold `DOWN` while powering up the device to return to the launcher.

### Sound quality
The volume isn't correctly attenuated on the GO, resulting in upper volume levels that are too loud and
lower levels that are distorted due to DAC resolution. A quick way to improve the audio is to cut one
of the speaker wire and add a `33 Ohm (or thereabout)` resistor in series. Soldering is better but not
required, twisting the wires tightly will work just fine.
[A more involved solution can be seen here.](https://wiki.odroid.com/odroid_go/silent_volume)
Alternatively you can use the headphones DAC mod mentioned earlier in this document.

### Game Boy SRAM *(aka Save/Battery/Backup RAM)*
In Retro-Go, save states will provide you with the best and most reliable save experience. That being said, please
read on if you need or want SRAM saves. The SRAM format is compatible with VisualBoyAdvance so it may be used to
import or export saves.

You can configure automatic SRAM saving in the options menu. A longer delay will reduce stuttering at the cost
of losing data when powering down too quickly. Also note that when *resuming* a game, Retro-Go will give priority
to a save state if present.

### ZIP files
Most Retro-Go applications now support ZIP files. ZIP archives should contain only one ROM file and nothing else. ZIP support also depends on available memory and larger ROMs may fail to load on some devices unfortunately.


# Development
If you wish to build or modify Retro-Go, you can find help in the following documents:

- Build instructions in [BUILDING.md](BUILDING.md)
- Theming instructions [THEMING.md](THEMING.md)
- Porting instructions in [PORTING.md](PORTING.md)
- Translating instructions in [LOCALIZATION.md](LOCALIZATION.md)


# Acknowledgements
- The NES/GBC/SMS emulators and base library were originally from the "Triforce" fork of the [official Go-Play firmware](https://github.com/othercrashoverride/go-play) by crashoverride, Nemo1984, and many others.
- The design of the launcher was originally inspired/copied from [pelle7's go-emu](https://github.com/pelle7/odroid-go-emu-launcher).
- PCE-GO is a fork of [HuExpress](https://github.com/kallisti5/huexpress) and [pelle7's port](https://github.com/pelle7/odroid-go-pcengine-huexpress/) was used as reference.
- The Lynx emulator is a port of [libretro-handy](https://github.com/libretro/libretro-handy).
- The SNES emulator is a port of [Snes9x 2005](https://github.com/libretro/snes9x2005).
- The DOOM engine is a port of [PrBoom 2.5.0](http://prboom.sourceforge.net/).
- The Genesis emulator is a port of [Gwenesis](https://github.com/bzhxx/gwenesis/) by bzhxx.
- The Game & Watch emulator is a port of [lcd-game-emulator](https://github.com/bzhxx/lcd-game-emulator) by bzhxx.
- The MSX emulator is a port of [fMSX](https://fms.komkon.org/fMSX/) by Marat Fayzullin.
- PNG support is provided by [lodepng](https://github.com/lvandeve/lodepng/).
- PCE cover art is from [Christian_Haitian](https://github.com/christianhaitian).
- Some icons from [Rokey](https://iconarchive.com/show/seed-icons-by-rokey.html).
- Background images from [es-theme-gbz35](https://github.com/rxbrad/es-theme-gbz35).
- Special thanks to [RGHandhelds](https://www.rghandhelds.com/) and [MyRetroGamecase](https://www.myretrogamecase.com/) for sending me a [G32](https://www.myretrogamecase.com/products/game-mini-g32-esp32-retro-gaming-console-1) device.
- The [ODROID-GO](https://forum.odroid.com/viewtopic.php?f=159&t=37599) community for encouraging the development of retro-go!

# License
Everything in this project is licensed under the [GPLv2 license](COPYING) with the exception of the following components:
- fmsx/components/fmsx (MSX Emulator, custom non-commercial license)
- handy-go/components/handy (Lynx emulator, zlib)
=======
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
>>>>>>> 27f56b10fa45a1c1c599680f6b7b2c75af2a7580
