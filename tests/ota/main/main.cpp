#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "nvs_comp.h"
#include "ota.h"
#include "iot_button.h"
#include "http-client.h"
#include "wifi.h"

#ifdef CONFIG_IDF_TARGET_ESP32C3
#define GPIO_PIN 9
#else
#define GPIO_PIN 0
#endif

#define TAG "main"

#define FIRMWARE_URL "http://192.168.0.5/ota/firmware.bin"
#define SSID "esp32"
#define PASSWORD "espressif"

NVS nvs;
OTA ota;
HttpClient client;
WiFi* wifi_itf;

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "station connected");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "station disconnected");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        char myIP[20];
        memset(myIP, 0, 20);
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        sprintf(myIP, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "got ip: %s", myIP );
    }
}

static void button_single_click_cb(void *arg)
{
    ESP_LOGI("", "BUTTON_SINGLE_CLICK");

    ota.init();
    ota.begin();
    
    esp_http_client_config_t config = {};
    config.url = FIRMWARE_URL;
    client.init(&config);

    client.open();
    client.getHeaders();
    printf("status: %d, length: %lld\n", client.status(), client.length());

    if(client.status() != 200) return;

    int len = 0;
    int total = 0;
    char buf[1025] = {};
    size_t _len = 1024;
    do
    {
        len = client.read(buf, _len);
        total += len;
        ota.write((uint8_t*)buf, _len);
    } while (len > 0);

    client.cleanup();
    printf("status: %d, total: %d\n", client.status(), total);

    ota.finish();
    ESP_LOGI("", "prepare to restart");
    esp_restart();
}

extern "C" void app_main(void)
{
    nvs.init();

    // create gpio button
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = GPIO_PIN,
            .active_level = 0,
        },
    };
    button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);
    if(NULL == gpio_btn) {
        ESP_LOGE("", "Button create failed");
    }

    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_single_click_cb);

    esp_log_level_set("wifi", ESP_LOG_NONE);
    esp_log_level_set("wifi_init", ESP_LOG_NONE);

    wifi_itf = new WiFi();
    wifi_itf->init();

    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableSTA(SSID, PASSWORD, true));
}
