#pragma once
#include "esp_err.h"
#include "mqtt_client.h"


class MQTT
{
private:
    esp_mqtt_client_handle_t client = NULL;
    esp_mqtt_client_config_t *mqtt_cfg = NULL;
public:
    MQTT();
    ~MQTT();

public:
    void init(const char* uri);
    void init(const char* host, uint32_t port, esp_mqtt_transport_t transport = MQTT_TRANSPORT_OVER_TCP);
    void init(esp_mqtt_client_config_t *mqtt_cfg);
    void credentials(const char* user, const char* pass);
    void lastWill(const char* topic, const char* msg, int len = 0, int qos = 0, int retain = false);
    void autoReconnect(bool en);
    void addEventsHandler(mqtt_event_callback_t event_handle);

    esp_err_t deinit();
    esp_err_t connect(esp_event_handler_t handler = nullptr);
    esp_err_t stop();
    esp_err_t disconnect();
    esp_err_t reconnect();

    esp_err_t subscribe(const char *topic, int qos = 0);
    esp_err_t unsubscribe(const char *topic);
    esp_err_t publish(const char *topic, const char *data, int len, int qos = 0, int retain = false);

    esp_err_t URI(const char* uri);
};

