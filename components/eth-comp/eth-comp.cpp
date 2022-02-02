#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "lwip/dns.h"
#include "esp_check.h"

#include "eth-comp.h"

#define TAG "ETH"


ETH::ETH(uint8_t phy_power, int phy_reset)
{
    pin_phy_power = (gpio_num_t)phy_power;
    pin_phy_reset = phy_reset;
}

ETH::~ETH()
{
}

esp_err_t ETH::init(uint8_t type)
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_init());
    cfg = ESP_NETIF_DEFAULT_ETH();
    phy_config = (eth_phy_config_t)ETH_PHY_DEFAULT_CONFIG();
    mac_config = (eth_mac_config_t)ETH_MAC_DEFAULT_CONFIG();
    eth_netif = esp_netif_new(&cfg);

    phy_config.phy_addr = 0;
    phy_config.reset_gpio_num = pin_phy_reset;
#if ESP_IDF_VERSION_MAJOR > 4
    esp_rom_gpio_pad_select_gpio(pin_phy_power);
#else
    gpio_pad_select_gpio(pin_phy_power);
#endif
    gpio_set_direction(pin_phy_power, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_phy_power, 1);
    vTaskDelay(pdMS_TO_TICKS(10));										

    mac_config.smi_mdc_gpio_num = 23;
    mac_config.smi_mdio_gpio_num = 18;
#if CONFIG_ETH_USE_ESP32_EMAC
    mac = esp_eth_mac_new_esp32(&mac_config);
#endif
    if(type == ETH_PHY_IP101){
        phy = esp_eth_phy_new_ip101(&phy_config);
    }
    else if(type == ETH_PHY_RTL8201){
        phy = esp_eth_phy_new_rtl8201(&phy_config);
    }
    else if(type == ETH_PHY_LAN8720){
        phy = esp_eth_phy_new_lan87xx(&phy_config);
        // phy = esp_eth_phy_new_lan8720(&phy_config);
    }
    else if(type == ETH_PHY_DP83848){
        phy = esp_eth_phy_new_dp83848(&phy_config);
    }

    if(phy == NULL) return ESP_FAIL;

    return ESP_OK;
}

esp_err_t ETH::deinit()
{
    ESP_RETURN_ON_ERROR(esp_eth_del_netif_glue(glue_handle), TAG, "");
    
    ESP_RETURN_ON_ERROR(esp_eth_driver_uninstall(eth_handle), TAG, "");

    return ESP_OK;
}

esp_err_t ETH::start()
{
    config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_RETURN_ON_ERROR(esp_eth_driver_install(&config, &eth_handle), TAG, "failed to install driver");

    glue_handle = esp_eth_new_netif_glue(eth_handle);
    ESP_RETURN_ON_ERROR(esp_netif_attach(eth_netif, glue_handle), TAG, "failed to attach interface");

    ESP_RETURN_ON_ERROR(esp_eth_start(eth_handle), TAG, "failed to start eth");

    return ESP_OK;
}

esp_err_t ETH::stop()
{
    ESP_RETURN_ON_ERROR(esp_eth_stop(eth_handle), TAG, "failed to stop eth");
    return ESP_OK;
}

esp_err_t ETH::getSpeed(void *data)
{
    ESP_RETURN_ON_ERROR(esp_eth_ioctl(eth_handle, ETH_CMD_G_SPEED, data), TAG, "");
    return ESP_OK;
}

esp_err_t ETH::getDuplex(void *data)
{
    ESP_RETURN_ON_ERROR(esp_eth_ioctl(eth_handle, ETH_CMD_G_DUPLEX_MODE, data), TAG, "");
    return ESP_OK;
}

bool ETH::setConfig(uint32_t local_ip, uint32_t gateway, uint32_t subnet, uint32_t dns1, uint32_t dns2)
{
    esp_err_t err = ESP_OK;
    esp_netif_ip_info_t info;
	// bool staticIP;
    if(local_ip != (uint32_t)0x00000000 && local_ip != IPADDR_NONE){
        info.ip.addr = local_ip;
        info.gw.addr = gateway;
        info.netmask.addr = subnet;
    } else {
        info.ip.addr = 0;
        info.gw.addr = 0;
        info.netmask.addr = 0;
	}

    err = esp_netif_dhcpc_stop(eth_netif);
    if(err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED){
        return false;
    }

    err = esp_netif_set_ip_info(eth_netif, &info);
    if(err != ESP_OK){
        return false;
    }
    
    if(info.ip.addr){
        // staticIP = true;
    } else {
        err = esp_netif_dhcpc_start(eth_netif);
        if(err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED){
            return false;
        }
        // staticIP = false;
    }

    ip_addr_t dns;
    dns.type = IPADDR_TYPE_V4;

    if(dns1 != (uint32_t)0x00000000 && dns1 != IPADDR_NONE) {
        // Set DNS1-Server
        dns.u_addr.ip4.addr = static_cast<uint32_t>(dns1);
        dns_setserver(0, &dns);
    }

    if(dns2 != (uint32_t)0x00000000 && dns2 != IPADDR_NONE) {
        // Set DNS2-Server
        dns.u_addr.ip4.addr = static_cast<uint32_t>(dns2);
        dns_setserver(1, &dns);
    }

    return true;
}

bool ETH::setConfig(const char* local_ip, const char* gateway, const char* subnet, const char* dns1, const char* dns2)
{
    printf("ETH IP: %s\n", local_ip);
    ip_addr_t _local_ip = {};
    ip_addr_t _gateway = {};
    ip_addr_t _subnet = {};
    ip_addr_t _dns1 = {};
    ip_addr_t _dns2 = {};
    ipaddr_aton(local_ip, &_local_ip);
    ipaddr_aton(gateway, &_gateway);
    ipaddr_aton(subnet, &_subnet);
    ipaddr_aton(dns1, &_dns1);
    ipaddr_aton(dns2, &_dns2);

    return setConfig(_local_ip.u_addr.ip4.addr, _gateway.u_addr.ip4.addr, _subnet.u_addr.ip4.addr, _dns1.u_addr.ip4.addr, _dns2.u_addr.ip4.addr);
}

esp_err_t ETH::getMAC(uint8_t* mac)
{
    ESP_RETURN_ON_ERROR(esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac), TAG, "failed to get eth mac");
    return ESP_OK;
}

esp_err_t ETH::setHostName(const char* name)
{
    ESP_RETURN_ON_ERROR(esp_netif_set_hostname(eth_netif, name), TAG, "failed to set ETH hostname: %s", name);
    return ESP_OK;
}
