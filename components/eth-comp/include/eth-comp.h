#pragma once

#include "esp_err.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "driver/gpio.h"

#define ETH_PHY_IP101    0x01
#define ETH_PHY_RTL8201  0x02
#define ETH_PHY_LAN8720  0x03
#define ETH_PHY_DP83848  0x04



class ETH
{
private:
    eth_mac_config_t mac_config;
    eth_phy_config_t phy_config;
    esp_eth_mac_t *mac;
    esp_eth_phy_t *phy;
    esp_netif_t *eth_netif;
    esp_eth_handle_t eth_handle = NULL;
    esp_eth_config_t config;
    esp_netif_config_t cfg;
    int pin_phy_reset;
    gpio_num_t pin_phy_power;
    esp_eth_netif_glue_handle_t glue_handle;

public:
    ETH(uint8_t phy_power, int phy_reset = -1);
    ~ETH();

public:
    esp_err_t init(uint8_t type = ETH_PHY_LAN8720);
    esp_err_t start();
    esp_err_t stop();
    esp_err_t deinit();
    esp_err_t getSpeed(void *data);
    esp_err_t getDuplex(void *data);
    bool setConfig(uint32_t local_ip, uint32_t gateway, uint32_t subnet, uint32_t dns1, uint32_t dns2);
    bool setConfig(const char* local_ip, const char* gateway, const char* subnet, const char* dns1, const char* dns2);
    esp_err_t getMAC(uint8_t* mac);
    esp_err_t setHostName(const char* name);
};

