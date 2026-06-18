# chaeng (T320-S3)

ESP32-S3 handheld **chaeng**, hardware: **T320B7-C12-16** 3.2" IPS TFT (ST7789, 320×240).

Build target: `--target=t320-s3`  
Firmware image: **`retro-go_chaeng_t320-s3.img`**

## Hardware

| Function | GPIO |
|----------|------|
| SD CMD (MOSI) | 11 |
| SD CLK | 13 |
| SD DATA (MISO) | 9 |
| SD CS / CD | 10 |
| I2S DOUT | 40 |
| I2S BCLK | 41 |
| I2S LRCK | 42 |
| KEY BOOT | 0 (Menu; hold at boot for recovery) |
| KEY MENU | 18 |
| KEY OPTION | 8 |
| KEY SELECT | 16 |
| KEY START | 17 |
| KEY A / B | 15 / 5 |
| D-pad | 19 / 6 / 7 / 20 |
| Battery ADC | 4 |
| TFT BLK | 39 |
| TFT MOSI / CLK / DC / RST / CS | 12 / 48 / 47 / 3 / 14 |
| Status LED | 38 (WS2812 / XL-2020RGBC-2812B) |
| MIC WS / SCK / DIN | 1 / 2 / 21 (unused by Retro-Go) |

## Build

```bash
./rg_tool.py --target=t320-s3 build-img
./rg_tool.py --target=t320-s3 --port=COMx install
```

Display tuning: change `RG_SCREEN_ROTATION` (0–7) and `RG_SCREEN_RGB_BGR` (0/1) in `config.h`. Current: rotation `5` → MADCTL `0xA0` (MY|MV|BGR).
