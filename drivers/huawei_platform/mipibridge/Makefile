# Makefile for the mipibridge.
#
# Each configuration option enables a list of files.

obj-$(CONFIG_HUAWEI_MIPIBRIDGE) += mipibridge_driver.o
obj-$(CONFIG_HUAWEI_MIPIBRIDGE) += mipibridge_uart.o
obj-$(CONFIG_HUAWEI_MIPIBRIDGE) += mipibridge_spi.o
obj-$(CONFIG_HUAWEI_MIPIBRIDGE) += jw/hv6100.o
obj-$(CONFIG_HUAWEI_MIPIBRIDGE) += gy/ug1009.o

EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea \
		-I$(srctree)/include/chipset_common/log \
		-I$(srctree)/include/dfx
