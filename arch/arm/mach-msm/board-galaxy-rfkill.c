/*
 * Copyright (C) 2008 Google, Inc.
 * Author: Nick Pelly <npelly@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/* Control bluetooth power for trout platform */
/* BT on/off */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/rfkill.h>
#include <linux/delay.h>

#include <asm/gpio.h>
#include <mach/hardware.h>

#include "board-galaxy.h"

/* dgahn.bcm_mutex */
extern int bcm4325_set_core_power(unsigned bcm_core, unsigned pow_on);
static DEFINE_MUTEX(bcm4325_pwr_lock);
#define BCM4325_BT 0
#define BCM4325_WLAN 1

//int btsleep_start(void);
//void btsleep_stop(void);

static struct rfkill *bt_rfk;
static const char bt_name[] = "bcm4329";		// how to? -> It needs to be edited

/* dgahn.bcm_mutex */
int bcm4325_set_core_power(unsigned bcm_core, unsigned pow_on)
{
	unsigned gpio_rst, gpio_rst_another;

	switch(bcm_core) {
	case BCM4325_BT:
		gpio_rst = GPIO_BT_RESET;
		gpio_rst_another = GPIO_WLAN_RESET;
		break;

	case BCM4325_WLAN:
		gpio_rst = GPIO_WLAN_RESET;
		gpio_rst_another = GPIO_BT_RESET;
		break;

	default:
		printk(KERN_ERR "bcm4325_power: Unknown bcm4325 core!\n");
		return -1;
	}

	mutex_lock(&bcm4325_pwr_lock);

	/* if another core is OFF */
	if( gpio_get_value(gpio_rst_another) == 0 )
	{
		/* enable BT_REG_ON */
		gpio_direction_output(GPIO_BT_REG_ON, pow_on);
		printk("bcm4325_power: Set BT_REG_ON %s because %s is OFF now.\n",
		gpio_get_value(GPIO_BT_REG_ON)?"High":"Low", bcm_core?"BT":"WLAN");
		msleep(150);
	}

	/* enable specified core */
	gpio_direction_output(gpio_rst, pow_on);
	printk("bcm4325_power: Set %s %s\n",
		bcm_core?"WLAN_RESET":"BT_RESET", gpio_get_value(gpio_rst)?"High":"Low");

	mutex_unlock(&bcm4325_pwr_lock);

	return 0;
}
EXPORT_SYMBOL(bcm4325_set_core_power);

/* /sys/class/rfkill/rfkill0/state */
static int bluetooth_set_power(void *data, bool blocked)
{
	if (blocked) {
		bcm4325_set_core_power(BCM4325_BT, 0);  /* dgahn.bcm_mutex */
		//btsleep_stop();
	}
	else {
		bcm4325_set_core_power(BCM4325_BT, 1);  /* dgahn.bcm_mutex */
		//btsleep_start();
	}
	return 0;
}

static struct rfkill_ops galaxy_rfkill_ops = {
	.set_block = bluetooth_set_power,
};

static int galaxy_rfkill_probe(struct platform_device *pdev)
{
	int rc = 0;
	bool default_state = true;  /* off */

	/* default to bluetooth off */
	bluetooth_set_power(NULL, default_state);

	bt_rfk = rfkill_alloc(bt_name, &pdev->dev, RFKILL_TYPE_BLUETOOTH,
			&galaxy_rfkill_ops, NULL);
	if (!bt_rfk)
		return -ENOMEM;

	rfkill_set_states(bt_rfk, default_state, false);

//	bt_rfk->name = bt_name;
//	bt_rfk->state = default_state;
//	/* userspace cannot take exclusive control */
//	bt_rfk->user_claim_unsupported = 1;
//	bt_rfk->user_claim = 0;
//	bt_rfk->data = NULL;  // user data
//	bt_rfk->toggle_radio = bluetooth_set_power;

	rc = rfkill_register(bt_rfk);

	if (!rc)
		goto err_rfkill_reg;

	return 0;

err_rfkill_reg:
	rfkill_destroy(bt_rfk);

	return rc;
}

static int galaxy_rfkill_remove(struct platform_device *dev)
{
	rfkill_unregister(bt_rfk);
	rfkill_destroy(bt_rfk);

	return 0;
}

static struct platform_driver galaxy_rfkill_driver = {
	.probe = galaxy_rfkill_probe,
	.remove = galaxy_rfkill_remove,
	.driver = {
		.name = "galaxy_rfkill",
		.owner = THIS_MODULE,
	},
};

static int __init galaxy_rfkill_init(void)
{
	return platform_driver_register(&galaxy_rfkill_driver);
}

static void __exit galaxy_rfkill_exit(void)
{
	platform_driver_unregister(&galaxy_rfkill_driver);
}

module_init(galaxy_rfkill_init);
module_exit(galaxy_rfkill_exit);
MODULE_DESCRIPTION("galaxy rfkill");
MODULE_AUTHOR("Nick Pelly <npelly@google.com>");
MODULE_LICENSE("GPL");
