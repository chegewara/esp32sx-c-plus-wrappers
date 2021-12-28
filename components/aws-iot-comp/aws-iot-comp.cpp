#include <stdio.h>
#include <cstring>
#include "aws-iot.h"

#define TAG "AwsIoT"

extern const char aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const char aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");

static void aws_iot_task(void* params)
{
    AwsIoT* aws = (AwsIoT*)((Task*)params)->args();
    SemaphoreHandle_t yieldSemaphore = aws->yieldSemaphore;
    //Max time the yield function will wait for read messages
    xSemaphoreTake(yieldSemaphore, portMAX_DELAY);
    IoT_Error_t rc;
    if (aws->is_shadow)
    {
        rc = aws_iot_shadow_yield(&aws->client, 100);
    } else {
        rc = aws_iot_mqtt_yield(&aws->client, 100);
    }

    aws->last_status = rc;
    xSemaphoreGive(yieldSemaphore);
    vTaskDelay(10 / portTICK_RATE_MS);
    if(NETWORK_ATTEMPTING_RECONNECT == rc) {
        return;
    }
}


AwsIoT::AwsIoT(bool shadow):is_shadow{shadow}
{
}

AwsIoT::~AwsIoT()
{
}

void AwsIoT::configMqtt(IoT_Client_Init_Params initParams, IoT_Client_Connect_Params connectParams)
{
    mqttInitParams = initParams;
    mqttConnectParams = connectParams;
}

void AwsIoT::configShadow(ShadowInitParameters_t initParams, ShadowConnectParameters_t connectParams)
{
    shadowInitParams = initParams;
    shadowConnectParams = connectParams;
}

void AwsIoT::certificates(const char* cert, const char* key)
{
    if (is_shadow)
    {
        shadowInitParams.pRootCA = aws_root_ca_pem_start;
        shadowInitParams.pClientCRT = cert;
        shadowInitParams.pClientKey = key;        
    } else {
        mqttInitParams.pRootCALocation = aws_root_ca_pem_start;
        mqttInitParams.pDeviceCertLocation = cert;
        mqttInitParams.pDevicePrivateKeyLocation = key;
    }
}

IoT_Error_t AwsIoT::init(char* url, uint16_t port)
{
    yieldSemaphore = xSemaphoreCreateMutex();
    xSemaphoreGive(yieldSemaphore);
    IoT_Error_t rc = FAILURE;
    if(yieldSemaphore == NULL){
        printf("semaphore failure\n");
        return rc;
    }

    if (is_shadow)
    {
        shadowInitParams.pHost = url;
        shadowInitParams.port = port;
        rc = aws_iot_shadow_init(&client, &shadowInitParams);
    } else {
        mqttInitParams.pHostURL = url;
        mqttInitParams.port = port;
        rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    }

    if(SUCCESS != rc)
    {
        ESP_LOGE(TAG, "mqtt init error: %d", rc);
        return rc;
    }

    internalTask = new Task("aws-iot");
    internalTask->onLoop(aws_iot_task);
    internalTask->create(3000, 5, this);
    return rc;
}

IoT_Error_t AwsIoT::connect(const char* thing_name)
{
    IoT_Error_t rc;
    if (is_shadow)
    {
        if(thing_name == NULL) return NULL_VALUE_ERROR;
        shadowConnectParams.pMyThingName = thing_name;
        shadowConnectParams.pMqttClientId = thing_name;
        shadowConnectParams.mqttClientIdLen = (uint16_t) strlen(thing_name);
        rc = aws_iot_shadow_connect(&client, &shadowConnectParams);
    } else {
        rc = aws_iot_mqtt_connect(&client, &mqttConnectParams);
    }

    return rc;
}

void AwsIoT::onDisconnect(iot_disconnect_handler disconnectHandler)
{

}

void AwsIoT::disconnect()
{

}

IoT_Error_t AwsIoT::autoReconnect(bool en)
{
    IoT_Error_t rc = aws_iot_mqtt_autoreconnect_set_status(&client, en);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Auto Reconnect error: %d", rc);
    }
    return rc;
}

void AwsIoT::onSubscribe()
{

}

IoT_Error_t AwsIoT::subscribe(const char* topic, pApplicationHandler_t callback, QoS qos, void* args)
{
    xSemaphoreTake(yieldSemaphore, 1000 / portTICK_RATE_MS);
    IoT_Error_t rc = aws_iot_mqtt_subscribe(&client, topic, strlen(topic), qos, callback, args);
    xSemaphoreGive(yieldSemaphore);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Error subscribing : %d ", rc);
    }
    return rc;
}

IoT_Error_t AwsIoT::publish(const char* topic, IoT_Publish_Message_Params params)
{
    xSemaphoreTake(yieldSemaphore, 1000 / portTICK_RATE_MS);
    IoT_Error_t rc = aws_iot_mqtt_publish(&client, topic, strlen(topic), &params);
    xSemaphoreGive(yieldSemaphore);
    if (params.qos == QOS1 && rc == MQTT_REQUEST_TIMEOUT_ERROR) {
        ESP_LOGW(TAG, "QOS1 publish ack not received.");
        rc = SUCCESS;
    }
    if(rc != SUCCESS) ESP_LOGE(TAG, "Error publishing : %d ", rc);
    return rc;
}

IoT_Error_t AwsIoT::publish(const char* topic, const char* payload, QoS qos)
{
    IoT_Publish_Message_Params params = { };
    params.qos = qos,
    params.isRetained = 0,
    params.payload = (void *) payload,
    params.payloadLen = strlen(payload),

    xSemaphoreTake(yieldSemaphore, 100 / portTICK_RATE_MS);
    IoT_Error_t rc = aws_iot_mqtt_publish(&client, topic, strlen(topic), &params);
    xSemaphoreGive(yieldSemaphore);
    if (qos == QOS1 && rc == MQTT_REQUEST_TIMEOUT_ERROR) {
        ESP_LOGW(TAG, "QOS1 publish ack not received.");
        rc = SUCCESS;
    }
    if(rc != SUCCESS) ESP_LOGE(TAG, "Error publishing : %d ", rc);
    return rc;
}

IoT_Error_t AwsIoT::status()
{
    // if(yieldSemaphore == NULL)
    // {
    //     return SUCCESS;
    // }
    if(xSemaphoreTake(yieldSemaphore, 100 / portTICK_RATE_MS) != pdTRUE) return last_status;
    xSemaphoreGive(yieldSemaphore);
    return last_status;
}

IoT_Error_t AwsIoT::updateShadow(char* json_string, fpActionCallback_t callback, uint8_t timeout_seconds, void *pContextData, bool isPersistent)
{
    xSemaphoreTake(yieldSemaphore, 100 / portTICK_RATE_MS);
    IoT_Error_t rc = aws_iot_shadow_update(&client, shadowConnectParams.pMyThingName, json_string, callback, pContextData, timeout_seconds, isPersistent);
    xSemaphoreGive(yieldSemaphore);
    if(rc) ESP_LOGE(TAG, "failed to update shadow: %d\n", rc);

    return rc;
}
