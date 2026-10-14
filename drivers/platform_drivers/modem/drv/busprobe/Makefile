
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

subdir-ccflags-y := -Werror
# qic_dump_register.c calls the closed-source dfx_qic_mdm_modid_register(), which
# is not shipped.  Only build it when the feature is enabled, matching the
# CONFIG_AP_QIC guard in g_modem_drv_module_init_func[] and the module object
# list (which builds qic only for CFG_CONFIG_AP_QIC=YES).
ifeq ($(strip $(CFG_CONFIG_AP_QIC)),YES)
obj-y           += qic_dump_register.o
endif

ifeq ($(strip $(CFG_CONFIG_AP_NOC)), YES)
obj-y           += noc_dump_register.o
endif
