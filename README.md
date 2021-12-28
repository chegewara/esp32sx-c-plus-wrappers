## Info
This is set of C++ esp-idf wrappers which can be used in esp-idf, arduino and PIO projects.
Those components are not meant to be some kind of high quality components to use by all, not yet, its more like speed up of projects i am doing.
Since its early stage all components are still WiP, with no documentation, some are missing `sdkconfig.defaults` and most does not have good quality or any error checks yet.
In the future i may add components for different devices.

NOTE: i am always trying to make components compatible with esp-idf master branch, so if some componets do not build, its most likelythe reason

## Common components

1. wifi                 `done + wip`
2. OTA                  `done + wip`
3. NVS                  `done`
4. I2C (master-slave)   `done + wip`
5. UART                 `done + wip`
6. freertos CLI         `done + wip`
7. websocket client     ``
8. sntp client          `done`
9. http/s client        `done + wip`
10. http/s server       `done + wip`
11. mdns                ``
12. aws-iot             `wip` (aws mqtt and aws shadow)
13. wifi provisioning   `done + wip` (AP only)
14. mqtt                `done + wip`
15. ethernet            `done + wip`
16. smartconfig         `done + wip`
17. json                `done + wip`
18. freertos Queue      `done + wip`
19. freertos tasks      `done + wip`
20. partitions          `???`
21. base64              `done`
22. event loop          `done + wip`
23. timers              `done + wip`



---

## Less common components
101. USB host            `done + wip`
102. USB device          ``
103. tools               `wip` 

1001. azureIoT            `???`



If anyone will find this repository useful and would like to support me on paypal, i dont mind.
