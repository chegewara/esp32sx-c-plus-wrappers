#include <stdio.h>
#include <cstring>
#include "esp_log.h"
#include "wifi_provisioning/scheme_softap.h"
#include "esp_wifi.h"

#include "wifi-prov.h"

#define TAG "Provision"

/* Event handler for catching system events */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_PROV_EVENT)
    {
        switch (event_id)
        {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "Provisioning started");
            break;
        case WIFI_PROV_CRED_RECV:
        {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials"
                          "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *)wifi_sta_cfg->ssid,
                     (const char *)wifi_sta_cfg->password);
            break;
        }
        case WIFI_PROV_CRED_FAIL:
        {
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                          "\n\tPlease reset to factory and retry provisioning",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning successful");
            break;
        case WIFI_PROV_END:
            /* De-initialize manager once provisioning is finished */
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
        }
    }
}

Provision::Provision()
{
}

Provision::~Provision()
{
}

void Provision::init()
{
    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
        .app_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, this));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, this));
}

bool Provision::isProvisioned()
{
    bool provisioned = false;

    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
    return provisioned;
}

esp_err_t Provision::start(wifi_prov_security_t security, const char *pop)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Starting provisioning");

    if (!service_name)
    {
        uint8_t mac[6];
        const char *ssid_prefix = "PROV_";
        service_name = (char *)calloc(1, 13);
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        snprintf(service_name, 12, "%s%02X%02X%02X", ssid_prefix, mac[3], mac[4], mac[5]);
    }
    /* Start provisioning service */
    err = wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key);

    return err;
}

esp_err_t Provision::customCreate(const char *name)
{
    return wifi_prov_mgr_endpoint_create(name);
}

esp_err_t Provision::customRegister(const char *name, protocomm_req_handler_t prov_data_handler)
{
    return wifi_prov_mgr_endpoint_register(name, prov_data_handler, this);
}

void Provision::serviceName(char *ssid_service)
{
    service_name = ssid_service;
}

void Provision::deinit()
{
    wifi_prov_mgr_deinit();
}
