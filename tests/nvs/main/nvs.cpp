#include <stdio.h>
#include "nvs_comp.h"

extern "C" void app_main(void)
{
    NVS nvs;
    NVS nvs2;
    ESP_ERROR_CHECK(nvs2.init("nvs1"));

    ESP_ERROR_CHECK(nvs.init());
    ESP_ERROR_CHECK(nvs.open("test"));
    ESP_ERROR_CHECK(nvs.setString("test_v", "this is test string"));
    ESP_ERROR_CHECK(nvs.commit());
    nvs_iterator_t it;
    do{
        nvs_entry_info_t info;
        it = nvs.valueInfo(&info);
        if(it) {
            printf("namespace: %s, key '%s', type '%d' \n", info.namespace_name, info.key, info.type);

            char str[128] = {};
            size_t len = 128;
            ESP_ERROR_CHECK(nvs.getString(info.key, str, &len));
            printf("key: %s, value: %s\n", info.key, str);
        }
    }while(it != NULL) ;
    nvs.release();

    nvs_stats_t nvs_stats;
    ESP_ERROR_CHECK(nvs.getStats(&nvs_stats));
    printf("UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d), Count = (%d)\n",
        nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries, nvs_stats.namespace_count);

    ESP_ERROR_CHECK(nvs2.getStats(&nvs_stats));
    printf("UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d), Count = (%d)\n",
        nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries, nvs_stats.namespace_count);

    nvs.close();
}
