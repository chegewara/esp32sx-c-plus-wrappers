#pragma once
#include <vector>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "events.h"

ESP_EVENT_DECLARE_BASE(SOCKET_EVENT);

#define TCP_SOCKET_TYPE SOCK_STREAM
#define UDP_SOCKET_TYPE SOCK_DGRAM
#define RAW_SOCKET_TYPE SOCK_RAW

#define SOCKET_CONNECTED    0
#define SOCKET_DATA         1
#define SOCKET_CLOSE        2

typedef struct 
{
    int len;
    int sockfd;
    struct sockaddr_storage source_addr;
    void* data;
}packet_datagram_t;


class Socket
{
    friend void socket_task(void *p);
private:
    int sock_type;
    int socketfd = -1;
    int addr_family;
    int bind_port = 3333;
    int ip_protocol = 0; // only important when RAW socket
    uint16_t buf_size = 256;
    TaskHandle_t task_handle;
    std::vector<int> tcp_sockets;
public:
    Socket(int type = TCP_SOCKET_TYPE, int family = AF_INET);
    ~Socket() {}
public:
    void setPort(int port = 3333);
    void setType(int type = TCP_SOCKET_TYPE, int family = AF_INET);
    void setBufferSize(uint16_t size);

    void create();
    void setOpt(int level, int name, void* val, int len);
    void setConOpt(int sockfd, int level, int name, void* val, int len);
    // https://man7.org/linux/man-pages/man7/tcp.7.html
    void reuse(bool val);
    void keepAlive(int sockfd, bool val, int idle = 5, int interval = 5, int count = 5);    
    int bind(uint8_t addr = INADDR_ANY, int port = 0);
    void listen();
    int accept(sockaddr_storage* source_addr);
    void start(int stack = 3 * 1024, int prio = 1);
    void stop();
    void send(int sockfd, void* data, int len, struct sockaddr_storage* source_addr = NULL);

    int lastError();

    void close(int sockfd = -1);

    int createMulticast(uint8_t ttl = 60);
    int addToGroup(const char* group);
    void publish(const char* group, void *data, int len);
    int getSocket();

private:
    void onConnect(int sockfd, struct sockaddr_storage* source_addr);
    void onData(void* data, int len);
    void onClose(int err);
};
