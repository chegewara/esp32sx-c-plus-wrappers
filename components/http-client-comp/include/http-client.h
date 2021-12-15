#pragma once

#include "esp_err.h"
#include "esp_http_client.h"



class HttpClient
{
private:
    esp_http_client_handle_t handle;

public:
    HttpClient();
    ~HttpClient();

public:
    void init(const esp_http_client_config_t *config);
    esp_err_t addPostData(const char *data, int len);
    int getPostData(char **data);
    esp_err_t addHeader(const char *key, const char *value);
    esp_err_t method(esp_http_client_method_t method);
    esp_err_t perform();


    esp_err_t open(int write_len = 0);
    void getHeaders();
    int read(char *buffer, int len);
    int write(const char *buffer, int len);
    esp_err_t close();
    esp_err_t cleanup();
    esp_err_t setUrl(const char *url);
    int status();
    int64_t length();
    bool isChunked();
};


