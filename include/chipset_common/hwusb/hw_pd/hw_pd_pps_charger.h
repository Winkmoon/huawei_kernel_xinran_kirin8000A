/*
 * hw_pd_pps_charger.h
 *
 * Header file of PPS charger of PD driver
 *
 * Copyright (C) 2024 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#ifndef _HW_PD_PPS_CHARGER_H_
#define _HW_PD_PPS_CHARGER_H_
 
/* used for PPS charger */
#define PD_PDO_MAX_COUNT   7
#define PPS_APDO_CAPS_MAX  PD_PDO_MAX_COUNT
#define PPS_ADPO_CHECK_MAX 20
 
struct pps_pdo_raw {
	uint32_t data[PD_PDO_MAX_COUNT];
};
 
struct pps_apdo_cap {
	int cur;
	int vol_max;
	int vol_min;
};
 
enum pps_set_state {
	PPS_SET_STATE_START,
	PPS_SET_STATE_ADJUST,
	PPS_SET_STATE_STOP,
};
 
struct hw_pd_pps_ops {
	int (*get_apdo_caps)(struct pps_apdo_cap *caps, int num, struct pps_pdo_raw *raw);
	int (*set_state)(enum pps_set_state state, int mv, int ma);
	int (*get_status)(int *mv, int *ma);
};
 
/*
 * PD driver notify pps that it has got the vdo data from PD dirver.
 * It means that emark detection has been done, and we can check
 * whether the charger supports PPS.
 */
void hw_pd_pps_notify_vdo(void);
 
/*
 * PD driver notify pps that the new sink state.  It is the value what
 * we requested. The actual value should be queried by sending reqeust
 * PPS-GET-STATE. If return value is non-zero, PD driver should not
 * process this message anymore.
 */
int hw_pd_pps_filter_sink(int mv, int ma);
 
/* Called while USB cable is removed. */
void hw_pd_pps_notify_stop(void);
 
/* Init the pps charger. */
void hw_pd_pps_init(struct hw_pd_pps_ops *ops);
 
/* Uninit the pps charger. */
void hw_pd_pps_uninit(void);
 
#endif /* _ADAPTER_PROTOCOL_PD_H_ */
 