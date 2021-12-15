#include <stdio.h>

#include "queue.h"
#include "tools.h"
#include "tasks.h"

Queue queue;
Task task;

static void loop(void* p)
{
    char buf[128] = {};
    if(queue.receive((void*)buf, 0)) // wait in ticks, not ms
    {
        printf("receive: count: %d, item: %s\n", queue.count(), buf);
    } else {
        printf("queue is empty => %d\n", queue.count());
    }
    delay(500);
}

extern "C" void app_main(void)
{
    task.create();
    task.onLoop(loop);
    queue.create(3, 128);

    while (1)
    {
        delay(100);
        if(!queue.send("test", 10)) // wait in ticks, not ms
        {
            printf("queue is full\n");
            queue.reset();
        } else {
            printf("send item to queue\n");
        }
    }
}
