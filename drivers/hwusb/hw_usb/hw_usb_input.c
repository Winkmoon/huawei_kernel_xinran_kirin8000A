/*
 * hw_usb.c
 *
 * usb driver
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <chipset_common/hwusb/hw_usb.h>
#include <log/hw_log.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <securec.h>
#include <chipset_common/hwusb/hw_usb_input.h>

#define HWLOG_TAG hw_usb
HWLOG_REGIST();

#define HW_USB_UDCSTR_LEN 64

#define HW_USB_INPUT_STATE_ON  1
#define HW_USB_INPUT_STATE_OFF 0

#define HW_USB_CHAR_ZERO '0'

struct usb_filter_info {
	struct list_head node;
	struct input_dev *dev;
};

static LIST_HEAD(input_filter_list);
static DEFINE_MUTEX(input_filter_mutex);
static char g_udc_name[HW_USB_UDCSTR_LEN] = {"usb"};
static unsigned int g_usb_input_state;
static struct work_struct g_usb_input_work;
struct input_usb_filter g_input_usb_filter;

ssize_t input_hw_usb_status_show(char *buf)
{
	hwlog_info("%s: state=%u\n", __func__, g_usb_input_state);
	return scnprintf(buf, PAGE_SIZE, "%d", g_usb_input_state);
}

ssize_t input_hw_usb_status_store(const char *buf, size_t size)
{
	g_usb_input_state = buf[0] - HW_USB_CHAR_ZERO;
	if (g_usb_input_state == HW_USB_INPUT_STATE_ON)
		hw_usb_send_event(USB_INPUT_PLUG_IN);
	else if (g_usb_input_state == HW_USB_INPUT_STATE_OFF)
		hw_usb_send_event(USB_INPUT_PLUG_OUT);
	else
		return size;
	hwlog_info("%s: state=%u\n", __func__, g_usb_input_state);
	return size;
}

static void usb_input_send_state_event(unsigned int state)
{
	hwlog_info("%s: state=%u\n", __func__, state);
	if ((state == HW_USB_INPUT_STATE_ON) || (state == HW_USB_INPUT_STATE_OFF)) {
		g_usb_input_state = state;
		schedule_work(&g_usb_input_work);
	}
}

ssize_t input_hw_usb_show(char *buf)
{
	struct usb_filter_info *info = NULL;
	ssize_t sz;
	const char *name = NULL;

	sz = scnprintf(buf, PAGE_SIZE, "List device of UDC [%s]:\n", g_udc_name);
	mutex_lock(&input_filter_mutex);
	list_for_each_entry(info, &input_filter_list, node) {
		name = info->dev->name ? info->dev->name : "NA";
		hwlog_info("%s: show filter %p of dev %p, name is %s\n",
			__func__, info, info->dev, name);
		sz += scnprintf(buf + strlen(buf), PAGE_SIZE, "%s\n", name);
	}
	mutex_unlock(&input_filter_mutex);
	hwlog_info("%s: buf=%s\n", __func__, buf);

	return sz;
}

ssize_t input_hw_usb_store(const char *buf, size_t size)
{
	const char *val = NULL;
	char *end = NULL;
	int ret;

	hwlog_info("%s: buf=%s\n", __func__, buf);
	if (strncmp(buf, "udc=", strlen("udc=")) == 0) {
		val = buf + strlen("udc=");

		if (strlen(val) <= 0) {
			hwlog_err("Invalid UDC name size\n");
			return -EINVAL;
		}

		ret = strcpy_s(g_udc_name, HW_USB_UDCSTR_LEN, val);
		if (ret) {
			hwlog_err("Copy UDC name failed\n");
			return -EINVAL;
		}

		end = strchr(g_udc_name, ';');
		if (end)
			*end = 0;

		hwlog_info("Set UDC name=[%s]\n", g_udc_name);
	} if (strncmp(buf, "triger", strlen("triger")) == 0) {
		bool evt = false;
		mutex_lock(&input_filter_mutex);
		evt = !list_empty(&input_filter_list);
		mutex_unlock(&input_filter_mutex);
		if (evt) {
			usb_input_send_state_event(HW_USB_INPUT_STATE_ON);
		}
	}

	return size;
}

void usb_input_event_work(struct work_struct *work)
{
	hwlog_info("%s: state=%u\n", __func__, g_usb_input_state);
	if (g_usb_input_state == HW_USB_INPUT_STATE_ON)
		hw_usb_send_event(USB_INPUT_PLUG_IN);
	else if (g_usb_input_state == HW_USB_INPUT_STATE_OFF)
		hw_usb_send_event(USB_INPUT_PLUG_OUT);
}

static void create_filter(struct input_dev *dev, const char *path)
{
	struct usb_filter_info *info = NULL;
	bool evt = false;

	info = (struct usb_filter_info *)kzalloc(sizeof(struct usb_filter_info), GFP_KERNEL);
	if (!info) {
		hwlog_err("%s: alloc mem failed\n", __func__);
		return;
	}
	info->dev = dev;

	mutex_lock(&input_filter_mutex);
	evt = list_empty(&input_filter_list);
	list_add_tail(&info->node, &input_filter_list);
	mutex_unlock(&input_filter_mutex);

	hwlog_info("%s: create filter %p of dev %p, name=%s\n", __func__,
		info, info->dev, (info->dev->name ? info->dev->name : "NA"));
	if (evt)
		usb_input_send_state_event(HW_USB_INPUT_STATE_ON);
}

static void destroy_filter(struct input_dev *dev)
{
	struct usb_filter_info *info = NULL;
	bool del = false;
	bool evt = false;

	hwlog_info("destroy_filter name:%s\n", dev->name);
	mutex_lock(&input_filter_mutex);
	list_for_each_entry(info, &input_filter_list, node) {
		hwlog_err("destroy_filter name:%s\n",(info->dev->name ? info->dev->name : "NA"));
		if (info && (info->dev == dev)) {
			list_del_init(&info->node);
			del = true;
			break;
		}
	}
	evt = list_empty(&input_filter_list);
	mutex_unlock(&input_filter_mutex);

	if (!del)
		return;

	if (info) {
		hwlog_info("%s: delete filter %p of dev %p, name=%s\n", __func__,
			info, info->dev, (info->dev->name ? info->dev->name : "NA"));
		kfree(info);
	}
	if (evt)
		usb_input_send_state_event(HW_USB_INPUT_STATE_OFF);
}

static int usb_input_add_dev(struct input_dev *dev)
{
	const char *path = NULL;

	path = kobject_get_path(&dev->dev.kobj, GFP_KERNEL);
	if (!path) {
		hwlog_err("%s: get obj name failed\n", __func__);
		return -ENODEV;
	}
	hwlog_info("%s: path:%s g_udc_name:%s\n", __func__, path, g_udc_name);
	if (strlen(g_udc_name) && strstr(path, g_udc_name)){
		create_filter(dev, path);
	}
	kfree(path);

	return 0;
}

static void usb_input_del_dev(struct input_dev *dev)
{
	hwlog_info("%s\n", __func__);
	destroy_filter(dev);
}

struct input_usb_filter g_input_usb_filter = {
	.add_dev = usb_input_add_dev,
	.del_dev = usb_input_del_dev,
};

void hw_usb_input_init(void)
{
	INIT_WORK(&g_usb_input_work, usb_input_event_work);
	input_register_usb_filter(&g_input_usb_filter);
}