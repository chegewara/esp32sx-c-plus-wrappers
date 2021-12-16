#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_sntp.h"
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"

#include "sntp-client.h"

static xSemaphoreHandle semaphore;

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI("SNTP", "Notification of a time synchronization event => %ld", tv->tv_sec);
    xSemaphoreGive(semaphore);
}

SNTP::SNTP(const char* s)
{
    server = s;
}

SNTP::~SNTP()
{
    sntp_stop();
    vSemaphoreDelete(semaphore);
}

void SNTP::init(uint32_t interval)
{
    if(semaphore) return;
    semaphore = xSemaphoreCreateBinary();
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, server);
    setInterval(interval);
    sntp_init();
    xSemaphoreTake(semaphore, 0);
}

bool SNTP::syncWait(uint32_t timeout)
{
    if(semaphore == NULL) return false;
    sntp_restart();
    if(xSemaphoreTake(semaphore, timeout) == pdTRUE && sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED)
    {
        return true;
    } else{
        ESP_LOGI("SNTP", "status: %d", sntp_get_sync_status());
    }

    return false;
}

void SNTP::setInterval(uint32_t interval)
{
    sntp_set_sync_interval(interval * 1000);
}

time_t SNTP::getTime()
{
    time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	return now;
}

tm* SNTP::getLocalTime()
{
    time_t rawtime = getTime();
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return timeinfo;
}

void SNTP::setEpoch(uint32_t sec, uint32_t us)
{
    sntp_set_system_time(sec, us);
}

uint32_t SNTP::getEpoch()
{
    uint32_t sec;
    uint32_t us;
    sntp_get_system_time(&sec, &us);
    return sec;
}
