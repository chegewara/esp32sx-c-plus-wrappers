Example how to implement and test encryption secure V2 with virtual efuses stored in flash. Useful for debuging purpose without bricking device.

## In case of error:
- when device is running the same way as empty it is required to flash bootloader separately:
`idf.py bootloader-flash -p /dev/ttyUSBx`
