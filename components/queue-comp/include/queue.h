#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

class Queue
{
private:
    QueueHandle_t queue_handle;

public:
    Queue();
    ~Queue();

public:
    bool create(size_t count, size_t size);
    void remove();
    bool send(const void *item, uint32_t wait = portMAX_DELAY);
    bool receive(void *item, uint32_t wait = portMAX_DELAY);
    bool peek(void *item, uint32_t wait = portMAX_DELAY);
    void reset();
    int32_t count();
    bool overwrite(const void *item);
    QueueHandle_t handle() { return queue_handle; }
};
