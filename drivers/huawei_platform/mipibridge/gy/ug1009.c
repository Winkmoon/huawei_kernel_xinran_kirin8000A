/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "ug1009.h"
#include "mipibridge.h"

/* ug1009 code addr */
#define UG1009_ID_CODE_ADDR 	0x11
#define UG1009_STATUS_CODE_ADDR	0x41
#define UG1009_USER_CODE_ADDR 	0x13
#define UG1009_USER_CODE_FW_AAR_OFFSET	26
#define UG1009_ID_CODE_FW_AAR_OFFSET	28
#define UG1009_CODE_LEN 4
#define UG1009_ID_CODE_CMD		0x0120681b

/* ug1009 jtag write buff len */
#define UG1009_WRITE_BUFF_LEN 		256
#define UG1009_WRITE_BUFF_MAX_LEN	300
#define UG1009_JTAG_DIN_COUNT		8
#define UG1009_JTAG_TMS_COUNT		7
#define UG1009_JTAG_EMPTY_DATA		0xFF

#define UG1009_JTAG_TAP_COUNT_32	32
#define UG1009_JTAG_TAP_COUNT_8		8
#define UG1009_JTAG_TAP_COUNT_5		5
#define UG1009_JTAG_RUNTEST_TIMES_6	6000
#define UG1009_JTAG_RUNTEST_TIMES_5	5000
#define UG1009_JTAG_RUNTEST_TIMES_1	1000

/* ug1009 spi config */
#define UG1009_SPI_CLK					3000000
#define UG1009_SPI_SET_RECONFIGN		0x70
#define UG1009_SPI_SET_RECONFIGN_1		0x22
#define UG1009_SPI_SET_RECONFIGN_2		0x11
#define UG1009_SPI_SET_JTAG_INIT_RESET	0x50
#define UG1009_SPI_CHIP_ERASE			0x25
#define UG1009_SPI_READ_VERIFY_RESULT	0x30
#define UG1009_SPI_READ_DEVICE_CODE 	0x40
#define UG1009_SPI_XPAGE_WRITE_READ		0x90
#define UG1009_SPI_READ_FIFO			0xC0
#define UG1009_SPI_TEST_CMD				0x33
#define UG1009_SPI_CMD_RETRY			2
#define UG1009_SPI_READ_CODE_RETRY		5
#define UG1009_SPI_IP_RETRY				1
#define UG1009_SPI_TEMP_BUFF_LEN2		2
#define UG1009_SPI_TEMP_BUFF_LEN		4
#define UG1009_SPI_DELAY_US_5			5
#define UG1009_SPI_DELAY_MS_10			10
#define UG1009_SPI_DELAY_MS_100			100
#define UG1009_SPI_DELAY_MS_180			180
#define UG1009_SPI_ERASE_OK				0x04
#define UG1009_SPI_VERIFY_OK			0xA0
#define UG1009_SPI_WRITE_CMD_1			0x47
#define UG1009_SPI_WRITE_CMD_2			0x57
#define UG1009_SPI_WRITE_CMD_3			0x31
#define UG1009_SPI_WRITE_CMD_4			0x4E

/* ug1009 uart cmd */
#define UG1009_MSG_IDX_HEADER_1		0
#define UG1009_MSG_IDX_HEADER_2		1
#define UG1009_MSG_IDX_HEADER_3		2
#define UG1009_MSG_IDX_INFO_1		3
#define UG1009_MSG_IDX_INFO_2		4
#define UG1009_MSG_IDX_INFO_3		5
#define UG1009_MSG_IDX_INFO_4		6
#define UG1009_MSG_IDX_TAIL_1		7
#define UG1009_MSG_IDX_TAIL_2		8
#define UG1009_MSG_IDX_TAIL_3		9
#define UG1009_MSG_HEADER_1	0xAA
#define UG1009_MSG_HEADER_2	0x87
#define UG1009_MSG_HEADER_3	0x37
#define UG1009_MSG_TAIL_1	0xAA
#define UG1009_MSG_TAIL_2	0x55
#define UG1009_MSG_TAIL_3	0xFF
#define UG1009_MSG_PRE_CHECK	0xA1011009
#define UG1009_MSG_CHECK_STATUS_NUM	5
#define UG1009_REG_CTRL_ADDR		0x00
#define UG1009_REG_VER_ADDR			0x01
#define UG1009_REG_STATUS1_ADDR		0x02
#define UG1009_REG_STATUS2_ADDR		0x03
#define UG1009_REG_ID_ADDR			0x04

#define UG1009_UART_MSG_BUFF_LEN		10
#define UG1009_UART_MSG_RECV_TIMEOUT	100
#define UG1009_UART_BUFF_LEN		32
#define UG1009_UART_MSG_MAX_LEN		100
#define UG1009_UART_REPORT_RGE		0xd103
#define UG1009_UART_CHECK_CMD		0xA0
#define UG1009_UART_READ_CMD		0xB0
#define UG1009_UART_WRITE_CMD		0xC0
#define UG1009_UART_WRITE_ACK		0xC1

/* ug1009 upgrade result */
#define UG1009_ADDR_RESULT			0x6C010
#define UG1009_RESULT_SIZE			4
#define UG1009_IDX_STATUS_ERASE		0
#define UG1009_IDX_STATUS_VERIFY	1
#define UG1009_IDX_VERIFY_NUM_H		2
#define UG1009_IDX_VERIFY_NUM_L		3
#define UG1009_SUCC			0x10
#define UG1009_ERR			0x01
#define UG1009_FW_VER_SIZE		4
#define UG1009_FW_VER_ADDR_0	0
#define UG1009_FW_VER_ADDR_1	1
#define UG1009_FW_VER_ADDR_2	2
#define UG1009_FW_VER_ADDR_3	3

#define UG1009_SHIFT_13 	13
#define UG1009_SHIFT_6 		6

#define UG1009_STATUS_ERR_1 	0x15421
#define UG1009_STATUS_ERR_2 	0x35421
#define UG1009_STATUS_ERR_3 	0x11421
#define UG1009_STATUS_ERR_4 	0x31421
#define UG1009_STATUS_SUCC_1	0x1F020
#define UG1009_STATUS_SUCC_2	0x3F020

#define UG1009_T_US				10000000
#define UG1009_T_US_RATIOS		10
#define UG1009_ID_OFFSET_1		1
#define UG1009_ID_OFFSET_2		2
#define UG1009_ID_OFFSET_3		3

/* jtag gpio */
static int tck_gpio = 0;
static int tdo_gpio = 0;
static int tdi_gpio = 0;
static int tms_gpio = 0;

/* jtag gpio */
#define TCK(a)	gpio_set_value(tck_gpio, a)
#define TDI(a)	gpio_set_value(tdi_gpio, a)
#define TMS(a)	gpio_set_value(tms_gpio, a)
#define TDO 	gpio_get_value(tdo_gpio)

/* firmware bin path */
#define FW_PATH_GW_IP "mipibridge/ug1009_ip.bin"
#define FW_PATH_GW_LCD "mipibridge/ug1009_lcd.bin"
#define UG1009_FW_IP_MAX_LEN		0x11000
#define UG1009_FW_IP_MIN_LEN		0x7000
#define UG1009_FW_LCD_MAX_LEN		0x20000
#define UG1009_FW_LCD_MIN_LEN		0x11000

static uint8_t g_ug1009_trans_buffer[UG1009_UART_BUFF_LEN] = {0};
static uint8_t g_wdata_buf[UG1009_WRITE_BUFF_MAX_LEN] = {0};
static const struct firmware *g_firmware_ip = NULL;
static const struct firmware *g_firmware_lcd = NULL;

static int ug1009_jtag_gpio_init(void)
{
	int ret = MIPI_BRIDGE_OK;

	/* TCK */
	ret = gpio_request_one(tck_gpio, GPIOF_OUT_INIT_LOW, "tck_gpio");
	if (ret) {
		mipibridge_errmsg("gpio_request %d failed\n", tck_gpio);
		return ret;
	}
	/* TDO */
	ret = gpio_request_one(tdo_gpio, GPIOF_IN, "tdo_gpio");
	if (ret) {
		gpio_free(tck_gpio);
		mipibridge_errmsg("gpio_request %d failed\n", tdo_gpio);
		return ret;
	}
	/* TDI */
	ret = gpio_request_one(tdi_gpio, GPIOF_OUT_INIT_LOW, "tdi_gpio");
	if (ret) {
		gpio_free(tck_gpio);
		gpio_free(tdo_gpio);
		mipibridge_errmsg("gpio_request %d failed\n", tdi_gpio);
		return ret;
	}
	return ret;
	/* TMS is spi cs */
}

static void ug1009_spi_gpio_init(void)
{
	/* tck */
	gpio_set_value(tck_gpio, 0);
	gpio_free(tck_gpio);
	/* tdo */
	gpio_set_value(tdo_gpio, 0);
	gpio_free(tdo_gpio);
	/* tdi */
	gpio_set_value(tdi_gpio, 0);
	gpio_free(tdi_gpio);
	/* TMS is spi cs */
	gpio_set_value(tms_gpio, 1);
	gpio_set_value(tms_gpio, 0);
	gpio_set_value(tms_gpio, 1);
	gpio_set_value(tms_gpio, 0);
	gpio_set_value(tms_gpio, 1);
}

static int check_IDcode(uint32_t code_dev, uint32_t code_fsfile)
{
	if (code_dev == code_fsfile)
		return MIPI_BRIDGE_OK;

	mipibridge_infomsg("code mismatch! code_dev:%x code_fsfile:%x", code_dev, code_fsfile);
	return MIPI_BRIDGE_FAIL;
}

static void jtag_write(uint8_t din, uint8_t dout, uint8_t tms, uint8_t LSB)
{
	int i = 0;
	uint8_t _tmp_din = 0;

	if (LSB == 0) {
		for (i = 0; i < UG1009_JTAG_DIN_COUNT; i++)
			_tmp_din |= ((din & (1 << i)) >> i) << (UG1009_JTAG_TMS_COUNT - i);
		din = _tmp_din;
	}

	TMS(0);
	for (i = 0; i < UG1009_JTAG_DIN_COUNT; i++) {
		if (i == UG1009_JTAG_TMS_COUNT && tms == 1)
			TMS(1);

		TDI((din >> i) & 0x01);
		TCK(0);
		TCK(1);
	}
}

static void jtag_move_tap_one_clock(uint8_t tms_value)
{
	TMS(tms_value);
	TCK(0);
	TCK(1);
}

static void jtag_move_tap(TAP_TypeDef TAP_From, TAP_TypeDef TAP_To)
{
	int i = 0;
	if ((TAP_From == TAP_UNKNOWN) && (TAP_To == TAP_IDLE)) {
		for (i = 0; i < UG1009_JTAG_TAP_COUNT_8; i++)
			jtag_move_tap_one_clock(1);
		jtag_move_tap_one_clock(0);
		jtag_move_tap_one_clock(0);
	} else if ((TAP_From == TAP_IDLE) && (TAP_To == TAP_IDLE)) {
		for (i = 0; i < UG1009_JTAG_TAP_COUNT_5; i++)
			jtag_move_tap_one_clock(0);
	} else if ((TAP_From == TAP_IDLE) && (TAP_To == TAP_IRSHIFT)) {
		jtag_move_tap_one_clock(1);
		jtag_move_tap_one_clock(1);
		jtag_move_tap_one_clock(0);
		jtag_move_tap_one_clock(0);
	} else if ((TAP_From == TAP_IDLE) && (TAP_To == TAP_DRSHIFT)) {
		jtag_move_tap_one_clock(1);
		jtag_move_tap_one_clock(0);
		jtag_move_tap_one_clock(0);
	} else if ((TAP_From == TAP_IREXIT1) && (TAP_To == TAP_IDLE)) {
		jtag_move_tap_one_clock(1);
		jtag_move_tap_one_clock(0);
		jtag_move_tap_one_clock(0);
	} else if ((TAP_From == TAP_DREXIT1) && (TAP_To == TAP_IDLE)) {
		jtag_move_tap_one_clock(1);
		jtag_move_tap_one_clock(0);
		jtag_move_tap_one_clock(0);
	} else {
		mipibridge_infomsg("error: tap walking.\r\n");
	}
}

static void runtest_xxus(uint32_t xxus, uint32_t freq_hz, uint32_t cycle)
{
	uint32_t count = 0;
	uint32_t t_us = UG1009_T_US / freq_hz;
	uint32_t count_t = 0;

	count = xxus * UG1009_T_US_RATIOS / t_us;
	count_t = count + 1;

	mipibridge_flowmsg("rentest:%uus, %uhz, T:%fus ,cycle:%u count:%f\n", xxus, freq_hz, t_us, cycle, count);

	while (count_t-- > 1) {
		TCK(0);
		TCK(1);
		TCK(0);
	}
}

static void jtag_write_inst(uint8_t inst)
{
	jtag_move_tap(TAP_IDLE, TAP_IRSHIFT);
	jtag_write(inst, 0x0, 0x1, 1);
	jtag_move_tap(TAP_IREXIT1, TAP_IDLE);
	jtag_move_tap(TAP_IDLE, TAP_IDLE);
	jtag_move_tap(TAP_IDLE, TAP_IDLE);
	jtag_move_tap(TAP_IDLE, TAP_IDLE);
}

static uint32_t jtag_read_code(uint8_t inst)
{
	uint8_t i = 0;
	uint32_t res_code = 0;

	jtag_write_inst(inst);
	jtag_move_tap(TAP_IDLE, TAP_DRSHIFT);

	for (i = 0; i < UG1009_JTAG_TAP_COUNT_32; i++) {
		if (i == UG1009_JTAG_TAP_COUNT_32 - 1)
			TMS(1);

		TCK(0);
		TCK(1);
		res_code |= TDO ? 1 << i : 0;
	}

	jtag_move_tap(TAP_DREXIT1, TAP_IDLE);

	return res_code;
}

static void gw_jtag_reset(void)
{
	jtag_move_tap(TAP_UNKNOWN, TAP_IDLE);
	jtag_move_tap(TAP_UNKNOWN, TAP_IDLE);
}

static int done_final_is_low(uint32_t code)
{
	return ((code >>= UG1009_SHIFT_13) & 0x01) ? MIPI_BRIDGE_FAIL : MIPI_BRIDGE_OK;
}

static void jtag_runtest(int cycles)
{
	int i = 0;
	for (i = 0; i < cycles; i++)
		jtag_move_tap_one_clock(0);
}

static int jtag_erase_sram(void)
{
	uint32_t code_t = 0;

	mipibridge_infomsg("start\n");
	gw_jtag_reset();

	jtag_write_inst(ISC_ENABLE);
	jtag_write_inst(ISC_ERASE);
	jtag_write_inst(ISC_NOOP);

	runtest_xxus(UG1009_JTAG_RUNTEST_TIMES_6, USE_FREQ_XHZ, DELAY_LEN);

	jtag_write_inst(ERASE_DONE);
	jtag_write_inst(ISC_NOOP);
	jtag_write_inst(ISC_DISABLE);
	jtag_write_inst(ISC_NOOP);
	jtag_runtest(UG1009_JTAG_RUNTEST_TIMES_5);

	code_t = jtag_read_code(READ_STATUS_CODE);
	return done_final_is_low(code_t);
}

static void clear_error(void)
{
	gw_jtag_reset();
	jtag_write_inst(ISC_DISABLE);
	jtag_write_inst(ISC_NOOP);
	gw_jtag_reset();
	jtag_write_inst(REPROGRAM);
	gw_jtag_reset();

	jtag_write_inst(ISC_ENABLE);
	jtag_write_inst(ISC_DISABLE);
	jtag_write_inst(ISC_NOOP);
	jtag_write_inst(ISC_NOOP);
	jtag_write_inst(ISC_ENABLE);
	jtag_write_inst(ISC_DISABLE);
	jtag_write_inst(ISC_NOOP);
	jtag_write_inst(ISC_NOOP);
	jtag_write_inst(ISC_NOOP);
}

static uint32_t read_fsfile_idcode(uint8_t *fs_ip, uint32_t fs_ip_size)
{
	uint32_t idcode_addr = UG1009_ID_CODE_FW_AAR_OFFSET;

	if (fs_ip_size < (UG1009_ID_CODE_FW_AAR_OFFSET + UG1009_CODE_LEN)) {
		mipibridge_errmsg("fssize %u err\n", fs_ip_size);
		return 0;
	}

	return FOUR_U8_TO_U32(*(fs_ip + idcode_addr), *(fs_ip + idcode_addr + UG1009_ID_OFFSET_1),
		*(fs_ip + idcode_addr + UG1009_ID_OFFSET_2), *(fs_ip + idcode_addr + UG1009_ID_OFFSET_3));
}

static uint32_t read_device_code(uint8_t inst)
{
	uint8_t tmp_tchar[UG1009_SPI_TEMP_BUFF_LEN] = {0};
	uint8_t tmp_rchar[UG1009_SPI_TEMP_BUFF_LEN] = {0};

	tmp_tchar[0] = UG1009_SPI_READ_DEVICE_CODE;
	tmp_tchar[1] = inst;
	mipibridge_spi_send_transfer(tmp_tchar, UG1009_SPI_TEMP_BUFF_LEN2);

	udelay(UG1009_SPI_DELAY_US_5);

	mipibridge_spi_recive_transfer(UG1009_SPI_READ_FIFO, tmp_rchar, UG1009_SPI_TEMP_BUFF_LEN);

	return FOUR_U8_TO_U32(tmp_rchar[0], tmp_rchar[1], tmp_rchar[UG1009_ID_OFFSET_2], tmp_rchar[UG1009_ID_OFFSET_3]);
}

static void ug1009_jtag_switch_spi(void)
{
	uint8_t tmp_tchar[UG1009_WRITE_BUFF_LEN] = {0};

	/* IP_soft_reset */
	tmp_tchar[0] = UG1009_SPI_SET_RECONFIGN;
	tmp_tchar[1] = UG1009_SPI_SET_RECONFIGN_1;
	if (mipibridge_spi_send_transfer(tmp_tchar, UG1009_SPI_TEMP_BUFF_LEN2) < 0) {
		mipibridge_errmsg("mipibridge_spi_send_transfer fail\n");
		return;
	}
	udelay(10);

	/* set_jtag_tap_init */
	tmp_tchar[0] = UG1009_SPI_SET_JTAG_INIT_RESET;
	if (mipibridge_spi_send_transfer(tmp_tchar, 1) < 0) {
		mipibridge_errmsg("mipibridge_spi_send_transfer fail\n");
		return;
	}
	udelay(10);
}

static void ug1009_spi_switch_jtag(void)
{
	gw_jtag_reset();
	jtag_write_inst(ISC_DISABLE);
	jtag_write_inst(ISC_NOOP);
}

static void print_jtag_code(void)
{
	mipibridge_infomsg("jtag id code: 0x%08x, status code: 0x%08x, version code:0x%08x\n",
		jtag_read_code(UG1009_ID_CODE_ADDR),
		jtag_read_code(UG1009_STATUS_CODE_ADDR),
		jtag_read_code(UG1009_USER_CODE_ADDR));
}

static void print_spi_code(void)
{
	mipibridge_infomsg("spi id code: 0x%08x, status code: 0x%08x, version code:0x%08x\n",
		read_device_code(UG1009_ID_CODE_ADDR),
		read_device_code(UG1009_STATUS_CODE_ADDR),
		read_device_code(UG1009_USER_CODE_ADDR));
}

static void ug1009_jtag_prog_sram(uint8_t *fs_p, uint32_t fssize)
{
	int i = 0;
	u32 status_code = 0;

	if (!fs_p || !g_firmware_ip) {
		mipibridge_errmsg("fs_p or g_firmware_ip is NULL!\n");
		return;
	}
	if (fssize != g_firmware_ip->size) {
		mipibridge_errmsg("err size! fssize = %u, g_firmware_ip->size=%u\n", fssize, g_firmware_ip->size);
		return;
	}

	gw_jtag_reset();

	jtag_erase_sram();
	status_code = jtag_read_code(READ_STATUS_CODE);
	if (status_code == UG1009_STATUS_ERR_1 || status_code == UG1009_STATUS_ERR_2 ||
		status_code == UG1009_STATUS_ERR_3 || status_code == UG1009_STATUS_ERR_4) {
		print_jtag_code();
		clear_error();
		jtag_erase_sram();
	}

	jtag_write_inst(ISC_ENABLE);
	jtag_write_inst(INIT_ADDRESS);
	jtag_write_inst(FAST_PROGRAM);

	jtag_move_tap(TAP_IDLE, TAP_DRSHIFT);

	for (i = 0; i < fssize; i++) {
		/* program the fs file, write each byte with MSB */
		jtag_write(fs_p[i], 0x00, 0x00, 0);
	}

	jtag_write(UG1009_JTAG_EMPTY_DATA, 0x00, 0x01, 1);

	jtag_move_tap(TAP_DREXIT1, TAP_IDLE);
	jtag_move_tap(TAP_IDLE, TAP_IDLE);

	jtag_write_inst(ISC_DISABLE);
	jtag_write_inst(ISC_NOOP);
	jtag_runtest(UG1009_JTAG_RUNTEST_TIMES_1);
	mipibridge_infomsg("end\n");
}

static int ug1009_jtag_program_sram(uint8_t *fs_ip, uint32_t fs_ip_size)
{
	int ret = MIPI_BRIDGE_FAIL;
	int retry = 0;
	uint32_t idcode = 0;
	uint32_t status_code = 0;
	int i = 0;

	if (!fs_ip) {
		mipibridge_errmsg("firmware ip is err\n");
		return MIPI_BRIDGE_FAIL;
	}

	ret = check_IDcode(UG1009_ID_CODE_CMD, read_fsfile_idcode(fs_ip, fs_ip_size));
	if (ret < 0) {
		mipibridge_infomsg("----!!fs file and fpga device mismatching---\n");
		return ret;
	}

	do {
		retry++;
		ug1009_jtag_prog_sram(fs_ip, fs_ip_size);
		ug1009_spi_gpio_init();
		ret = mipibridge_set_spi_freq_and_mode(UG1009_SPI_CLK, 0);
		if (ret) {
			mipibridge_errmsg("set spi err, ret=%d\n", ret);
			return MIPI_BRIDGE_FAIL;
		}

		for(i = 0; i < UG1009_SPI_READ_CODE_RETRY; i++) {
			ug1009_jtag_switch_spi();
			idcode = read_device_code(READ_ID_CODE);
			status_code = read_device_code(READ_STATUS_CODE);
			if(status_code == UG1009_STATUS_SUCC_1 || status_code == UG1009_STATUS_SUCC_2)
				break;
			mipibridge_infomsg("read_device_code retry, idcode=0x%x, status_code=0x%x, i=%d\n", idcode, status_code, i);
			mdelay(UG1009_SPI_DELAY_MS_10);
		}
		if(status_code == UG1009_STATUS_SUCC_1 || status_code == UG1009_STATUS_SUCC_2)
			break;

		ret = ug1009_jtag_gpio_init();
		if (ret) {
			mipibridge_errmsg("jtag gpio init err, ret=%d\n", ret);
			return MIPI_BRIDGE_FAIL;
		}		
		ug1009_spi_switch_jtag();
		mipibridge_errmsg("sram err! write %d times, idcode=0x%x, status_code=0x%x\n", retry, idcode, status_code);
		continue;
	} while(retry < UG1009_SPI_CMD_RETRY);

	if(status_code != UG1009_STATUS_SUCC_1 && status_code != UG1009_STATUS_SUCC_2) {
		ret = MIPI_BRIDGE_FAIL;
	}
	mipibridge_infomsg("sram write %d times, idcode=0x%x, status_code=0x%x\n", retry, idcode, status_code);
	return ret;
}

static uint32_t read_fsfile_usercode(uint8_t *fs_lcd, uint32_t fs_lcd_size)
{
	uint32_t usercode_addr = 0;

	if (fs_lcd_size <= (UG1009_USER_CODE_FW_AAR_OFFSET + UG1009_CODE_LEN)) {
		mipibridge_errmsg("fssize %u err\n", fs_lcd_size);
		return 0;
	}

	usercode_addr = fs_lcd_size - UG1009_USER_CODE_FW_AAR_OFFSET;

	return FOUR_U8_TO_U32(*(fs_lcd + usercode_addr), *(fs_lcd + usercode_addr + 1),
		*(fs_lcd + usercode_addr + UG1009_FW_VER_ADDR_2), *(fs_lcd + usercode_addr + UG1009_FW_VER_ADDR_3));
}

static int ug1009_is_upgrade_fw(const struct firmware *fw_lcd)
{
	uint32_t bridge_version = 0;
	uint32_t fw_version = 0;

	if (!fw_lcd || !(fw_lcd->data)) {
		mipibridge_errmsg("firmware lcd err!!!\n");
		return FIRMWARE_ERROR_FW;
	}

	gw_jtag_reset();
	bridge_version = jtag_read_code(UG1009_USER_CODE_ADDR);
	fw_version = read_fsfile_usercode((uint8_t *)fw_lcd->data, fw_lcd->size);
	mipibridge_infomsg("bridge_version=0x%x, firmware_version=0x%x\n", bridge_version, fw_version);
	if (bridge_version == 0xFFFFFFFF) {
		mipibridge_errmsg("empty firmware, need update!\n");
		return FIRMWARE_UPGRADE_CHECK_OK;
	}

	if (fw_version <= bridge_version) {
		mipibridge_errmsg("older firmware\n");
		return FIRMWARE_UPGRADE_OLD;
	}
	return FIRMWARE_UPGRADE_CHECK_OK; /* need to upgrade */
}

static int ug1009_write_one_xpage(uint8_t *data, uint32_t address, uint32_t load_len, bool is_verify)
{
	uint8_t tmp_rchar[UG1009_SPI_TEMP_BUFF_LEN] = {0x01, 0x01, 0x01, 0x01};
	int i = 0;
	int ret = 0;
	uint32_t addr = (address << UG1009_SHIFT_6);
	uint32_t len = 0;
	uint32_t len_max = UG1009_WRITE_BUFF_MAX_LEN - UG1009_SPI_TEMP_BUFF_LEN;
	bool is_max = (load_len == UG1009_WRITE_BUFF_LEN);

	if (!data || load_len > UG1009_WRITE_BUFF_LEN) {
		mipibridge_errmsg("data is NULL, load_len=%u\n", load_len);
		return MIPI_BRIDGE_FAIL;
	}

	ug1009_jtag_switch_spi();

	g_wdata_buf[len++] = UG1009_SPI_XPAGE_WRITE_READ;
	g_wdata_buf[len++] = (uint8_t)(addr >> SHIFT_24);
	g_wdata_buf[len++] = (uint8_t)(addr >> SHIFT_16);
	g_wdata_buf[len++] = (uint8_t)(addr >> SHIFT_8);
	g_wdata_buf[len++] = (uint8_t)(addr & 0xff);

	for (i = 0; i < UG1009_WRITE_BUFF_LEN && len < len_max; i += UG1009_SPI_TEMP_BUFF_LEN) {
		g_wdata_buf[len++] = (is_max || (i + UG1009_ID_OFFSET_3) < load_len) ? *(data + i + UG1009_ID_OFFSET_3) : 0xff;
		g_wdata_buf[len++] = (is_max || (i + UG1009_ID_OFFSET_2) < load_len) ? *(data + i + UG1009_ID_OFFSET_2) : 0xff;
		g_wdata_buf[len++] = (is_max || (i + 1) < load_len) ? *(data + i + 1) : 0xff;
		g_wdata_buf[len++] = (is_max || i < load_len) ? *(data + i) : 0xff;
	}
	ret = mipibridge_spi_send_transfer(g_wdata_buf, len);
	if (ret < 0) {
		mipibridge_errmsg("write data fail!\n");
		goto err;
	}
	mdelay(2);

	if (is_verify) {
		/* 1: length of recv bytes */
		if (mipibridge_spi_recive_transfer(UG1009_SPI_READ_VERIFY_RESULT, tmp_rchar, 1) < 0) {
			mipibridge_errmsg("mipibridge_spi_recive_transfer fail!\n");
			return MIPI_BRIDGE_FAIL;
		}

		if (tmp_rchar[0] != UG1009_SPI_VERIFY_OK) {
			mipibridge_errmsg(" addr=0x%x, fail tmp_rchar[0] = 0x%x, is not 0x%x!\n",
				address, tmp_rchar[0], UG1009_SPI_VERIFY_OK);
			return MIPI_BRIDGE_FAIL;
		}
	}
	return MIPI_BRIDGE_OK;

err:
	mipibridge_errmsg("write_one_xpage fail!\n");
	return MIPI_BRIDGE_FAIL;
}

static int ug1009_spi_flash_write(uint8_t *fs_lcd, uint32_t fs_lcd_size)
{
	uint32_t i = 0;
	uint32_t pack_num = fs_lcd_size >> SHIFT_8;
	uint32_t load_page = 0;
	uint32_t load_len = 0;
	uint8_t *data_addr = fs_lcd;
	uint8_t tmp_tchar[UG1009_WRITE_BUFF_LEN] = {0};

	if (!fs_lcd) {
		mipibridge_errmsg("fw lcd is err\n");
		return MIPI_BRIDGE_FAIL;
	}

	mipibridge_infomsg("pack_num is %u\n", pack_num);

	for (i = 0; i <= pack_num; i++) {
		/* first send the last pack */
		load_len = (i != pack_num) ? UG1009_WRITE_BUFF_LEN : (fs_lcd_size % UG1009_WRITE_BUFF_LEN);
		if (ug1009_write_one_xpage(data_addr, load_page, load_len, true) < 0) {
			mipibridge_errmsg("ug1009_write_one_xpage fail\n");
			return MIPI_BRIDGE_FAIL;
		}
		load_page += 1;
		data_addr += UG1009_WRITE_BUFF_LEN;
	}

	if (memset_s(tmp_tchar, UG1009_WRITE_BUFF_LEN, 0xff, UG1009_WRITE_BUFF_LEN) != EOK) {
		mipibridge_errmsg("memcpy_s fail err\n");
		return MIPI_BRIDGE_FAIL;
	}

	tmp_tchar[0] = UG1009_SPI_WRITE_CMD_1;
	tmp_tchar[1] = UG1009_SPI_WRITE_CMD_2;
	tmp_tchar[2] = UG1009_SPI_WRITE_CMD_3;
	tmp_tchar[3] = UG1009_SPI_WRITE_CMD_4;
	if (ug1009_write_one_xpage(tmp_tchar, 0, UG1009_SPI_TEMP_BUFF_LEN, false) < 0) {
		mipibridge_errmsg("write xpage fail\n");
		return MIPI_BRIDGE_FAIL;
	}

	/* reconfig_n */
	tmp_tchar[0] = UG1009_SPI_SET_RECONFIGN;
	tmp_tchar[1] = UG1009_SPI_SET_RECONFIGN_2;
	/* 2: length of send bytes */
	if (mipibridge_spi_send_transfer(tmp_tchar, UG1009_SPI_TEMP_BUFF_LEN2) < 0) {
		mipibridge_errmsg("send transfer fail\n");
		return MIPI_BRIDGE_FAIL;
	}
	mdelay(UG1009_SPI_DELAY_MS_100);
	return MIPI_BRIDGE_OK;
}

static int ug1009_spi_flash_erash(void)
{
	uint8_t tmp_tchar[UG1009_SPI_TEMP_BUFF_LEN] = {0};
	uint8_t tmp_rchar[UG1009_SPI_TEMP_BUFF_LEN] = {0xff, 0xff, 0xff, 0xff};
	int retry = 0;

	ug1009_jtag_switch_spi();
	/* erase_flash */
	tmp_tchar[0] = UG1009_SPI_CHIP_ERASE;
	if (mipibridge_spi_send_transfer(tmp_tchar, 1) < 0)
		goto err;
	mdelay(UG1009_SPI_DELAY_MS_180);

	do {
		if (mipibridge_spi_recive_transfer(UG1009_SPI_READ_VERIFY_RESULT, tmp_rchar, 1) < 0) {
			mipibridge_errmsg("mipibridge_spi_recive_transfer fail!\n");
			return MIPI_BRIDGE_FAIL;
		}
		retry++;
		mdelay(UG1009_SPI_DELAY_MS_10);
	} while ((tmp_rchar[0] & UG1009_SPI_ERASE_OK) && retry < UG1009_SPI_CMD_RETRY);

	if (tmp_rchar[0] & UG1009_SPI_ERASE_OK) {
		mipibridge_errmsg("mipibridge_spi_recive_transfer retry %d fail tmp_rchar[0] = 0x%x, is not 0x%x!\n",
			UG1009_SPI_CMD_RETRY, tmp_rchar[0], UG1009_SPI_ERASE_OK);
		return MIPI_BRIDGE_FAIL;
	}
	mipibridge_infomsg("erash done!\n");
	print_spi_code();
	return MIPI_BRIDGE_OK;
err:
	mipibridge_errmsg("ug1009_spi_flash_erash fail!\n");
	return MIPI_BRIDGE_FAIL;
}

static int ug1009_spi_firmware_flash_load(uint8_t *fs_lcd, uint32_t fs_lcd_size)
{
	int ret = MIPI_BRIDGE_OK;
	uint8_t tmp_tchar[UG1009_SPI_TEMP_BUFF_LEN] = {0};
	uint32_t status_code = 0;

	if (!fs_lcd) {
		mipibridge_errmsg("fw lcd is err\n");
		return MIPI_BRIDGE_FAIL;
	}

	if (ug1009_spi_flash_erash() < 0) {
		mipibridge_errmsg("erase err!\n");
		ret = MIPI_BRIDGE_FAIL;
		goto err;
	}

	if (ug1009_spi_flash_write(fs_lcd, fs_lcd_size) < 0) {
		mipibridge_errmsg("spi write err!\n");
		ret = MIPI_BRIDGE_FAIL;
		goto err;
	}
err:
	mipibridge_infomsg("ret = %d\n", ret);
	ret = ug1009_jtag_gpio_init();
	if (ret) {
		mipibridge_errmsg("jtag gpio init err, ret=%d\n", ret);
		return MIPI_BRIDGE_FAIL;
	}
	gw_jtag_reset();
	jtag_write_inst(ISC_DISABLE);
	jtag_write_inst(ISC_NOOP);
	print_jtag_code();
	return ret;
}

static int ug1009_uart_parse_recv_data(uint8_t *buffer, size_t len, unsigned int *value)
{
	bool sts_ok = false;
	int i = 0;

	if (!buffer || len < UG1009_UART_MSG_BUFF_LEN  || len > UG1009_UART_MSG_MAX_LEN) {
		mipibridge_errmsg("buffer err, len = %d\n", len);
		return MIPI_BRIDGE_FAIL;
	}

	for (i = 0; i < len - UG1009_MSG_IDX_TAIL_3; i++) {
		/* 0:head1 1:head2 2:head3 3:bit25~31 4:bit16~24 5:bit8~15 6:bit0~7 7:tail1 8:tail2 9:tail3 */
		sts_ok = buffer[UG1009_MSG_IDX_HEADER_1 + i] == UG1009_MSG_HEADER_1 &&
			buffer[UG1009_MSG_IDX_HEADER_2 + i] == UG1009_MSG_HEADER_2 &&
			buffer[UG1009_MSG_IDX_HEADER_3 + i] == UG1009_MSG_HEADER_3 &&
			buffer[UG1009_MSG_IDX_TAIL_1 + i] == UG1009_MSG_TAIL_1 &&
			buffer[UG1009_MSG_IDX_TAIL_2 + i] == UG1009_MSG_TAIL_2 &&
			buffer[UG1009_MSG_IDX_TAIL_3 + i] == UG1009_MSG_TAIL_3;
		if (sts_ok) {
			*value = FOUR_U8_TO_U32(buffer[UG1009_MSG_IDX_INFO_1 + i], buffer[UG1009_MSG_IDX_INFO_2 + i],
				buffer[UG1009_MSG_IDX_INFO_3 + i], buffer[UG1009_MSG_IDX_INFO_4 + i]);
			mipibridge_infomsg("recv data = 0x%x\n",*value);
			return MIPI_BRIDGE_OK;
		} else
			mipibridge_infomsg("buffer[%d] = 0x%02x\n", i, buffer[i]);
	}

	for (i = 0; i < len; i++)
		mipibridge_errmsg("ignore invalid data, buffer[%d] = 0x%02x\n", i, buffer[i]);
	return MIPI_BRIDGE_FAIL;
}

static int ug1009_uart_send_pre_check(void)
{
	int ret = MIPI_BRIDGE_OK;
	int len = 0;
	size_t read_len = 0;
	unsigned int check_data = UG1009_MSG_PRE_CHECK;
	unsigned int recv_data = 0;
	uint8_t buffer[UG1009_UART_BUFF_LEN] = {0};

	/* add protocol header 0xAA 0x87 0x37 */
	g_ug1009_trans_buffer[len++] = UG1009_MSG_HEADER_1;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_HEADER_2;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_HEADER_3;
	/* add serialNum and change the 0x5c to 0x5c 0x5c */
	g_ug1009_trans_buffer[len++] = UG1009_UART_CHECK_CMD; /* A0:check */
	g_ug1009_trans_buffer[len++] = 0x00;
	g_ug1009_trans_buffer[len++] = 0x00;
	g_ug1009_trans_buffer[len++] = 0x00;
	/* add protocol tail 0xAA 0x55 0xFF */
	g_ug1009_trans_buffer[len++] = UG1009_MSG_TAIL_1;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_TAIL_2;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_TAIL_3;
	ret = mipibridge_uart_write(g_ug1009_trans_buffer, len);
	if (ret < 0) {
		mipibridge_errmsg("uart write err ret = %d\n", ret);
		return ret;
	}

	ret = mipibridge_uart_read(buffer, &read_len, UG1009_UART_MSG_RECV_TIMEOUT, UG1009_UART_MSG_BUFF_LEN);
	if (ret < 0) {
		mipibridge_errmsg("uart read err ret = %d\n", ret);
		return ret;
	}

	ret = ug1009_uart_parse_recv_data(buffer, read_len, &recv_data);
	if (ret < 0 || recv_data != check_data) {
		mipibridge_errmsg("data mismatch, check_data 0x%x is not 0x%x, ret = %d\n", check_data, recv_data, ret);
		return ret;
	}

	return ret;
}

static int ug1009_uart_write_data(u8 addr, unsigned int value)
{
	int ret = MIPI_BRIDGE_OK;
	int len = 0;
	size_t read_len = 0;
	unsigned int check_data = (UG1009_UART_WRITE_ACK << SHIFT_24) | (addr << SHIFT_16) | LOWER_16_BITS(value);
	unsigned int recv_data = 0;
	u8 buffer[UG1009_UART_BUFF_LEN] = {0};

	/* add protocol header 0xAA 0x87 0x37 */
	g_ug1009_trans_buffer[len++] = UG1009_MSG_HEADER_1;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_HEADER_2;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_HEADER_3;

	g_ug1009_trans_buffer[len++] = UG1009_UART_WRITE_CMD;
	g_ug1009_trans_buffer[len++] = addr;
	g_ug1009_trans_buffer[len++] = LOWER_8_BITS((value >> SHIFT_8));
	g_ug1009_trans_buffer[len++] = LOWER_8_BITS(value);
	/* add protocol tail 0xAA 0x55 0xFF */
	g_ug1009_trans_buffer[len++] = UG1009_MSG_TAIL_1;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_TAIL_2;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_TAIL_3;
	ret = mipibridge_uart_write(g_ug1009_trans_buffer, len);
	if (ret < 0) {
		mipibridge_errmsg("uart write err ret = %d\n", ret);
		return ret;
	}

	ret = mipibridge_uart_read(buffer, &read_len, UG1009_UART_MSG_RECV_TIMEOUT, UG1009_UART_MSG_BUFF_LEN);
	if (ret < 0) {
		mipibridge_errmsg("uart read err ret = %d\n", ret);
		return ret;
	}

	ret = ug1009_uart_parse_recv_data(buffer, read_len, &recv_data);
	if (ret < 0 || recv_data != check_data) {
		mipibridge_errmsg("data mismatch, check_data 0x%x is not 0x%x, ret = %d\n", check_data, recv_data, ret);
		return ret;
	}

	return ret;
}

static int ug1009_uart_read_data(u8 addr, unsigned int *recv_data)
{
	int ret = MIPI_BRIDGE_OK;
	int len = 0;
	size_t read_len = 0;
	u8 buffer[UG1009_UART_BUFF_LEN] = {0};

	/* add protocol header 0xAA 0x87 0x37 */
	g_ug1009_trans_buffer[len++] = UG1009_MSG_HEADER_1;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_HEADER_2;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_HEADER_3;

	g_ug1009_trans_buffer[len++] = UG1009_UART_READ_CMD;
	g_ug1009_trans_buffer[len++] = addr;
	g_ug1009_trans_buffer[len++] = 0x00;
	g_ug1009_trans_buffer[len++] = 0x00;
	/* add protocol tail 0xAA 0x55 0xFF */
	g_ug1009_trans_buffer[len++] = UG1009_MSG_TAIL_1;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_TAIL_2;
	g_ug1009_trans_buffer[len++] = UG1009_MSG_TAIL_3;
	ret = mipibridge_uart_write(g_ug1009_trans_buffer, len);
	if (ret < 0) {
		mipibridge_errmsg("uart write err ret = %d\n", ret);
		return ret;
	}

	ret = mipibridge_uart_read(buffer, &read_len, UG1009_UART_MSG_RECV_TIMEOUT, UG1009_UART_MSG_BUFF_LEN);
	if (ret < 0) {
		mipibridge_errmsg("uart read err ret = %d\n", ret);
		return ret;
	}

	ret = ug1009_uart_parse_recv_data(buffer, read_len, recv_data);
	if (ret < 0) {
		mipibridge_errmsg("parse_recv_data err, ret = %d\n", ret);
		return ret;
	}

	return ret;
}

static int ug1009_uart_send_data(u8 addr, unsigned int value)
{
	int ret = MIPI_BRIDGE_OK;

	ret = ug1009_uart_send_pre_check();
	if (ret < 0) {
		mipibridge_errmsg("send_pre_check err ret = %d\n", ret);
		return ret;
	}

	ret = ug1009_uart_write_data(addr, value);
	if (ret < 0) {
		mipibridge_errmsg("write_data err ret = %d\n", ret);
		return ret;
	}

	return ret;
}

void ug1009_uart_off_lcd(void)
{
	int ret = 0;

	mipibridge_uart_recv_disable();
	ret = ug1009_uart_write_data(0x0, 0x0);
	if (ret < 0) {
		mipibridge_errmsg("write_data 0x0 err ret = %d\n", ret);
		return;
	}

	ret = ug1009_uart_write_data(0x0, 0x1);
	if (ret < 0) {
		mipibridge_errmsg("write_data  0x1 err ret = %d\n", ret);
		return;
	}
}

static int ug1009_uart_recv_data(u8 addr, unsigned int *recv_data)
{
	int ret = MIPI_BRIDGE_OK;

	ret = ug1009_uart_send_pre_check();
	if (ret < 0) {
		mipibridge_errmsg("send_pre_check err ret = %d\n", ret);
		return ret;
	}

	ret = ug1009_uart_read_data(addr, recv_data);
	if (ret < 0) {
		mipibridge_errmsg("read_data err ret = %d\n", ret);
		return ret;
	}

	return ret;
}

static void ug1009_uart_read_ic_status(struct mipibridge_device_info *di)
{
	int i = 0;
	int ret = 0;
	int sts = 0;
	unsigned int recv_data[UG1009_MSG_CHECK_STATUS_NUM] = {0};

	if (!di) {
		mipibridge_errmsg("di NULL\n");
		return;
	}

	for (i = 0; i < UG1009_MSG_CHECK_STATUS_NUM; i++)
		ug1009_uart_recv_data(i, &recv_data[i]);

	mipibridge_errmsg(" 0x0000 | 0x0001 | 0x0002 | 0x0003 | 0x0004\n");
	mipibridge_errmsg("--------+-------+--------+--------+--\n");
	mipibridge_errmsg(" 0x%04x | 0x%04x | 0x%04x | 0x%04x | 0x%04x\n",
		LOWER_16_BITS(recv_data[UG1009_REG_CTRL_ADDR]),
		LOWER_16_BITS(recv_data[UG1009_REG_VER_ADDR]),
		LOWER_16_BITS(recv_data[UG1009_REG_STATUS1_ADDR]),
		LOWER_16_BITS(recv_data[UG1009_REG_STATUS2_ADDR]),
		LOWER_16_BITS(recv_data[UG1009_REG_ID_ADDR]));
	mipibridge_infomsg("=====================================\n");
}

static void ug1009_uart_recv_ic_status(u8 *recv_buff, unsigned int len)
{
	int i = 0;
	int ret = 0;
	unsigned int recv_err_data = 0;
	struct mipibridge_hievent_info hievent_info = { 0 };

	if (!recv_buff || len < UG1009_UART_MSG_BUFF_LEN  || len > UG1009_UART_MSG_MAX_LEN) {
		mipibridge_errmsg("buffer err, len = %d\n", len);
		return;
	}

	ret = ug1009_uart_parse_recv_data(recv_buff, len, &recv_err_data);
	if (ret < 0) {
		mipibridge_errmsg("uart_recv err!, ret = %d\n", ret);
		return;
	}

	if (LOWER_16_BITS(recv_err_data >> SHIFT_16) == UG1009_UART_REPORT_RGE) {
		mipibridge_infomsg("mipibridge report err is 0x%x, hiview report\n", recv_err_data);
		hievent_info.hievent_id = MIPIBRIDGE_REG_STATUS_NO;
		ret = snprintf_s(hievent_info.content, sizeof(hievent_info.content), sizeof(hievent_info.content) - 1,
			"mipibridge gy report err is 0x%x", recv_err_data);
		if (ret < 0) {
			mipibridge_errmsg("snprintf_s err\n");
			return;
		}
		mipibridge_hiview_report(&hievent_info);
	}
}

static int ug1009_read_dual_firmware(struct device *dev)
{
	int ret = MIPI_BRIDGE_OK;
	bool sts_ok = false;

	if (!dev) {
		mipibridge_errmsg("dev NULL\n");
		return FIRMWARE_UPGRADE_FAIL;
	}

	ret = request_firmware(&g_firmware_ip, FW_PATH_GW_IP, dev);
	if (ret < 0) {
		ret = FIRMWARE_NOT_FIND_FW;
		mipibridge_errmsg("request firmware ip fail %d\n", ret);
		goto err;
	}

	ret = request_firmware(&g_firmware_lcd, FW_PATH_GW_LCD, dev);
	if (ret < 0) {
		ret = FIRMWARE_NOT_FIND_FW;
		mipibridge_errmsg("request firmware lcd fail %d\n", ret);
		goto err;
	}

	sts_ok = (g_firmware_ip && g_firmware_lcd &&
		g_firmware_ip->data && g_firmware_lcd->data);
	if (!sts_ok) {
		ret = FIRMWARE_ERROR_FW;
		mipibridge_errmsg("firmware data err!\n");
		goto err;
	}

	sts_ok = (g_firmware_ip->size < UG1009_FW_IP_MAX_LEN &&
		g_firmware_ip->size > UG1009_FW_IP_MIN_LEN &&
		g_firmware_lcd->size < UG1009_FW_LCD_MAX_LEN &&
		g_firmware_lcd->size > UG1009_FW_LCD_MIN_LEN);
	if (!sts_ok) {
		ret = FIRMWARE_ERROR_FW;
		mipibridge_errmsg("firmware size err! firmware_ip_size=%u, firmware_lcd_size=%u\n",
			g_firmware_ip->size, g_firmware_lcd->size);
		goto err;
	}

	mipibridge_infomsg("succ, firmware_ip_size=%u, firmware_lcd_size=%u\n",
		g_firmware_ip->size, g_firmware_lcd->size);
	return ret;

err:
	mipibridge_infomsg("read ug1009 firmware err! ret = %d\n", ret);
	release_firmware(g_firmware_ip);
	release_firmware(g_firmware_lcd);
	return ret;
}

static int ug1009_jtag_spi_firmware_load(struct device *dev)
{
	int ret = 0;
	int retry = 0;

	if (!g_firmware_ip || !g_firmware_lcd) {
		mipibridge_errmsg("firmware is NULL!\n");
		return FIRMWARE_ERROR_FW;
	}

	gw_jtag_reset();
	jtag_write_inst(ISC_DISABLE);
	jtag_write_inst(ISC_NOOP);

	ret = ug1009_is_upgrade_fw(g_firmware_lcd);
	if (ret != FIRMWARE_UPGRADE_CHECK_OK) {
		mipibridge_errmsg("check fw version err, not upgrade!\n");
		goto err;
	}

	do {
		retry++;
		ret = ug1009_jtag_program_sram((uint8_t *)(g_firmware_ip->data), g_firmware_ip->size);
		if (ret < 0) {
			mipibridge_errmsg("sram err! ret = %d\n", ret);
			ret = FIRMWARE_UPGRADE_FAIL;
			goto err;
		}

		ret = ug1009_spi_firmware_flash_load((uint8_t *)(g_firmware_lcd->data), g_firmware_lcd->size);
		if (ret == MIPI_BRIDGE_OK && ug1009_is_upgrade_fw(g_firmware_lcd) == FIRMWARE_UPGRADE_OLD) {
			mipibridge_infomsg("prog em-flash sucess!!\n");
			ret = FIRMWARE_UPGRADE_SUCC;
			break;
		} else {
			mipibridge_errmsg("prog em-flash false! retry=%d\n", retry);
			ret = FIRMWARE_UPGRADE_FAIL;
			continue;
		}
	} while (retry < UG1009_SPI_CMD_RETRY);

err:
	/* enable mipibridge err report */
	mipibridge_uart_recv_enable(UG1009_UART_MSG_BUFF_LEN);
	release_firmware(g_firmware_ip);
	release_firmware(g_firmware_lcd);
	mipibridge_infomsg("ug1009 firmware update %d times, ret=%d\n", retry, ret);
	/* FIRMWARE_UPGRADE_SUCC:success, other:fail */
	return ret;
}

int ug1009_probe(struct mipibridge_device_info *di)
{
	struct device_node *dn = NULL;
	int ret = MIPI_BRIDGE_OK;

	if (!di || !di->dev ) {
		mipibridge_errmsg("dev NULL\n");
		return MIPI_BRIDGE_FAIL;
	}

	dn = of_get_child_by_name(di->dev->of_node, PANEL_UG1009);
	if (dn) {
		mipibridge_parse_u32(dn, "tck_gpio", &tck_gpio, 0);
		mipibridge_parse_u32(dn, "tdo_gpio", &tdo_gpio, 0);
		mipibridge_parse_u32(dn, "tdi_gpio", &tdi_gpio, 0);
		mipibridge_parse_u32(dn, "tms_gpio", &tms_gpio, 0);
	}
	ret = ug1009_jtag_gpio_init();
	if (ret) {
		mipibridge_errmsg("jtag gpio init err, ret=%d\n", ret);
		return MIPI_BRIDGE_FAIL;
	}

	if (di->uart.support && di->uart.uart_recv_support) {
		/* only read once */
		ug1009_uart_read_ic_status(di);
		di->uart.recv_fuc = ug1009_uart_recv_ic_status;
		/* enable mipibridge err report */
		mipibridge_uart_recv_enable(UG1009_UART_MSG_BUFF_LEN);
	}

	if (di->fw.support) {
		di->fw.fw_update = ug1009_jtag_spi_firmware_load;
		di->fw.open_fw = ug1009_read_dual_firmware;
		di->fw.off_lcd = ug1009_uart_off_lcd;
	}

	return MIPI_BRIDGE_OK;
}
