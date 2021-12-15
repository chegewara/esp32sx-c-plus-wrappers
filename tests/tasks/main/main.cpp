#include <stdio.h>

#include "tasks.h"
#include "tools.h"
#include "freertos/semphr.h"
#include "esp_expression_with_stack.h"


Task task1("task1");
Task task2("task2");

static void loop1(void* p)
{
    uint32_t val;
    task1.notifyWait(&val);
    Task* t = (Task*)p;
    TaskStatus_t pxTaskStatus;
    t->status(&pxTaskStatus);
    printf("loop1 task [%d] name: %s, watermark: %d, stack: 0x%p\n", pxTaskStatus.xTaskNumber, pxTaskStatus.pcTaskName, pxTaskStatus.usStackHighWaterMark, pxTaskStatus.pxStackBase);
    task2.status(&pxTaskStatus, true, eInvalid);
    printf("loop1 task [%d] name: %s, run time: %d, state: %d\n", pxTaskStatus.xTaskNumber, pxTaskStatus.pcTaskName, pxTaskStatus.ulRunTimeCounter, pxTaskStatus.eCurrentState);
}

static void loop2(void* p)
{
    Task* t = (Task*)p;
    TaskStatus_t pxTaskStatus;
    t->status(&pxTaskStatus);
    printf("loop2 task [%d] name: %s, watermark: %d, state: %d\n", pxTaskStatus.xTaskNumber, pxTaskStatus.pcTaskName, pxTaskStatus.usStackHighWaterMark, pxTaskStatus.eCurrentState);
    char buffer[1000] = {};
    Task::stats(buffer);
    // printf("%s\n\n", buffer);
    delay(1000);
    Task::list(buffer);
    // printf("%s\n\n", buffer);
    // task1.remove();
}

extern "C" void app_main(void)
{
    task1.onLoop(loop1);
    task1.create(3000);


    task2.create(4000, 10, NULL);
    task2.onLoop(loop2);

    while (1)
    {
        task2.suspend();
        delay(1000);
        task2.resume();
        delay(200);
        task1.notify();
    }    
}

/*
void external_stack_function(void)
{
    printf("Executing this printf from external stack! \n");

    char buffer[1000] = {};
    Task::list(buffer);
    printf("%s\n\n", buffer);

}

//Let's suppose we want to call printf using a separated stack space
//allowing the app to reduce its stack size.
#define SIZE 4 * 4096
extern "C" void app_main()
{
    external_stack_function();

    //Allocate a stack buffer, from heap or as a static form:
    portSTACK_TYPE *shared_stack = (uint8_t*)malloc(SIZE * sizeof(portSTACK_TYPE));
    assert(shared_stack != NULL);

    //Allocate a mutex to protect its usage:
    SemaphoreHandle_t printf_lock = xSemaphoreCreateMutex();
    assert(printf_lock != NULL);

    //Call the desired function using the macro helper:
    esp_execute_shared_stack_function(printf_lock,
                                    shared_stack,
                                    SIZE,
                                    external_stack_function);

    vSemaphoreDelete(printf_lock);
    free(shared_stack);

    external_stack_function();

}

*/