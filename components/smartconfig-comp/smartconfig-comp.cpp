#include <stdio.h>
#include "smartconfig.h"
#include "esp_log.h"
#include "esp_event.h"

#define TAG "Smartconfig"

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password");
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;

        uint8_t rvd_data[33] = { 0 };
        ESP_LOGI(TAG, "SSID:%s", evt->ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", evt->password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            esp_err_t err = ESP_OK;
            ESP_ERROR_CHECK_WITHOUT_ABORT(err =  esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i=0; i<33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {

    }
}

Smartconfig::Smartconfig()
{
}

Smartconfig::~Smartconfig()
{
}

esp_err_t Smartconfig::start()
{
    esp_err_t err = ESP_OK;
    const smartconfig_start_config_t config = SMARTCONFIG_START_CONFIG_DEFAULT();
    err = esp_smartconfig_start(&config);

    ESP_ERROR_CHECK_WITHOUT_ABORT(err =  esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, event_handler, this) );

    return err;
}

esp_err_t Smartconfig::stop()
{
    esp_err_t err = ESP_OK;
    err = esp_smartconfig_stop();

    return err;
}

esp_err_t Smartconfig::timeout(uint8_t time_s)
{
    esp_err_t err = ESP_OK;
    err = esp_esptouch_set_timeout(time_s);

    return err;
}

esp_err_t Smartconfig::type(smartconfig_type_t type)
{
    esp_err_t err = ESP_OK;
    err = esp_smartconfig_set_type(type);

    return err;
}

esp_err_t Smartconfig::mode(bool enable)
{
    esp_err_t err = ESP_OK;
    err = esp_smartconfig_fast_mode(enable);

    return err;
}

esp_err_t Smartconfig::received(uint8_t *rvd_data, uint8_t len)
{
    esp_err_t err = ESP_OK;
    err = esp_smartconfig_get_rvd_data(rvd_data, len);

    return err;
}

