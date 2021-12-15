#include <stdio.h>
#include <cstring>
#include "esp_log.h"
#include "esp_err.h"

#include "ota.h"

#define TAG "OTA component"
#define SKIP_VERSION_CHECK 1
OTA::OTA()
{
    update_partition = NULL;
    configured = NULL;
    running = NULL;
}

esp_err_t OTA::init(const esp_partition_t *start_from)
{
    esp_err_t err = ESP_OK;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */

    ESP_LOGI(TAG, "Init OTA");

    configured = esp_ota_get_boot_partition();
    running = esp_ota_get_running_partition();

    if (configured != running)
    {
        ESP_LOGI(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGI(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

    update_partition = esp_ota_get_next_update_partition(start_from);
    if(!update_partition) return ESP_FAIL;
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);

    return err;
}

esp_err_t OTA::begin()
{
    esp_err_t err = ESP_OK;
    if(!update_partition) return ESP_FAIL;

    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        esp_ota_abort(update_handle);
        return err;
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");

    return err;
}

esp_err_t OTA::write(uint8_t *data, size_t len)
{
    esp_err_t err = compare(data, len);
    if(err) return err;

    err = esp_ota_write(update_handle, (const void *)data, len);
    if (err != ESP_OK)
    {
        esp_ota_abort(update_handle);
        return err;
    }

    return err;
}

esp_err_t OTA::finish()
{
    esp_err_t err = esp_ota_end(update_handle);
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED)
        {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        }
        else
        {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        return err;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        return err;
    }

    return err;
}

esp_err_t OTA::compare(uint8_t* data, size_t len)
{
    esp_err_t err = ESP_OK;
    esp_app_desc_t new_app_info;
    if (image_header_was_checked == false)
    {
        if (len > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))
        {
            // check current version with downloading
            memcpy(&new_app_info, &data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
            ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

            esp_app_desc_t running_app_info;
            if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
            {
                ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
            }

            const esp_partition_t *last_invalid_app = esp_ota_get_last_invalid_partition();
            esp_app_desc_t invalid_app_info;
            if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK)
            {
                ESP_LOGE(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
            }

            // check current version with last invalid partition
            if (last_invalid_app != NULL)
            {
                if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0)
                {
                    ESP_LOGE(TAG, "New version is the same as invalid version.");
                    ESP_LOGE(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                    ESP_LOGE(TAG, "The firmware has been rolled back to the previous version.");
                    return ESP_FAIL;
                }
            }

            image_header_was_checked = true;

            if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0)
            {
#ifndef SKIP_VERSION_CHECK
                ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
                return ESP_FAIL;
#endif
                ESP_LOGW(TAG, "Current running version is the same as a new. We will continue the update anyway.");
            }
        }
        else
        {
            return ESP_ERR_INVALID_SIZE;
        }
    }
    return err;
}

// esp_http_client_config_t config = {
//     .url = url,

//     // .cert_pem = (char *)server_cert_pem_start,
//     .timeout_ms = 10000,
//     .transport_type = HTTP_TRANSPORT_OVER_TCP,
//     .keep_alive_enable = true,
// };

// config.skip_cert_common_name_check = true;

// esp_http_client_handle_t client = esp_http_client_init(&config);
// if (client == NULL)
// {
//     ESP_LOGE(TAG, "Failed to initialise HTTP connection");
//     update_ota_status(0, -1, "Failed to initialise HTTP connection");
//     task_fatal_error();
//     // return;
// }

// err = esp_http_client_open(client, 0);
// if (err != ESP_OK)
// {
//     ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
//     update_ota_status(0, -1, "Failed to open HTTP connection");
//     esp_http_client_cleanup(client);
//     task_fatal_error();
//     return;
// }
// esp_http_client_fetch_headers(client);

// int binary_file_length = 0;
// /*deal with all receive packet*/
// bool image_header_was_checked = false;
// while (1)
// {
//     int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
//     if (data_read < 0)
//     {
//         ESP_LOGE(TAG, "Error: SSL data read error");
//         update_ota_status(binary_file_length, -1, "Error: SSL data read error");
//         http_cleanup(client);
//         task_fatal_error();
//         return;
//     }
//     else if (data_read > 0)
//     {
//         if (image_header_was_checked == false)
//         {
//             esp_app_desc_t new_app_info;
//             if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))
//             {
//                 // check current version with downloading
//                 memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
//                 ESP_LOGE(TAG, "New firmware version: %s", new_app_info.version);

//                 esp_app_desc_t running_app_info;
//                 if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
//                 {
//                     ESP_LOGE(TAG, "Running firmware version: %s", running_app_info.version);
//                 }

//                 const esp_partition_t *last_invalid_app = esp_ota_get_last_invalid_partition();
//                 esp_app_desc_t invalid_app_info;
//                 if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK)
//                 {
//                     ESP_LOGE(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
//                 }

//                 // check current version with last invalid partition
//                 if (last_invalid_app != NULL)
//                 {
//                     if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0)
//                     {
//                         ESP_LOGE(TAG, "New version is the same as invalid version.");
//                         ESP_LOGE(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
//                         ESP_LOGE(TAG, "The firmware has been rolled back to the previous version.");
//                         http_cleanup(client);
//                         infinite_loop();
//                         return;
//                     }
//                 }
// #ifndef SKIP_VERSION_CHECK
//                 if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0)
//                 {
//                     ESP_LOGE(TAG, "Current running version is the same as a new. We will not continue the update.");
//                     http_cleanup(client);
//                     infinite_loop();
//                     return;
//                 }
// #endif

//                 image_header_was_checked = true;

//                 err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
//                 if (err != ESP_OK)
//                 {
//                     ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
//                     update_ota_status(binary_file_length, -1, "esp_ota_begin failed");
//                     http_cleanup(client);
//                     esp_ota_abort(update_handle);
//                     task_fatal_error();
//                     return;
//                 }
//                 ESP_LOGE(TAG, "esp_ota_begin succeeded");
//             }
//             else
//             {
//                 ESP_LOGE(TAG, "received package is not fit len");
//                 update_ota_status(binary_file_length, -1, "received package is not fit len");
//                 http_cleanup(client);
//                 esp_ota_abort(update_handle);
//                 task_fatal_error();
//                 return;
//             }
//         }
//         err = esp_ota_write(update_handle, (const void *)ota_write_data, data_read);
//         if (err != ESP_OK)
//         {
//             update_ota_status(binary_file_length, -1, "esp_ota_write");
//             http_cleanup(client);
//             esp_ota_abort(update_handle);
//             task_fatal_error();
//             return;
//         }
//         if (binary_file_length == 0)
//             update_ota_status(binary_file_length, false, "begin");
//         binary_file_length += data_read;
//         if ((binary_file_length % (20 * 1024)) == 0)
//         {
//             update_ota_status(binary_file_length, false, "progress");
//             ESP_LOGI(TAG, "Written image length %d", binary_file_length);
//         }
//     }
//     else if (data_read == 0)
//     {
//         /*
//             * As esp_http_client_read never returns negative error code, we rely on
//             * `errno` to check for underlying transport connectivity closure if any
//             */
//         if (errno == ECONNRESET || errno == ENOTCONN)
//         {
//             ESP_LOGE(TAG, "Connection closed, errno = %d", errno);
//             break;
//         }
//         if (esp_http_client_is_complete_data_received(client) == true)
//         {
//             ESP_LOGE(TAG, "Connection closed");
//             break;
//         }
//     }
// }
// ESP_LOGE(TAG, "Total Write binary data length: %d", binary_file_length);
// if (esp_http_client_is_complete_data_received(client) != true)
// {
//     ESP_LOGE(TAG, "Error in receiving complete file");
//     update_ota_status(binary_file_length, -1, "error in receiving complete file");
//     http_cleanup(client);
//     esp_ota_abort(update_handle);
//     task_fatal_error();
//     return;
// }

// ESP_LOGE(TAG, "Prepare to restart system!");
// update_ota_status(binary_file_length, true, "finished");
// delay(3000);
// esp_restart();
// return;