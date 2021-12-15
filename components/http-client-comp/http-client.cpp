#include <stdio.h>
#include "http-client.h"




HttpClient::HttpClient()
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::init(const esp_http_client_config_t *config)
{
    handle = esp_http_client_init(config);
}

esp_err_t HttpClient::perform()
{
    esp_err_t err = ESP_OK;
    err = esp_http_client_perform(handle);

    return err;
}


esp_err_t HttpClient::addPostData(const char *data, int len)
{
    esp_err_t err = ESP_OK;
    err = esp_http_client_set_post_field(handle, data, len);

    return err;
}

int HttpClient::getPostData(char **data)
{
    return esp_http_client_get_post_field(handle, data);
}

esp_err_t HttpClient::addHeader(const char *key, const char *value)
{
    esp_err_t err = ESP_OK;
    err = esp_http_client_set_header(handle, key, value);

    return err;
}

esp_err_t HttpClient::method(esp_http_client_method_t method)
{
    esp_err_t err = ESP_OK;
    err = esp_http_client_set_method(handle, method);

    return err;
}

int HttpClient::read(char *buffer, int len)
{
    return esp_http_client_read(handle, buffer, len);
}

int HttpClient::write(const char *buffer, int len)
{
    return esp_http_client_write(handle, buffer, len);
}

esp_err_t HttpClient::open(int write_len)
{
    esp_err_t err = ESP_OK;
    err = esp_http_client_open(handle, write_len);

    return err;
}

esp_err_t HttpClient::close()
{
    esp_err_t err = ESP_OK;
    err = esp_http_client_close(handle);

    return err;
}

esp_err_t HttpClient::cleanup()
{
    return esp_http_client_cleanup(handle);
}

esp_err_t HttpClient::setUrl(const char *url)
{
    esp_err_t err = ESP_OK;
    err = esp_http_client_set_url(handle, url);

    return err;
}

void HttpClient::getHeaders()
{
    esp_http_client_fetch_headers(handle);
}

int HttpClient::status()
{
    return esp_http_client_get_status_code(handle);
}

int64_t HttpClient::length()
{
    return esp_http_client_get_content_length(handle);
}

bool HttpClient::isChunked()
{
    return esp_http_client_is_chunked_response(handle);
}
