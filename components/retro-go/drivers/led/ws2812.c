#include "ws2812.h"
#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"

#define WS2812_RESOLUTION_HZ 10000000

typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} ws2812_encoder_t;

static rmt_channel_handle_t led_chan;
static rmt_encoder_handle_t led_encoder;
static bool ws2812_ready;

RMT_ENCODER_FUNC_ATTR
static size_t ws2812_encode(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data,
                            size_t data_size, rmt_encode_state_t *ret_state)
{
    ws2812_encoder_t *led_encoder = __containerof(encoder, ws2812_encoder_t, base);
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;

    switch (led_encoder->state) {
    case 0:
        encoded_symbols += led_encoder->bytes_encoder->encode(
            led_encoder->bytes_encoder, channel, primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE)
            led_encoder->state = 1;
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    // fall-through
    case 1:
        encoded_symbols += led_encoder->copy_encoder->encode(
            led_encoder->copy_encoder, channel, &led_encoder->reset_code,
            sizeof(led_encoder->reset_code), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = RMT_ENCODING_RESET;
            state |= RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL)
            state |= RMT_ENCODING_MEM_FULL;
    }
out:
    *ret_state = state;
    return encoded_symbols;
}

static esp_err_t ws2812_encoder_del(rmt_encoder_t *encoder)
{
    ws2812_encoder_t *led_encoder = __containerof(encoder, ws2812_encoder_t, base);
    rmt_del_encoder(led_encoder->bytes_encoder);
    rmt_del_encoder(led_encoder->copy_encoder);
    free(led_encoder);
    return ESP_OK;
}

RMT_ENCODER_FUNC_ATTR
static esp_err_t ws2812_encoder_reset(rmt_encoder_t *encoder)
{
    ws2812_encoder_t *led_encoder = __containerof(encoder, ws2812_encoder_t, base);
    rmt_encoder_reset(led_encoder->bytes_encoder);
    rmt_encoder_reset(led_encoder->copy_encoder);
    led_encoder->state = RMT_ENCODING_RESET;
    return ESP_OK;
}

static esp_err_t ws2812_new_encoder(uint32_t resolution, rmt_encoder_handle_t *ret_encoder)
{
    ws2812_encoder_t *led_encoder = rmt_alloc_encoder_mem(sizeof(ws2812_encoder_t));
    if (!led_encoder)
        return ESP_ERR_NO_MEM;

    led_encoder->base.encode = ws2812_encode;
    led_encoder->base.del = ws2812_encoder_del;
    led_encoder->base.reset = ws2812_encoder_reset;

    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = 0.3 * resolution / 1000000,
            .level1 = 0,
            .duration1 = 0.9 * resolution / 1000000,
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = 0.9 * resolution / 1000000,
            .level1 = 0,
            .duration1 = 0.3 * resolution / 1000000,
        },
        .flags.msb_first = 1,
    };

    esp_err_t err = rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder->bytes_encoder);
    if (err != ESP_OK) {
        free(led_encoder);
        return err;
    }

    err = rmt_new_copy_encoder(&(rmt_copy_encoder_config_t){}, &led_encoder->copy_encoder);
    if (err != ESP_OK) {
        rmt_del_encoder(led_encoder->bytes_encoder);
        free(led_encoder);
        return err;
    }

    uint32_t reset_ticks = resolution / 1000000 * 50 / 2;
    led_encoder->reset_code = (rmt_symbol_word_t){
        .level0 = 0,
        .duration0 = reset_ticks,
        .level1 = 0,
        .duration1 = reset_ticks,
    };

    *ret_encoder = &led_encoder->base;
    return ESP_OK;
}

static bool ws2812_flush(const uint8_t grb[3])
{
    if (!ws2812_ready)
        return false;

    rmt_transmit_config_t tx_config = {.loop_count = 0};
    if (rmt_transmit(led_chan, led_encoder, grb, 3, &tx_config) != ESP_OK)
        return false;
    return rmt_tx_wait_all_done(led_chan, 100) == ESP_OK;
}

bool ws2812_init(int gpio_num)
{
    if (ws2812_ready)
        return true;

    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = gpio_num,
        .mem_block_symbols = 64,
        .resolution_hz = WS2812_RESOLUTION_HZ,
        .trans_queue_depth = 2,
    };

    if (rmt_new_tx_channel(&tx_chan_config, &led_chan) != ESP_OK)
        return false;
    if (ws2812_new_encoder(WS2812_RESOLUTION_HZ, &led_encoder) != ESP_OK)
        return false;
    if (rmt_enable(led_chan) != ESP_OK)
        return false;

    ws2812_ready = true;
    return ws2812_set_rgb(0, 0, 0);
}

void ws2812_deinit(void)
{
    if (!ws2812_ready)
        return;

    ws2812_set_rgb(0, 0, 0);
    rmt_disable(led_chan);
    rmt_del_encoder(led_encoder);
    rmt_del_channel(led_chan);
    ws2812_ready = false;
}

bool ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
#ifndef RG_WS2812_BRIGHTNESS
#define RG_WS2812_BRIGHTNESS 64
#endif
    const uint32_t scale = RG_WS2812_BRIGHTNESS;
    const uint8_t grb[3] = {
        (uint8_t)((g * scale) / 255),
        (uint8_t)((r * scale) / 255),
        (uint8_t)((b * scale) / 255),
    };
    return ws2812_flush(grb);
}

bool ws2812_set_rgb565(uint16_t color)
{
    if (color == 0 || (int16_t)color < 0)
        return ws2812_set_rgb(0, 0, 0);

    const uint8_t r = ((color >> 11) & 0x1F) << 3;
    const uint8_t g = ((color >> 5) & 0x3F) << 2;
    const uint8_t b = (color & 0x1F) << 3;
    return ws2812_set_rgb(r, g, b);
}
