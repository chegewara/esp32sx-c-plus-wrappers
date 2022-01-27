#include <stdio.h>

#include "wifi.h"
#include "esp_log.h"
#include "nvs_comp.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "events.h"

#include "socket-server.h"

#define TAG "main"

#define SSID "esp32"
#define PASSWORD "espressif"

Socket sock;
NVS nvs;

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
    }
}

static void evt_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGW("", "Base event: %s, event ID: %d", event_base, event_id);
    switch (event_id)
    {
    case SOCKET_DATA:{
        packet_datagram_t* packet = (packet_datagram_t*) event_data;
        printf("%.*s", packet->len, (char*)packet->data);
        sock.send(packet->sockfd, packet->data, packet->len, &packet->source_addr);
        free(packet->data); // always free data buffer when used
        break;
    }
    case SOCKET_CLOSE:
        printf("socket is closed now with errno: %d\n", *(int*)event_data);
        break;
    
    default:
        break;
    }
}

extern "C" void app_main(void)
{
    EventLoop::registerEventDefault(wifi_event_handler, WIFI_EVENT, ESP_EVENT_ANY_ID, NULL);
    EventLoop::registerEventDefault(wifi_event_handler, IP_EVENT, IP_EVENT_STA_GOT_IP, NULL);
    EventLoop::registerEventDefault(evt_handler, SOCKET_EVENT, ESP_EVENT_ANY_ID, NULL);

    esp_log_level_set("wifi", ESP_LOG_NONE);
    ESP_ERROR_CHECK(nvs.init());
    WIFI.init();
    WIFI.enableSTA(SSID, PASSWORD, true);

    sock.setPort(3333);
    sock.setType(TCP_SOCKET_TYPE);
    sock.setBufferSize(128); // default buffer size is 256 bytes
    sock.create();
    sock.bind(); // see comment in source file
    sock.listen();
    
    sock.start();
}
