#include "esp_log.h"
#include "esp_err.h"
#include "http-server.h"
#include "index_ssl.h"

#define TAG "SSL server"

extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
extern const unsigned char cacert_pem_end[]   asm("_binary_cacert_pem_end");
static int cacert_len = cacert_pem_end - cacert_pem_start;

extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
static int prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

static HttpServer ssl_server;
static int wss_fd = 0;

static esp_err_t request_handler_ssl(httpd_req_t *req)
{
    return httpd_resp_sendstr(req, index_html_ssl);
}

#ifdef CONFIG_HTTPD_WS_SUPPORT

static void wss_async_send(void *arg)
{
    const char * data = "Async data";
    httpd_ws_frame_t* ws_pkt = ssl_server.buildPacket(data, strlen(data));
    ssl_server.wsSendAsync(wss_fd, ws_pkt);
    free(ws_pkt);
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
    esp_err_t ret = ssl_server.wsGetPacket(req, &ws_pkt, buf, 128);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);

    return ret;
}
#endif

void initSSLserver()
{
    ssl_server.initSSL(cacert_pem_start, cacert_len, prvtkey_pem_start, prvtkey_len);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ssl_server.startSSL());

    ssl_server.registerPath("/", request_handler_ssl);
    ssl_server.registerPath("/ws", wss_request_handler, HTTP_GET, true);
}

void sendWSSpacket()
{
#ifdef CONFIG_HTTPD_WS_SUPPORT
    if(wss_fd != 0)ESP_ERROR_CHECK_WITHOUT_ABORT(httpd_queue_work(ssl_server.getHandle(), wss_async_send, NULL));
#endif
}
