#pragma once

#include "esp_http_server.h"
#if CONFIG_ESP_HTTPS_SERVER_ENABLE
#include "esp_https_server.h"
#endif
#include "esp_err.h"

typedef esp_err_t (*request_handler_t)(httpd_req_t *r);

class HttpServer
{
private:
    // httpd_handle_t handle;
    httpd_config_t config;
#ifdef CONFIG_ESP_HTTPS_SERVER_ENABLE
    httpd_ssl_config_t config_ssl;
#endif
    uint16_t _port;

public:
    HttpServer(uint16_t port = 80);
    ~HttpServer();

public:
    void init(uint16_t ctrl_port = 32768);
    void init(httpd_config_t _config);
    void initSSL(const uint8_t* cacert_pem, int cacert_len, const uint8_t* prvtkey_pem, int prvtkey_len);

    esp_err_t start();
    esp_err_t startSSL();

    void setPort(uint16_t port);
    esp_err_t registerPath(const char* uri, request_handler_t fn, httpd_method_t method = HTTP_GET, bool is_websocket = false);


    httpd_handle_t handle;
};
