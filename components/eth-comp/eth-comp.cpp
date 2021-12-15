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
    esp_err_t err = ESP_OK;
    // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_loop_create_default());
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_init());
    cfg = ESP_NETIF_DEFAULT_ETH();
    phy_config = (eth_phy_config_t)ETH_PHY_DEFAULT_CONFIG();
    mac_config = (eth_mac_config_t)ETH_MAC_DEFAULT_CONFIG();
    eth_netif = esp_netif_new(&cfg);
    // Set default handlers to process TCP/IP stuffs
    // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_eth_set_default_handlers(eth_netif));

    phy_config.phy_addr = 0;
    phy_config.reset_gpio_num = pin_phy_reset;
    gpio_pad_select_gpio(pin_phy_power);
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

    return err;
}

esp_err_t ETH::deinit()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_eth_del_netif_glue(glue_handle));
    return esp_eth_driver_uninstall(eth_handle);
}

esp_err_t ETH::start()
{
    esp_err_t err = ESP_OK;
    config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_eth_driver_install(&config, &eth_handle));
    /* attach Ethernet driver to TCP/IP stack */
    glue_handle = esp_eth_new_netif_glue(eth_handle);
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_attach(eth_netif, glue_handle));
    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_eth_start(eth_handle));

    return err;
}

esp_err_t ETH::stop()
{
    return esp_eth_stop(eth_handle);
}

esp_err_t ETH::getSpeed(void *data)
{
    return esp_eth_ioctl(eth_handle, ETH_CMD_G_SPEED, data);
}

esp_err_t ETH::getDuplex(void *data)
{
    return esp_eth_ioctl(eth_handle, ETH_CMD_G_DUPLEX_MODE, data);
}

bool ETH::setConfig(uint32_t local_ip, uint32_t gateway, uint32_t subnet, uint32_t dns1, uint32_t dns2)
{
    esp_err_t err = ESP_OK;
    esp_netif_ip_info_t info;
	bool staticIP;
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
    if(err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED){
        // log_e("DHCP could not be stopped! Error: %d", err);
        return false;
    }

    err = esp_netif_set_ip_info(eth_netif, &info);
    if(err != ESP_OK){
        // log_e("STA IP could not be configured! Error: %d", err);
        return false;
    }
    
    if(info.ip.addr){
        staticIP = true;
    } else {
        err = esp_netif_dhcpc_start(eth_netif);
        if(err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED){
            // log_w("DHCP could not be started! Error: %d", err);
            return false;
        }
        staticIP = false;
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
    char* data[4] = {};
    uint8_t n = 0;
    char *ptr = strtok((char*)local_ip, ".");
    while(ptr != NULL)
    {
        data[n++] = ptr;
        ptr = strtok(NULL, ".");
    }
    uint32_t _local_ip = 0;
    if (n == 4)
    {
        _local_ip = atoi(data[0]) + (atoi(data[1]) << 8) + (atoi(data[2]) << 16) + (atoi(data[3]) << 24);
    }
    printf("IP: %x\n", _local_ip);    

    n = 0;
    ptr = strtok((char*)gateway, ".");
    while(ptr != NULL)
    {
        data[n++] = ptr;
        ptr = strtok(NULL, ".");
    }
    uint32_t _gateway = 0;
    if (n == 4)
    {
        _gateway = atoi(data[0]) + (atoi(data[1]) << 8) + (atoi(data[2]) << 16) + (atoi(data[3]) << 24);
    }
    printf("gateway: %x\n", _gateway);

    n = 0;
    ptr = strtok((char*)subnet, ".");
    while(ptr != NULL)
    {
        data[n++] = ptr;
        ptr = strtok(NULL, ".");
    }
    uint32_t _subnet = 0;
        if (n == 4)
    {
        _subnet = atoi(data[0]) + (atoi(data[1]) << 8) + (atoi(data[2]) << 16) + (atoi(data[3]) << 24);
    }
    printf("subnet: %x\n", _subnet);

    n = 0;
    ptr = strtok((char*)dns1, ".");
    while(ptr != NULL)
    {
        data[n++] = ptr;
        ptr = strtok(NULL, ".");
    }
    uint32_t _dns1 = 0;
    if (n == 4)
    {
        _dns1 = atoi(data[0]) + (atoi(data[1]) << 8) + (atoi(data[2]) << 16) + (atoi(data[3]) << 24);
    }
    printf("DNS1: %u\n", _dns1);

    n = 0;
    ptr = strtok((char*)dns2, ".");
    while(ptr != NULL)
    {
        data[n++] = ptr;
        ptr = strtok(NULL, ".");
    }
    uint32_t _dns2 = 0;
    if (n == 4)
    {
        _dns2 = atoi(data[0]) + (atoi(data[1]) << 8) + (atoi(data[2]) << 16) + (atoi(data[3]) << 24);
    }
    printf("DNS2: %u\n", _dns2);


    return setConfig(_local_ip, _gateway, _subnet, _dns1, _dns2);
}
