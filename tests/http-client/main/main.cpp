#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_http_client.h"

#include "nvs_comp.h"
#include "wifi.h"
#include "http-client.h"

#define TAG "main"

#define SSID "esp32"
#define PASSWORD "espressif"
#define URL_PAGE1 "http://www.google.com/"
#define URL_PAGE2 "https://www.google.com/"

WiFi* wifi_itf;
NVS nvs;
HttpClient* client;
static bool isConnected = false;
static bool isClientCon = false;

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            isClientCon = true;
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

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
        wifi_event_sta_connected_t* event = (wifi_event_sta_connected_t*) event_data;
        ESP_LOGI(TAG, "station connected");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI(TAG, "station disconnected");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        char myIP[20];
        memset(myIP, 0, 20);
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        sprintf(myIP, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "got ip: %s", myIP );
        isConnected = true;
    } else {
        printf("base: %s, event: %d\n", event_base, event_id);
    }
}

extern "C" void app_main(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE);
    ESP_ERROR_CHECK(nvs.init());
    wifi_itf = new WiFi();
    wifi_itf->init();

    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableSTA(SSID, PASSWORD));
    wifi_itf->connect();
    while(!isConnected) vTaskDelay(10);

    esp_http_client_config_t config = {};
    config.url = URL_PAGE1;
    config.event_handler = _http_event_handle;
    config.skip_cert_common_name_check = true;

    client = new HttpClient();
    client->init(&config);
    client->perform();
    printf("length: %lld\n", client->length());
    vTaskDelay(50);
    client->setUrl(URL_PAGE2);

    client->open();
    client->getHeaders();
    printf("status: %d, length: %lld\n\n\n", client->status(), client->length());

    int len = 0;
    int total = 0;
    char buf[1025] = {};
    size_t _len = 1024;
    do
    {
        len = client->read(buf, _len);
        total += len;
    } while (len > 0);

    client->cleanup();
    printf("status: %d, total: %d\n", client->status(), total);
    delete client;
}
