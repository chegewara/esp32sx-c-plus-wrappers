#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "iot_button.h"

#include "nvs_comp.h"
#include "ota.h"
#include "http-client.h"
#include "events.h"

#define USE_WIFI 1

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
#ifdef USE_WIFI
#include "wifi.h"
WiFi wifi;

static void network_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
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
        wifi.connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        char myIP[20];
        memset(myIP, 0, 20);
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        sprintf(myIP, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "got ip: %s", myIP );
    }
}

#else

// #include "eth-comp.h"
ETH* eth;
static bool got_ip = false;
static void button_single_click_cb(void *arg);
static void network_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == ETH_EVENT){    
        uint8_t mac_addr[6] = {0};
        /* we can get the ethernet driver handle from event data */
        esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

        switch (event_id) {
        case ETHERNET_EVENT_CONNECTED:
            esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
            ESP_LOGI(TAG, "Ethernet Link Up");
            ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Ethernet Link Down");
            break;
        case ETHERNET_EVENT_START:
            ESP_LOGI(TAG, "Ethernet Started");
            break;
        case ETHERNET_EVENT_STOP:
            ESP_LOGI(TAG, "Ethernet Stopped");
            break;
        default:
            break;
        }
    } else if(event_base == IP_EVENT) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        const esp_netif_ip_info_t *ip_info = &event->ip_info;

        ESP_LOGI(TAG, "Ethernet Got IP Address");
        ESP_LOGI(TAG, "~~~~~~~~~~~");
        ESP_LOGI(TAG, "ETHIP:   " IPSTR, IP2STR(&ip_info->ip));
        ESP_LOGI(TAG, "ETHMASK: " IPSTR, IP2STR(&ip_info->netmask));
        ESP_LOGI(TAG, "ETHGW:   " IPSTR, IP2STR(&ip_info->gw));
        ESP_LOGI(TAG, "~~~~~~~~~~~");
        got_ip = true;
    }
}
#endif

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
    esp_log_level_set("wifi", ESP_LOG_NONE);
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs.init());
    EventLoop::initDefault();
#ifdef USE_WIFI
    EventLoop::registerEventDefault(network_event_handler, WIFI_EVENT, ESP_EVENT_ANY_ID, NULL);
    EventLoop::registerEventDefault(network_event_handler, IP_EVENT, IP_EVENT_STA_GOT_IP, NULL);
#else
    EventLoop::registerEventDefault(network_event_handler, ETH_EVENT, ESP_EVENT_ANY_ID, NULL);
    EventLoop::registerEventDefault(network_event_handler, IP_EVENT, IP_EVENT_ETH_GOT_IP, NULL);
#endif

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
#ifdef USE_WIFI
    wifi.init();
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi.enableSTA(SSID, PASSWORD, true));
#else
#define	PIN_PHY_POWER	12
    eth = new ETH(PIN_PHY_POWER);
    ESP_ERROR_CHECK_WITHOUT_ABORT(eth->init());
    ESP_ERROR_CHECK_WITHOUT_ABORT(eth->start());

    while(!got_ip)
        vTaskDelay(100);

    button_single_click_cb(NULL); // esp32 with ETH board, i dont have access to boot button
#endif
}
