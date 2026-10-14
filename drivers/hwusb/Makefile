obj-$(CONFIG_HW_GADGET) += hw_gadget/
obj-$(CONFIG_HUAWEI_USB) += hw_usb/
ifeq ($(CONFIG_HUAWEI_USB_VENDOR),y)
obj-y += hw_dp/
else
obj-y += hw_pd/
endif
ifeq ($(CONFIG_HW_PD_COMMON),y)
obj-y += hw_pd_common/
else
obj-y += hw_pd_common/hw_pd_pps_charger.o
endif