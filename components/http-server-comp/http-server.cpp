#include <stdio.h>
#include "esp_log.h"
#include "http-server.h"

#define TAG "HTTP server"

const char cacert_pem[] = {
"-----BEGIN CERTIFICATE-----\n"
"MIIDKzCCAhOgAwIBAgIUBxM3WJf2bP12kAfqhmhhjZWv0ukwDQYJKoZIhvcNAQEL\n"
"BQAwJTEjMCEGA1UEAwwaRVNQMzIgSFRUUFMgc2VydmVyIGV4YW1wbGUwHhcNMTgx\n"
"MDE3MTEzMjU3WhcNMjgxMDE0MTEzMjU3WjAlMSMwIQYDVQQDDBpFU1AzMiBIVFRQ\n"
"UyBzZXJ2ZXIgZXhhbXBsZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
"ALBint6nP77RCQcmKgwPtTsGK0uClxg+LwKJ3WXuye3oqnnjqJCwMEneXzGdG09T\n"
"sA0SyNPwrEgebLCH80an3gWU4pHDdqGHfJQa2jBL290e/5L5MB+6PTs2NKcojK/k\n"
"qcZkn58MWXhDW1NpAnJtjVniK2Ksvr/YIYSbyD+JiEs0MGxEx+kOl9d7hRHJaIzd\n"
"GF/vO2pl295v1qXekAlkgNMtYIVAjUy9CMpqaQBCQRL+BmPSJRkXBsYk8GPnieS4\n"
"sUsp53DsNvCCtWDT6fd9D1v+BB6nDk/FCPKhtjYOwOAZlX4wWNSZpRNr5dfrxKsb\n"
"jAn4PCuR2akdF4G8WLUeDWECAwEAAaNTMFEwHQYDVR0OBBYEFMnmdJKOEepXrHI/\n"
"ivM6mVqJgAX8MB8GA1UdIwQYMBaAFMnmdJKOEepXrHI/ivM6mVqJgAX8MA8GA1Ud\n"
"EwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBADiXIGEkSsN0SLSfCF1VNWO3\n"
"emBurfOcDq4EGEaxRKAU0814VEmU87btIDx80+z5Dbf+GGHCPrY7odIkxGNn0DJY\n"
"W1WcF+DOcbiWoUN6DTkAML0SMnp8aGj9ffx3x+qoggT+vGdWVVA4pgwqZT7Ybntx\n"
"bkzcNFW0sqmCv4IN1t4w6L0A87ZwsNwVpre/j6uyBw7s8YoJHDLRFT6g7qgn0tcN\n"
"ZufhNISvgWCVJQy/SZjNBHSpnIdCUSJAeTY2mkM4sGxY0Widk8LnjydxZUSxC3Nl\n"
"hb6pnMh3jRq4h0+5CZielA4/a+TdrNPv/qok67ot/XJdY3qHCCd8O2b14OVq9jo=\n"
"-----END CERTIFICATE-----"
};

const char prvtkey_pem[] = {
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCwYp7epz++0QkH\n"
"JioMD7U7BitLgpcYPi8Cid1l7snt6Kp546iQsDBJ3l8xnRtPU7ANEsjT8KxIHmyw\n"
"h/NGp94FlOKRw3ahh3yUGtowS9vdHv+S+TAfuj07NjSnKIyv5KnGZJ+fDFl4Q1tT\n"
"aQJybY1Z4itirL6/2CGEm8g/iYhLNDBsRMfpDpfXe4URyWiM3Rhf7ztqZdveb9al\n"
"3pAJZIDTLWCFQI1MvQjKamkAQkES/gZj0iUZFwbGJPBj54nkuLFLKedw7DbwgrVg\n"
"0+n3fQ9b/gQepw5PxQjyobY2DsDgGZV+MFjUmaUTa+XX68SrG4wJ+DwrkdmpHReB\n"
"vFi1Hg1hAgMBAAECggEAaTCnZkl/7qBjLexIryC/CBBJyaJ70W1kQ7NMYfniWwui\n"
"f0aRxJgOdD81rjTvkINsPp+xPRQO6oOadjzdjImYEuQTqrJTEUnntbu924eh+2D9\n"
"Mf2CAanj0mglRnscS9mmljZ0KzoGMX6Z/EhnuS40WiJTlWlH6MlQU/FDnwC6U34y\n"
"JKy6/jGryfsx+kGU/NRvKSru6JYJWt5v7sOrymHWD62IT59h3blOiP8GMtYKeQlX\n"
"49om9Mo1VTIFASY3lrxmexbY+6FG8YO+tfIe0tTAiGrkb9Pz6tYbaj9FjEWOv4Vc\n"
"+3VMBUVdGJjgqvE8fx+/+mHo4Rg69BUPfPSrpEg7sQKBgQDlL85G04VZgrNZgOx6\n"
"pTlCCl/NkfNb1OYa0BELqWINoWaWQHnm6lX8YjrUjwRpBF5s7mFhguFjUjp/NW6D\n"
"0EEg5BmO0ePJ3dLKSeOA7gMo7y7kAcD/YGToqAaGljkBI+IAWK5Su5yldrECTQKG\n"
"YnMKyQ1MWUfCYEwHtPvFvE5aPwKBgQDFBWXekpxHIvt/B41Cl/TftAzE7/f58JjV\n"
"MFo/JCh9TDcH6N5TMTRS1/iQrv5M6kJSSrHnq8pqDXOwfHLwxetpk9tr937VRzoL\n"
"CuG1Ar7c1AO6ujNnAEmUVC2DppL/ck5mRPWK/kgLwZSaNcZf8sydRgphsW1ogJin\n"
"7g0nGbFwXwKBgQCPoZY07Pr1TeP4g8OwWTu5F6dSvdU2CAbtZthH5q98u1n/cAj1\n"
"noak1Srpa3foGMTUn9CHu+5kwHPIpUPNeAZZBpq91uxa5pnkDMp3UrLIRJ2uZyr8\n"
"4PxcknEEh8DR5hsM/IbDcrCJQglM19ZtQeW3LKkY4BsIxjDf45ymH407IQKBgE/g\n"
"Ul6cPfOxQRlNLH4VMVgInSyyxWx1mODFy7DRrgCuh5kTVh+QUVBM8x9lcwAn8V9/\n"
"nQT55wR8E603pznqY/jX0xvAqZE6YVPcw4kpZcwNwL1RhEl8GliikBlRzUL3SsW3\n"
"q30AfqEViHPE3XpE66PPo6Hb1ymJCVr77iUuC3wtAoGBAIBrOGunv1qZMfqmwAY2\n"
"lxlzRgxgSiaev0lTNxDzZkmU/u3dgdTwJ5DDANqPwJc6b8SGYTp9rQ0mbgVHnhIB\n"
"jcJQBQkTfq6Z0H6OoTVi7dPs3ibQJFrtkoyvYAbyk36quBmNRjVh6rc8468bhXYr\n"
"v/t+MeGJP/0Zw8v/X2CFll96\n"
"-----END PRIVATE KEY-----"
};

HttpServer::HttpServer(uint16_t port)
{
    handle = NULL;
    _port = port;
}

HttpServer::~HttpServer()
{
}

void HttpServer::init(uint16_t ctrl_port)
{
    config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_open_sockets = 7;
    config.stack_size = 10 * 1024;
    config.max_uri_handlers = 15;
    config.server_port = _port;
    config.ctrl_port = ctrl_port;
}

void HttpServer::init(httpd_config_t _config)
{
    config = _config;
}

void HttpServer::initSSL(const uint8_t* cacert_pem, int cacert_len, const uint8_t* prvtkey_pem, int prvtkey_len)
{
#ifdef CONFIG_ESP_HTTPS_SERVER_ENABLE
    config_ssl = HTTPD_SSL_CONFIG_DEFAULT();

    config_ssl.httpd.uri_match_fn = httpd_uri_match_wildcard;

    config_ssl.cacert_len = cacert_len;
    config_ssl.cacert_pem = cacert_pem;
    config_ssl.prvtkey_len = prvtkey_len;
    config_ssl.prvtkey_pem = prvtkey_pem;    
    // config_ssl.transport_mode = HTTPD_SSL_TRANSPORT_INSECURE;
    printf("cert len: %d\n", config_ssl.cacert_len);
    // config_ssl.httpd.max_open_sockets = 7;
    // config_ssl.httpd.stack_size = 10 * 1024;
    // config_ssl.httpd.max_uri_handlers = 15;
    // config_ssl.httpd.server_port = _port;
    // config_ssl.httpd.ctrl_port = ctrl_port;

#endif
}

esp_err_t HttpServer::startSSL()
{
    esp_err_t err = ESP_FAIL;
#ifdef CONFIG_ESP_HTTPS_SERVER_ENABLE
    err = httpd_ssl_start(&handle, &config_ssl);
#endif
    return err;
}

esp_err_t HttpServer::start()
{
    ESP_LOGI(TAG, "Starting HTTP Server");
    if (httpd_start(&handle, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        // free(server_data->scratch);
        // free(server_data);
        return ESP_FAIL;
    }

    return ESP_OK;
}

void HttpServer::setPort(uint16_t port)
{
    _port = port;
    config.server_port = port;
}

esp_err_t HttpServer::registerPath(const char* uri, request_handler_t fn, httpd_method_t method, bool is_websocket)
{
    esp_err_t err = ESP_OK;

    httpd_uri_t path = {
        .uri       = uri,
        .method    = method,
        .handler   = fn,
        .user_ctx  = this,
    };

#ifdef CONFIG_HTTPD_WS_SUPPORT
    path.is_websocket = is_websocket;
#endif
    err = httpd_register_uri_handler(handle, &path);

    return err;
}


