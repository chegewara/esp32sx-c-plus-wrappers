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
        packet_datagram_t packet;
        int socketfda = me->accept(&packet.source_addr);
        packet.sockfd = socketfda;
        while (1){
            memset(rx_buffer, 0, me->buf_size);
            len = recv(socketfda, rx_buffer, me->buf_size, 0);
            if (len < 0)
            {
                ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
                me->onClose(errno);
                me->close(socketfda);
                break;
            }
            else if (len == 0)
            {
                ESP_LOGD(TAG, "Connection closed");
                me->onClose(0);
                me->close(socketfda);
                break;
            }
            else
            {
                ESP_LOGD(TAG, "Received %d bytes", len);
                void* data = calloc(len, 1);
                packet.len = len;
                packet.data = data;
                memcpy(data, rx_buffer, len);
                me->onData(&packet, sizeof(packet_datagram_t));
            }
        }
    } else if(me->sock_type == UDP_SOCKET_TYPE) {
        while (1)
        {
            ESP_LOGD(TAG, "Waiting for data");
            memset(rx_buffer, 0, me->buf_size);
            packet_datagram_t packet;
            socklen_t socklen = sizeof(packet.source_addr);
            int len = recvfrom(me->socketfd, rx_buffer, me->buf_size, 0, (struct sockaddr *)&packet.source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0)
            {
                ESP_LOGW(TAG, "recvfrom failed: errno %d", errno);
                me->onClose(errno);
                me->close();
                break;
            }
            // Data received
            else
            {
                void* data = calloc(len, 1);
                packet.len = len;
                packet.data = data;
                packet.sockfd = me->socketfd;
                memcpy(data, rx_buffer, len);
                me->onData(&packet, sizeof(packet_datagram_t));
            }
        }
    }
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

void Socket::setBufferSize(uint16_t size)
{
    buf_size = size;
}

void Socket::create()
{
    socketfd = socket(addr_family, sock_type, ip_protocol);
    reuse(true);
}

void Socket::setOpt(int level, int name, void* val, int len)
{
    setsockopt(socketfd, level, name, val, len);
}

void Socket::setConOpt(int sockfd, int level, int name, void* val, int len)
{
    setsockopt(sockfd, level, name, val, len);
}

void Socket::reuse(bool val)
{
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(bool));
}

int Socket::bind(uint8_t addr, int port)
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
        // bind may fail with EADDRINUSE when esp32 disconnect server and is trying to re-create and bind socket
        // in suck case 120 seconds 
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        return errno;
    }
    ESP_LOGD(TAG, "Socket bound, port %d", bind_port);
    return 0;
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

int Socket::accept(sockaddr_storage* source_addr)
{
    int sockfd = -1;
    socklen_t addr_len = sizeof(*source_addr);
    if (sock_type == TCP_SOCKET_TYPE)
    {
        do{
            if(socketfd > 0){
                fcntl(socketfd, F_SETFL, O_NONBLOCK); // i decided to make accept non blocking, because socketfd may change in meantime
                sockfd = ::accept(socketfd, (struct sockaddr *)source_addr, &addr_len);
            }
            vTaskDelay(10);
        }while(sockfd < 0);
        tcp_sockets.push_back(sockfd);
        onConnect(sockfd, source_addr);
        keepAlive(sockfd, true);
        start();
    }
    else if (sock_type == UDP_SOCKET_TYPE)
    {
        ESP_LOGD(TAG, "UDP server don't use accept");
    }
    return sockfd;
}

void Socket::start(int stack, int prio)
{
    if(xTaskCreate(socket_task, "socket_task", stack, this, prio, &task_handle) == pdTRUE)
    {
        ESP_LOGD(TAG, "create task: %p", task_handle);
    }
}

void Socket::stop()
{
    if(socketfd < 0) return;
    for (std::vector<int>::iterator it = tcp_sockets.begin(); it != tcp_sockets.end(); ++it)
    {
        int sockfd = *it;
        int err = ::shutdown(sockfd, SHUT_RDWR);
        if(err < 0) ESP_LOGW(TAG, "shutdown sockfd: %d, errno: %d", sockfd, errno);
    }
    vTaskDelete(task_handle);

    tcp_sockets.erase(tcp_sockets.begin(), tcp_sockets.end());
    int err = ::close(socketfd);
    if(err < 0) ESP_LOGW(TAG, "close socketfd: %d, errno: %d", socketfd, errno);
    socketfd = -1;
}

void Socket::keepAlive(int sockfd, bool val, int idle, int interval, int count)
{
    setConOpt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (bool*)&val, sizeof(bool));
    setConOpt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));
    setConOpt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));
    setConOpt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(int));
}

void Socket::close(int sockfd)
{
    if(sock_type == TCP_SOCKET_TYPE){
        int err = ::close(sockfd);
        if(err < 0) ESP_LOGW(TAG, "close sockfd: %d, errno: %d", sockfd, errno);
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

void Socket::send(int sockfd, void *data, int len, struct sockaddr_storage* source_addr)
{
    if(sock_type == TCP_SOCKET_TYPE){
        int to_write = len;
        while (to_write > 0)
        {
            int written = ::send(sockfd, data + (len - to_write), to_write, 0);
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

void Socket::onConnect(int sockfd, struct sockaddr_storage* source_addr)
{
    packet_datagram_t packet = { };
    packet.sockfd = sockfd;

    memcpy(&packet.source_addr, source_addr, sizeof(struct sockaddr_storage));
    EventLoop::postDefault(SOCKET_EVENT, SOCKET_CONNECTED, &packet, sizeof(packet_datagram_t), 100);
}

void Socket::onData(void *data, int len)
{
    EventLoop::postDefault(SOCKET_EVENT, SOCKET_DATA, data, len, 100);
}

void Socket::onClose(int err)
{
    EventLoop::postDefault(SOCKET_EVENT, SOCKET_CLOSE, &err, sizeof(int));
}

int Socket::getSocket()
{
    return socketfd;
}

int Socket::createMulticast(uint8_t ttl)
{
    struct sockaddr_in saddr = { };
    int err = 0;

    socketfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (socketfd < 0) {
        ESP_LOGE(TAG, "Failed to create socket. Error %d", errno);
        return errno;
    }

    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(bind_port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = ::bind(socketfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (err < 0) {
        ESP_LOGE(TAG, "Failed to bind socket. Error %d", errno);
        return errno;
    }

    setsockopt(socketfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(uint8_t));
    if (err < 0) {
        ESP_LOGE(TAG, "Failed to set IP_MULTICAST_TTL. Error %d", errno);
        return errno;
    }

    return 0;
}

int Socket::addToGroup(const char* group)
{
    struct ip_mreq imreq = { };
    int err = 0;
    imreq.imr_interface.s_addr = IPADDR_ANY;

    err = inet_aton(group, &imreq.imr_multiaddr.s_addr);
    if (err != 1) {
        ESP_LOGE(TAG, "Configured IPV4 multicast address '%s' is invalid.", group);
        goto err;
    }
    ESP_LOGD(TAG, "Configured IPV4 Multicast address %s", inet_ntoa(imreq.imr_multiaddr.s_addr));
    if (!IP_MULTICAST(ntohl(imreq.imr_multiaddr.s_addr))) {
        ESP_LOGW(TAG, "Configured IPV4 multicast address '%s' is not a valid multicast address. This will probably not work.", group);
    }

    err = setsockopt(socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imreq, sizeof(struct ip_mreq));
    if (err < 0) {
        ESP_LOGE(TAG, "Failed to set IP_ADD_MEMBERSHIP. Error %d", errno);
        goto err;
    }

 err:
    return errno;
}

void Socket::publish(const char* group, void *data, int len)
{
    struct addrinfo hints = { };
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = addr_family;

    struct addrinfo *res;

    int err = getaddrinfo(group, NULL, &hints, &res);
    if (err < 0) {
        ESP_LOGE(TAG, "getaddrinfo() failed for IPV4 destination address. error: %d", err);
        return;
    }
    if (res == 0) {
        ESP_LOGE(TAG, "getaddrinfo() did not return any addresses");
        return;
    }
    ((struct sockaddr_in *)res->ai_addr)->sin_port = htons(bind_port);

    err = sendto(socketfd, data, len, 0, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (err < 0) {
        ESP_LOGE(TAG, "IPV4 sendto failed. errno: %d", errno);
    }
}
