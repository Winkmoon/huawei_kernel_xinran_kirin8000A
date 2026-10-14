

#ifndef __NFC_DEV_Q_H__
#define __NFC_DEV_Q_H__

#include <linux/miscdevice.h>
#include <linux/reboot.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#define ENABLE_START	0
#define ENABLE_END		1

#define CARD_UNKNOWN	0
#define CARD1_SELECT	1
#define CARD2_SELECT	2

enum {
    NFC_POWER_OFF,
    NFC_POWER_ON,
    NFC_VOTE_POWER_OFF = 0x10, // Vote for NFC power-off to SE.
    NFC_VOTE_POWER_ON = 0x11, // Vote for NFC power-oon to SE.
};

struct caxx_dev {
	struct miscdevice caxx_device;
	int sim_switch;
	int sim_status;
	int enable_status;
};

#ifdef PLATFORM_DEBUG_ENABLE
static inline struct caxx_dev *get_ps_plat_from_dev(struct device *dev)
{
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	return (struct caxx_dev *)i2c_get_clientdata(i2c_client_dev);
}
#endif
#endif