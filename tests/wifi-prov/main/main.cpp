#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "nvs_comp.h"
#include "wifi.h"
#include "wifi-prov.h"
#include "events.h"

#define TAG "main"

WiFi* wifi_itf;
NVS nvs;
Provision provisioning;


esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen, uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf) {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI("", "wifi event: %d", event_id);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
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
        vTaskDelay(1000);
        wifi_itf->connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        char myIP[20];
        memset(myIP, 0, 20);
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        sprintf(myIP, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "got ip: %s", myIP );
    } else {
        printf("base: %s, event: %d\n", event_base, event_id);
    }
}

extern "C" void app_main(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE);
    EventLoop::registerEventDefault(wifi_event_handler, WIFI_EVENT, ESP_EVENT_ANY_ID, NULL);
    EventLoop::registerEventDefault(wifi_event_handler, IP_EVENT, IP_EVENT_STA_GOT_IP, NULL);

    ESP_ERROR_CHECK(nvs.init());
    wifi_itf = new WiFi();
    wifi_itf->init();

    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableSTA());
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableAP("provisioning"));

    ESP_ERROR_CHECK_WITHOUT_ABORT(provisioning.init());
    if (!provisioning.isProvisioned())
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(provisioning.customCreate("custom-data"));
        ESP_ERROR_CHECK_WITHOUT_ABORT(provisioning.start(WIFI_PROV_SECURITY_1));
        ESP_ERROR_CHECK_WITHOUT_ABORT(provisioning.customRegister("custom-data", custom_prov_data_handler));
    } else {
        provisioning.deinit();
    }
}
