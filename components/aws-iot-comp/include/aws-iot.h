#pragma once
#include "esp_err.h"
#include "esp_log.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

#include "tasks.h"

static void aws_iot_task(void* params);

class AwsIoT
{
    friend void aws_iot_task(void* params);
private:
    AWS_IoT_Client client;
    IoT_Client_Init_Params mqttInitParams;
    IoT_Client_Connect_Params mqttConnectParams;
    ShadowInitParameters_t shadowInitParams;
    ShadowConnectParameters_t shadowConnectParams;
    IoT_Error_t last_status;
    Task* internalTask = NULL;
    SemaphoreHandle_t yieldSemaphore = NULL;
    bool is_shadow = true;

public:
    AwsIoT(bool shadow = false);
    ~AwsIoT();

public:

    void configMqtt(IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault, IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault);
    void configShadow(ShadowInitParameters_t initParams = ShadowInitParametersDefault, ShadowConnectParameters_t connectParams = ShadowConnectParametersDefault);
    IoT_Error_t init(char* url, uint16_t port);
    void certificates(const char* cert, const char* key);
    IoT_Error_t connect(const char* thing_name = NULL);
    void onDisconnect(iot_disconnect_handler disconnectHandler);
    void disconnect();
    IoT_Error_t autoReconnect(bool en);
    void onSubscribe();
    IoT_Error_t subscribe(const char* topic, pApplicationHandler_t callback, QoS qos = QOS0, void* args = NULL);
    IoT_Error_t publish(const char* topic, IoT_Publish_Message_Params params);
    IoT_Error_t publish(const char* topic, const char* payload, QoS qos = QOS0);
    IoT_Error_t status();
    // shadow
    IoT_Error_t updateShadow(char* json_string, fpActionCallback_t callback = NULL, uint8_t timeout_seconds = 4, void *pContextData = NULL, bool isPersistent = true);
};

