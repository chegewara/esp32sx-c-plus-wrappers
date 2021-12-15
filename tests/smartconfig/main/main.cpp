#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi.h"
#include "esp_log.h"
#include "nvs_comp.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "smartconfig.h"

#define TAG "main"

WiFi wifi_itf;
NVS nvs;
Smartconfig smartconfig;


void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_event_sta_connected_t* event = (wifi_event_sta_connected_t*) event_data;
        ESP_LOGI(TAG, "station connected");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI(TAG, "station disconnected");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;

        ESP_LOGI(TAG, "SSID:%s", (char*)evt->ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", (char*)evt->password);

        ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf.enableSTA((char*)evt->ssid, (char*)evt->password, true));
    } else {
        printf("base: %s, event: %d\n", event_base, event_id);
    }
}

static void evt_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGW("", "ANY base: %s, event: %d", event_base, event_id);
}

extern "C" void app_main(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE);
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs.init());
    wifi_itf.init();

    esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, evt_handler, NULL);
    esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf.enableSTA());

    bool provisioned = false;
    /* Get Wi-Fi Station configuration */
    wifi_config_t wifi_cfg;
    if (esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg) != ESP_OK) {
        if (strlen((const char *) wifi_cfg.sta.ssid)) {
            provisioned = true;
        }
    }

    if(!provisioned){
        smartconfig.start();
    }
}
