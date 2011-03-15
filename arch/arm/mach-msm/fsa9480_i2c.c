/*
 * Copyright (C) 2007-2008 SAMSUNG Corporation.
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

#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <mach/vreg.h>
#include <asm/io.h>

#include <linux/uaccess.h>

#include "board-galaxy.h"

#define ALLOW_USPACE_RW		1

static struct i2c_client *pclient;

#define FSA9480_DEVICE_ID_ADD		0x01
#define FSA9480_REG_CTRL		0x02
#define FSA9480_REG_INT1		0x03
#define FSA9480_REG_INT2		0x04
#define FSA9480_REG_INT1_MASK		0x05
#define FSA9480_REG_INT2_MASK		0x06
#define FSA9480_DEVICE_TYPE1_ADD	0x0A
#define FSA9480_DEVICE_TYPE2_ADD	0x0B

/* Control */
#define CON_SWITCH_OPEN		(1 << 4)
#define CON_RAW_DATA		(1 << 3)
#define CON_MANUAL_SW		(1 << 2)
#define CON_WAIT		(1 << 1)
#define CON_INT_MASK		(1 << 0)
#define CON_MASK		(CON_SWITCH_OPEN | CON_RAW_DATA | \
				CON_MANUAL_SW | CON_WAIT)

#define IRQ_USB_DET	MSM_GPIO_TO_INT(GPIO_USB_DET)

#define DEBUG_USB_SWITCH

#ifndef DEBUG_USB_SWITCH
#define DBG(fmt, arg...) do {} while (0)
#else
#define DBG(fmt, arg...) printk(KERN_INFO "%s: " fmt "\n", __func__, ## arg)
#endif

DECLARE_MUTEX(fsa_sem);

struct fsa9480_data {
	struct work_struct work;
};

static DECLARE_WAIT_QUEUE_HEAD(g_data_ready_wait_queue);


int fsa9480_i2c_tx_data(char* txData, int length)
{
	int rc; 

	struct i2c_msg msg[] = {
		{
			.addr = pclient->addr,
			.flags = 0,
			.len = length,
			.buf = txData,		
		},
	};
    
	rc = i2c_transfer(pclient->adapter, msg, 1);
	if (rc < 0) {
//		printk(KERN_ERR "fsa9480: fsa9480_i2c_tx_data error %d\n", rc);
		return rc;
	}
	return 0;
}

	
static int fsa9480_i2c_write(unsigned char u_addr, unsigned char u_data)
{
	int rc;
	unsigned char buf[2];

	buf[0] = u_addr;
	buf[1] = u_data;
    
	rc = fsa9480_i2c_tx_data(buf, 2);
	if(rc < 0)
		printk(KERN_ERR "fsa9480: txdata error %d add:0x%02x data:0x%02x\n", rc, u_addr, u_data);
	return rc;	
}


static int fsa9480_i2c_rx_data(char* rxData, int length)
{
	int rc;

	struct i2c_msg msgs[] = {
		{
			.addr = pclient->addr,
			.flags = 0,      
			.len = 1,
			.buf = rxData,
		},
		{
			.addr = pclient->addr,
			.flags = I2C_M_RD,//|I2C_M_NO_RD_ACK,
			.len = length,
			.buf = rxData,
		},
	};

	rc = i2c_transfer(pclient->adapter, msgs, 2);
      
	if (rc < 0) {
//		printk(KERN_ERR "fsa9480: fsa9480_i2c_rx_data error %d\n", rc);
		return rc;
	}
      
	return 0;
}

static int fsa9480_i2c_read(unsigned char u_addr, unsigned char *pu_data)
{
	int rc;
	unsigned char buf;

	buf = u_addr;
	rc = fsa9480_i2c_rx_data(&buf, 1);
	if (!rc)
		*pu_data = buf;
	else 
		printk(KERN_ERR "fsa9480: rxdata error %d add:0x%02x\n", rc, u_addr);
	return rc;	
}

static int fsa9480_read_irq(int *value)
{
	int ret;

	ret = i2c_smbus_read_i2c_block_data(pclient,
			FSA9480_REG_INT1, 2, (u8 *)value);
	*value &= 0xffff;

	if (ret < 0)
		dev_err(&pclient->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static void fsa9480_chip_init(void)
{
	printk(KERN_INFO "fsa9480: init\n");
	if (!pclient) 
		return;

	msleep(2);
	DBG("sensor init sequence done");
}

unsigned char fsa9480_i2c_read_reg(unsigned char addr, unsigned char *reg_value)
{
        unsigned char data = 0;

	if (fsa9480_i2c_read(addr, &data) < 0 )
		return -EIO;

	DBG("I2C read 0x%x Reg Data : 0x%x",addr, data);
	*reg_value = data;
	return 0;
}

unsigned char fsa9480_i2c_write_reg(unsigned char addr, unsigned char reg_value)
{
	DBG("I2C write 0x%x Reg Data : 0x%x", addr, reg_value);
	if (!fsa9480_i2c_write(addr, reg_value) < 0)
		return -EIO;
	return 0;
}

static int fsa9480_init_client(struct i2c_client *client)
{
	/* Initialize the fsa9480 Chip */
	init_waitqueue_head(&g_data_ready_wait_queue);
	return 0;
}

extern int fsa_init_done;
int fsa_suspended = 0;

extern irqreturn_t batt_level_update_isr(int, void *);

static irqreturn_t usb_switch_interrupt_handler(int irq, void *data)
{
	unsigned char ctrl; 
	int status;

	printk("fsa9480: IRQ FIRED\n");
	fsa9480_read_irq(&status);
	fsa9480_i2c_read_reg(FSA9480_REG_CTRL, &ctrl);
	ctrl &= ~CON_INT_MASK;
	fsa9480_i2c_write_reg(FSA9480_REG_CTRL, ctrl);

	//FIXME: cable status need to be handle here
	batt_level_update_isr(irq, data);

	printk("fsa9480: status = %d\n", status);

	return IRQ_HANDLED;
}

static int fsa9480_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct fsa9480_data *mt;
	int err = 0;
	unsigned char cont_reg;
	int ret, intr;
	printk(KERN_INFO "fsa9480: probe\n");
	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		goto exit_check_functionality_failed;		
	
	if(!(mt = kzalloc( sizeof(struct fsa9480_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	i2c_set_clientdata(client, mt);
	fsa9480_init_client(client);
	pclient = client;
	fsa9480_chip_init();
	
	fsa9480_i2c_write_reg(FSA9480_REG_CTRL, 0x1E); // FSA9480 initilaization
	fsa9480_i2c_read_reg(FSA9480_REG_CTRL, &cont_reg); // FSA9480 initilaization check
	DBG("Initial control reg 0x02 : 0x%x", cont_reg);


	/* clear interrupt */
	fsa9480_read_irq(&intr);

	/* unmask interrupt (attach/detach only) */
	fsa9480_i2c_write_reg(FSA9480_REG_INT1_MASK, 0xfc);
	fsa9480_i2c_write_reg(FSA9480_REG_INT2_MASK, 0x1f);

	fsa_init_done = 1;

	ret = gpio_request(GPIO_USB_DET, "usb_switch");
	if (ret < 0)
		goto err_request_usb_det_gpio;
	DBG("gpio_request ok");

	ret = gpio_direction_input(GPIO_USB_DET);
	if (ret < 0)
		goto err_set_usb_det_gpio_direction;
	DBG("direction input ok");

	ret = request_threaded_irq(IRQ_USB_DET, NULL,
			  usb_switch_interrupt_handler,
			  IRQF_TRIGGER_LOW | IRQF_ONESHOT , "usb_switch", NULL);
	if (ret < 0) {
		printk("fsa9480: request irq failed\n");
		goto err_request_usb_det_irq;
	}
	ret = set_irq_wake(IRQ_USB_DET, 1);
	if (ret < 0)
		printk("fsa9480: error setting irq wake\n");

	DBG("fsa9480 : request irq ok");

	return 0;
	
exit_alloc_data_failed:
exit_check_functionality_failed:

err_request_usb_det_irq:
	free_irq(IRQ_USB_DET,0);
err_set_usb_det_gpio_direction:
err_request_usb_det_gpio:
	gpio_free(IRQ_USB_DET);
	
	return err;
}

	
static int fsa9480_remove(struct i2c_client *client)
{
	struct fsa9480_data *mt = i2c_get_clientdata(client);
	free_irq(client->irq, mt);
#ifdef CONFIG_ANDROID_POWER
	android_unregister_early_suspend(&mt->early_suspend);
#endif
	i2c_set_clientdata(client, NULL);
	pclient = NULL;
	kfree(mt);
	return 0;
}

static int fsa9480_suspend(struct i2c_client *client, pm_message_t mesg)
{
	fsa_suspended = 1;
	return 0;
}

static int fsa9480_resume(struct i2c_client *client)
{
	fsa_suspended = 0;
	return 0;
}

static const struct i2c_device_id fsa9480_id[] = {
	{ "fsa9480", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, fsa9480_id);

static struct i2c_driver fsa9480_driver = {
	.id_table	= fsa9480_id,
	.probe = fsa9480_probe,
	.remove = fsa9480_remove,
	.suspend = fsa9480_suspend,
	.resume = fsa9480_resume,
	.driver = {		
		.name   = "fsa9480",
	},
};

static int __init fsa9480_init(void)
{
	printk(KERN_INFO "fsa9480_init\n");

	return i2c_add_driver(&fsa9480_driver);
}

static void __exit fsa9480_exit(void)
{
	i2c_del_driver(&fsa9480_driver);
}

EXPORT_SYMBOL(fsa9480_i2c_read_reg);

module_init(fsa9480_init);
module_exit(fsa9480_exit);

MODULE_AUTHOR("");
MODULE_DESCRIPTION("FSA9480 Driver");
MODULE_LICENSE("GPL");

