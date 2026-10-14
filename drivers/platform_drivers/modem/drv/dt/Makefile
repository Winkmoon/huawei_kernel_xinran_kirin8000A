#
# Makefile for the dts drivers.
#
# this mk is for phone (without ko) and mbb

-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/dt/

ifeq ($(strip $(CFG_CONFIG_KERNEL_DTS_DECOUPLED)),YES)
# for phone(enable dt decouple)
subdir-ccflags-y+= -I$(srctree)/drivers/platform_drivers/modem/drv/dt/fdt/
obj-y           += device_tree_fdt.o
obj-y           += device_tree_load_acore_dtb.o

# The local libfdt copies compile to the same global symbols as the in-kernel
# lib/libfdt (built when CONFIG_LIBFDT=y).  They coexist when the modem driver is
# a module (separate .ko) but clash at the vmlinux link step for the built-in
# build (CONFIG_MODEM_DRIVER=y).  Keep them only for the module/non-built-in
# case; the built-in driver links against the kernel's libfdt, which provides the
# same fdt_* API.
ifneq ($(strip $(CONFIG_MODEM_DRIVER)),y)
obj-y           += fdt/fdt.o
obj-y           += fdt/fdt_ro.o
obj-y           += fdt/fdt_overlay.o
obj-y           += fdt/fdt_rw.o
obj-y           += fdt/fdt_strerror.o
obj-y           += fdt/fdt_wip.o
endif
else
# for mbb and phone(disable dt decouple)
obj-y           += device_tree_native.o
endif

ifeq ($(strip $(CFG_BSP_CONFIG_PHONE_TYPE)),YES)
# for phone(for load fw_dt(not care dt decouple))
obj-y           += device_tree_load_fw_dtb.o
endif

# for mbb and phone(not care dt decouple)
obj-y   += device_tree_load_untils.o
obj-y   += device_tree_load_ccore_dt.o
obj-y   += device_tree_driver.o
GCOV_PROFILE := y