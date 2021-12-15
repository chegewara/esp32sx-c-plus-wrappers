idf_component_register(SRCS 
    "host/usb_host.cpp" 
    "acm/usb_acm.cpp" 
    "msc/usb_msc.cpp" 
    "host/usb_device.cpp"
    "msc/vfs/diskio_rawmsc.cpp"
    # "usbip/tcp_server.c"
    # "usbip/usbip.cpp"
    
    INCLUDE_DIRS "include"

    PRIV_REQUIRES usb fatfs
)
