#pragma once

#include <stdint.h>
#include <stdbool.h>

bool ws2812_init(int gpio_num);
bool ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b);
bool ws2812_set_rgb565(uint16_t color);
void ws2812_deinit(void);
