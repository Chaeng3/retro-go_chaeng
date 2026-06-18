// Target definition
#define RG_TARGET_NAME             "chaeng"
#define RG_PROJECT_NAME            "chaeng"

// Storage (SDSPI: CMD/MOSI=11, CLK=13, MISO=9, CS/CD=10)
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI3_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT

// Audio (I2S: DOUT=40, BCLK=41, LRCK=42)
#define RG_AUDIO_USE_INT_DAC        0
#define RG_AUDIO_USE_EXT_DAC        1

// Video — T320B7-C12-16: 3.2" IPS, ST7789, 240(RGB)×320, landscape 320×240
#define RG_SCREEN_DRIVER            0   // ILI9341/ST7789
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_VISIBLE_AREA      {0, 0, 0, 0}
#define RG_SCREEN_SAFE_AREA         {0, 0, 0, 0}
// MADCTL via ili9341.h: (ROTATION<<5)|BGR. 5=MY|MV → 0xA0 (landscape); RGB (BGR=0) fixes red/blue swap.
#define RG_SCREEN_ROTATION          5
#define RG_SCREEN_RGB_BGR           0
#define RG_SCREEN_INIT()                                                                                         \
    ILI9341_CMD(0xCF, 0x00, 0xc3, 0x30);                                                                         \
    ILI9341_CMD(0xED, 0x64, 0x03, 0x12, 0x81);                                                                   \
    ILI9341_CMD(0xE8, 0x85, 0x00, 0x78);                                                                         \
    ILI9341_CMD(0xCB, 0x39, 0x2c, 0x00, 0x34, 0x02);                                                             \
    ILI9341_CMD(0xF7, 0x20);                                                                                     \
    ILI9341_CMD(0xEA, 0x00, 0x00);                                                                               \
    ILI9341_CMD(0xC0, 0x1B);                                                                                     \
    ILI9341_CMD(0xC1, 0x12);                                                                                     \
    ILI9341_CMD(0xC5, 0x32, 0x3C);                                                                               \
    ILI9341_CMD(0xC7, 0x91);                                                                                     \
    ILI9341_CMD(0xB1, 0x00, 0x10);                                                                               \
    ILI9341_CMD(0xB6, 0x0A, 0xA2);                                                                               \
    ILI9341_CMD(0xF6, 0x01, 0x30);                                                                               \
    ILI9341_CMD(0xF2, 0x00);                                                                                     \
    ILI9341_CMD(0x26, 0x01);                                                                                     \
    ILI9341_CMD(0xE0, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00); \
    ILI9341_CMD(0x21);                       /* INVON — IPS normally-black, fixes inverted colors */              \
    ILI9341_CMD(0xE1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F);

// Input — GPIO keys, active low with pull-up
// Hold KEY_BOOT (GPIO0) or KEY_MENU (GPIO18) at power-on for recovery
#define RG_RECOVERY_BTN             RG_KEY_MENU
#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_MENU,   .num = GPIO_NUM_0,  .pullup = 1, .level = 0},\
    {RG_KEY_MENU,   .num = GPIO_NUM_18, .pullup = 1, .level = 0},\
    {RG_KEY_UP,     .num = GPIO_NUM_7,  .pullup = 1, .level = 0},\
    {RG_KEY_RIGHT,  .num = GPIO_NUM_6,  .pullup = 1, .level = 0},\
    {RG_KEY_DOWN,   .num = GPIO_NUM_20, .pullup = 1, .level = 0},\
    {RG_KEY_LEFT,   .num = GPIO_NUM_19, .pullup = 1, .level = 0},\
    {RG_KEY_SELECT, .num = GPIO_NUM_16, .pullup = 1, .level = 0},\
    {RG_KEY_START,  .num = GPIO_NUM_17, .pullup = 1, .level = 0},\
    {RG_KEY_OPTION, .num = GPIO_NUM_8,  .pullup = 1, .level = 0},\
    {RG_KEY_A,      .num = GPIO_NUM_15, .pullup = 1, .level = 0},\
    {RG_KEY_B,      .num = GPIO_NUM_5,  .pullup = 1, .level = 0},\
}

// Battery (BAT = GPIO4 → ADC1_CH3)
#define RG_BATTERY_DRIVER           1
#define RG_BATTERY_ADC_UNIT         ADC_UNIT_1
#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_3
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// XL-2020RGBC-2812B (WS2812) on GPIO38 — RMT, GRB order
#define RG_LED_WS2812             1
#define RG_WS2812_BRIGHTNESS        48
#define RG_GPIO_LED                 GPIO_NUM_38

// SPI TFT (ST7789)
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI            GPIO_NUM_12
#define RG_GPIO_LCD_CLK             GPIO_NUM_48
#define RG_GPIO_LCD_CS              GPIO_NUM_14
#define RG_GPIO_LCD_DC              GPIO_NUM_47
#define RG_GPIO_LCD_BCKL            GPIO_NUM_39
#define RG_GPIO_LCD_RST             GPIO_NUM_3

// SPI SD
#define RG_GPIO_SDSPI_MISO          GPIO_NUM_9
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_11
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_13
#define RG_GPIO_SDSPI_CS            GPIO_NUM_10

// I2S speaker
#define RG_GPIO_SND_I2S_BCK         41
#define RG_GPIO_SND_I2S_WS          42
#define RG_GPIO_SND_I2S_DATA        40

// MIC (WS=1, SCK=2, DIN=21) — not used by Retro-Go

// Simplified Chinese UI + Fusion Pixel font (SD card UTF-8 filenames need sdkconfig FATFS UTF-8)
#define RG_CHINESE_SUPPORT          1
#define RG_FONT_CHINESE             RG_FONT_FUSIONPIXEL_12
#define RG_LANG_DEFAULT             RG_LANG_CHS
