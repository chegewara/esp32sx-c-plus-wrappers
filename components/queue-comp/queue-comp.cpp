#include <stdio.h>

#include "queue.h"

Queue::Queue()
{
    queue_handle = NULL;
}

Queue::~Queue()
{
    remove();
}

bool Queue::create(size_t count, size_t size)
{
    queue_handle = xQueueCreate(count, size);

    return queue_handle != NULL;
}

void Queue::remove()
{
    if (queue_handle == NULL)
        return;

    vQueueDelete(queue_handle);
    queue_handle = NULL;
}

bool Queue::send(const void *item, uint32_t wait)
{
    if (queue_handle == NULL)
        return false;

    return xQueueSend(queue_handle, item, wait) == pdTRUE;
}

bool Queue::receive(void *item, uint32_t wait)
{
    if (queue_handle == NULL)
        return false;

    return xQueueReceive(queue_handle, item, wait) == pdTRUE;
}

void Queue::reset()
{
    if (queue_handle == NULL)
        return;

    xQueueReset(queue_handle);
}

bool Queue::peek(void *item, uint32_t wait)
{
    if (queue_handle == NULL)
        return false;

    return xQueuePeek(queue_handle, item, wait) == pdTRUE;
}

int32_t Queue::count()
{
    if (queue_handle == NULL)
        return -1;

    return uxQueueMessagesWaiting(queue_handle);
}

bool Queue::overwrite(const void *item)
{
    if (queue_handle == NULL)
        return false;

    return xQueueOverwrite(queue_handle, item) == pdTRUE;
}
