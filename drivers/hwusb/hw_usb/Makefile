ifneq ($(CONFIG_USB_HW_F_ACM_MUSL),y)
obj-$(CONFIG_HUAWEI_USB) += hw_usb_input.o
endif
obj-$(CONFIG_HUAWEI_USB) += hw_usb.o
ifeq ($(CONFIG_HUAWEI_USB_VENDOR),y)
obj-$(CONFIG_HUAWEI_USB) += hw_usb_monitor.o
endif