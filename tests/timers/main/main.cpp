#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "timer.h"

static void esp_timer_cb(void* arg);

#define TIMER_DIVIDER         (80)  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
timer_config_t config1 = {
    .alarm_en = TIMER_ALARM_EN,
    .counter_en = TIMER_PAUSE,
    .counter_dir = TIMER_COUNT_UP,
    .auto_reload = TIMER_AUTORELOAD_DIS,
    .clk_src = TIMER_SRC_CLK_APB,
    .divider = TIMER_DIVIDER,
};
Timer timer1(TIMER_GROUP_0, TIMER_0);

esp_timer_create_args_t config2 = {
    .callback = esp_timer_cb,
    .arg = NULL,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "soft_timer",
    .skip_unhandled_events = true
} ;
Timer timer2;

static bool IRAM_ATTR timer_group_isr_callback(void *args)
{
    timer1.clear();
    uint64_t timer_counter_value = timer1.getTime();
    ets_printf("now : %lld\n", timer_counter_value);

    uint64_t timer_counter_value2 = (uint64_t) (1 * TIMER_SCALE);
    if (timer1.autoreload())
    {
        timer1.setAlarm(timer_counter_value2);
    }
    ets_printf("next: %lld\n", timer1.getTime());
    return true;
}

static void esp_timer_cb(void* arg)
{
    ESP_LOGI("", "soft timer");
}

extern "C" void app_main(void)
{
    timer1.init(&config1);
    timer1.setAlarm(1 * TIMER_SCALE);

    timer1.enable();
    timer1.callback(timer_group_isr_callback);
    timer1.start();

    timer2.create(&config2);
    timer2.start(1000 * 1000, false);
    ESP_LOGI("", "one shot");
    vTaskDelay(500);
    timer2.start(1000 * 1000);
    ESP_LOGI("", "periodic start");
    vTaskDelay(500);
    timer1.~Timer();
    timer2.~Timer();
}
