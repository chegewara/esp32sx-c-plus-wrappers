#pragma once
#include "esp_err.h"
#include "driver/timer.h"

class Timer
{
private:
    timer_config_t config;
    timer_group_t group;
    timer_idx_t timer_id;
    void* _args;
    bool soft_timer;
    esp_timer_handle_t soft_handle; // soft timer handle

public:
    Timer();
    Timer(timer_group_t group_num, timer_idx_t timer_num);
    ~Timer();

public:
    esp_err_t init(const timer_config_t *config);
    esp_err_t deinit();
    esp_err_t create(const esp_timer_create_args_t *create_args);
    esp_err_t remove();
    esp_err_t start();
    esp_err_t start(uint64_t timeout_us, bool periodic = true);
    esp_err_t stop();
    esp_err_t enable();
    esp_err_t disable();
    void clear();

    uint64_t getTime();
    void setAlarm(uint64_t alarm_us);
    // divider();
    // mode();
    bool autoreload();
    void autoreload(bool en);


    esp_err_t callback(timer_isr_t isr_handler, void *args = NULL);

};
