#include <stdio.h>
#include "esp_system.h"
#include "esp_idf_version.h"
#include "esp_timer.h"

#include "tools.h"


esp_reset_reason_t Tools::resetReason()
{
    return esp_reset_reason();
}

uint32_t Tools::minFreeHeap()
{
    return esp_get_minimum_free_heap_size();
}

uint32_t Tools::internalFreeHeap()
{
    return esp_get_free_internal_heap_size();
}

uint32_t Tools::freeHeap()
{
    return esp_get_free_heap_size();
}

void Tools::systemAbort(const char* msg)
{
    return esp_system_abort(msg);
}

esp_err_t Tools::shutdownHandler(shutdown_handler_t handle)
{
    return esp_register_shutdown_handler(handle);
}

const char* Tools::idfVersion()
{
    return esp_get_idf_version();
}

int Tools::watermark(TaskHandle_t task)
{
    return uxTaskGetStackHighWaterMark(task);
}

void Tools::_delay(int ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

uint64_t Tools::micros()
{
    return esp_timer_get_time();
}


// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/mem_alloc.html