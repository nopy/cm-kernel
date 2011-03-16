/* linux/arch/arm/mach-msm/board-galaxy.c
 *
 * Copyright (C) 2007 Google, Inc.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without galaxyn the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/bootmem.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>

#include <mach/hardware.h>
#include <mach/irqs.h>
#include <mach/gpio.h>
#include <mach/socinfo.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/setup.h>

#include <mach/irqs.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_hsusb.h>
#include <mach/msm_rpcrouter.h>
#include <mach/camera.h>
#include <mach/memory.h>
#include <mach/msm_fb.h>
#include <mach/vreg.h>
#include <mach/usbdiag.h>
#include <mach/bcm_bt_lpm.h>
#include <mach/msm_serial_hs.h>
#include <mach/system.h>

#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c.h>
#include <linux/android_pmem.h>
#include <linux/usb/android_composite.h>

#include <linux/fsa9480.h>

#include "devices.h"
#include "board-galaxy.h"
#include "proc_comm.h"

extern int galaxy_init_mmc(void);
extern int init_keypad(void);

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.start = MSM_PMEM_MDP_BASE,
	.size = MSM_PMEM_MDP_SIZE,
	.no_allocator = 0,
	.cached = 1,
};

static struct android_pmem_platform_data android_pmem_camera_pdata = {
	.name = "pmem_camera",
	.start = MSM_PMEM_CAMERA_BASE,
	.size = MSM_PMEM_CAMERA_SIZE,
	.no_allocator = 0,
	.cached = 1,
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.start = MSM_PMEM_ADSP_BASE,
	.size = MSM_PMEM_ADSP_SIZE,
	.no_allocator = 0,
	.cached = 0,
};


static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct platform_device android_pmem_camera_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_camera_pdata },
};

static struct resource ram_console_resource[] = {
	{
		.start	= MSM_RAM_CONSOLE_BASE,
		.end	= MSM_RAM_CONSOLE_BASE + MSM_RAM_CONSOLE_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device ram_console_device = {
	.name = "ram_console",
	.id = -1,
	.num_resources  = ARRAY_SIZE(ram_console_resource),
	.resource       = ram_console_resource,
};

/* i2c devices */
/* sensors */
static struct i2c_gpio_platform_data sensors_i2c_pdata = {
        .sda_pin = GPIO_SENSORS_I2C_SDA,
        .sda_is_open_drain = 0,
        .scl_pin = GPIO_SENSORS_I2C_SCL,
        .scl_is_open_drain = 0,
        .udelay = 2,
};

static struct platform_device sensors_i2c_bus = {
        .name = "i2c-gpio",
        .id = I2C_BUS_NUM_SENSORS,
        .dev.platform_data = &sensors_i2c_pdata,
};

static struct i2c_board_info sensors_i2c_devices[] = {
	{
		I2C_BOARD_INFO("bma150", 0x38),
		.type = "bma150",
	},
	{
		I2C_BOARD_INFO("akm8973", 0x3C >> 1),
	},
	{
		I2C_BOARD_INFO("gp2ap002", 0x88 >> 1),
		.type = "gp2ap002",
		.irq = MSM_GPIO_TO_INT(GPIO_PROXIMITY_IRQ),
	},
};

/* AMP, sound & usb switch */
static struct i2c_gpio_platform_data amp_i2c_pdata = {
	.sda_pin	= GPIO_AMP_I2C_SDA,
        .sda_is_open_drain = 0,
	.scl_pin	= GPIO_AMP_I2C_SCL,
        .scl_is_open_drain = 0,
//	.udelay = 5,
};

static struct platform_device amp_i2c_bus = {
        .name = "i2c-gpio",
        .id = I2C_BUS_NUM_AMP,
        .dev.platform_data = &amp_i2c_pdata,
};

static void fsa9480_attached_callback(u8 attached) {
	pr_info("fsa9480_attached_callback: %d", attached);
}

static struct fsa9480_platform_data fsa9480_data = {
	.usb_cb = fsa9480_attached_callback,
};


static struct i2c_board_info amp_i2c_devices[] = {
	{
		I2C_BOARD_INFO("max9877", 0x9A >> 1),
	},
	{
		I2C_BOARD_INFO("fsa9480", 0x4A >> 1),
		.platform_data = &fsa9480_data,
	},
};

/* touchscreen */
static struct i2c_gpio_platform_data touch_i2c_pdata = {
	.sda_pin	= GPIO_TOUCH_I2C_SDA,
        .sda_is_open_drain = 0,
	.scl_pin	= GPIO_TOUCH_I2C_SCL,
        .scl_is_open_drain = 0,
	.udelay = 5,
};

static struct platform_device touch_i2c_bus = {
        .name = "i2c-gpio",
        .id = I2C_BUS_NUM_TOUCH,
        .dev.platform_data = &touch_i2c_pdata,
};

static struct i2c_board_info touch_i2c_device = {
	I2C_BOARD_INFO("melfas-tsi-ts", 0x20),
	.irq = MSM_GPIO_TO_INT(GPIO_TOUCH_IRQ),
};

/* camera */
static struct i2c_gpio_platform_data cam_i2c_pdata = {
	.sda_pin = GPIO_CAM_5M_I2C_SDA,
        .sda_is_open_drain = 0,
	.scl_pin = GPIO_CAM_5M_I2C_SCL,
        .scl_is_open_drain = 0,
//	.udelay = 5,
};

static struct platform_device cam_i2c_bus = {
	.name = "i2c-gpio",
	.id = I2C_BUS_NUM_CAMERA,
	.dev.platform_data = &cam_i2c_pdata,
};

static struct i2c_board_info cam_i2c_device = {
	I2C_BOARD_INFO("m4mo", 0x3F >> 1),
};

static struct i2c_gpio_platform_data cam_pm_i2c_pdata = {
	.sda_pin    = GPIO_CAM_PM_I2C_SDA,
        .sda_is_open_drain = 0,
	.scl_pin    = GPIO_CAM_PM_I2C_SCL,
        .scl_is_open_drain = 0,
};

static struct platform_device cam_pm_i2c_bus = {
	.name = "i2c-gpio",
	.id =  I2C_BUS_NUM_CAMERA_PM,
	.dev.platform_data = &cam_pm_i2c_pdata,
};

static struct i2c_board_info cam_pm_i2c_device = {
	I2C_BOARD_INFO("cam_pm_lp8720_i2c", 0x7D),
};


/* usb */
#define HSUSB_API_INIT_PHY_PROC	2
#define HSUSB_API_PROG		0x30000064
#define HSUSB_API_VERS MSM_RPC_VERS(1,1)

static void internal_phy_reset(void)
{
	struct msm_rpc_endpoint *usb_ep;
	int rc;
	struct hsusb_phy_start_req {
		struct rpc_request_hdr hdr;
	} req;

	printk(KERN_INFO "msm_hsusb_phy_reset\n");

	usb_ep = msm_rpc_connect(HSUSB_API_PROG, HSUSB_API_VERS, 0);
	if (IS_ERR(usb_ep)) {
		printk(KERN_ERR "%s: init rpc failed! error: %ld\n",
				__func__, PTR_ERR(usb_ep));
		goto close;
	}
	rc = msm_rpc_call(usb_ep, HSUSB_API_INIT_PHY_PROC,
			&req, sizeof(req), 5 * HZ);
	if (rc < 0)
		printk(KERN_ERR "%s: rpc call failed! (%d)\n", __func__, rc);

close:
	msm_rpc_close(usb_ep);
}

static void usb_connected(int state) {
	printk(KERN_INFO "%s: %d", __FUNCTION__, state);
}

static int galaxy_phy_init_seq[] = { 0x1D, 0x0D, 0x1D, 0x10, -1 };

static struct msm_hsusb_platform_data msm_hsusb_pdata = {
	.phy_init_seq = galaxy_phy_init_seq,
	.phy_reset = internal_phy_reset,
	.usb_connected = usb_connected,
};

static char *usb_functions_ums[] = {
	"usb_mass_storage",
};

static char *usb_functions_ums_adb[] = {
	"usb_mass_storage",
	"adb",
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
};

#ifdef CONFIG_USB_ANDROID_DIAG
static char *usb_functions_adb_diag[] = {
	"usb_mass_storage",
	"adb",
	"diag",
};
#endif

#ifdef CONFIG_USB_ANDROID_ACM
static char *usb_functions_adb_ums_acm[] = {
	"usb_mass_storage",
	"adb",
	"acm",
};
#endif

static char *usb_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif
	"usb_mass_storage",
	"adb",
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif
};

static struct android_usb_product usb_products[] = {
	{
		.product_id	= 0x6640,
		.num_functions	= ARRAY_SIZE(usb_functions_adb_ums_acm),
		.functions	= usb_functions_adb_ums_acm,
	},
	{
		.product_id	= 0x6601,
		.num_functions	= ARRAY_SIZE(usb_functions_ums_adb),
		.functions	= usb_functions_ums_adb,
	},
};

#ifdef CONFIG_USB_ANDROID_RNDIS
static struct usb_ether_platform_data rndis_pdata = {
	/* ethaddr is filled by board_serialno_setup */
	.vendorID	= 0x0bb4,
	.vendorDescr	= "HTC",
};

static struct platform_device rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};
#endif

static struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns = 1,
	.vendor = "Samsung",
	.product = "I7500",
	.release = 0x0100,
};

static struct platform_device usb_mass_storage_device = {
	.name = "usb_mass_storage",
	.id = -1,
	.dev = {
		.platform_data = &mass_storage_pdata,
	},
};

#ifdef CONFIG_USB_ANDROID_DIAG
struct usb_diag_platform_data usb_diag_pdata = {
  .ch_name = DIAG_LEGACY,
  //.update_pid_and_serial_num = usb_diag_update_pid_and_serial_num,
};

struct platform_device usb_diag_device = {
  .name = "usb_diag",
  .id = -1,
  .dev  = {
    .platform_data = &usb_diag_pdata,
  },
};
#endif

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id = 0x04E8,
	.product_id = 0x6640,
	.version = 0x0100,
	.serial_number = "I7500KzEpBWA",
	.product_name = "I7500",
	.manufacturer_name = "SAMSUNG",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
};

static struct platform_device android_usb_device = {
	.name = "android_usb",
	.id = -1,
	.dev = {
		.platform_data = &android_usb_pdata,
	},
};


static struct resource resources_hw3d[] = {
	{
		.start	= 0xA0000000,
		.end	= 0xA00fffff,
		.flags	= IORESOURCE_MEM,
		.name	= "regs",
	},
	{
		.flags	= IORESOURCE_MEM,
		.start  = MSM_PMEM_GPU0_BASE,
		.end    = MSM_PMEM_GPU0_BASE+MSM_PMEM_GPU0_SIZE-1,
		.name	= "smi",
	},
	{
		.flags	= IORESOURCE_MEM,
		.start  = MSM_PMEM_GPU1_BASE,
		.end    = MSM_PMEM_GPU1_BASE+MSM_PMEM_GPU1_SIZE-1,
		.name	= "ebi",
	},
	{
		.start	= INT_GRAPHICS,
		.end	= INT_GRAPHICS,
		.flags	= IORESOURCE_IRQ,
		.name	= "gfx",
	},
};

static struct platform_device hw3d_device = {
	.name		= "msm_hw3d",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_hw3d),
	.resource	= resources_hw3d,
};

#define SND(num, desc) { .name = desc, .id = num }
static struct snd_endpoint snd_endpoints_list[] = {
//	SND(0, "HANDSET"),
//	SND(1, "SPEAKER"),
//	SND(2, "HEADSET"),
//	SND(3, "BT"),
//	SND(44, "BT_EC_OFF"),
//	SND(10, "HEADSET_AND_SPEAKER"),
//	SND(256, "CURRENT"),
	SND(0, "HANDSET"),
	SND(2, "HEADSET"),
	SND(3, "STEREO_HEADSET"),
	SND(3, "HEADSET_AND_SPEAKER"),
	SND(6, "SPEAKER"),
	SND(26, "SPEAKER_MIDI"),
	SND(12 , "BT"),
	SND(16, "IN_S_SADC_OUT_HANDSET"),
	SND(25, "IN_S_SADC_OUT_SPEAKER_PHONE"),
	SND(28, "CURRENT"),

	/* Bluetooth accessories. */

	SND(12, "HTC BH S100"),
	SND(13, "HTC BH M100"),
	SND(14, "Motorola H500"),
	SND(15, "Nokia HS-36W"),
	SND(16, "PLT 510v.D"),
	SND(17, "M2500 by Plantronics"),
	SND(18, "Nokia HDW-3"),
	SND(19, "HBH-608"),
	SND(20, "HBH-DS970"),
	SND(21, "i.Tech BlueBAND"),
	SND(22, "Nokia BH-800"),
	SND(23, "Motorola H700"),
	SND(24, "HTC BH M200"),
	SND(25, "Jabra JX10"),
	SND(26, "320Plantronics"),
	SND(27, "640Plantronics"),
	SND(28, "Jabra BT500"),
	SND(29, "Motorola HT820"),
	SND(30, "HBH-IV840"),
	SND(31, "6XXPlantronics"),
	SND(32, "3XXPlantronics"),
	SND(33, "HBH-PV710"),
	SND(34, "Motorola H670"),
	SND(35, "HBM-300"),
	SND(36, "Nokia BH-208"),
	SND(37, "Samsung WEP410"),
	SND(38, "Jabra BT8010"),
	SND(39, "Motorola S9"),
	SND(40, "Jabra BT620s"),
	SND(41, "Nokia BH-902"),
	SND(42, "HBH-DS220"),
	SND(43, "HBH-DS980"),
};
#undef SND

static struct msm_snd_endpoints galaxy_snd_endpoints = {
	.endpoints = snd_endpoints_list,
	.num = ARRAY_SIZE(snd_endpoints_list),
};

static struct platform_device galaxy_snd = {
	.name = "msm_snd",
	.id = -1,
	.dev	= {
		.platform_data = &galaxy_snd_endpoints,
	},
};
void __init msm_add_usb_devices(void)
{
	msm_device_hsusb.dev.platform_data = &msm_hsusb_pdata;
	platform_device_register(&msm_device_hsusb);
	platform_device_register(&usb_mass_storage_device);
	platform_device_register(&android_usb_device);
#ifdef CONFIG_USB_ANDROID_RNDIS
	platform_device_register(&rndis_device);
#endif
#ifdef CONFIG_USB_ANDROID_DIAG
	platform_device_register(&usb_diag_device);
#endif
}

static uint32_t new_board_gpio_table[] = {
	PCOM_GPIO_CFG(GPIO_SEND_END,  0, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* SEND_END */
//	PCOM_GPIO_CFG(GPIO_FLASH_DETECT,  0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), /* T_FLASH_DET */
//	PCOM_GPIO_CFG(85,  0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* WLAN_BT_REG_ON */
//	PCOM_GPIO_CFG(109, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* BT_RESET */
};

static struct bcm_bt_lpm_platform_data bcm_bt_lpm_pdata = {
	.gpio_wake = GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_BT_HOST_WAKE,
	.request_clock_off_locked = msm_hs_request_clock_off_locked,
	.request_clock_on_locked = msm_hs_request_clock_on_locked,
};

struct platform_device bcm_bt_lpm_device = {
	.name = "bcm_bt_lpm",
	.id = 0,
	.dev = {
		.platform_data = &bcm_bt_lpm_pdata,
	},
};


#ifdef CONFIG_SERIAL_MSM_HS
static struct msm_serial_hs_platform_data msm_uart_dm1_pdata = {
	.rx_wakeup_irq = MSM_GPIO_TO_INT(45),
	.inject_rx_on_wakeup = 1,
	.rx_to_inject = 0x32,
};
#endif

static unsigned bt_config_uart[] = {
#ifdef CONFIG_SERIAL_MSM_HS
	PCOM_GPIO_CFG(43, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA),	/* RFR */
	PCOM_GPIO_CFG(44, 2, GPIO_INPUT,  GPIO_PULL_UP, GPIO_2MA),	/* CTS */
	PCOM_GPIO_CFG(45, 2, GPIO_INPUT,  GPIO_PULL_UP, GPIO_2MA),	/* Rx */
	PCOM_GPIO_CFG(46, 3, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA),	/* Tx */
#else
	PCOM_GPIO_CFG(43, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA), /* RTS */
	PCOM_GPIO_CFG(44, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA), /* CTS */
	PCOM_GPIO_CFG(45, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA), /* RX */
	PCOM_GPIO_CFG(46, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA), /* TX */
#endif
};

static uint32_t camera_off_gpio_table[] = {
	/* parallel CAMERA interfaces */
	PCOM_GPIO_CFG(4, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT4 */
	PCOM_GPIO_CFG(5, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT5 */
	PCOM_GPIO_CFG(6, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT6 */
	PCOM_GPIO_CFG(7, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT7 */
	PCOM_GPIO_CFG(8, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT8 */
	PCOM_GPIO_CFG(9, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* PCLK */
	PCOM_GPIO_CFG(13, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* HSYNC_IN */
	PCOM_GPIO_CFG(14, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* VSYNC_IN */
	PCOM_GPIO_CFG(15, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* MCLK */
	// PCOM_GPIO_CFG(23, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), // PGH CAM_FLASH_EN
	// PCOM_GPIO_CFG(31, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), // PGH CAM_FLASH_SET
	PCOM_GPIO_CFG(36, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	PCOM_GPIO_CFG(37, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	PCOM_GPIO_CFG(60, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	PCOM_GPIO_CFG(61, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
};

static uint32_t camera_on_gpio_table[] = {
	/* parallel CAMERA interfaces */
	PCOM_GPIO_CFG(4, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT4 */
	PCOM_GPIO_CFG(5, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT5 */
	PCOM_GPIO_CFG(6, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT6 */
	PCOM_GPIO_CFG(7, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT7 */
	PCOM_GPIO_CFG(8, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT8 */
	PCOM_GPIO_CFG(9, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_16MA), /* PCLK */
	PCOM_GPIO_CFG(13, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* HSYNC_IN */
	PCOM_GPIO_CFG(14, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* VSYNC_IN */

#if 1//PGH EDITED FOR CLOCK Stabilization 2009-05-23
	PCOM_GPIO_CFG(15, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* MCLK */
#else//ORG
	PCOM_GPIO_CFG(15, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_16MA), /* MCLK */
#endif//PGH

	// PCOM_GPIO_CFG(23, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), // PGH CAM_FLASH_EN
	// PCOM_GPIO_CFG(31, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), // PGH CAM_FLASH_SET

	PCOM_GPIO_CFG(36, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA),
	PCOM_GPIO_CFG(37, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA),

	PCOM_GPIO_CFG(60, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA),
	PCOM_GPIO_CFG(61, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA),
};

static struct platform_device *devices[] __initdata = {
	//&bcm_bt_lpm_device,
	// &msm_device_uart1,
	&ram_console_device,
	&msm_device_uart3,
#ifdef CONFIG_SERIAL_MSM_HS
	&msm_device_uart_dm1,
#endif

	&msm_device_smd,
	&msm_device_nand,

	&android_pmem_device,
	&android_pmem_adsp_device,
	&android_pmem_camera_device,

	&hw3d_device,
	&galaxy_snd,

	/* i2c */
	&msm_device_i2c,
	&sensors_i2c_bus,
	&amp_i2c_bus,
	&touch_i2c_bus,
	&cam_i2c_bus,
	&cam_pm_i2c_bus,

};

extern struct sys_timer msm_timer;

static struct msm_acpu_clock_platform_data galaxy_clock_data = {
	.acpu_switch_time_us = 50,
	.max_speed_delta_khz = 256000,
	.vdd_switch_time_us = 62,
	.power_collapse_khz = 19200000,
//	.wait_for_irq_khz = 128000000,
	.wait_for_irq_khz = 176000000,
};

static void galaxy_phy_reset(void)
{
	gpio_set_value(GPIO_USB_PHY_RST, 0);
	mdelay(10);
	gpio_set_value(GPIO_USB_PHY_RST, 1);
	mdelay(10);
	return;
}


static void bcm4325_init(void)	// added for bcm4325
{

	gpio_request(GPIO_BT_WAKE, "bt_wake");
	gpio_direction_output(GPIO_BT_WAKE, 0);       // BT_WAKE_N
	msleep(100);
	gpio_request(GPIO_WLAN_RESET, "wlan_reset");
	gpio_direction_output(GPIO_WLAN_RESET, 0);       // WLAN
	gpio_request(GPIO_BT_RESET, "bt_reset");
	gpio_direction_output(GPIO_BT_RESET, 0);       // BT
	gpio_request(GPIO_BT_REG_ON, "bt_reg_on");
	gpio_direction_output(GPIO_BT_REG_ON, 0);       // REG_ON
}

static void galaxy_reset(void)
{
	gpio_set_value(GPIO_PS_HOLD, 0);
}

static void __init config_gpios(void) {
	config_gpio_table(bt_config_uart, ARRAY_SIZE(bt_config_uart));
	config_gpio_table(camera_off_gpio_table, ARRAY_SIZE(camera_off_gpio_table));

	gpio_request(GPIO_TOUCH_IRQ, "touchscreen");
	gpio_direction_input(GPIO_TOUCH_IRQ);
}


static void __init galaxy_init(void)
{
	if (socinfo_init() < 0)
		BUG();

	config_gpios();

	msm_acpu_clock_init(&galaxy_clock_data);


	msm_hw_reset_hook = galaxy_reset;

	init_keypad();

	/* register i2c devices */
	/* each pair of SCL and SDA lines is one bus */
	i2c_register_board_info(I2C_BUS_NUM_SENSORS, sensors_i2c_devices, ARRAY_SIZE(sensors_i2c_devices));
	i2c_register_board_info(I2C_BUS_NUM_AMP, amp_i2c_devices, ARRAY_SIZE(amp_i2c_devices));
	i2c_register_board_info(I2C_BUS_NUM_TOUCH, &touch_i2c_device, 1);
	i2c_register_board_info(I2C_BUS_NUM_CAMERA_PM, &cam_pm_i2c_device, 1);
	i2c_register_board_info(I2C_BUS_NUM_CAMERA, &cam_i2c_device, 1);

	galaxy_init_mmc();
#ifdef CONFIG_SERIAL_MSM_HS
  msm_device_uart_dm1.dev.platform_data = &msm_uart_dm1_pdata;
#endif

	msm_add_usb_devices();
	//msm_fb_add_devices();

	msm_device_uart_dm1.dev.platform_data = NULL;

	config_gpio_table(new_board_gpio_table, ARRAY_SIZE(new_board_gpio_table));

	platform_add_devices(devices, ARRAY_SIZE(devices));

	bcm4325_init();
}

static void __init galaxy_fixup(struct machine_desc *desc, struct tag *tags,
				 char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 1;
	mi->bank[0].start = PHYS_OFFSET;
	mi->bank[0].node = PHYS_TO_NID(PHYS_OFFSET);
	mi->bank[0].size = MSM_LINUX_SIZE;
}

//static struct map_desc galaxy_io_desc[] __initdata = {
//	{
//		.virtual = (unsigned long) MSM_WEB_BASE,
//		.pfn = __phys_to_pfn(MSM_WEB_PHYS),
//		.length = MSM_WEB_SIZE,
//		.type = MT_DEVICE_NONSHARED,
//	},
//};

static void __init galaxy_map_io(void)
{
	msm_map_common_io();
//	iotable_init(galaxy_io_desc, ARRAY_SIZE(galaxy_io_desc));
	msm_clock_init(msm_clocks_7x01a, msm_num_clocks_7x01a);
}

MACHINE_START(GALAXY, "galaxy")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= 0x10000100,
	.fixup		= galaxy_fixup,
	.map_io		= galaxy_map_io,
	.init_irq	= msm_init_irq,
	.init_machine	= galaxy_init,
	.timer		= &msm_timer,
MACHINE_END
