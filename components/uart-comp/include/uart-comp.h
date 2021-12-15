#pragma once

#include "esp_err.h"
#include "driver/uart.h"

#define BUF_SIZE (1024)

class UART
{
protected:
    QueueHandle_t uart_queue;
    uart_port_t port_num;   

public:
    UART(){}
    UART(uart_port_t port_num);
    ~UART();

public:
    esp_err_t init();
    esp_err_t init(uart_config_t* uart_config);
    esp_err_t setPins(int tx_io_num = UART_PIN_NO_CHANGE, int rx_io_num = UART_PIN_NO_CHANGE, int rts_io_num = UART_PIN_NO_CHANGE, int cts_io_num = UART_PIN_NO_CHANGE);

    int read(void *buf, uint32_t length, TickType_t ticks_to_wait = 100);
    int write(const void *buf, size_t size);
    esp_err_t flush();
    esp_err_t baudrate(uint32_t baudrate);

    BaseType_t waitRX(uart_event_t* event, TickType_t ticks_to_wait = 100);
};


