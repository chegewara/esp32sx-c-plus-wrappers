#pragma once

// #define USE_EMBEDDED

#ifdef USE_EMBEDDED
extern const char aws_thing_cert[] asm("_binary_test_cert_pem_start");
extern const char aws_thing_key[] asm("_binary_test_key_pem_start");;
#else
#include "certs.h"
#endif


#define AWS_ACCOUNT "---"
#define AWS_DOMAIN_URL "-ats.iot.us-east-1.amazonaws.com"
#define AWS_ENDPOINT AWS_ACCOUNT AWS_DOMAIN_URL
#define AWS_PORT 8883

#define WIFI_SSID "esp32"
#define WIFI_PASS "espressif"

extern AwsIoT awsMqtt;
extern AwsIoT awsShadow;

void connect_aws();
void connect_shadow();
