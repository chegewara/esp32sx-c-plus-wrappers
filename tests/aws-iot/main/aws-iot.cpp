#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "wifi.h"
#include "nvs_comp.h"
#include "aws-iot.h"

#include "main.h"

#define TAG "main"

WiFi* wifi;
NVS nvs;


void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI("", "wifi base: %s, event: %d", event_base, event_id);
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi->connect();
    }
}

extern "C" void app_main(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE);
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs.init());
    wifi = new WiFi();
    wifi->init();

    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi->enableSTA(WIFI_SSID, WIFI_PASS, true));

    connect_aws();
    connect_shadow();

    while (1)
    {
        IoT_Error_t rc = awsMqtt.status();
        if(rc != SUCCESS)
        {
            ESP_LOGW(TAG, "last mqtt error: %d", rc);
        }
        rc = awsShadow.status();
        if(rc != SUCCESS)
        {
            ESP_LOGW(TAG, "last shadow error: %d", rc);
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
