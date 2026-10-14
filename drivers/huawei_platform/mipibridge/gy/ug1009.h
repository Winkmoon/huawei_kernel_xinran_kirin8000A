/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2024 Huawei Technologies Co., Ltd.
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

#ifndef _UG1009_H_
#define _UG1009_H_

#include "../mipibridge.h"

#define DELAY_LEN 1
#define USE_FREQ_XHZ CAL_TO_2_5MHZ

#define CAL_TO_1_3MHZ 1300000
#define CAL_TO_1_5MHZ 1500000
#define CAL_TO_2_0MHZ 2000000
#define CAL_TO_2_5MHZ 2500000
#define CAL_TO_4_0MHZ 4000000

typedef enum
{
	ISC_NOOP = 0x02,
	ISC_ERASE = 0x05,
	ERASE_DONE = 0x09,
	READ_ID_CODE = 0x11,
	INIT_ADDRESS = 0x12,
	READ_USER_CODE = 0x13,
	ISC_ENABLE = 0x15,
	FAST_PROGRAM = 0x17,
	READ_STATUS_CODE = 0x41,
	JTAG_EF_PROGRAM = 0x71,
	JTAG_EF_READ = 0x73,
	JTAG_EF_ERASE = 0x75,
	JTAG_EF_2ND_ENABLE = 0x78,
	JTAG_EF_2ND_DISABLE = 0x79,
	ISC_DISABLE = 0x3A,
	REPROGRAM = 0x3C,
	Bypass = 0xFF
} GWFPGA_Inst_Def;

typedef enum
{
	TAP_RESET,
	TAP_IDLE,
	TAP_DRSELECT,
	TAP_DRCAPTURE,
	TAP_DRSHIFT,
	TAP_DREXIT1,
	TAP_DRPAUSE,
	TAP_DREXIT2,
	TAP_DRUPDATE,
	TAP_IRSELECT,
	TAP_IRCAPTURE,
	TAP_IRSHIFT,
	TAP_IREXIT1,
	TAP_IRPAUSE,
	TAP_IREXIT2,
	TAP_IRUPDATE,
	TAP_UNKNOWN
} TAP_TypeDef;

int ug1009_probe(struct mipibridge_device_info *di);


#endif /* _UG1009_H_ */
