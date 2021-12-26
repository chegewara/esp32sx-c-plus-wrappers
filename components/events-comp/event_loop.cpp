#include <stdio.h>
#include "events.h"

bool EventLoop::default_running = false;

EventLoop::EventLoop() {}

EventLoop::~EventLoop() {}

esp_err_t EventLoop::init(const char* name)
{
    if(event_loop != NULL) return ESP_FAIL;
    esp_event_loop_args_t event_loop_args = {};
    event_loop_args.queue_size = 5;
    event_loop_args.task_name = name;
    event_loop_args.task_priority = 10;
    event_loop_args.task_stack_size = 3 * 1024;
    event_loop_args.task_core_id = 1;
    return esp_event_loop_create(&event_loop_args, &event_loop);
}

esp_err_t EventLoop::init(const esp_event_loop_args_t* event_loop_args)
{
    if(event_loop != NULL) return ESP_FAIL;
    return esp_event_loop_create(event_loop_args, &event_loop);
}

esp_err_t EventLoop::deinit()
{
    if(event_loop == NULL) return ESP_FAIL;
    return esp_event_loop_delete(event_loop);
}

esp_err_t EventLoop::initDefault()
{
    esp_err_t err = ESP_OK;
    if (!default_running)
    {
        err = esp_event_loop_create_default();
    }

    default_running = err == ESP_OK || err == ESP_ERR_INVALID_STATE;
    return default_running? ESP_OK : err;
}

esp_err_t EventLoop::deinitDefault()
{
    return esp_event_loop_delete_default();
}

esp_err_t EventLoop::run(size_t timeout)
{
    if(event_loop == NULL) return ESP_FAIL;
    return esp_event_loop_run(event_loop, timeout);
}

esp_err_t EventLoop::registerEvent(esp_event_handler_t event_handler, esp_event_base_t event_base, int32_t event_id, void *event_handler_arg)
{
    if(event_loop == NULL) return ESP_FAIL;
    return esp_event_handler_instance_register_with(event_loop, event_base, event_id, event_handler, this, NULL);
}

esp_err_t EventLoop::registerEventDefault(esp_event_handler_t event_handler, esp_event_base_t event_base, int32_t event_id, void *event_handler_arg)
{
    initDefault();
    return esp_event_handler_instance_register(event_base, event_id, event_handler, event_handler_arg, NULL);
}

esp_err_t EventLoop::unregisterEvent(esp_event_base_t event_base, int32_t event_id)
{
    if(event_loop == NULL) return ESP_FAIL;
    return esp_event_handler_instance_unregister_with(event_loop, event_base, event_id, NULL);
}

esp_err_t EventLoop::unregisterEventDefault(esp_event_base_t event_base, int32_t event_id)
{
    return esp_event_handler_instance_unregister(event_base, event_id, NULL);
}

esp_err_t EventLoop::post(esp_event_base_t event_base, int32_t event_id, void *event_data, size_t event_data_size, TickType_t timeout)
{
    if(event_loop == NULL) return ESP_FAIL;
    return esp_event_post_to(event_loop, event_base, event_id, event_data, event_data_size, timeout);
}

esp_err_t EventLoop::postDefault(esp_event_base_t event_base, int32_t event_id, void *event_data, size_t event_data_size, TickType_t timeout)
{
    return esp_event_post(event_base, event_id, event_data, event_data_size, timeout);
}


#if CONFIG_ESP_EVENT_POST_FROM_ISR
esp_err_t EventLoop::postISR(esp_event_base_t event_base, int32_t event_id, BaseType_t *task_unblocked, void *event_data, size_t event_data_size)
{
    if(event_loop == NULL) return ESP_FAIL;
    return  esp_event_isr_post_to(event_loop, event_base, event_id, event_data, event_data_size, task_unblocked);
}

esp_err_t EventLoop::postDefaultISR(esp_event_base_t event_base, int32_t event_id, BaseType_t *task_unblocked, void *event_data, size_t event_data_size)
{
    return esp_event_isr_post(event_base, event_id, event_data, event_data_size, task_unblocked);
}
#endif

esp_err_t EventLoop::dump(FILE* file)
{
    return esp_event_dump(file);
}

