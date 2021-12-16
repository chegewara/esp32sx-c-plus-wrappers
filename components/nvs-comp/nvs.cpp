#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs_comp.h"

NVS::NVS(){ }

esp_err_t NVS::init(const char* name)
{
    esp_err_t err;
    strcpy(part_name, name);

    err = nvs_flash_init_partition(part_name);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase_partition(part_name));
        err = nvs_flash_init_partition(part_name);
    }

    itr = NULL;
    return err;
}

esp_err_t NVS::open(const char* nmsp)
{
    esp_err_t err = ESP_OK;
    strcpy(_namespace, nmsp);
    err = nvs_open_from_partition(part_name, _namespace, NVS_READWRITE, &my_handle);

    return err;
}

void NVS::close()
{
    nvs_close(my_handle);
}

esp_err_t NVS::commit()
{
    return nvs_commit(my_handle);
}

esp_err_t NVS::eraseValue(const char *key)
{
    esp_err_t err = ESP_OK;
    err = nvs_erase_key(my_handle, key);
    return err;
}

esp_err_t NVS::eraseAll()
{
    esp_err_t err = ESP_OK;
    err = nvs_erase_all(my_handle);
    return err;
}

esp_err_t NVS::getStats(nvs_stats_t *nvs_stats)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_stats(part_name, nvs_stats);
    return err;
}

nvs_iterator_t NVS::find(nvs_type_t type)
{
    itr = nvs_entry_find(part_name, _namespace, type);
    return itr;
}

nvs_iterator_t NVS::next()
{
    if (itr == NULL)
    {
        return find();
    }

    return nvs_entry_next(itr);
}

nvs_iterator_t NVS::valueInfo(nvs_entry_info_t *out_info)
{
    itr = next();
    if(itr) nvs_entry_info(itr, out_info);
    return itr;    
}

void NVS::release()
{
    nvs_release_iterator(itr);
}

void NVS::dump()
{
    nvs_iterator_t it;
    do{
        nvs_entry_info_t info;
        it = valueInfo(&info);
        if(it) {
            printf("namespace: %s, key '%s', type %d \n", info.namespace_name, info.key, info.type);

            char str[128] = {};
            size_t len = 128;
            uint8_t val = 0;
            if(info.type == NVS_TYPE_STR) 
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(getString(info.key, str, &len));
                printf("key: %s, value: %s\n", info.key, str);
            }
            else if(info.type == NVS_TYPE_BLOB) 
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(getBlob(info.key, str, &len));
                printf("key: %s, value: %s\n", info.key, str);
            }
            else 
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(getValue(info.key, &val));
                printf("key: %s, value: %d\n", info.key, val);
            }
        }
    }while(it != NULL) ;
    release();
}

//-----------------------get value by type------------------------//
esp_err_t NVS::getString(const char* key, char* value, size_t* len)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_str(my_handle, key, value, len);
    return err;
}

esp_err_t NVS::getValue(const char* key, int8_t* value)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_i8(my_handle, key, value);
    return err;
}

esp_err_t NVS::getValue(const char* key, uint8_t* value)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_u8(my_handle, key, value);
    return err;
}

esp_err_t NVS::getValue(const char* key, int16_t* value)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_i16(my_handle, key, value);
    return err;
}

esp_err_t NVS::getValue(const char* key, uint16_t* value)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_u16(my_handle, key, value);
    return err;
}

esp_err_t NVS::getValue(const char* key, int32_t* value)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_i32(my_handle, key, value);
    return err;
}

esp_err_t NVS::getValue(const char* key, uint32_t* value)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_u32(my_handle, key, value);
    return err;
}

esp_err_t NVS::getValue(const char* key, int64_t* value)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_i64(my_handle, key, value);
    return err;
}

esp_err_t NVS::getValue(const char* key, uint64_t* value)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_u64(my_handle, key, value);
    return err;
}

esp_err_t NVS::getBlob(const char* key, void *out_value, size_t *length)
{
    esp_err_t err = ESP_OK;
    err = nvs_get_blob(my_handle, key, out_value, length);
    return err;
}

//-----------------------set value by type------------------------//
esp_err_t NVS::setString(const char* key, const char* value)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_str(my_handle, key, value);
    return err;
}

esp_err_t NVS::setValue(const char* key, int8_t value)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_i8(my_handle, key, value);
    return err;
}

esp_err_t NVS::setValue(const char* key, uint8_t value)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_u8(my_handle, key, value);
    return err;
}

esp_err_t NVS::setValue(const char* key, int16_t value)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_i16(my_handle, key, value);
    return err;
}

esp_err_t NVS::setValue(const char* key, uint16_t value)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_u16(my_handle, key, value);
    return err;
}

esp_err_t NVS::setValue(const char* key, int32_t value)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_i32(my_handle, key, value);
    return err;
}

esp_err_t NVS::setValue(const char* key, uint32_t value)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_u32(my_handle, key, value);
    return err;
}

esp_err_t NVS::setValue(const char* key, int64_t value)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_i64(my_handle, key, value);
    return err;
}

esp_err_t NVS::setValue(const char* key, uint64_t value)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_u64(my_handle, key, value);
    return err;
}

esp_err_t NVS::setBlob(const char* key,  const void *value, size_t length)
{
    esp_err_t err = ESP_OK;
    err = nvs_set_blob(my_handle, key, value, length);
    return err;
}





