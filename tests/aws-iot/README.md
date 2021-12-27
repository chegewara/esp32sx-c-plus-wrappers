## Test example requirements

Mqtt and shadow requirements:
- aws account
- IoT thing certificates in `things` folder as embedded files or in `certs.h` file
- if certificates are used from `certs.h` then we need to modify `CMakeLists.txt`
- update in main.h: `#define AWS_ACCOUNT "----"`
- update region in main.h: `#define AWS_DOMAIN_URL` 
- update in main.h wifi credentials

In example we are using both, AWS mqtt and AWS shadow, but it is suggested to use only 1 option, unless really required.
`esp-aws-iot` requires `jsmn` component, which is no longer internal component in esp-idf v5.x, but it is cloned/copied from espressif components manager (https://components.espressif.com/component/espressif/jsmn).

## Logs

```
I (533) : wifi base: WIFI_EVENT, event: 2
I (543) WiFi class: wifi_init_sta finished. SSID:esp32 password:espressif
I (543) mqtt: Connecting to AWS... ----ats.iot.us-east-1.amazonaws.com
E (563) aws_iot: failed! mbedtls_net_connect returned -0x52
aws status: -23
E (2273) aws_iot: failed! mbedtls_net_connect returned -0x52
I (2343) : wifi base: WIFI_EVENT, event: 4
aws status: -23
E (3283) aws_iot: failed! mbedtls_net_connect returned -0x52
aws status: -23
E (4293) aws_iot: failed! mbedtls_net_connect returned -0x52
aws status: -23
I (5433) : wifi base: IP_EVENT, event: 0
I (5433) esp_netif_handlers: sta ip: 192.168.0.64, mask: 255.255.255.0, gw: 192.168.0.1
I (9993) aws_iot: Subscribe callback
I (9993) aws_iot: test/test     {"test":123}
I (10233) aws_iot: Subscribe callback
I (10233) aws_iot: test/test    {"test":123}
I (10383) shadow: Connecting to AWS... ----ats.iot.us-east-1.amazonaws.com
I (11143) aws_iot: Subscribe callback
I (11143) aws_iot: test/test    {"test":123}
I (12663) shadow: Update Shadow string is ready: {"state":{"reported":{"windowOpen":false,"temperature":20.000000}}, "clientToken":"test-0"}
I (15223) shadow: Update accepted
I (15423) aws_iot: Subscribe callback 1
I (15423) aws_iot: test/test1   {"test": 123}
```