#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "socket-server.h"

#define TAG "SOCKET"

ESP_EVENT_DEFINE_BASE(SOCKET_EVENT);

void socket_task(void *p)
{
    Socket *me = (Socket *)p;
    int len;
    uint8_t *rx_buffer = (uint8_t *)calloc(me->buf_size, 1);

    if(me->sock_type == TCP_SOCKET_TYPE){
        me->accept();
        while (1){
            memset(rx_buffer, 0, me->buf_size);
            len = recv(me->socketfda, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0)
            {
                ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            }
            else if (len == 0)
            {
                ESP_LOGW(TAG, "Connection closed");
                me->onClose();
                break;
            }
            else
            {
                ESP_LOGI(TAG, "Received %d bytes", len);
                void* data = calloc(len, 1);
                memcpy(data, rx_buffer, len);
                packet_datagram_t packet = {
                    .len = len,
                    .data = data
                };
                me->onData(&packet, sizeof(packet_datagram_t));
            }
        }
    } else if(me->sock_type == UDP_SOCKET_TYPE) {
        while (1)
        {
            ESP_LOGI(TAG, "Waiting for data");
            memset(rx_buffer, 0, me->buf_size);
            packet_datagram_t packet;
            socklen_t socklen = sizeof(packet.source_addr);
            int len = recvfrom(me->socketfd, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr *)&packet.source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0)
            {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                me->onClose();
                break;
            }
            // Data received
            else
            {
                void* data = calloc(len, 1);
                packet.len = len;
                packet.data = data;
                memcpy(data, rx_buffer, len);
                me->onData(&packet, sizeof(packet_datagram_t));
            }
        }
    }

    me->close();
    printf("delete socket task\n\n");
    free(rx_buffer);
    vTaskDelete(NULL);
}

Socket::Socket(int type, int family)
{
    sock_type = type;
    addr_family = family;
}

void Socket::setPort(int port)
{
    bind_port = port;
}

void Socket::setType(int type, int family)
{
    sock_type = type;
    addr_family = family;
}

void Socket::create()
{
    socketfd = socket(addr_family, sock_type, ip_protocol);
    reuse(true);
}

void Socket::setOpt(int level, int name, int *val)
{
    setsockopt(socketfd, level, name, val, sizeof(int));
}

void Socket::setConOpt(int level, int name, int *val)
{
    setsockopt(socketfda, level, name, val, sizeof(int));
}

void Socket::reuse(bool val)
{
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(bool));
}

void Socket::bind(uint8_t addr, int port)
{
    if (port > 0)
        bind_port = port;

    struct sockaddr_storage dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(addr);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(bind_port);
    int err = ::bind(socketfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
    }
    ESP_LOGI(TAG, "Socket bound, port %d", bind_port);
}

void Socket::listen()
{
    if (sock_type == TCP_SOCKET_TYPE)
    {
        int err = ::listen(socketfd, 1);
        if (err != 0)
        {
            ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        }
    }
}

void Socket::accept()
{
    socklen_t addr_len = sizeof(source_addr);
    if (sock_type == TCP_SOCKET_TYPE)
    {
        socketfda = ::accept(socketfd, (struct sockaddr *)&source_addr, &addr_len);
        keepAlive(true);
    }
    else if (sock_type == UDP_SOCKET_TYPE)
    {
        ESP_LOGD(TAG, "UDP server don't use accept");
    }
}

void Socket::start(int stack, int prio)
{
    xTaskCreate(socket_task, "socket_task", stack, this, prio, &task_handle);
}

void Socket::stop()
{
    if(task_handle != NULL) vTaskDelete(task_handle);
    close();
    ::close(socketfd);
    socketfd = -1;
    task_handle = NULL;
}

void Socket::keepAlive(bool val, int idle, int interval, int count)
{
    setConOpt(SOL_SOCKET, SO_KEEPALIVE, (int*)&val);
    setConOpt(IPPROTO_TCP, TCP_KEEPIDLE, &idle);
    setConOpt(IPPROTO_TCP, TCP_KEEPINTVL, &interval);
    setConOpt(IPPROTO_TCP, TCP_KEEPCNT, &count);
}

void Socket::close()
{
    if(sock_type == TCP_SOCKET_TYPE){
        shutdown(socketfda, SHUT_RDWR);
        ::close(socketfda);
        socketfda = -1;
    }
    else if(sock_type == UDP_SOCKET_TYPE){
    } else {
        printf("not supported\n");
    }
}

int Socket::lastError()
{
    return errno;
}

void Socket::send(void *data, int len, struct sockaddr_storage* source_addr)
{
    if(sock_type == TCP_SOCKET_TYPE){
        int to_write = len;
        while (to_write > 0)
        {
            int written = ::send(socketfda, data + (len - to_write), to_write, 0);
            if (written < 0)
            {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            to_write -= written;
        }
    } else if(sock_type == UDP_SOCKET_TYPE){
        int err = sendto(socketfd, data, len, 0, (struct sockaddr *)source_addr, sizeof(*source_addr));
        if (err < 0)
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        }
    } else {
        printf("not supported\n");
    }
}

void Socket::onClose()
{
    EventLoop::postDefault(SOCKET_EVENT, SOCKET_CLOSE);
}

void Socket::onData(void *data, int len)
{
    EventLoop::postDefault(SOCKET_EVENT, SOCKET_DATA, data, len, 100);
}
