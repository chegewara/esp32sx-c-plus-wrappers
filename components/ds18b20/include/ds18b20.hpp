#pragma once
#include "owb.h"
#include "owb_rmt.h"
#include "owb_gpio.h"
#include "ds18b20.h"

class DS18B20
{
private:
    uint8_t num_devices;
    uint8_t max_num_devices;
    OneWireBus * owb;
    owb_rmt_driver_info rmt_driver_info;
    OneWireBus_ROMCode* device_rom_codes;
    DS18B20_Info** devices;
    float* readings;
    DS18B20_ERROR* errors;
    DS18B20_RESOLUTION resolution;
public:
    DS18B20(uint8_t count = 1, DS18B20_RESOLUTION resol = DS18B20_RESOLUTION::DS18B20_RESOLUTION_12_BIT);
    ~DS18B20();

public:
    esp_err_t init(gpio_num_t pin, rmt_channel_t tx_channel = RMT_CHANNEL_0, rmt_channel_t rx_channel = RMT_CHANNEL_4);
    uint8_t search(bool use_crc = true);
    uint8_t read();
    void addSensors(uint8_t num = 1);
    void removeSensors(uint8_t num = 1);
    float getTemp(uint8_t num = 0);
    uint8_t getCount();

private:
    esp_err_t allocate();
    void deallocate();
};
