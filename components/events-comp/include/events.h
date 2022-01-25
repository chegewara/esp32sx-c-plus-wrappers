#pragma once 
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_err.h"


class EventLoop
{
private:
    esp_event_loop_handle_t event_loop;
    static bool default_running;

public:
    EventLoop();
    ~EventLoop();

public:
    esp_err_t init(const char* name = NULL);
    esp_err_t init(const esp_event_loop_args_t *event_loop_args);
    esp_err_t deinit();
    static esp_err_t initDefault();
    static esp_err_t deinitDefault();
    esp_err_t run(size_t timeout = 0);

    esp_err_t post(esp_event_base_t event_base, int32_t event_id, void *event_data = NULL, size_t event_data_size = 0, TickType_t timeout = 0);
    static esp_err_t postDefault(esp_event_base_t event_base, int32_t event_id, void *event_data = NULL, size_t event_data_size = 0, TickType_t timeout = 50);
#if CONFIG_ESP_EVENT_POST_FROM_ISR
    esp_err_t postISR(esp_event_base_t event_base, int32_t event_id, BaseType_t *task_unblocked, void *event_data = NULL, size_t event_data_size = 0);
    static esp_err_t postDefaultISR(esp_event_base_t event_base, int32_t event_id, BaseType_t *task_unblocked, void *event_data = NULL, size_t event_data_size = 0);
#endif

    esp_err_t registerEvent(esp_event_handler_t event_handler, esp_event_base_t event_base = ESP_EVENT_ANY_BASE, int32_t event_id = ESP_EVENT_ANY_ID, void *event_handler_arg = NULL);
    esp_err_t unregisterEvent(esp_event_base_t event_base = ESP_EVENT_ANY_BASE, int32_t event_id = ESP_EVENT_ANY_ID);
    static esp_err_t registerEventDefault(esp_event_handler_t event_handler, esp_event_base_t event_base = ESP_EVENT_ANY_BASE, int32_t event_id = ESP_EVENT_ANY_ID, void *event_handler_arg = NULL);
    static esp_err_t unregisterEventDefault(esp_event_base_t event_base = ESP_EVENT_ANY_BASE, int32_t event_id = ESP_EVENT_ANY_ID);

    esp_err_t dump(FILE* file);
};

