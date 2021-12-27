#include <stdio.h>
#include "aws-iot.h"

#include "main.h"

#define TAG "shadow"

AwsIoT awsShadow(true);

static void ShadowUpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status, const char *pReceivedJsonDocument, void *pContextData)
{
    IOT_UNUSED(pThingName);
    IOT_UNUSED(action);
    IOT_UNUSED(pReceivedJsonDocument);
    IOT_UNUSED(pContextData);


    if(SHADOW_ACK_TIMEOUT == status) {
        ESP_LOGE(TAG, "Update timed out");
    } else if(SHADOW_ACK_REJECTED == status) {
        ESP_LOGE(TAG, "Update rejected");
    } else if(SHADOW_ACK_ACCEPTED == status) {
        ESP_LOGI(TAG, "Update accepted");
    }
}

static void windowActuate_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext)
{
    IOT_UNUSED(pJsonString);
    IOT_UNUSED(JsonStringDataLen);

    if(pContext != NULL) {
        ESP_LOGI(TAG, "Delta - Window state changed to %d", *(bool *) (pContext->pData));
    }
}

static void prepare_shadow_update(char* JsonDocumentBuffer, size_t sizeOfJsonDocumentBuffer)
{
    jsonStruct_t windowActuator;
    jsonStruct_t temperatureHandler;
    bool windowOpen = false;
    float temperature = 20.0f;
    windowActuator.cb = windowActuate_Callback;
    windowActuator.pData = &windowOpen;
    windowActuator.pKey = "windowOpen";
    windowActuator.type = SHADOW_JSON_BOOL;
    windowActuator.dataLength = sizeof(bool);

    temperatureHandler.cb = NULL;
    temperatureHandler.pKey = "temperature";
    temperatureHandler.pData = &temperature;
    temperatureHandler.type = SHADOW_JSON_FLOAT;
    temperatureHandler.dataLength = sizeof(float);
    IoT_Error_t rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
    if(SUCCESS == rc) {
        rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 2, &windowActuator, &temperatureHandler);
        if(SUCCESS == rc) {
            rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
            if(SUCCESS == rc) {
                ESP_LOGI(TAG, "Update Shadow string is ready: %s", JsonDocumentBuffer);
                return;
            } else {
                ESP_LOGE(TAG, "failed to finalize document: %d", rc);
            }
        }
    }
}

void connect_shadow()
{
    awsShadow.configShadow();
    awsShadow.certificates(aws_thing_cert, aws_thing_key);
    awsShadow.init(AWS_ENDPOINT, AWS_PORT);

    ESP_LOGI(TAG, "Connecting to AWS... %s", AWS_ENDPOINT);
    IoT_Error_t rc;
    do {
        rc = awsShadow.connect("test");
        if(SUCCESS != rc) {
            vTaskDelay(1000 / portTICK_RATE_MS);
            ESP_LOGW(TAG, "awsShadow connect status: %d\n", rc);
        }
    } while(SUCCESS != rc);

    char JsonDocumentBuffer[200];
    size_t sizeOfJsonDocumentBuffer = sizeof(JsonDocumentBuffer) / sizeof(JsonDocumentBuffer[0]);

    prepare_shadow_update(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
    awsShadow.updateShadow(JsonDocumentBuffer, ShadowUpdateStatusCallback);
    awsShadow.publish("$aws/things/test/shadow/update", "{\"state\": {\"desired\": {\"powerOn\": 1},\"reported\": {\"powerOn\": 1}}}");
    awsShadow.publish("test/test1", "{\"test\": 123}");
}
