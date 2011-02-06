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

#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c.h>
#include <linux/android_pmem.h>
#include <linux/usb/android_composite.h>

#include "devices.h"
#include "board-galaxy.h"
#include "proc_comm.h"

extern int galaxy_init_mmc(void);

static struct msm_pmem_setting pmem_settings = {
	.pmem_start = MSM_PMEM_MDP_BASE,
	.pmem_size = MSM_PMEM_MDP_SIZE,
	.pmem_adsp_start = MSM_PMEM_ADSP_BASE,
	.pmem_adsp_size = MSM_PMEM_ADSP_SIZE,
	.pmem_gpu0_start = MSM_PMEM_GPU0_BASE,
	.pmem_gpu0_size = MSM_PMEM_GPU0_SIZE,
	.pmem_gpu1_start = 0,	// will be allocated
	.pmem_gpu1_size = MSM_PMEM_GPU1_SIZE,
	.pmem_camera_start = MSM_PMEM_CAMERA_BASE,
	.pmem_camera_size = MSM_PMEM_CAMERA_SIZE,
	.pmem_kernel_ebi1_start = 0, // will be allocated
	.pmem_kernel_ebi1_size = MSM_PMEM_KERNEL_EBI1_SIZE,
	.ram_console_start = 0,
	.ram_console_size = 0,
};

struct android_pmem_platform_data android_pmem_kernel_ebi1_pdata = {
	.name = "pmem_kernel_ebi1",
	/* if no allocator_type, defaults to PMEM_ALLOCATORTYPE_BITMAP,
	 * the only valid choice at this time. The board structure is
	 * set to all zeros by the C runtime initialization and that is now
	 * the enum value of PMEM_ALLOCATORTYPE_BITMAP, now forced to 0 in
	 * include/linux/android_pmem.h.
	 */
	.cached = 0,
};

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.start = MSM_PMEM_MDP_BASE,
	.size = MSM_PMEM_MDP_SIZE,
	.no_allocator = 0,
	.cached = 1,
};

static struct android_pmem_platform_data android_pmem_camera_pdata = {
    .name = "pmem_camera",
    .cached = 1,
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
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

static struct platform_device android_pmem_kernel_ebi1_device = {
	.name = "android_pmem",
	.id = 5,
	.dev = { .platform_data = &android_pmem_kernel_ebi1_pdata },
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

static struct i2c_board_info amp_i2c_devices[] = {
	{
		I2C_BOARD_INFO("max9877", 0x9A >> 1),
	},
	{
		I2C_BOARD_INFO("fsa9480", 0x4A >> 1),
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


static void hsusb_gpio_init(void)
{
	if (gpio_request(111, "ulpi_data_0"))
		pr_err("failed to request gpio ulpi_data_0\n");
	if (gpio_request(112, "ulpi_data_1"))
		pr_err("failed to request gpio ulpi_data_1\n");
	if (gpio_request(113, "ulpi_data_2"))
		pr_err("failed to request gpio ulpi_data_2\n");
	if (gpio_request(114, "ulpi_data_3"))
		pr_err("failed to request gpio ulpi_data_3\n");
	if (gpio_request(115, "ulpi_data_4"))
		pr_err("failed to request gpio ulpi_data_4\n");
	if (gpio_request(116, "ulpi_data_5"))
		pr_err("failed to request gpio ulpi_data_5\n");
	if (gpio_request(117, "ulpi_data_6"))
		pr_err("failed to request gpio ulpi_data_6\n");
	if (gpio_request(118, "ulpi_data_7"))
		pr_err("failed to request gpio ulpi_data_7\n");
	if (gpio_request(119, "ulpi_dir"))
		pr_err("failed to request gpio ulpi_dir\n");
	if (gpio_request(120, "ulpi_next"))
		pr_err("failed to request gpio ulpi_next\n");
	if (gpio_request(121, "ulpi_stop"))
		pr_err("failed to request gpio ulpi_stop\n");
}

static unsigned usb_gpio_lpm_config[] = {
	GPIO_CFG(111, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 0 */
	GPIO_CFG(112, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 1 */
	GPIO_CFG(113, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 2 */
	GPIO_CFG(114, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 3 */
	GPIO_CFG(115, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 4 */
	GPIO_CFG(116, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 5 */
	GPIO_CFG(117, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 6 */
	GPIO_CFG(118, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 7 */
	GPIO_CFG(119, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DIR */
	GPIO_CFG(120, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* NEXT */
	GPIO_CFG(121, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),	/* STOP */
};

static unsigned usb_gpio_lpm_unconfig[] = {
	GPIO_CFG(111, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 0 */
	GPIO_CFG(112, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 1 */
	GPIO_CFG(113, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 2 */
	GPIO_CFG(114, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 3 */
	GPIO_CFG(115, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 4 */
	GPIO_CFG(116, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 5 */
	GPIO_CFG(117, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 6 */
	GPIO_CFG(118, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 7 */
	GPIO_CFG(119, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DIR */
	GPIO_CFG(120, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* NEXT */
	GPIO_CFG(121, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), /* STOP */
};

static int usb_config_gpio(int config)
{
	int pin, rc;

	if (config) {
		for (pin = 0; pin < ARRAY_SIZE(usb_gpio_lpm_config); pin++) {
			rc = gpio_tlmm_config(usb_gpio_lpm_config[pin],
					      GPIO_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, usb_gpio_lpm_config[pin], rc);
				return -EIO;
			}
		}
	} else {
		for (pin = 0; pin < ARRAY_SIZE(usb_gpio_lpm_unconfig); pin++) {
			rc = gpio_tlmm_config(usb_gpio_lpm_unconfig[pin],
					      GPIO_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, usb_gpio_lpm_config[pin], rc);
				return -EIO;
			}
		}
	}

	return 0;
}
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

static int galaxy_phy_init_seq[] = { 0x1D, 0x0D, 0x1D, 0x10, -1 };

static struct msm_hsusb_platform_data msm_hsusb_pdata = {
	.phy_init_seq = galaxy_phy_init_seq,
	.phy_reset = internal_phy_reset,
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
	.vendor = "Qualcomm",
	.product = "Halibut",
	.release = 0x0100,
};

static struct platform_device usb_mass_storage_device = {
	.name = "usb_mass_storage",
	.id = -1,
	.dev = {
		.platform_data = &mass_storage_pdata,
	},
};

static char *usb_functions[] = { "usb_mass_storage",};
static char *usb_functions_adb[] = { "usb_mass_storage", "adb", };

static struct android_usb_product usb_products[] = {
	{
		.product_id	= 0x6640,
		.num_functions	= ARRAY_SIZE(usb_functions_adb),
		.functions	= usb_functions_adb,
	},
	{
		.product_id	= 0x6601,
		.num_functions	= ARRAY_SIZE(usb_functions),
		.functions	= usb_functions,
	},
};

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id = 0x04E8,
	.product_id = 0x6640,
	.version = 0x0100,
	.serial_number = "I7500KzEpBWA",
	.product_name = "I7500",
	.manufacturer_name = "SAMSUNG",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_adb),
	.functions = usb_functions_adb,
};

static struct platform_device android_usb_device = {
	.name = "android_usb",
	.id = -1,
	.dev = {
		.platform_data = &android_usb_pdata,
	},
};


//static struct msm_panel_common_pdata mdp_pdata = {
//	.gpio = 97,
//};
//
//static struct mddi_platform_data mddi_pdata = {
//    .mddi_power_save = msm_fb_mddi_power_save,
//};

static struct resource msm_fb_resources[] = {
	{
		.start = MSM_FB_BASE,
		.end = MSM_FB_BASE + MSM_FB_SIZE - 1,
		.flags  = IORESOURCE_DMA,
	}
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
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
//		.start  = MSM_PMEM_GPU1_BASE,
//		.end    = MSM_PMEM_GPU1_BASE+MSM_PMEM_GPU1_SIZE-1,
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

void __init msm_add_mem_devices(struct msm_pmem_setting *setting)
{
	if (setting->pmem_size) {
		android_pmem_pdata.start = setting->pmem_start;
		android_pmem_pdata.size = setting->pmem_size;
		platform_device_register(&android_pmem_device);
	}

	if (setting->pmem_adsp_size) {
		android_pmem_adsp_pdata.start = setting->pmem_adsp_start;
		android_pmem_adsp_pdata.size = setting->pmem_adsp_size;
		platform_device_register(&android_pmem_adsp_device);
	}

//	if (setting->pmem_gpu0_size && setting->pmem_gpu1_size) {
//		struct resource *res;
//
//		res = platform_get_resource_byname(&hw3d_device, IORESOURCE_MEM,
//						   "smi");
//		res->start = setting->pmem_gpu0_start;
//		res->end = res->start + setting->pmem_gpu0_size - 1;
//
//		res = platform_get_resource_byname(&hw3d_device, IORESOURCE_MEM,
//						   "ebi");
//		res->start = android_pmem_gpu1_pdata.start;
//		res->end = res->start + setting->pmem_gpu1_size - 1;
//		platform_device_register(&hw3d_device);
//	}


	if (setting->pmem_camera_size) {
		android_pmem_camera_pdata.start = setting->pmem_camera_start;
		android_pmem_camera_pdata.size = setting->pmem_camera_size;
		platform_device_register(&android_pmem_camera_device);
	}

	if (setting->pmem_kernel_ebi1_size) {
		android_pmem_kernel_ebi1_pdata.start = setting->pmem_kernel_ebi1_start;
		android_pmem_kernel_ebi1_pdata.size = setting->pmem_kernel_ebi1_size;
		platform_device_register(&android_pmem_kernel_ebi1_device);
	}

//	if (setting->ram_console_size) {
//		ram_console_resource[0].start = setting->ram_console_start;
//		ram_console_resource[0].end = setting->ram_console_start
//			+ setting->ram_console_size - 1;
//		platform_device_register(&ram_console_device);
//	}
//	platform_device_register(&ram_console_device);
}

static void __init msm_fb_add_devices(void)
{
//	msm_fb_register_device("mdp", &mdp_pdata);
//	msm_fb_register_device("pmdh", &mddi_pdata);
}

void __init msm_add_usb_devices(void (*phy_reset) (void))
{
	/* setup */
	hsusb_gpio_init();
	usb_config_gpio(1);

	if (phy_reset)
		msm_hsusb_pdata.phy_reset = phy_reset;
	msm_device_hsusb.dev.platform_data = &msm_hsusb_pdata;
	platform_device_register(&msm_device_hsusb);
#ifdef CONFIG_USB_ANDROID_RNDIS
	platform_device_register(&rndis_device);
#endif
	platform_device_register(&usb_mass_storage_device);
	platform_device_register(&android_usb_device);
}

static struct platform_device *devices[] __initdata = {
	&msm_device_uart3,
	&msm_device_smd,
	&msm_device_nand,
	&msm_device_i2c,
	&msm_fb_device,
	//&usb_mass_storage_device,
	&hw3d_device,
	&ram_console_device,
	//&msm_device_i2c,
	//&fish_battery_device,

	/* i2c */
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
	.wait_for_irq_khz = 128000000,
};

static void galaxy_phy_reset(void)
{
	return;
}

static void __init galaxy_init(void)
{
	msm_acpu_clock_init(&galaxy_clock_data);

	msm_add_mem_devices(&pmem_settings);

	/* register i2c devices */
	/* each pair of SCL and SDA lines is one bus */
	i2c_register_board_info(I2C_BUS_NUM_SENSORS, sensors_i2c_devices, ARRAY_SIZE(sensors_i2c_devices));
	i2c_register_board_info(I2C_BUS_NUM_AMP, amp_i2c_devices, ARRAY_SIZE(amp_i2c_devices));
	i2c_register_board_info(I2C_BUS_NUM_TOUCH, &touch_i2c_device, 1);
	i2c_register_board_info(I2C_BUS_NUM_CAMERA_PM, &cam_pm_i2c_device, 1);
	i2c_register_board_info(I2C_BUS_NUM_CAMERA, &cam_i2c_device, 1);

	galaxy_init_mmc();

	msm_add_usb_devices(galaxy_phy_reset);

	platform_add_devices(devices, ARRAY_SIZE(devices));

	msm_fb_add_devices();
	msm_hsusb_set_vbus_state(1);
}

static void __init galaxy_fixup(struct machine_desc *desc, struct tag *tags,
				 char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 1;
	mi->bank[0].start = PHYS_OFFSET;
	mi->bank[0].node = PHYS_TO_NID(PHYS_OFFSET);
	mi->bank[0].size = MSM_LINUX_SIZE;
}

static void __init msm_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

//	size = MSM_KERNEL_PANIC_DUMP_SIZE;
//	addr = alloc_bootmem(size);
//	MSM_KERNEL_PANIC_DUMP_ADDR = addr;

	size = MSM_PMEM_KERNEL_EBI1_SIZE;
	addr = alloc_bootmem(size);//, 0x100000);
	android_pmem_kernel_ebi1_pdata.start = __pa(addr);
	android_pmem_kernel_ebi1_pdata.size = size;
	pr_info("allocating %lu bytes at %p (%lx physical) for kernel"
		" ebi1 pmem arena\n", size, addr, __pa(addr));

//	size = MSM_PMEM_GPU1_SIZE;
//	addr = alloc_bootmem(size);//, 0x100000);
//	android_pmem_gpu1_pdata.start = __pa(addr);
//	android_pmem_gpu1_pdata.size = size;
//	printk(KERN_INFO "allocating %lu bytes at %p (%lx physical)"
//	       "for gpu1 pmem\n", size, addr, __pa(addr));
}

static void __init galaxy_map_io(void)
{
	msm_map_common_io();
	msm_allocate_memory_regions();
	msm_clock_init(msm_clocks_7x01a, msm_num_clocks_7x01a);
}

MACHINE_START(GALAXY, "galaxy")
	.boot_params	= 0x10000100,
	//.fixup		= galaxy_fixup,
	.map_io		= galaxy_map_io,
	.init_irq	= msm_init_irq,
	.init_machine	= galaxy_init,
	.timer		= &msm_timer,
MACHINE_END
