#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "uart-comp.h"

#define TAG "main"
QueueHandle_t uart0_queue;
#define BUF_SIZE (1024)
#define RXD2 4
#define TXD2 5

UART uart0(0);
UART uart1(1);

static void uart_event_task0(void *pvParameters);
static void uart_event_task1(void *pvParameters)
{
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(uart1.waitRX(&event) == pdTRUE) {
            bzero(dtmp, BUF_SIZE);
            // ESP_LOGI(TAG, "uart[%d] event:", uart1.port_num);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    printf("len: %d\n", uart1.read(dtmp, event.size, 5));
                    uart1.flush();
                    ESP_LOGI(TAG, "[UART DATA]: %s", dtmp);
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    uart1.flush();
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    uart1.flush();
                    break;

                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
    uart1.init();
    uart1.setPins(TXD2, RXD2);

    uart0.init();

    xTaskCreate(uart_event_task0, "uart_event_task", 3048, NULL, 12, NULL);
    xTaskCreate(uart_event_task1, "uart_event_task", 3048, NULL, 12, NULL);
}


static void uart_event_task0(void *pvParameters)
{
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(uart0.waitRX(&event)) {
            bzero(dtmp, BUF_SIZE);
            // ESP_LOGI(TAG, "uart[%d] event:", uart0.port_num);
            switch(event.type) {
                case UART_DATA:
                    uart0.read(dtmp, event.size, portMAX_DELAY);
                    ESP_LOGI(TAG, "[UART DATA]: %s", dtmp);
                    if (dtmp[0] == '$')
                    {
                        uart1.write((const char*) dtmp, event.size);
                    } else if(dtmp[0] == '%'){
                        // parseCommand();
                    }
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    uart0.flush();
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    uart0.flush();
                    break;

                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}
