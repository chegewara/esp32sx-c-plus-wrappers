#include <stdio.h>
#include "esp_log.h"

#include "ds18b20.hpp"
#define TAG "ds18b20"

DS18B20::DS18B20(uint8_t count, DS18B20_RESOLUTION resol)
{
    max_num_devices = count;
    resolution = resol;
    allocate();
}

DS18B20::~DS18B20()
{
    for (int i = 0; i < num_devices; ++i)
    {
        ds18b20_free(&devices[i]);
    }
    owb_uninitialize(owb);
    deallocate();
}

esp_err_t DS18B20::allocate()
{
    device_rom_codes = (OneWireBus_ROMCode*)calloc(max_num_devices, sizeof(OneWireBus_ROMCode));
    if(device_rom_codes == NULL) return ESP_ERR_NO_MEM;
    devices = (DS18B20_Info**)calloc(max_num_devices, sizeof(DS18B20_Info*));
    if(devices == NULL) return ESP_ERR_NO_MEM;
    readings = (float*)calloc(max_num_devices, sizeof(float));
    if(readings == NULL) return ESP_ERR_NO_MEM;
    errors = (DS18B20_ERROR*)calloc(max_num_devices, sizeof(DS18B20_ERROR));
    if(errors == NULL) return ESP_ERR_NO_MEM;

    return ESP_OK;
}

void DS18B20::deallocate()
{
    free(device_rom_codes);
    free(devices);
    free(readings);
    free(errors);
}

esp_err_t DS18B20::init(gpio_num_t pin, rmt_channel_t tx_channel, rmt_channel_t rx_channel)
{
    owb = owb_rmt_initialize(&rmt_driver_info, pin, tx_channel, rx_channel);
    if(owb == NULL) return ESP_FAIL;
    search();

    return ESP_OK;
}

uint8_t DS18B20::search(bool use_crc)
{
    for (int i = 0; i < num_devices; ++i)
    {
        ds18b20_free(&devices[i]);
    }
    num_devices = 0;

    deallocate();
    if(allocate()) {
        ESP_LOGE(TAG, "failed to allocate memory");
        return 0;
    }

    OneWireBus_SearchState search_state = {};
    bool found = false;

    owb_search_first(owb, &search_state, &found);
    while (found && num_devices < max_num_devices)
    {
        char rom_code_s[17];
        owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
        ESP_LOGD(TAG, "  %d : %s", num_devices, rom_code_s);
        device_rom_codes[num_devices] = search_state.rom_code;
        ++num_devices;
        owb_search_next(owb, &search_state, &found);
    }
    ESP_LOGD(TAG, "Found %d device%s", num_devices, num_devices == 1 ? "" : "s");

    for (int i = 0; i < num_devices; ++i)
    {
        DS18B20_Info * ds18b20_info = ds18b20_malloc();
        devices[i] = ds18b20_info;

        if (num_devices == 1)
        {
            ESP_LOGD(TAG, "Single device optimisations enabled\n");
            ds18b20_init_solo(ds18b20_info, owb);
        }
        else
        {
            ds18b20_init(ds18b20_info, owb, device_rom_codes[i]);
        }
        if(use_crc) ds18b20_use_crc(ds18b20_info, true);
        ds18b20_set_resolution(ds18b20_info, resolution);
    }

    bool parasitic_power = false;
    ds18b20_check_for_parasite_power(owb, &parasitic_power);
    if (parasitic_power) {
        ESP_LOGW(TAG, "Parasitic-powered devices detected\n");
    }
    owb_use_parasitic_power(owb, parasitic_power);

    return num_devices;
}

uint8_t DS18B20::read()
{
    uint8_t errors_count = 0;
    if (num_devices > 0)
    {
        ds18b20_convert_all(owb);
        ds18b20_wait_for_conversion(devices[0]);

        ESP_LOGD(TAG, "Temperature readings (degrees C):");
        for (int i = 0; i < num_devices; ++i)
        {
            errors[i] = ds18b20_read_temp(devices[i], &readings[i]);
            if(errors[i] != DS18B20_OK) errors_count++;
            ESP_LOGD(TAG, "\t%d: %.2f\t%s", i, readings[i], errors[i] != DS18B20_OK?"error":"");
        }
    }

    return errors_count;
}

void DS18B20::addSensors(uint8_t num)
{
    max_num_devices += num;
    search();
}

void DS18B20::removeSensors(uint8_t num)
{
    max_num_devices -= num;
    search();
}

float DS18B20::getTemp(uint8_t num)
{
    if (num >= num_devices || errors[num] != DS18B20_OK)
    {
        return -253.0f;
    }

    return readings[num];
}

uint8_t DS18B20::getCount()
{
    return num_devices;
}

