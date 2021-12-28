#include <stdio.h>
#include "base64-comp.h"



Base64::Base64()
{
}

Base64::~Base64()
{
}

int Base64::encode(const char *src, size_t slen, char *dst, size_t dlen)
{
    size_t olen = 0;
    int ret = mbedtls_base64_encode((uint8_t*)dst, dlen, &olen, (const uint8_t*)src, slen);
    if(ret < 0) return ret;
    return olen;
}

int Base64::decode(const char *src, size_t slen, char *dst, size_t dlen)
{
    size_t olen = 0;
    int ret = mbedtls_base64_decode((uint8_t*)dst, dlen, &olen, (const uint8_t*)src, slen);
    if(ret < 0) return ret;
    return olen;
}

int Base64::encodedSize(const char *src, size_t slen)
{
    size_t olen = 0;
    int ret = mbedtls_base64_encode(NULL, 0, &olen, (const uint8_t*)src, slen);
    return olen;
}

int Base64::decodedSize(const char *src, size_t slen)
{
    size_t olen = 0;
    int ret = mbedtls_base64_decode(NULL, 0, &olen, (const uint8_t*)src, slen);
    return olen;
}




