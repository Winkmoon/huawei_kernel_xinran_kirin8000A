/*
 * Stub for <hooks/reg/iot_hook_pfa.h>.
 *
 * The open-source package ships the generic IOT hook framework
 * (include/bmi/acore/kernel/hooks/mdrv_bmi_ih.h) but not the per-subsystem
 * "hooks/reg/iot_hook_*.h" registration headers -- neither this one nor
 * hooks/reg/iot_hook_dt.h exist anywhere in the tree.
 *
 * pfa/core/pfa_adp.c includes this header but references nothing from it, so an
 * empty definition is enough to build the built-in (CONFIG_MODEM_DRIVER=y)
 * configuration.  pfa/core/pfa_kernel_hook.c does use the hook macros, but it is
 * only built when CFG_CONFIG_PFA_FW=YES.
 */
#ifndef __IOT_HOOK_PFA_H__
#define __IOT_HOOK_PFA_H__

#endif /* __IOT_HOOK_PFA_H__ */
