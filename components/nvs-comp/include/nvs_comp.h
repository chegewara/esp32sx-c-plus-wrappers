#pragma once
#include <stdio.h>
#include "esp_err.h"
#include "nvs.h"


class NVS
{
private:
    nvs_handle_t my_handle;
    char part_name[32];
    char _namespace[16];
    nvs_iterator_t itr;
public:
    NVS();
    ~NVS(){};


public:
    esp_err_t init(const char* name = NVS_DEFAULT_PART_NAME);
    esp_err_t open(const char* nmsp);
    void close();
    esp_err_t commit();
    esp_err_t eraseValue(const char* key);
    esp_err_t eraseAll();
    esp_err_t getStats(nvs_stats_t *nvs_stats);
    nvs_iterator_t find(nvs_type_t type = NVS_TYPE_ANY);
    nvs_iterator_t next();
    nvs_iterator_t valueInfo(nvs_entry_info_t *out_info);
    void release();


    esp_err_t getString(const char* key, char* value, size_t* len);
    esp_err_t getValue(const char* key, int8_t* value);
    esp_err_t getValue(const char* key, uint8_t* value);
    esp_err_t getValue(const char* key, int16_t* value);
    esp_err_t getValue(const char* key, uint16_t* value);
    esp_err_t getValue(const char* key, int32_t* value);
    esp_err_t getValue(const char* key, uint32_t* value);
    esp_err_t getValue(const char* key, int64_t* value);
    esp_err_t getValue(const char* key, uint64_t* value);
    esp_err_t getBlob(const char* key, void *value, size_t *length);

    esp_err_t setString(const char* key, const char* value);
    esp_err_t setValue(const char* key, int8_t value);
    esp_err_t setValue(const char* key, uint8_t value);
    esp_err_t setValue(const char* key, int16_t value);
    esp_err_t setValue(const char* key, uint16_t value);
    esp_err_t setValue(const char* key, int32_t value);
    esp_err_t setValue(const char* key, uint32_t value);
    esp_err_t setValue(const char* key, int64_t value);
    esp_err_t setValue(const char* key, uint64_t value);
    esp_err_t setBlob(const char* key,  const void *value, size_t length);

};
