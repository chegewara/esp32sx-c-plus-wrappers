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
#include "events.h"

#define TAG "main"

#define SSID "esp32"
#define PASSWORD "espressif"
#define AP1_SSID "test123"
#define AP2_SSID "test1"

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

static esp_err_t getAPrecords(uint16_t count)
{
    wifi_ap_record_t ap_records[count] = {};
    ESP_RETURN_ON_ERROR(WIFI.getStations(ap_records, &count), TAG, "");
    for (size_t i = 0; i < count; i++)
    {
        ESP_LOGI(TAG, "SSID: %s, power: %d", ap_records[i].ssid, ap_records[i].rssi);
    }
    return ESP_OK;
}

extern "C" void app_main(void)
{
    esp_err_t ret = ESP_OK;
    char buffer[80];
    uint16_t count = 0;
    wifi_ap_record_t ap_info;

    EventLoop::registerEventDefault(wifi_event_handler, WIFI_EVENT, ESP_EVENT_ANY_ID, NULL);
    EventLoop::registerEventDefault(wifi_event_handler, IP_EVENT, IP_EVENT_STA_GOT_IP, NULL);
    esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, evt_handler, NULL);

    esp_log_level_set("wifi", ESP_LOG_NONE);
    ESP_ERROR_CHECK(nvs.init());
    ESP_GOTO_ON_ERROR(WIFI.init(), exit, TAG, "");

    ESP_GOTO_ON_ERROR(WIFI.enableAP(AP1_SSID), exit, TAG, "");
    ESP_GOTO_ON_ERROR(WIFI.enableAP(AP2_SSID), exit, TAG, "");
    ESP_GOTO_ON_ERROR(WIFI.enableSTA(), exit, TAG, "");
    ESP_GOTO_ON_ERROR(WIFI.setHostname(AP2_SSID, false), exit, TAG, "");

    ESP_GOTO_ON_ERROR(WIFI.scan(&count), exit, TAG, "");
    WIFI.getSTAinfo(&ap_info);

    ESP_GOTO_ON_ERROR(getAPrecords(count), exit, TAG, "");
    ESP_GOTO_ON_ERROR(WIFI.enableSTA(SSID, PASSWORD, true), exit, TAG, "");
    ESP_GOTO_ON_ERROR(WIFI.disconnect(), exit, TAG, "");
    vTaskDelay(500);
    ESP_GOTO_ON_ERROR(WIFI.stop(), exit, TAG, "");
    vTaskDelay(200);
    ESP_GOTO_ON_ERROR(WIFI.enableSTA(SSID, PASSWORD, true), exit, TAG, "");

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

exit:
    struct tm *timeinfo = sntp.getLocalTime();
    strftime(buffer, 80, "%d/%m/%Y %H:%M:%S GMT", timeinfo);
    printf("current time: %s\n", buffer);
}
