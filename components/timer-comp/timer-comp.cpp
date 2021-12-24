#include <stdio.h>
#include "esp_timer.h"

#include "timer.h"


Timer::Timer()
{
    soft_timer = true;
}

Timer::Timer(timer_group_t group_num, timer_idx_t timer_num)
{
    group = group_num;
    timer_id = timer_num;
    soft_timer = false;
}

Timer::~Timer() {
    remove();
    deinit();
}

esp_err_t Timer::init(const timer_config_t *_config)
{
    if(soft_timer) return ESP_FAIL;
    esp_err_t err = timer_init(group, timer_id, _config);
    timer_set_counter_value(group, timer_id, 0);
    config = *_config;

    return enable();
}

esp_err_t Timer::start()
{
    if(soft_timer) return ESP_FAIL;
    return timer_start(group, timer_id);
}

esp_err_t Timer::start(uint64_t timeout_us, bool periodic)
{
    if(!periodic)return esp_timer_start_once(soft_handle, timeout_us);

    uint64_t period = timeout_us;
    return esp_timer_start_periodic(soft_handle, period);
}

esp_err_t Timer::stop()
{
    if(soft_timer) return esp_timer_stop(soft_handle);
    return timer_pause(group, timer_id);
}

esp_err_t Timer::deinit()
{
    if(soft_timer) return ESP_FAIL;
    return timer_deinit(group, timer_id);
}

esp_err_t Timer::create(const esp_timer_create_args_t *create_args)
{
    if(!soft_timer) return ESP_FAIL;
    
    return esp_timer_create(create_args, &soft_handle);
}

esp_err_t Timer::remove()
{
    if(!soft_timer) return ESP_FAIL;
    stop();
    return esp_timer_delete(soft_handle);
}

esp_err_t Timer::callback(timer_isr_t handler, void *args)
{
    if(soft_timer) return ESP_FAIL;

    _args = args;
    return timer_isr_callback_add(group, timer_id, handler, this, 0);
}

esp_err_t Timer::enable()
{
    if(soft_timer) return ESP_FAIL;
    return timer_enable_intr(group, timer_id);
}

esp_err_t Timer::disable()
{
    if(soft_timer) return ESP_FAIL;
    return timer_disable_intr(group, timer_id);
}

uint64_t Timer::getTime()
{
    if(soft_timer) return esp_timer_get_time();
    return timer_group_get_counter_value_in_isr(group, timer_id);
}

void Timer::setAlarm(uint64_t alarm_us)
{
    if(soft_timer) return;
    uint64_t next_timer_counter_value = getTime() + alarm_us;
    timer_group_set_alarm_value_in_isr(group, timer_id, next_timer_counter_value);
}

bool Timer::autoreload()
{
    if(soft_timer) return false;
    return timer_group_get_auto_reload_in_isr(group, timer_id) == TIMER_AUTORELOAD_DIS;
}

void Timer::autoreload(bool en)
{
    if(soft_timer) return;
    timer_set_auto_reload(group, timer_id, (timer_autoreload_t)en);
}

void Timer::clear()
{
    if(soft_timer) return;
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
}

