#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi.h"
#include "esp_log.h"
#include "nvs_comp.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "sntp-client.h"

#define TAG "main"

#define SSID "esp32"
#define PASSWORD "espressif"
#define AP1_SSID "test123"
#define AP2_SSID "test1"

WiFi* wifi_itf;
NVS nvs;
SNTP sntp;

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI("", "wifi event: %d", event_id);
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
        sntp.init();
    }
}

static void evt_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGW("", "Base event: %s, event ID: %d", event_base, event_id);
}

extern "C" void app_main(void)
{
    esp_event_loop_create_default();
    esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, evt_handler, NULL);

    esp_log_level_set("wifi", ESP_LOG_NONE);
    ESP_ERROR_CHECK(nvs.init());
    wifi_itf = new WiFi();
    wifi_itf->init();

    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableAP(AP1_SSID));
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableAP(AP2_SSID));
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableSTA());
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->setHostname(AP2_SSID, false));

    uint16_t count = 0;
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->scan(&count));
    wifi_ap_record_t ap_info;
    wifi_itf->getSTAinfo(&ap_info);

    wifi_ap_record_t ap_records[count] = {}; 
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->getStations(ap_records, &count));
    for (size_t i = 0; i < count; i++)
    {
        ESP_LOGI(TAG, "SSID: %s, power: %d", ap_records[i].ssid, ap_records[i].rssi);
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableSTA(SSID, PASSWORD, true));
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->disconnect());
    vTaskDelay(500);
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->stop());
    vTaskDelay(2000);
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableSTA(SSID, PASSWORD, true));

    vTaskDelay(200);

    while(!sntp.syncWait())
        vTaskDelay(1000);

    sntp.setEpoch(0);
    printf("time synced => %d\n", sntp.getEpoch());
    vTaskDelay(10);
    if(sntp.syncWait())
        printf("time synced => %d\n", sntp.getEpoch());
    else{
        printf("failed to sync\n");
    }
    vTaskDelay(500);
    printf("time synced => %d\n", sntp.getEpoch());
}
