/**
 * http/s server test example
 * this example is running 2 servers, 1 http and 1 https server, each with websocket and async sending
*/
#include "esp_err.h"
#include "esp_wifi.h"

#include "wifi.h"
#include "nvs_comp.h"

#define TAG "main"

#define SSID "esp32"
#define PASSWORD "espressif"
#define AP_SSID "esp32"

static WiFi wifi_itf;
static NVS nvs;

void initTCPserver();
void initSSLserver();
void sendWSpacket();
void sendWSSpacket();

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs.init());
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf.init());

    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf.enableSTA(SSID, PASSWORD, true));
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf.enableAP(AP_SSID));

    initTCPserver();
    initSSLserver();

    while(1)
    {
        sendWSpacket();
        sendWSSpacket();
        vTaskDelay(100);
    }
}
