#pragma once
#include <stdio.h>
#include "esp_err.h"
#include "esp_netif_types.h"
#include "esp_wifi_types.h"

class WiFi
{
private:
    esp_netif_t *esp_netif_ap = nullptr;
    esp_netif_t *esp_netif_sta = nullptr;
    wifi_mode_t mode;

public:
    WiFi();
    ~WiFi(){};

public:
    esp_err_t init();
    esp_err_t enableAP(const char* ssid, const char* pass = "", uint8_t max_conn = 3);
    esp_err_t enableSTA(const char* ssid = NULL, const char* pass = NULL, bool _connect = false);
    esp_err_t connect();
    esp_err_t disconnect();
    esp_err_t setHostname(const char*, bool sta = true);

    esp_err_t scan(uint16_t* ap_count = nullptr, bool block = true);
    esp_err_t getStations(wifi_ap_record_t *ap_records, uint16_t *number);
    esp_err_t stop();
    esp_err_t start();
    esp_err_t setMode(wifi_mode_t);
    esp_err_t setPowerSave(wifi_ps_type_t type);
    esp_err_t getMAC(wifi_interface_t ifx, uint8_t mac[6]);
    esp_err_t setPower(int8_t power);
    esp_err_t getSTAinfo(wifi_ap_record_t *ap_info);
    esp_err_t getAPinfo(wifi_sta_list_t *sta);
};
