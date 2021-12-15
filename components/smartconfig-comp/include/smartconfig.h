#pragma once

#include "esp_smartconfig.h"
#include "esp_err.h"


class Smartconfig
{
private:
    
public:
    Smartconfig();
    ~Smartconfig();

public:
    esp_err_t start();
    esp_err_t stop();
    esp_err_t timeout(uint8_t timeout);
    esp_err_t type(smartconfig_type_t type);
    esp_err_t mode(bool enable);
    esp_err_t received(uint8_t *rvd_data, uint8_t len);
};

