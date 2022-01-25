#pragma once
#include "lwip/sockets.h"
#include "events.h"

ESP_EVENT_DECLARE_BASE(SOCKET_EVENT);

#define TCP_SOCKET_TYPE SOCK_STREAM
#define UDP_SOCKET_TYPE SOCK_DGRAM
#define RAW_SOCKET_TYPE SOCK_RAW

#define SOCKET_DATA     0
#define SOCKET_CLOSE    1

typedef struct 
{
    int len;
    struct sockaddr_storage source_addr;
    void* data;
}packet_datagram_t;


class Socket
{
    friend void socket_task(void *p);
private:
    int sock_type;
    int socketfd = -1;
    int socketfda = -1;
    int addr_family;
    int bind_port = 3333;
    int ip_protocol = 0; // only important when RAW socket
    int buf_size = 256;
    TaskHandle_t task_handle;
    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
public:
    Socket(int type = TCP_SOCKET_TYPE, int family = AF_INET);
    ~Socket() {}
public:
    void setPort(int port = 3333);
    void setType(int type = TCP_SOCKET_TYPE, int family = AF_INET);

    void create();
    void setOpt(int level, int name, int* val);
    void setConOpt(int level, int name, int* val);
    // https://man7.org/linux/man-pages/man7/tcp.7.html
    void reuse(bool val);
    void keepAlive(bool val, int idle = 5, int interval = 5, int count = 5);    
    void bind(uint8_t addr = INADDR_ANY, int port = 0);
    void listen();
    void accept();
    void start(int stack = 3 * 1024, int prio = 1);
    void stop();
    void send(void* data, int len, struct sockaddr_storage* source_addr = NULL);

    int lastError();

    void close();
private:
    void onClose();
    void onData(void* data, int len);
};