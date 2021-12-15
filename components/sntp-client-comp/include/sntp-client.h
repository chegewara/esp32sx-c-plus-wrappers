#pragma once

class SNTP
{
private:
    const char* server;
public:
    SNTP(const char* server = "pool.ntp.org");
    ~SNTP();

public:
    /**
     * @param interval - interval between poll sync
     */
    void init(uint32_t interval = 3600);
    time_t getTime();
    bool syncWait(uint32_t timeout = 0xffffffff);
    void setInterval(uint32_t interval);
    void setEpoch(uint32_t sec, uint32_t us = 0);
    uint32_t getEpoch();
};

