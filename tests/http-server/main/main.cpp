/**
 * http/s servertest example
 * this example is running 2 servers, 1 http and 1 https server, each with websocket and async sending
*/
#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "http-server.h"
#include "wifi.h"
#include "nvs_comp.h"

#define TAG ""

#define SSID "esp32"
#define PASSWORD "espressif"
#define AP_SSID "esp32"

static const char index_html_ssl[] = {
"<html>"
"<script>"
"document.addEventListener('DOMContentLoaded', event => {"
""
"    let webSocket = new WebSocket('wss://' + window.location.host + '/ws');"
"    webSocket.onopen = function (event) {"
"        console.log('Connected');"
"        webSocket.send('get stats');"
"    };"
""
"    webSocket.onmessage = function (event) {"
"        console.log('onmessage');"
"    };"
""
"    webSocket.onerror = function (event) {"
"        console.log('onerror');"
"    };"
"});"
"</script>"
};

static const char index_html[] = {
"<html>"
"<script>"
"document.addEventListener('DOMContentLoaded', event => {"
""
"    let webSocket = new WebSocket('ws://' + window.location.host + '/ws');"
"    webSocket.onopen = function (event) {"
"        console.log('Connected');"
"        webSocket.send('get stats');"
"    };"
""
"    webSocket.onmessage = function (event) {"
"        console.log('onmessage');"
"    };"
""
"    webSocket.onerror = function (event) {"
"        console.log('onerror');"
"    };"
"});"
"</script>"
};

static WiFi wifi_itf;
static NVS nvs;
static HttpServer ssl_server;
static HttpServer tcp_server;
static int ws_fd = 0;
static int wss_fd = 0;

static esp_err_t request_handler(httpd_req_t *req)
{
    return httpd_resp_sendstr(req, index_html);
}

static esp_err_t request_handler_ssl(httpd_req_t *req)
{
    return httpd_resp_sendstr(req, index_html_ssl);
}

static void wss_async_send(void *arg)
{
    const char * data = "Async data";
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    httpd_ws_send_frame_async(ssl_server.handle, wss_fd, &ws_pkt);
}

static void ws_async_send(void *arg)
{
    const char * data = "Async data";
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    httpd_ws_send_frame_async(tcp_server.handle, ws_fd, &ws_pkt);
}

static esp_err_t ws_request_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ws_fd = httpd_req_to_sockfd(req);
        ESP_LOGI(TAG, "Handshake done, the new websocket connection was opened");
        return ESP_OK;
    }
    uint8_t buf[128] = { 0 };
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = buf;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 128);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }
    ESP_LOGD(TAG, "Got packet with message: %s", ws_pkt.payload);
    ESP_LOGD(TAG, "Packet type: %d", ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        strcmp((char*)ws_pkt.payload,"get stats") == 0) {
        // return trigger_async_send(req->handle, req);
    }

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    return ret;
}

static esp_err_t wss_request_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        wss_fd = httpd_req_to_sockfd(req);
        ESP_LOGI(TAG, "Handshake done, the new websocket connection was opened");
        return ESP_OK;
    }
    uint8_t buf[128] = { 0 };
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = buf;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 128);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }
    ESP_LOGD(TAG, "Got packet with message: %s", ws_pkt.payload);
    ESP_LOGD(TAG, "Packet type: %d", ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        strcmp((char*)ws_pkt.payload,"get stats") == 0) {
        // return trigger_async_send(req->handle, req);
    }

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    return ret;
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs.init());
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf.init());

    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf.enableSTA(SSID, PASSWORD));
    ESP_ERROR_CHECK_WITHOUT_ABORT(wifi_itf.enableAP(AP_SSID));

    extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
    extern const unsigned char cacert_pem_end[]   asm("_binary_cacert_pem_end");
    int cacert_len = cacert_pem_end - cacert_pem_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
    int prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

    ssl_server.initSSL(cacert_pem_start, cacert_len, prvtkey_pem_start, prvtkey_len);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ssl_server.startSSL());

    ssl_server.registerPath("/", request_handler_ssl);
    ssl_server.registerPath("/ws", wss_request_handler, HTTP_GET, true);


    tcp_server.init(32769);
    tcp_server.setPort(80);
    ESP_ERROR_CHECK_WITHOUT_ABORT(tcp_server.start());

    tcp_server.registerPath("/", request_handler);
    tcp_server.registerPath("/ws", ws_request_handler, HTTP_GET, true);


    while(1)
    {
        if(ws_fd != 0)ESP_ERROR_CHECK_WITHOUT_ABORT(httpd_queue_work(tcp_server.handle, ws_async_send, NULL));
        if(wss_fd != 0)ESP_ERROR_CHECK_WITHOUT_ABORT(httpd_queue_work(ssl_server.handle, wss_async_send, NULL));
        vTaskDelay(100);
    }
}
