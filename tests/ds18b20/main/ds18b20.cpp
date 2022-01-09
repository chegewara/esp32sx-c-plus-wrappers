#include <stdio.h>

#include "ds18b20.hpp"

DS18B20 sensor(1);

extern "C" void app_main(void)
{
    sensor.init(GPIO_NUM_14);
    sensor.read();
    sensor.addSensors();
    sensor.removeSensors();
    sensor.addSensors();
    while (1)
    {
        sensor.read();
        printf("Temperature readings (degrees C):\n");
        for (size_t i = 0; i <= sensor.getCount(); i++)
        {
            printf("\t%d: %.2f\n", i, sensor.getTemp(i));
        }
        
        vTaskDelay(100);
    }
}
