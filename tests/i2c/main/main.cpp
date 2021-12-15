#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c-comp.h"
#include "iot_button.h"

#define TAG "main"

#define SDA_MASTER  1
#define SCL_MASTER  2
#define SDA_SLAVE   4
#define SCL_SLAVE   5
#define SLAVE_ADDRESS  0x05

static i2c I2Cmaster(0, I2C_MODE_MASTER);
static i2c I2CSlave(1, I2C_MODE_SLAVE);

static uint8_t write_event;
static uint8_t read_event;

static uint8_t regs[32][8] = {};

static void button_single_click_cb(void *arg)
{
    ESP_LOGI(TAG, "BUTTON_SINGLE_CLICK");

    uint8_t _buf[256] = {};
    I2Cmaster.read((uint8_t*) _buf, 8, read_event);
    ESP_LOG_BUFFER_HEX(TAG, _buf, 8);

    read_event++;
}

static void button_double_click_cb(void *arg)
{
    ESP_LOGI(TAG, "BUTTON_DOUBLE_CLICK");
    
    uint8_t _buf[256] = {write_event, write_event, write_event};
    I2Cmaster.write((uint8_t*) _buf, 8, write_event);

    write_event++;
}

IRAM_ATTR void receiveEvent(int evt, uint8_t *_data, size_t _len)
{
    printf("received data len: %d\n", _len);
    memcpy(regs[evt], _data, 8);
    ESP_LOG_BUFFER_HEX(TAG, _data, _len);
}

extern "C" void app_main(void)
{
    bool ready = I2CSlave.begin(SDA_SLAVE, SCL_SLAVE, SLAVE_ADDRESS);
    if (!ready)
    {
        printf("I2C slave init failed\n");
    }
    printf("Slave joined I2C bus with addr #%d\n", SLAVE_ADDRESS);

    if(I2Cmaster.begin(SDA_MASTER, SCL_MASTER, SLAVE_ADDRESS))
        printf("begin OK\n");

    I2Cmaster.scan();
    // create gpio button
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = 0,
            .active_level = 0,
        },
    };
    button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);
    if(NULL == gpio_btn) {
        ESP_LOGE(TAG, "Button create failed");
    }

    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_single_click_cb);
    iot_button_register_cb(gpio_btn, BUTTON_DOUBLE_CLICK, button_double_click_cb);

    printf("count: %d\n\n", I2Cmaster.scan());
    while(1)
    {
        uint8_t buf[256] = {};
        int len = I2CSlave.get(buf);

        if (len == 1)
        {
            I2CSlave.set(&regs[buf[0]][0], 8);
        } else if (len > 0) {
            receiveEvent(buf[0], &buf[1], len);
        }
        vTaskDelay(10);
    }
}
