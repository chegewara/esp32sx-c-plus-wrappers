#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"

class Tools
{
private:
    
public:
    Tools(){}
    ~Tools(){}

public:
    static esp_reset_reason_t resetReason();
    static uint32_t minFreeHeap();
    static uint32_t internalFreeHeap();
    static uint32_t freeHeap();
    static void systemAbort(const char* msg = "abort");
    static esp_err_t shutdownHandler(shutdown_handler_t handle);
    static int watermark(TaskHandle_t task = NULL);
    static void _delay(int ms);
    static uint64_t micros();

    static const char* idfVersion();
};

#define delay(ms) do{Tools::_delay(ms);}while(0);
#define millis() ({uint64_t val;val=Tools::micros()/1000; val;})
