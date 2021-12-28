#pragma once
#include "mbedtls/base64.h"

class Base64
{
private:
    
public:
    Base64();
    ~Base64();
public:
    static int encode(const char *src, size_t slen, char *dst = NULL, size_t dlen = 0);
    static int decode(const char *src, size_t slen, char *dst = NULL, size_t dlen = 0);
    static int encodedSize(const char *src, size_t slen);
    static int decodedSize(const char *src, size_t slen);
};

