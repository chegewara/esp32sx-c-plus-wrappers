#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "usb/usb_host.h"

#include "usb_host.hpp"

USBhost host;

void client_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg)
{
    if (event_msg->event == USB_HOST_CLIENT_EVENT_NEW_DEV)
    {
        usb_device_info_t info = host.getDeviceInfo();
        ESP_LOGI("", "device speed: %s, device address: %d, max ep_ctrl size: %d, config: %d", info.speed ? "USB_SPEED_FULL" : "USB_SPEED_LOW", info.dev_addr, info.bMaxPacketSize0, info.bConfigurationValue);
        const usb_device_desc_t *dev_desc = host.getDeviceDescriptor();
        ESP_LOG_BUFFER_HEX("", dev_desc->val, 18);
    }
    else
    {
        ESP_LOGW("", "DEVICE gone event");
    }
}

extern "C" void app_main(void)
{
    host.registerClientCb(client_event_callback);
    host.init();
}
