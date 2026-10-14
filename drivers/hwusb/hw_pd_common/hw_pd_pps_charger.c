/*
 * hw_pd_pps_charger.c
 *
 * Source file of PPS charger of PD driver
 *
 * Copyright (C) 2024 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/jiffies.h>

#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_pps_charger.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include "securec.h"

#define HWLOG_TAG huawei_pd_pps
HWLOG_REGIST();

#define PPS_FAILED         (-1)

#define PPS_CUR_MIN        100                  /* 100ma */
#define PPS_VOL_MIN        3000                 /* 3000 mv */
#define PPS_VOL_MAX        30000                /* 30000 mv */
#define PPS_VOL_MAX_SC2    15000                /* 15000mv */
#define PPS_POWER_LIMIT    (30 * 1000 * 1000)  /* 30W */

#define MIN_VAL(a, b)      ((a) < (b) ? (a) : (b))
#define MAX_VAL(a, b)      ((a) > (b) ? (a) : (b))

struct pps_status_rec {
	int vol;
	int cur;
	unsigned int tm;
};

static int g_cur_ma = 0;
static int g_cur_mv = 0;
static int g_mv_max = 0;
static int g_mv_min = 0;
static int g_ma_max = 0;
static int g_ma_set = 0;

static bool g_pps_mode = false;
static bool g_pps_register_suc = false;
static bool g_support_sc4 = true;
static bool g_support_pps_status = false;
static unsigned int g_pps_status_interval = 100;  /* interval for get pps-status */
static struct pps_status_rec g_last_pps_status;

static struct pps_apdo_cap g_pps_apdo_caps[PPS_APDO_CAPS_MAX];

static int g_pwr_curve_cnt = 0;
static struct adp_pwr_curve_para *g_tmp_curve = NULL;
static struct hw_pd_pps_ops *g_pd_pps_ops = NULL;

static struct pps_pdo_raw pdo_black_list[] = {
	{ 0x0a01912c, 0x0002d12c, 0x0003c12c, 0x0004b12c, 0x00064145, 0xc9a43741 },
	{ 0 }
};

static struct pps_pdo_raw pdo_grey_list[] = {
	/* 90W  Huawei Matebook inbox charger */
	{ 0x0a01912c, 0x0002d12c, 0x0003c1f4, 0x0004b1f4, 0x000641c2, 0xc92c2264, 0xc9a4225a },
	/* 66W Huawei   GaN ultra-thin charger */
	{ 0x0a01912c, 0x0002d12c, 0x0003c0c8, 0x0004b0d2, 0x000640c8, 0xc8dc3728, 0xc9903728 },
	{ 0 }
};

static bool pps_charger_valid(struct pps_pdo_raw *raw, struct pps_pdo_raw *list, int num)
{
	int i;

	for (i = 0; i < num; i++) {
		if (list[i].data[0] == 0)
			break;
		if (memcmp(&list[i], raw, sizeof(struct pps_pdo_raw)) == 0) {
			hwlog_err("%s match black list %d, total %d\n", __func__, i, num);
			return true;
		}
	}

	return false;
}

#define pps_charger_checkin(raw, list) \
	pps_charger_valid((raw), (list), (sizeof(list) / sizeof(struct pps_pdo_raw)))
#define pps_charger_in_black(raw) pps_charger_checkin((raw), pdo_black_list)
#define pps_charger_in_grey(raw) pps_charger_checkin((raw), pdo_grey_list)

static inline int pps_get_cap_cur(struct pps_apdo_cap *cap, int volt, int *ma)
{
	if ((cap->cur) && (cap->vol_max >= volt) && (cap->vol_min <= volt)) {
		*ma = cap->cur;
		return 0;
	}
	return PPS_FAILED;
}

static inline int pps_get_cap_volt(struct pps_apdo_cap *cap, int cur, int *mv)
{
	if ((cap->vol_min) && (cap->cur) && (cur <= cap->cur)) {
		*mv = cap->vol_min;
		return 0;
	}
	return PPS_FAILED;
}

static int pps_set_state(enum pps_set_state state, int mv, int ma)
{
	if (!g_pd_pps_ops || !g_pd_pps_ops->set_state) {
		return PPS_FAILED;
	}

	return g_pd_pps_ops->set_state(state, mv, ma);
}

static int pps_get_status(int *mv, int *ma)
{
	int ret;

	if (!g_pd_pps_ops || !g_pd_pps_ops->get_status) {
		return PPS_FAILED;
	}

	if (g_pps_status_interval == 0)
		return g_pd_pps_ops->get_status(mv, ma);

	if (g_last_pps_status.tm && g_last_pps_status.vol && g_last_pps_status.cur &&
		((jiffies_to_msecs(jiffies) - g_last_pps_status.tm) < g_pps_status_interval)) {
		*mv = g_last_pps_status.vol;
		*ma = g_last_pps_status.cur;
		hwlog_debug("%s use last record\n", __func__);
		return 0;
	}

	(void)memset_s(&g_last_pps_status, sizeof(g_last_pps_status), 0, sizeof(g_last_pps_status));
	ret = g_pd_pps_ops->get_status(mv, ma);
	if (ret == 0) {
		g_last_pps_status.vol = *mv;
		g_last_pps_status.cur = *ma;
		g_last_pps_status.tm = jiffies_to_msecs(jiffies);
	}

	return ret;
}

static int pps_get_apdo_caps(struct pps_apdo_cap *caps, int num, struct pps_pdo_raw *raw)
{
	if (!g_pd_pps_ops || !g_pd_pps_ops->get_status) {
		return PPS_FAILED;
	}

	return g_pd_pps_ops->get_apdo_caps(caps, num, raw);
}

static int hw_pd_pps_update(int mv, int ma)
{
	int i;
	int ret;
	struct pps_apdo_cap *cap = NULL;

	(void)memset_s(&g_last_pps_status, sizeof(g_last_pps_status), 0, sizeof(g_last_pps_status));
	mv = MAX_VAL(g_mv_min, mv);
	ma = MAX_VAL(PPS_CUR_MIN, ma);

	for (i = 0; i < PPS_APDO_CAPS_MAX; i++) {
		cap = &g_pps_apdo_caps[i];
		if ((cap->cur >= ma) && (cap->vol_max >= mv) && (cap->vol_min <= mv))
			break;
	}
	if (i == PPS_APDO_CAPS_MAX) {
		hwlog_err("%s not support ma=%d, mv=%d\n", __func__, ma, mv);
		return -EINVAL;
	}

	if (!g_pps_mode) {
		ret = pps_set_state(PPS_SET_STATE_START, mv, ma);
		if (!ret)
			g_pps_mode = true;
	} else {
		ret = pps_set_state(PPS_SET_STATE_ADJUST, mv, ma);
	}

	if (ret == 0)
		g_ma_set = ma;

	hwlog_info("%s mode=%d, ma=%d, mv=%d, ret=%d\n", __func__, g_pps_mode, ma, mv, ret);
	return ret;
}

static int hw_pps_set_output_voltage(int volt)
{
	int ma = g_cur_ma;

	hwlog_info("%s set volt %d\n", __func__, volt);

	if ((volt > g_mv_max) || (volt < g_mv_min)) {
		hwlog_info("%s volt %d not support\n", __func__, volt);
		return -EINVAL;
	}

	return hw_pd_pps_update(volt, ma);
}

static int hw_pps_get_output_voltage(int *volt)
{
	int ret = PPS_FAILED;
	int ma = 0;

	if (!volt)
		return -EINVAL;

	if (g_support_pps_status) {
		ret = pps_get_status(volt, &ma);
		if ((ret == 0) && ((*volt <= 0) || (ma < 0))) {
			g_support_pps_status = false;
			hwlog_err("%s not support pps status, ret %d\n", __func__, ret);
			ret = PPS_FAILED;
		} else if (ret < 0) {
			hwlog_err("%s get pps failed ret %d\n", __func__, ret);
			return PPS_FAILED;
		}
	}

	if (ret != 0)
		ret = dcm_get_ic_vbus(SC_MODE, CHARGE_MULTI_IC, volt);
	if (ret) {
		hwlog_err("%s get volt ret %d\n", __func__, ret);
		*volt = g_pps_mode ? g_cur_mv : 0;
	}

	hwlog_info("%s ret %d\n", __func__, *volt);

	return 0;
}

static int hw_pps_set_output_current(int cur)
{
	int mv = g_cur_mv;
	int i;

	hwlog_info("%s set cur %d\n", __func__, cur);

	if (cur > g_ma_max) {
		hwlog_info("%s cur %d not support\n", __func__, cur);
		return -EINVAL;
	}

	if (!mv) {
		for (i = 0; i < PPS_APDO_CAPS_MAX; i++) {
			if (pps_get_cap_volt(&g_pps_apdo_caps[i], cur, &mv) == 0) {
				hwlog_info("%s set def cur %d\n", __func__, mv);
				break;
			}
		}
		if (i == PPS_APDO_CAPS_MAX) {
			hwlog_info("%s no compatible\n", __func__);
			return -EINVAL;
		}
	}

	return hw_pd_pps_update(mv, cur);
}

static int hw_pps_get_output_current(int *cur)
{
	int ret = PPS_FAILED;
	int mv = 0;

	if (!cur)
		return -EINVAL;

	if (g_support_pps_status) {
		ret = pps_get_status(&mv, cur);
		if ((ret == 0) && ((*cur < 0) || (mv < 0))) {
			g_support_pps_status = false;
			hwlog_err("%s not support pps status, ret %d\n", __func__, ret);
			ret = PPS_FAILED;
		} else if (ret < 0) {
			hwlog_err("%s get pps failed ret %d\n", __func__, ret);
			return PPS_FAILED;
		}
	}

	if (ret != 0)
		ret = dcm_get_ic_ibus(SC_MODE, CHARGE_MULTI_IC, cur);
	if (ret) {
		hwlog_err("%s get cur ret %d\n", __func__, ret);
		*cur = g_pps_mode ? g_cur_ma : 0;
	}

	hwlog_info("%s ret %d\n", __func__, *cur);

	return 0;
}

static int hw_pps_get_min_voltage(int *volt)
{
	if (!volt)
		return -EINVAL;

	*volt = g_mv_min;
	hwlog_info("%s ret %d\n", __func__, *volt);

	return 0;
}

static int hw_pps_get_max_voltage(int *volt)
{
	if (!volt)
		return -EINVAL;

	*volt = g_mv_max;
	hwlog_info("%s ret %d\n", __func__, *volt);

	return 0;
}

static int hw_pps_get_min_current(int *cur)
{
	if (!cur)
		return -EINVAL;

	*cur = PPS_CUR_MIN;
	hwlog_info("%s enter\n", __func__);

	return 0;
}

static int hw_pps_get_max_current(int *cur)
{
	if (!cur)
		return -EINVAL;

	*cur = g_ma_max;
	hwlog_info("%s ret %d\n", __func__, *cur);

	return 0;
}

static unsigned int pps_get_cap_mode(struct pps_apdo_cap *cap)
{
	unsigned int ret = 0;

	if (!cap->cur || !cap->vol_max || !cap->vol_min)
		return 0;

	if ((cap->vol_max >= 5000) && (cap->vol_min <= 5000))     /* 5000mv */
		ret |= ADAPTER_SUPPORT_LVC;
	if ((cap->vol_max >= 10000) && (cap->vol_min <= 10000))   /* 10000mv */
		ret |= ADAPTER_SUPPORT_SC;
	if (g_support_sc4) {
		if ((cap->vol_max >= 20000) && (cap->vol_min <= 20000))   /* 20000mv */
			ret |= ADAPTER_SUPPORT_SC4;
	}

	hwlog_info("%s return %u\n", __func__, ret);
	return ret;
}

static int hw_pps_get_support_mode(int *mode)
{
	struct pps_apdo_cap *cap = NULL;
	unsigned int spt = 0;
	int i;
	int j;

	if (!mode) {
		hwlog_info("%s empty mode\n", __func__);
		return ADAPTER_DETECT_OTHER;
	}

	for (j = 0; j < PPS_ADPO_CHECK_MAX; j++) {
		for (i = 0; i < PPS_APDO_CAPS_MAX; i++) {
			cap = &g_pps_apdo_caps[i];
			spt |= pps_get_cap_mode(cap);
		}
		if (!spt)
			power_msleep(DT_MSLEEP_50MS, 0, NULL);
		else
			break;
	}

	hwlog_info("%s detect %d, delay count %d\n", __func__, spt, j);
	*mode = (int)spt;
	return (spt == 0) ? ADAPTER_DETECT_OTHER : ADAPTER_DETECT_SUCC;
}

static int pps_get_curve_cur(int vol)
{
	int i;
	int ret = 0;
	struct pps_apdo_cap *cap = NULL;

	for (i = 0; i < PPS_APDO_CAPS_MAX; i++) {
		cap = &g_pps_apdo_caps[i];
		if ((vol >= cap->vol_min) && (vol <= cap->vol_max))
			ret = MAX_VAL(ret, cap->cur);
	}

	return ret;
}

static bool pps_prepare_curve_mem(int num)
{
	if (g_tmp_curve) {
		kfree(g_tmp_curve);
		g_tmp_curve = NULL;
	}
	g_tmp_curve = (struct adp_pwr_curve_para *)kzalloc(
		(sizeof(struct adp_pwr_curve_para) * num), GFP_KERNEL);
	if (!g_tmp_curve) {
		hwlog_err("%s alloc mem failed\n", __func__);
		return false;
	}
	return true;
}

static void pps_fill_power_curve(
	struct adp_pwr_curve_para *tmp_curve, int num)
{
	int i = 0;
	int size = 0;
	struct adp_pwr_curve_para *val = NULL;

	if (!pps_prepare_curve_mem(num)) {
		g_pwr_curve_cnt = 0;
		return;
	}

	val = g_tmp_curve;
	val[0].volt = tmp_curve[0].volt;
	val[0].cur = tmp_curve[0].cur;
	hwlog_info("%s: add curve[%d](init): volt=%d, cur=%d\n", __func__, i, val[0].volt, val[0].cur);
	size = 1;
	for (i = 1; i < num; i++) {
		hwlog_debug("%s tmp_curve[%d]: volt=%d, cur=%d\n",
			__func__, i, tmp_curve[0].volt, tmp_curve[0].cur);

		if ((tmp_curve[i].cur == tmp_curve[i - 1].cur) && (tmp_curve[i].cur == tmp_curve[i + 1].cur)) {
			hwlog_debug("%s: skip tmp_curve[%d]\n", __func__, i);
			continue;
		}

		if (tmp_curve[i].cur < tmp_curve[i - 1].cur) {
			val[size].volt = tmp_curve[i - 1].volt + 100; /* 100mv added */
			val[size].cur = tmp_curve[i].cur;
			hwlog_info("%s: add curve[%d](insert): volt=%d, cur=%d\n",
				__func__, size, val[size].volt, val[size].cur);
			size += 1;
		}

		val[size].volt = tmp_curve[i].volt;
		val[size].cur = tmp_curve[i].cur;
		hwlog_info("%s: add curve[%d]: volt=%d, cur=%d\n",
			__func__, size, val[size].volt, val[size].cur);
		size += 1;
	}
	g_pwr_curve_cnt = size;
}

static void pps_prepare_power_curve(void)
{
	struct adp_pwr_curve_para *tmp_curve = NULL;
	int num;
	int idx;
	int i;

	num = (g_mv_max - g_mv_min) / 1000 + 5;
	tmp_curve = (struct adp_pwr_curve_para *)kzalloc(
		(sizeof(struct adp_pwr_curve_para) * num), GFP_KERNEL);
	if (!tmp_curve) {
		hwlog_err("%s alloc mem %d failed\n", __func__, num);
		return;
	}

	tmp_curve[0].volt = g_mv_min;
	tmp_curve[0].cur = pps_get_curve_cur(g_mv_min);
	idx = 1;
	while (idx < num) {
		i = ((tmp_curve[idx - 1].volt + 1000) / 1000) * 1000;
		if (i > g_mv_max)
			break;

		tmp_curve[idx].volt = i;
		tmp_curve[idx].cur = pps_get_curve_cur(i);
		idx++;
	}

	pps_fill_power_curve(tmp_curve, idx);
	hwlog_info("%s: get %d curve value\n", __func__, g_pwr_curve_cnt);

	kfree(tmp_curve);
}

static int hw_pps_get_power_curve(
	struct adp_pwr_curve_para *val, int *size, int max_size)
{
	int len;
	int ret;

	if (!g_pwr_curve_cnt || !g_tmp_curve || !val || !size || !max_size)
		return -EIO;

	len = MIN_VAL(max_size, g_pwr_curve_cnt) * sizeof(struct adp_pwr_curve_para);
	ret = memcpy_s(val, len, g_tmp_curve, len);
	if (ret != 0) {
		hwlog_err("%s copy curve failed\n", __func__);
		return -ENOMEM;
	}

	*size = MIN_VAL(max_size, g_pwr_curve_cnt);
	hwlog_debug("%s: get %d curve value\n", __func__, *size);

	return 0;
}

static int hw_ppschg_hard_reset_master(void)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_prot_state(void)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_soft_reset_master(void)
{
	int ret;

	ret = pps_set_state(PPS_SET_STATE_STOP, 0, 0);
	hwlog_info("%s ret %d\n", __func__, ret);
	g_pps_mode = false;

	return 0;
}

static int hw_pps_soft_reset_slave(void)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_soft_reset_dpdm(void)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_device_info(struct adapter_device_info *info)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_chip_vendor_id(int *id)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_serial_no_id(int *id)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_set_output_enable(int enable)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_set_output_mode(int enable)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_set_reset(int enable)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_output_current_set(int *cur)
{
	if (cur)
		*cur = g_ma_set;
	hwlog_info("%s ret %d\n", __func__, g_ma_set);
	return 0;
}

static int hw_pps_get_max_power(int *max_pwr)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_power_drop_current(int *cur)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_inside_temp(int *temp)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_port_temp(int *temp)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_port_leakage_cur_flag(int *flag)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_auth_encrypt_start(int key)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_set_usbpd_enable(int enable, bool check_cport)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_set_default_state(void)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_set_default_param(void)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_set_init_data(struct adapter_init_data *data)
{
	int ret;
	int mv = 0;
	int ma = 0;

	if (!data) {
		hwlog_err("%s invalid param\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s vol=%d\n", __func__, data->init_voltage);

	ret = hw_pd_pps_update(data->init_voltage, 0);
	if (ret != 0) {
		hwlog_info("%s enter pps ret %d\n", __func__, ret);
		return ret;
	}

	g_support_pps_status = true;
	ret = pps_get_status(&mv, &ma);
	if ((ret != 0) || (mv <= 0) || (ma < 0)) {
		g_support_pps_status = false;
		hwlog_info("%s invalid status ret%d, mv=%d, ma=%d\n", __func__, ret, mv, ma);
	}

	return 0;
}

static int hw_pps_get_adp_type(int *type)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static int hw_pps_get_adp_code(int *code)
{
	hwlog_debug("%s enter\n", __func__);
	return 0;
}

static bool hw_pps_is_undetach_cable(void)
{
	hwlog_debug("%s enter\n", __func__);
	return false;
}

static struct adapter_protocol_ops pps_charger_protocol_ops = {
	.type_name = "hw_pps",
	.hard_reset_master = hw_ppschg_hard_reset_master,
	.set_output_voltage = hw_pps_set_output_voltage,
	.get_output_voltage = hw_pps_get_output_voltage,
	.set_output_current = hw_pps_set_output_current,
	.get_output_current = hw_pps_get_output_current,
	.get_min_voltage = hw_pps_get_min_voltage,
	.get_max_voltage = hw_pps_get_max_voltage,
	.get_min_current = hw_pps_get_min_current,
	.get_max_current = hw_pps_get_max_current,
	.detect_adapter_support_mode = hw_pps_get_support_mode,
	.get_support_mode = hw_pps_get_support_mode,
	.get_power_curve = hw_pps_get_power_curve,
	.get_protocol_register_state = hw_pps_get_prot_state,
	.soft_reset_master = hw_pps_soft_reset_master,
	.soft_reset_slave = hw_pps_soft_reset_slave,
	.soft_reset_dpdm = hw_pps_soft_reset_dpdm,
	.get_device_info = hw_pps_get_device_info,
	.get_chip_vendor_id = hw_pps_get_chip_vendor_id,
	.get_chip_serial_num = hw_pps_get_serial_no_id,
	.set_output_enable = hw_pps_set_output_enable,
	.set_output_mode = hw_pps_set_output_mode,
	.set_reset = hw_pps_set_reset,
	.get_output_current_set = hw_pps_get_output_current_set,
	.get_max_power = hw_pps_get_max_power,
	.get_power_drop_current = hw_pps_get_power_drop_current,
	.get_inside_temp = hw_pps_get_inside_temp,
	.get_port_temp = hw_pps_get_port_temp,
	.get_port_leakage_current_flag = hw_pps_get_port_leakage_cur_flag,
	.auth_encrypt_start = hw_pps_auth_encrypt_start,
	.set_usbpd_enable = hw_pps_set_usbpd_enable,
	.set_default_state = hw_pps_set_default_state,
	.set_default_param = hw_pps_set_default_param,
	.set_init_data = hw_pps_set_init_data,
	.get_adp_type = hw_pps_get_adp_type,
	.get_adp_code = hw_pps_get_adp_code,
	.is_undetach_cable = hw_pps_is_undetach_cable,
};

static int pps_check_cap_valid(struct pps_apdo_cap *cap, bool grey) 
{
	int ret = -EINVAL;
	int cur_limit;
	int vol_limit = g_support_sc4 ? PPS_VOL_MAX : PPS_VOL_MAX_SC2;

	if (grey) {
		hwlog_info("%s limit power of grey charger", __func__);
		vol_limit = MIN_VAL(10000, vol_limit);  /* 10000 mv */
		cap->cur = MIN_VAL(2000, cap->cur);  /* 2000 ma */
	}

	if (!cap->cur || !cap->vol_max || !cap->vol_min)
		goto check_finish;
	
	if ((cap->vol_max < PPS_VOL_MIN) || (cap->vol_max < cap->vol_min))
		goto check_finish;

	if (cap->vol_min >= vol_limit)
		goto check_finish;

	cap->vol_max = MIN_VAL(cap->vol_max, vol_limit);

	/* calculate the max current, align in 100ma, and reserve 400ma for limit */
	cur_limit = ((PPS_POWER_LIMIT / cap->vol_max + 400) / 100) * 100;
	if (!g_support_sc4 && (cap->cur > cur_limit)) {
		hwlog_info("%s limit current form %d to %d", __func__, cap->cur, cur_limit);
		cap->cur = cur_limit;
	}
	ret = 0;

check_finish:
	if (ret) {
		hwlog_err("%s invalid cap %d-%d-%d\n", __func__, cap->cur, cap->vol_max, cap->vol_min);
		(void)memset_s(cap, sizeof(struct pps_apdo_cap), 0, sizeof(struct pps_apdo_cap));
	}

	return ret;
}

void hw_pd_pps_notify_vdo(void)
{
	int ret;
	int i;
	struct pps_pdo_raw raw = {0};

	if (!g_pps_register_suc)
		return;

	g_mv_max = 0;
	g_ma_max = 0;
	g_mv_min = 0;
	g_pwr_curve_cnt = 0;
	(void)memset_s(g_pps_apdo_caps, sizeof(g_pps_apdo_caps), 0, sizeof(g_pps_apdo_caps));

	ret = pps_get_apdo_caps(g_pps_apdo_caps, PPS_APDO_CAPS_MAX, &raw);
	if (ret >= 0) {
		for (i = 0; i < PD_PDO_MAX_COUNT; i++)
			hwlog_info("%s get pdo[%d]=0x%08x\n", __func__, i, raw.data[i]);
	}
	if ((ret <= 0) || (ret > PPS_APDO_CAPS_MAX) || pps_charger_in_black(&raw)) {
		hwlog_info("%s invalid apdo, ret=%d\n", __func__, ret);
		(void)memset_s(g_pps_apdo_caps, sizeof(g_pps_apdo_caps), 0, sizeof(g_pps_apdo_caps));
		return;
	}

	for (i = 0; i < ret; i++) {
		if (pps_check_cap_valid(&g_pps_apdo_caps[i], pps_charger_in_grey(&raw)))
			continue;

		g_mv_max = MAX_VAL(g_mv_max, g_pps_apdo_caps[i].vol_max);

		if (g_mv_min == 0)
			g_mv_min = g_pps_apdo_caps[i].vol_min;
		else
			g_mv_min = MIN_VAL(g_mv_min, g_pps_apdo_caps[i].vol_min);

		if (g_ma_max == 0)
			g_ma_max = g_pps_apdo_caps[i].cur;
		else
			g_ma_max = MAX_VAL(g_ma_max, g_pps_apdo_caps[i].cur);

		hwlog_info("%s apdo cap %d: min_vol=%d, max_vol=%d, cur=%d\n",
			__func__, i, g_pps_apdo_caps[i].vol_min,
			g_pps_apdo_caps[i].vol_max, g_pps_apdo_caps[i].cur);
	}

	pps_prepare_power_curve();
}

int hw_pd_pps_filter_sink(int mv, int ma)
{
	if (!g_pps_register_suc)
		return 0;

	g_cur_ma = ma;
	g_cur_mv = mv;
	hwlog_info("%s: ma=%d, mv=%d, mode=%d\n", __func__, ma, mv, g_pps_mode);
	return (g_pps_mode) ? 1 : 0; 
}

void hw_pd_pps_notify_stop(void)
{
	int ret;

	if (!g_pps_register_suc || !g_pps_mode)
		return;

	ret = pps_set_state(PPS_SET_STATE_STOP, 0, 0);
	if (!ret) {
		(void)memset_s(g_pps_apdo_caps, sizeof(g_pps_apdo_caps), 0, sizeof(g_pps_apdo_caps));
		g_pps_mode = false;
	} else {
		hwlog_err("%s stop pps failed ret %d\n", __func__, ret);
	}
}

void hw_pd_pps_init(struct hw_pd_pps_ops *ops)
{
	int ret;

	if (!ops || !ops->get_apdo_caps || !ops->get_status || !ops->set_state) {
		hwlog_err("%s no ops\n", __func__);
		return;
	}

	g_pd_pps_ops = ops;
	ret = adapter_protocol_ops_register(&pps_charger_protocol_ops);
	hwlog_info("%s: reg ops ret %d\n", __func__, ret);
	g_pps_register_suc = (ret == 0);
}

void hw_pd_pps_uninit(void)
{
	hwlog_info("%s: enter\n", __func__);
	if (g_tmp_curve) {
		kfree(g_tmp_curve);
		g_tmp_curve = NULL;
	}
	g_pwr_curve_cnt = 0;
	g_pps_register_suc = false;
}

