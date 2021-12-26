#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "wifi.h"
#include "nvs_comp.h"
#include "mqtt-comp.h"

#define TAG "main"

WiFi* wifi_itf;
NVS nvs;
MQTT mqtt;

#define USE_EVENTS_LOOP 1


void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI("", "wifi base: %s, event: %d", event_base, event_id);
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_itf->connect();
    }
}

#ifdef USE_EVENTS_LOOP
static void mqtt_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    ESP_LOGI("", "mqtt base: %s, event: %d", event_base, event_id);
#else
esp_err_t mqtt_event_callback(esp_mqtt_event_handle_t event)
{
    ESP_LOGI("", "mqtt event: %d", event->event_id);
#endif
    switch (event->event_id)
    {
        case MQTT_EVENT_ERROR:
            mqtt.disconnect();
            break;
        case MQTT_EVENT_CONNECTED:
            mqtt.subscribe("test");
            break;
        case MQTT_EVENT_DISCONNECTED:
            break;
        case MQTT_EVENT_SUBSCRIBED:
            mqtt.publish("test", "data test 123", 0);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
        case MQTT_EVENT_PUBLISHED:
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            mqtt.publish("test1", "data test 123", 0);
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            break;
        case MQTT_EVENT_DELETED:
            break;
    default:
        break;
    }

#ifndef USE_EVENTS_LOOP
    return ESP_OK;
#endif
}

extern "C" void app_main(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE);
    ESP_ERROR_CHECK(nvs.init());
    wifi_itf = new WiFi();
    wifi_itf->init();

    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf->enableSTA("esp32", "espressif", true));

    mqtt.init("192.168.0.5", 1883);
    mqtt.lastWill("lwt", "this is the end");
#ifdef USE_EVENTS_LOOP
    mqtt.connect(mqtt_event_handler);
#else
    mqtt.addEventsHandler(mqtt_event_callback);
    mqtt.connect();
#endif
}
