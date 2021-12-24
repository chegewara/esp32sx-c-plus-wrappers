#include <stdio.h>
#include "tasks.h"
#include "tools.h"
#include "events.h"
#include "esp_log.h"

ESP_EVENT_DEFINE_BASE(MY_EVENT_BASE);
ESP_EVENT_DEFINE_BASE(MY_EVENT_BASE2);
#define MY_EVENT_1   0x1001
#define MY_EVENT_2    0x1002

EventLoop event1;
EventLoop event2;
Task task1("tasks1");
Task task2("tasks2");
Task task3("tasks3");

static void loop1(void* p)
{
    char* test = "test";
    event1.post(MY_EVENT_BASE, MY_EVENT_1);
    ESP_ERROR_CHECK_WITHOUT_ABORT(EventLoop::postDefault(MY_EVENT_BASE2, -2));
    ESP_ERROR_CHECK_WITHOUT_ABORT(event2.post(MY_EVENT_BASE2, MY_EVENT_2, test));
    delay(1000);
}

static void loop2(void* p)
{
    char* test = "test";
    event2.post(MY_EVENT_BASE, MY_EVENT_2, test, 5);
    delay(150);
}

static void loop3(void* p)
{
    printf("loop3\n");
    event2.run(200);
    delay(1000);
}

static void event_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI("", "event base: %s, event ID: %x, data: %p, class object: %p", event_base, event_id, event_data, event_handler_arg);
}

static void event_event_handler2(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI("", "event2 base: %s, event ID: %x, data: %p, class object: %p", event_base, event_id, event_data, event_handler_arg);
}

static void default_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI("", "default base: %s, event ID: %x, data: %p, class object: %p", event_base, event_id, event_data, event_handler_arg);
}

extern "C" void app_main(void)
{
    EventLoop::initDefault();
    EventLoop::registerEventDefault(default_event_handler);
    event1.init("test");
    event2.init();
    event1.registerEvent(event_event_handler2);
    event2.registerEvent(event_event_handler);

    task1.onLoop(loop1);
    task2.onLoop(loop2);
    task3.onLoop(loop3);

    task1.create(4000, 6);
    task2.create(4000, 6);
    task3.create(4000, 6);
}
