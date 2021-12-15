#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tools.h"

void shutdown()
{
    printf("esp32 is going to reset now\n");
    printf("%s\n", Tools::idfVersion());
    vTaskDelay(10);
}

extern "C" void app_main(void)
{
    esp_reset_reason_t reason;
    reason = Tools::resetReason();
    printf("reset reason: %d\n", reason);

    printf("Heap info:\n"
    "\t free: %d\n"
    "\t internal: %d\n"
    "\t minimum: %d\n\n", Tools::freeHeap(), Tools::internalFreeHeap(), Tools::minFreeHeap());

    Tools::shutdownHandler(shutdown);

    vTaskDelay(1000);
    Tools::systemAbort();
}

