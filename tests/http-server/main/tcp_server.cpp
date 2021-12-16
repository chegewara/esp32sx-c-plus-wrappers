#include "esp_log.h"
#include "esp_err.h"
#include "http-server.h"
#include "index.h"

#define TAG "TCP server"

static HttpServer tcp_server;
static int ws_fd = 0;

static esp_err_t request_handler(httpd_req_t *req)
{
    printf("has GET query Key: %d\n", tcp_server.hasKey(req, "test"));
    char buf[500] = {};
    if (req->method == HTTP_POST && tcp_server.getQuery(req, buf, 500) > 0)
    {
        char header[50] ={};
        if (tcp_server.getHeader(req, "content-type", header, 50) > 0)
        {
            printf("Content-type: %s\n", header);
        }
        
        printf("has POST query data: %s\n", buf);
    }
    
    return tcp_server.sendText(req, index_html);
}

static esp_err_t request_handler_chunked(httpd_req_t *req)
{
    const char* chunk1 = "<p>line 1</p>";
    const char* chunk2 = "<p>line 2</p>";
    const char* chunk3 = "<p>line 3</p>";
    tcp_server.sendChunk(req, chunk1);
    tcp_server.sendChunk(req, chunk2);
    tcp_server.sendChunk(req, chunk3);
    return tcp_server.sendChunk(req, NULL, 0);
}

static esp_err_t request_handler_plain(httpd_req_t *req)
{
    tcp_server.unregisterPath("/plain");
    tcp_server.setContentType(req, "text/plain");
    return tcp_server.sendText(req, index_html);
}

static esp_err_t request_handler_error(httpd_req_t *req)
{
    return tcp_server.sendError(req, HTTPD_401_UNAUTHORIZED, "error custom message: Unauthorized access");
}

#ifdef CONFIG_HTTPD_WS_SUPPORT

static void ws_async_send(void *arg)
{
    const char * data = "Async data";
    httpd_ws_frame_t* ws_pkt = tcp_server.buildPacket(data, strlen(data));
    tcp_server.wsSendAsync(ws_fd, ws_pkt);
    free(ws_pkt);
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
    esp_err_t ret = tcp_server.wsGetPacket(req, &ws_pkt, buf, 128);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);

    return ret;
}
#endif

void initTCPserver()
{
    tcp_server.init(32769);
    tcp_server.setPort(80);
    ESP_ERROR_CHECK_WITHOUT_ABORT(tcp_server.start());

    tcp_server.registerPath("/", request_handler);
    tcp_server.registerPath("/chunked", request_handler_chunked);
    tcp_server.registerPath("/plain", request_handler_plain);
    tcp_server.registerPath("/error", request_handler_error);
    tcp_server.registerPath("/", request_handler, HTTP_POST);
#ifdef CONFIG_HTTPD_WS_SUPPORT
    tcp_server.registerPath("/ws", ws_request_handler, HTTP_GET, true);
#endif
    tcp_server.registerPath("*", request_handler);
}

void sendWSpacket()
{
#ifdef CONFIG_HTTPD_WS_SUPPORT
    if(ws_fd != 0)ESP_ERROR_CHECK_WITHOUT_ABORT(httpd_queue_work(tcp_server.getHandle(), ws_async_send, NULL));
#endif
}
