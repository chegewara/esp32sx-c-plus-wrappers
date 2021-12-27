#include <stdio.h>
#include "aws-iot.h"

#include "main.h"

#define TAG "mqtt"

AwsIoT awsMqtt;

static void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData) {
	IOT_UNUSED(pData);
	IOT_UNUSED(pClient);
	IOT_INFO("Subscribe callback");
	IOT_INFO("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
}

static void iot_subscribe_callback_handler1(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData) {
	IOT_UNUSED(pData);
	IOT_UNUSED(pClient);
	IOT_INFO("Subscribe callback 1");
	IOT_INFO("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
}

void connect_aws()
{
    awsMqtt.configMqtt();

    awsMqtt.certificates(aws_thing_cert, aws_thing_key);
    awsMqtt.init(AWS_ENDPOINT, AWS_PORT);

    ESP_LOGI(TAG, "Connecting to AWS... %s", AWS_ENDPOINT);
    IoT_Error_t rc;
    do {
        rc = awsMqtt.connect();
        if(SUCCESS != rc) {
            vTaskDelay(1000 / portTICK_RATE_MS);
            ESP_LOGW(TAG, "awsMqtt connect status: %d\n", rc);
        }
    } while(SUCCESS != rc);
    awsMqtt.subscribe("test/test", iot_subscribe_callback_handler);
    awsMqtt.subscribe("test/test1", iot_subscribe_callback_handler1);
    awsMqtt.publish("iot/topic", "{\"test\": 123}");
    vTaskDelay(500 / portTICK_RATE_MS);
    awsMqtt.publish("iot/topic", "{\"test\": 123}");
    vTaskDelay(500 / portTICK_RATE_MS);
    awsMqtt.publish("iot/topic", "{\"test\": 123}");
}
