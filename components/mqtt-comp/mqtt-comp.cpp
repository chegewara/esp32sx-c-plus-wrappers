#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt-comp.h"


MQTT::MQTT()
{
}

MQTT::~MQTT()
{

    deinit();
}

void MQTT::init(const char* uri)
{
    if(!mqtt_cfg)
        mqtt_cfg = new esp_mqtt_client_config_t();
    mqtt_cfg->uri = uri;
#if ESP_IDF_VERSION_MAJOR > 4
    mqtt_cfg->set_null_client_id = true;
#endif
}

void MQTT::init(const char* host, uint32_t port, esp_mqtt_transport_t transport)
{
    if(!mqtt_cfg)
        mqtt_cfg = new esp_mqtt_client_config_t();

    mqtt_cfg->transport = transport;
    mqtt_cfg->host = host;
    mqtt_cfg->port = port;
#if ESP_IDF_VERSION_MAJOR > 4
    mqtt_cfg->set_null_client_id = true;
#endif
}

void MQTT::init(esp_mqtt_client_config_t *mqtt_cfg)
{
    this->mqtt_cfg = mqtt_cfg;
}

void MQTT::credentials(const char* user, const char* pass)
{
    mqtt_cfg->username = user;
    mqtt_cfg->password = pass;
}

void MQTT::lastWill(const char* topic, const char* msg, int len, int qos, int retain)
{
    mqtt_cfg->lwt_topic = topic;
    mqtt_cfg->lwt_msg = msg;
    mqtt_cfg->lwt_msg_len = len;
    mqtt_cfg->lwt_qos = qos;
    mqtt_cfg->lwt_retain = retain;
}

void MQTT::autoReconnect(bool en)
{
    mqtt_cfg->disable_auto_reconnect = !en;
}

esp_err_t MQTT::connect(esp_event_handler_t mqtt_event_callback)
{
    esp_err_t err = ESP_FAIL;
    if(!mqtt_cfg) return err;

    if(!client){
        client = esp_mqtt_client_init(mqtt_cfg);
        if (client == nullptr)
            return ESP_FAIL;
    }

    if(mqtt_event_callback){
        err = esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_callback, this);
        if(err) return err;
    }

    err = esp_mqtt_client_start(client);

    return err;
}

esp_err_t MQTT::stop()
{
    esp_err_t err = ESP_OK;
    err = esp_mqtt_client_stop(client);

    return err;
}

esp_err_t MQTT::disconnect()
{
    esp_err_t err = ESP_OK;
    err = esp_mqtt_client_disconnect(client);

    return err;
}

esp_err_t MQTT::reconnect()
{
    esp_err_t err = ESP_OK;
    err = esp_mqtt_client_reconnect(client);

    return err;
}

esp_err_t MQTT::subscribe(const char *topic, int qos)
{
    esp_err_t err = ESP_OK;
    int id = esp_mqtt_client_subscribe(client, topic, qos);
    if (id < 0)
    {
        err = ESP_FAIL;
    }
    
    return err;
}

esp_err_t MQTT::unsubscribe(const char *topic)
{
    esp_err_t err = ESP_OK;
    int id = esp_mqtt_client_unsubscribe(client, topic);
    if (id < 0)
    {
        err = ESP_FAIL;
    }
    
    return err;
}

esp_err_t MQTT::publish(const char *topic, const char *data, int len, int qos, int retain)
{
    esp_err_t err = ESP_OK;
    int id = esp_mqtt_client_publish(client, topic, data, len, qos, retain);
    if (id < 0)
    {
        err = ESP_FAIL;
    }
    return err;
}

esp_err_t MQTT::URI(const char* uri)
{
    esp_err_t err = ESP_OK;
    err = esp_mqtt_client_set_uri(client, uri);

    return err;
}

void MQTT::addEventsHandler(mqtt_event_callback_t event_handle)
{
    mqtt_cfg->event_handle = event_handle;
}

esp_err_t MQTT::deinit()
{
    esp_err_t err = esp_mqtt_client_destroy(client);
    client = NULL;

    return err;
}
