#include <stdio.h>
#include "nvs_comp.h"
NVS nvs;
NVS nvs2;

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs.init());
    ESP_ERROR_CHECK(nvs.open("test"));
    ESP_ERROR_CHECK(nvs.setString("test_v", "this is test string"));
    ESP_ERROR_CHECK(nvs.commit());

    nvs.dump();

    nvs_stats_t nvs_stats;
    ESP_ERROR_CHECK(nvs.getStats(&nvs_stats));
    printf("UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d), Count = (%d)\n",
        nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries, nvs_stats.namespace_count);

    nvs.close();
    printf("\n\n");
    ESP_ERROR_CHECK(nvs2.init("nvs1"));
    ESP_ERROR_CHECK(nvs2.open("test"));
    ESP_ERROR_CHECK(nvs2.setString("test_str", "this is test string on partition with label nvs1"));
    ESP_ERROR_CHECK(nvs2.commit());
    nvs2.dump();
    nvs2.close();

    ESP_ERROR_CHECK(nvs2.getStats(&nvs_stats));
    printf("UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d), Count = (%d)\n",
        nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries, nvs_stats.namespace_count);

}
