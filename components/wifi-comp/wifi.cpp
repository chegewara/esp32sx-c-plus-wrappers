#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_err.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "wifi.h"

#define TAG "WiFi class"

__attribute__((weak)) void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                              int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
        ESP_LOGI(TAG, "station connected");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGI(TAG, "station disconnected");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        char myIP[20];
        memset(myIP, 0, 20);
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        sprintf(myIP, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "got ip: %s", myIP);
    }
}

WiFi::WiFi()
{
}

esp_err_t WiFi::init()
{
    esp_err_t err = ESP_OK;
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        this));

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        this));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    mode = WIFI_MODE_NULL;

    ESP_ERROR_CHECK(setMode(mode));
    ESP_ERROR_CHECK(start());

    return err;
}
esp_err_t WiFi::enableAP(const char *ssid, const char *pass, uint8_t max_conn)
{
    esp_err_t err;

    if (!esp_netif_ap)
        esp_netif_ap = esp_netif_create_default_wifi_ap();
    if (!esp_netif_ap)
    {
        ESP_LOGE(TAG, "esp_netif_ap is NULL");
        return ESP_FAIL;
    }

    wifi_config_t wifi_config = {};
    wifi_config.ap.ssid_len = (uint8_t)strlen(ssid);
    wifi_config.ap.channel = 0;
    wifi_config.ap.max_connection = max_conn;

    strcpy((char *)wifi_config.ap.ssid, ssid);
    if (!pass || strlen(pass) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    else
    {
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        strcpy((char *)wifi_config.ap.password, pass);
    }

    if (mode == WIFI_MODE_NULL || mode == WIFI_MODE_AP)
        mode = WIFI_MODE_AP;
    else
        mode = WIFI_MODE_APSTA;

    ESP_ERROR_CHECK(setMode(mode));
    ESP_ERROR_CHECK(start());

    err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (err)
    {
        return err;
    }

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             ssid, pass);

    return ESP_OK;
}

esp_err_t WiFi::enableSTA(const char *ssid, const char *pass, bool _connect)
{
    esp_err_t err;

    if (!esp_netif_sta)
        esp_netif_sta = esp_netif_create_default_wifi_sta();
    if (!esp_netif_sta)
    {
        ESP_LOGE(TAG, "esp_netif_sta is NULL");
        return ESP_FAIL;
    }

    if (mode == WIFI_MODE_NULL || mode == WIFI_MODE_STA)
        mode = WIFI_MODE_STA;
    else
        mode = WIFI_MODE_APSTA;

    if ((err = setMode(mode)))
        return err;
    if ((err = start()))
        return err;

    if (ssid)
    {
        wifi_config_t wifi_config = {};
        strcpy((char *)wifi_config.sta.ssid, ssid);

        if (pass && strlen(pass) >= 8)
        {
            strcpy((char *)wifi_config.sta.password, pass);
        }

        err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
        if (err)
        {
            return err;
        }

        ESP_LOGI(TAG, "wifi_init_sta finished. SSID:%s password:%s",
                 ssid, pass);

        if (_connect)
        {
            return connect();
        }
    }

    return ESP_OK;
}

esp_err_t WiFi::connect()
{
    return esp_wifi_connect();
}

esp_err_t WiFi::disconnect()
{
    return esp_wifi_disconnect();
}

esp_err_t WiFi::setHostname(const char *name, bool sta)
{
    if (sta)
    {
        if (esp_netif_sta == NULL)
            return ESP_FAIL;
        return esp_netif_set_hostname(esp_netif_sta, name);
    }

    if (esp_netif_ap == NULL)
        return ESP_FAIL;
    return esp_netif_set_hostname(esp_netif_ap, name);
}

esp_err_t WiFi::scan(uint16_t *ap_count, bool block)
{
    esp_err_t err;
    if (esp_netif_sta == NULL)
    {
        return ESP_ERR_WIFI_NOT_INIT;
    }

    err = esp_wifi_scan_start(NULL, block);
    if (err)
    {
        ESP_LOGE(TAG, "failed to start scan: %d", err);
        return err;
    }

    if (block && ap_count)
    {
        err = esp_wifi_scan_get_ap_num(ap_count);
        ESP_LOGI(TAG, "Total APs scanned = %u", *ap_count);
    }

    return err;
}

esp_err_t WiFi::getStations(wifi_ap_record_t *ap_records, uint16_t *number)
{
    return esp_wifi_scan_get_ap_records(number, ap_records);
}

esp_err_t WiFi::stop()
{
    return esp_wifi_stop();
}

esp_err_t WiFi::start()
{
    return esp_wifi_start();
}

esp_err_t WiFi::setMode(wifi_mode_t mode)
{
    return esp_wifi_set_mode(mode);
}

esp_err_t WiFi::setPowerSave(wifi_ps_type_t type)
{
    return esp_wifi_set_ps(type);
}

esp_err_t WiFi::getMAC(wifi_interface_t ifx, uint8_t mac[6])
{
    return esp_wifi_get_mac(ifx, mac);
}

esp_err_t WiFi::setPower(int8_t power)
{
    return esp_wifi_set_max_tx_power(power);
}

esp_err_t WiFi::getSTAinfo(wifi_ap_record_t *ap_info)
{
    return esp_wifi_sta_get_ap_info(ap_info);
}

esp_err_t WiFi::getAPinfo(wifi_sta_list_t *sta)
{
    return esp_wifi_ap_get_sta_list(sta);
}
