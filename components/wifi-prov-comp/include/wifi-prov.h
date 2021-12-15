#pragma once

#include "esp_err.h"
#include "wifi_provisioning/manager.h"


class Provision
{
private:
    char *service_key = NULL;
    char *service_name = NULL;

public:
    Provision();
    ~Provision();

public:
    void init();
    bool isProvisioned();
    esp_err_t start(wifi_prov_security_t security = WIFI_PROV_SECURITY_0, const char *pop = "abcd1234");
    esp_err_t customCreate(const char* name);
    esp_err_t customRegister(const char *name, protocomm_req_handler_t prov_data_handler);
    void serviceName(char *ssid_service);
    void deinit();
    void reset();
};

