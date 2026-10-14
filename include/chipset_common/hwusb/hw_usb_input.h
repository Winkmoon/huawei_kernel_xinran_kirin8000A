 /*
  * hw_usb.h
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

#ifndef _HW_USB_INPUT_H_
#define _HW_USB_INPUT_H_

ssize_t input_hw_usb_status_show(char *buf);
ssize_t input_hw_usb_status_store(const char *buf, size_t size);
ssize_t input_hw_usb_show(char *buf);
ssize_t input_hw_usb_store(const char *buf, size_t size);
void hw_usb_input_init(void);

#endif /* _HW_USB_H_ */