#include <stdio.h>
#include <cstring>

#include "tasks.h"

TaskHandle_t Task::idle_handle = NULL;

static void task_runner(void *param)
{
    Task *task = (Task *)param;
    while (1)
    {
        task->run();
    }
}

Task::Task(const char *name)
{
    task_handle = NULL;
    _name = name;
}

Task::~Task()
{
    remove();
}

bool Task::create(int stack, uint8_t prio, void *args, bool _static)
{
    idle_handle = xTaskGetIdleTaskHandle();
    _args = args;
#ifdef CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY
    if (_static)
    {
        xStack = (StackType_t *)heap_caps_malloc(stack, MALLOC_CAP_SPIRAM);
        xTaskBuffer = (StaticTask_t *)malloc(sizeof(StaticTask_t));
        task_handle = xTaskCreateStatic(task_runner, _name, stack, this, prio, xStack, xTaskBuffer);
        return task_handle != NULL;
    }
#endif
    return xTaskCreate(task_runner, _name, stack, this, prio, &task_handle) == pdTRUE;
}

void Task::remove()
{
    if (task_handle != NULL)
    {
        vTaskDelete(task_handle);
    }
    task_handle = NULL;
}

void Task::suspend()
{
    if (task_handle != NULL)
        vTaskSuspend(task_handle);
}

void Task::resume()
{
    if (task_handle != NULL)
        vTaskResume(task_handle);
}

void Task::status(TaskStatus_t *pxTaskStatus, bool watermark, eTaskState eState)
{
#ifdef CONFIG_FREERTOS_USE_TRACE_FACILITY
    vTaskGetInfo(task_handle, pxTaskStatus, watermark, eState);
#endif
}

void Task::run()
{
    if (_loop)
        _loop(this);
}

void Task::onLoop(loop_t cb)
{
    _loop = cb;
}

void* Task::args()
{
    return _args;
}

bool Task::notify(uint32_t val, uint8_t action)
{
    if (task_handle == NULL)
        return false;

    bool ret = xTaskNotify(task_handle, val, (eNotifyAction)action);

    return ret;
}

bool Task::notifyWait(uint32_t *val, TickType_t wait)
{
    bool ret = xTaskNotifyWait(0, 0xffffffff, val, wait);

    return ret;
}

bool Task::give()
{
    if (task_handle == NULL)
        return false;

    return xTaskNotifyGive(task_handle);
}

uint32_t Task::take(bool clear, TickType_t wait)
{
    return ulTaskNotifyTake(clear, wait);
}

int Task::count()
{
    return uxTaskGetNumberOfTasks();
}

void Task::list(char *buf)
{
    vTaskList(buf);
}

void Task::stats(char *pcWriteBuffer)
{
#ifdef CONFIG_FREERTOS_USE_TRACE_FACILITY
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    uint32_t ulTotalRunTime, ulStatsAsPercentage;

    /* Make sure the write buffer does not contain a string. */
    *pcWriteBuffer = 0x00;

    /* Take a snapshot of the number of tasks in case it changes while this
   function is executing. */
    uxArraySize = count();

    /* Allocate a TaskStatus_t structure for each task.  An array could be
   allocated statically at compile time. */
    pxTaskStatusArray = (TaskStatus_t *)malloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL)
    {
        /* Generate raw status information about each task. */
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray,
                                           uxArraySize,
                                           &ulTotalRunTime);

        /* For percentage calculations. */
        ulTotalRunTime /= 100UL;

        /* Avoid divide by zero errors. */
        if (ulTotalRunTime > 0)
        {
            /* For each populated position in the pxTaskStatusArray array,
         format the raw data as human readable ASCII data. */
            for (x = 0; x < uxArraySize; x++)
            {
                /* What percentage of the total run time has the task used?
            This will always be rounded down to the nearest integer.
            ulTotalRunTimeDiv100 has already been divided by 100. */
                ulStatsAsPercentage =
                    pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;

                if (ulStatsAsPercentage > 0UL)
                {
                    sprintf(pcWriteBuffer, "%s\t\t%u\t\t%u\r\n",
                            pxTaskStatusArray[x].pcTaskName,
                            pxTaskStatusArray[x].ulRunTimeCounter,
                            ulStatsAsPercentage);
                }
                else
                {
                    /* If the percentage is zero here then the task has
               consumed less than 1% of the total run time. */
                    sprintf(pcWriteBuffer, "%s\t\t%u\t\t<1\r\n",
                            pxTaskStatusArray[x].pcTaskName,
                            pxTaskStatusArray[x].ulRunTimeCounter);
                }

                pcWriteBuffer += strlen((char *)pcWriteBuffer);
            }
        }

        /* The array is no longer needed, free the memory it consumes. */
        vPortFree(pxTaskStatusArray);
    }
#else
    strcpy(pcWriteBuffer, "please enable CONFIG_FREERTOS_USE_TRACE_FACILITY");
#endif
}
