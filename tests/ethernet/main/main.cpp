#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"

#include "eth-comp.h"
#include "nvs_comp.h"
#include "events.h"

#define	PIN_PHY_POWER	12
#define TAG "main"

NVS* nvs = NULL;
ETH* eth = NULL;

/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
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
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:   " IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK: " IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:   " IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
}

extern "C" void app_main(void)
{
    nvs = new NVS();
    ESP_ERROR_CHECK(nvs->init());
    EventLoop::registerEventDefault(eth_event_handler, ETH_EVENT, ESP_EVENT_ANY_ID, NULL);
    EventLoop::registerEventDefault(got_ip_event_handler, IP_EVENT, IP_EVENT_ETH_GOT_IP, NULL);

    eth = new ETH(PIN_PHY_POWER);
    eth->init();
    eth->setHostName("test-host");
    eth->start();

    uint32_t speed;
    uint32_t duplex;
    eth->getSpeed(&speed);
    eth->getDuplex(&duplex);
    printf("speed: %d, duplex: %d\n", speed, duplex);
    vTaskDelay(3000);
    eth->stop();
    vTaskDelay(3000);
    eth->deinit();
    vTaskDelay(3000);
    eth->start();
    vTaskDelay(5000);
    eth->stop();
    vTaskDelay(5000);
    eth->deinit();
    vTaskDelay(5000);
    delete eth;
    printf("end test\n");
}
