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
#include "events.h"

#define TAG "WiFi class"

WiFi WIFI;

WiFi::WiFi(){}

esp_err_t WiFi::init()
{
    EventLoop::initDefault();
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_init());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "failed to init wifi");

    ESP_RETURN_ON_ERROR(esp_wifi_set_storage(WIFI_STORAGE_RAM), TAG, "failed to set storage");

    mode = WIFI_MODE_NULL;

    ESP_RETURN_ON_ERROR(setMode(mode), TAG, "failed to set mode");
    ESP_RETURN_ON_ERROR(start(), TAG, "failed to start wifi");

    return ESP_OK;
}

esp_err_t WiFi::enableAP(const char *ssid, const char *pass, uint8_t max_conn)
{
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
    if (!pass || strlen(pass) < 8)
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

    ESP_RETURN_ON_ERROR(setMode(mode), TAG, "failed to set mode");
    ESP_RETURN_ON_ERROR(start(), TAG, "failed to start wifi");

    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_AP, &wifi_config), TAG, "failed to set AP config");

    ESP_LOGD(TAG, "wifi softap is ready. SSID:%s password:%s", ssid, pass);

    return ESP_OK;
}

esp_err_t WiFi::enableSTA(const char *ssid, const char *pass, bool _connect)
{
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

    ESP_RETURN_ON_ERROR(setMode(mode), TAG, "failed to set mode");
    ESP_RETURN_ON_ERROR(start(), TAG, "failed to start wifi");

    if (ssid)
    {
        wifi_config_t wifi_config = {};
        strcpy((char *)wifi_config.sta.ssid, ssid);

        if (pass && strlen(pass) >= 8)
        {
            strcpy((char *)wifi_config.sta.password, pass);
        }

        ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), TAG, "failed to set STA config");
        ESP_LOGD(TAG, "wifi_init_sta finished. SSID:%s password:%s", ssid, pass);

        if (_connect)
        {
            ESP_RETURN_ON_ERROR(connect(), TAG, "failed to connect to STA: %s", ssid);
        }
    }

    return ESP_OK;
}

esp_err_t WiFi::connect()
{
    if(mode != WIFI_MODE_AP) ESP_RETURN_ON_ERROR(esp_wifi_connect(), TAG, "failed to connect");
    return ESP_OK;
}

esp_err_t WiFi::disconnect()
{
    ESP_RETURN_ON_ERROR(esp_wifi_disconnect(), TAG, "failed to disconnect");
    return ESP_OK;
}

esp_err_t WiFi::setHostname(const char *name, bool sta)
{
    if (sta)
    {
        if (esp_netif_sta == NULL)
            return ESP_ERR_WIFI_NOT_INIT;
        ESP_RETURN_ON_ERROR(esp_netif_set_hostname(esp_netif_sta, name), TAG, "failed to set STA hostname: %s", name);
    } else {
        if (esp_netif_ap == NULL)
            return ESP_ERR_WIFI_NOT_INIT;
        ESP_RETURN_ON_ERROR(esp_netif_set_hostname(esp_netif_ap, name), TAG, "failed to set AP hostname: %s", name);
    }
    return ESP_OK;
}

esp_err_t WiFi::scan(uint16_t *ap_count, bool block)
{
    if (esp_netif_sta == NULL)
    {
        return ESP_ERR_WIFI_NOT_INIT;
    }

    ESP_RETURN_ON_ERROR(esp_wifi_scan_start(NULL, block), TAG, "failed to start scan");

    if (block && ap_count)
    {
        ESP_RETURN_ON_ERROR(esp_wifi_scan_get_ap_num(ap_count), TAG, "failed to get AP count");
        ESP_LOGI(TAG, "Total APs scanned = %u", *ap_count);
    }

    return ESP_OK;
}

esp_err_t WiFi::getStations(wifi_ap_record_t *ap_records, uint16_t *number)
{
    ESP_RETURN_ON_ERROR(esp_wifi_scan_get_ap_records(number, ap_records), TAG, "failed to get AP records");
    return ESP_OK;
}

esp_err_t WiFi::stop()
{
    ESP_RETURN_ON_ERROR(esp_wifi_stop(), TAG, "failed to stop wifi");
    return ESP_OK;
}

esp_err_t WiFi::start()
{
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "failed to start wifi");
    return ESP_OK;
}

esp_err_t WiFi::setMode(wifi_mode_t m)
{
    mode = m;
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(mode), TAG, "failed to set mode");
    return ESP_OK;
}

esp_err_t WiFi::setPowerSave(wifi_ps_type_t type)
{
    ESP_RETURN_ON_ERROR(esp_wifi_set_ps(type), TAG, "failed to set power save mode");
    return ESP_OK;
}

esp_err_t WiFi::getMAC(wifi_interface_t ifx, uint8_t mac[6])
{
    ESP_RETURN_ON_ERROR(esp_wifi_get_mac(ifx, mac), TAG, "failed to get mac");
    return ESP_OK;
}

esp_err_t WiFi::setPower(int8_t power)
{
    ESP_RETURN_ON_ERROR(esp_wifi_set_max_tx_power(power), TAG, "failed to set TX power");
    return ESP_OK;
}

esp_err_t WiFi::getSTAinfo(wifi_ap_record_t *ap_info)
{
    ESP_RETURN_ON_ERROR(esp_wifi_sta_get_ap_info(ap_info), TAG, "failed to get AP info");
    return ESP_OK;
}

esp_err_t WiFi::getAPinfo(wifi_sta_list_t *sta)
{
    ESP_RETURN_ON_ERROR(esp_wifi_ap_get_sta_list(sta), TAG, "failed to get STA list");
    return ESP_OK;
}

esp_err_t WiFi::setAPConfig(esp_netif_ip_info_t* ip_info)
{
    if (!esp_netif_ap)
        esp_netif_ap = esp_netif_create_default_wifi_ap();

	ESP_RETURN_ON_ERROR(esp_netif_dhcps_stop(esp_netif_ap), TAG, "failed to stop DHCP");
	ESP_RETURN_ON_ERROR(esp_netif_set_ip_info(esp_netif_ap, ip_info), TAG, "failed to setup IP");
	ESP_RETURN_ON_ERROR(esp_netif_dhcps_start(esp_netif_ap), TAG, "failed to start DHCP");

    return ESP_OK;
}

esp_err_t WiFi::setAPConfig(uint32_t local_ip, uint32_t gateway, uint32_t subnet)
{
    esp_netif_ip_info_t info;

    info.ip.addr = local_ip;
    info.gw.addr = gateway;
    info.netmask.addr = subnet;

    return setAPConfig(&info);
}

esp_err_t WiFi::setAPConfig(char* local_ip, char* gateway, char* subnet)
{
    ip_addr_t ip = {};
    ip_addr_t gw = {};
    ip_addr_t sub = {};
    ipaddr_aton(local_ip, &ip);
    ipaddr_aton(local_ip, &gw);
    ipaddr_aton(local_ip, &sub);

    return setAPConfig(ip.u_addr.ip4.addr, gw.u_addr.ip4.addr, sub.u_addr.ip4.addr);
}

esp_err_t WiFi::setSTAConfig(esp_netif_ip_info_t* ip_info, esp_netif_dns_info_t *dns)
{
    if (!esp_netif_sta)
        esp_netif_sta = esp_netif_create_default_wifi_ap();

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcpc_stop(esp_netif_sta));
	ESP_RETURN_ON_ERROR(esp_netif_set_ip_info(esp_netif_sta, ip_info), TAG, "failed to setup IP");
    if(dns) ESP_RETURN_ON_ERROR(esp_netif_set_dns_info(esp_netif_sta, ESP_NETIF_DNS_MAIN, dns), TAG, "failed to setup STA static dns");
    return ESP_OK;
}

esp_err_t WiFi::setSTAConfig(uint32_t local_ip, uint32_t gateway, uint32_t subnet, uint32_t dns)
{
    if(local_ip == 0) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcpc_start(esp_netif_sta));
        return ESP_OK;
    }

    esp_netif_ip_info_t info;
    info.ip.addr = local_ip;
    info.gw.addr = gateway;
    info.netmask.addr = subnet;

    esp_netif_dns_info_t _dns;
    _dns.ip.u_addr.ip4.addr = dns;

    return setSTAConfig(&info, &_dns);
}

esp_err_t WiFi::setSTAConfig(char* local_ip, char* gateway, char* subnet, char* dns)
{
    ip_addr_t ip {};
    ip_addr_t gw {};
    ip_addr_t sub {};
    ip_addr_t _dns {};
    ipaddr_aton(local_ip, &ip);
    ipaddr_aton(gateway, &gw);
    ipaddr_aton(subnet, &sub);
    ipaddr_aton(dns, &_dns);

    return setSTAConfig(ip.u_addr.ip4.addr, gw.u_addr.ip4.addr, sub.u_addr.ip4.addr, _dns.u_addr.ip4.addr);
}
