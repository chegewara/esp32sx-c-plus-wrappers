#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sstream>
typedef void (*loop_t)(void *);

#define NOACTION eNoAction
#define SETBITS eSetBits
#define INCREMENT eIncrement
#define SETVALUEALWAYS eSetValueWithOverwrite
#define SETVALUE eSetValueWithoutOverwrite

class Task
{
private:
    const char *_name;
    static TaskHandle_t idle_handle;
    TaskHandle_t task_handle;
    StaticTask_t *xTaskBuffer;
    void *_args;
    loop_t _loop;
    StackType_t *xStack;

public:
    Task(const char *name = "");
    ~Task();

public:
    bool create(int stack = 3000, uint8_t prio = 5, void *arg = NULL, bool _static = false);
    // bool createStatic(int stack = 3000, uint8_t prio = 5, void* arg = NULL);
    void remove();
    void suspend();
    void resume();
    TaskHandle_t handle() { return task_handle; }
    void status(TaskStatus_t *pxTaskStatus, bool watermark = true, eTaskState eState = eBlocked);

    void run();
    void onLoop(loop_t);
    void* args();

    bool notify(uint32_t val = 0, uint8_t action = NOACTION);
    bool notifyWait(uint32_t *val, TickType_t wait = portMAX_DELAY);
    bool give();
    uint32_t take(bool clear = true, TickType_t wait = portMAX_DELAY);

    static void stats(char *buffer);
    static int count();
    static void list(char *buffer);
};
