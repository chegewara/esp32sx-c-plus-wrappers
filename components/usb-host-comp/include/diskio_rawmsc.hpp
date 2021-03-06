#pragma once
#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)

#include <string.h>
#include "diskio_impl.h"
#include "ffconf.h"
#include "ff.h"
#include "esp_log.h"
#include "diskio_rawflash.h"
#include "esp_compiler.h"

esp_err_t ff_msc_register_raw_partition(BYTE pdrv, USBmscDevice* part_handle);
BYTE ff_msc_get_pdrv_raw(uint8_t lun);
esp_err_t vfs_fat_rawmsc_mount(const char* base_path,
    const esp_vfs_fat_mount_config_t* mount_config, uint8_t lun);

void vfs_fat_rawmsc_unmount(char *base_path, uint8_t lun);

#endif
