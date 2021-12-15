#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"

#include "uart-comp.h"

#define TAG "UART"

UART::UART(uart_port_t port)
{
    port_num = port;
}

UART::~UART()
{
    uart_driver_delete(port_num);
}

esp_err_t UART::init()
{
    esp_err_t err = ESP_OK;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    //Install UART driver, and get the queue.
    uart_driver_install(port_num, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0);
    uart_param_config(port_num, &uart_config);

    return setPins();
}

esp_err_t UART::init(uart_config_t* uart_config)
{
    esp_err_t err = ESP_OK;
    //Install UART driver, and get the queue.
    uart_driver_install(port_num, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0);
    uart_param_config(port_num, uart_config);

    return setPins();
}

esp_err_t UART::setPins(int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num)
{
    esp_err_t err = ESP_OK;
    err = uart_set_pin(port_num, tx_io_num, rx_io_num, rts_io_num, cts_io_num);

    return err;
}

int UART::read(void *buf, uint32_t length, TickType_t ticks_to_wait)
{
    esp_err_t err = ESP_OK;
    err = uart_read_bytes(port_num, buf, length, ticks_to_wait);

    return err;
}

int UART::write(const void *buf, size_t size)
{
    esp_err_t err = ESP_OK;
    err = uart_write_bytes(port_num, buf, size);

    return err;
}

esp_err_t UART::flush()
{
    esp_err_t err = ESP_OK;
    err = uart_flush(port_num);
    xQueueReset(uart_queue);

    return err;
}

esp_err_t UART::baudrate(uint32_t baudrate)
{
    esp_err_t err = ESP_OK;
    err = uart_set_baudrate(port_num, baudrate);

    return err;
}

BaseType_t UART::waitRX(uart_event_t *event, TickType_t ticks_to_wait)
{
    return xQueueReceive(uart_queue, (void * )event, ticks_to_wait);
}
