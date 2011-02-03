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
//	.start = MSM_PMEM_MDP_BASE,
//	.size = MSM_PMEM_MDP_SIZE,
	.no_allocator = 0,
	.cached = 1,
};

static struct android_pmem_platform_data android_pmem_gpu0_pdata = {
	.name = "pmem_gpu0",
	.start = MSM_PMEM_GPU0_BASE,
	.size = MSM_PMEM_GPU0_SIZE,
	.no_allocator = 1,
	.cached = 0,
};

static struct android_pmem_platform_data android_pmem_gpu1_pdata = {
	.name = "pmem_gpu1",
	.start = MSM_PMEM_GPU1_BASE,
	.size = MSM_PMEM_GPU1_SIZE,
	.no_allocator = 1,
	.cached = 0,
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

static struct platform_device android_pmem_gpu0_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_gpu0_pdata },
};

static struct platform_device android_pmem_gpu1_device = {
	.name = "android_pmem",
	.id = 3,
	.dev = { .platform_data = &android_pmem_gpu1_pdata },
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
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device ram_console_device = {
	.name = "ram_console",
	.id = -1,
	.num_resources  = ARRAY_SIZE(ram_console_resource),
	.resource       = ram_console_resource,
};


static int galaxy_phy_init_seq[] = { 0x1D, 0x0D, 0x1D, 0x10, -1 };

static struct msm_hsusb_platform_data msm_hsusb_pdata = {
	.phy_init_seq = galaxy_phy_init_seq,
};

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

static char *usb_functions[] = { "usb_mass_storage", "adb" };
static char *usb_functions_adb[] = { "usb_mass_storage", "adb" };

static struct android_usb_product usb_products[] = {
	{
		.product_id	= 0x6601,
		.num_functions	= ARRAY_SIZE(usb_functions),
		.functions	= usb_functions,
	},
	{
		.product_id	= 0x6640,
		.num_functions	= ARRAY_SIZE(usb_functions_adb),
		.functions	= usb_functions_adb,
	},
};

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id = 0x04E8,
	.product_id = 0x6640,
	.version = 0x0100,
	.serial_number = "42",
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

	if (setting->ram_console_size) {
		ram_console_resource[0].start = setting->ram_console_start;
		ram_console_resource[0].end = setting->ram_console_start
			+ setting->ram_console_size - 1;
		platform_device_register(&ram_console_device);
	}
}

static void __init msm_fb_add_devices(void)
{
//	msm_fb_register_device("mdp", &mdp_pdata);
//	msm_fb_register_device("pmdh", &mddi_pdata);
}


static struct platform_device *devices[] __initdata = {
	&msm_device_uart3,
	&msm_device_smd,
	&msm_device_nand,
	&msm_device_hsusb,
	&msm_device_i2c,
	&msm_fb_device,
	&usb_mass_storage_device,
	&android_usb_device,
	&hw3d_device,
	//&msm_device_i2c,
	//&fish_battery_device,
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
	msm_add_mem_devices(&pmem_settings);

	msm_device_hsusb.dev.platform_data = &msm_hsusb_pdata;
	msm_acpu_clock_init(&galaxy_clock_data);

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

	size = MSM_PMEM_GPU1_SIZE;
	addr = alloc_bootmem(size);//, 0x100000);
	android_pmem_gpu1_pdata.start = __pa(addr);
	android_pmem_gpu1_pdata.size = size;
	printk(KERN_INFO "allocating %lu bytes at %p (%lx physical)"
	       "for gpu1 pmem\n", size, addr, __pa(addr));
}

static void __init galaxy_map_io(void)
{
	msm_map_common_io();
	msm_allocate_memory_regions();
	msm_clock_init(msm_clocks_7x01a, msm_num_clocks_7x01a);
}

MACHINE_START(GALAXY, "galaxy-maldn-google-32")
	.boot_params	= 0x10000100,
	//.fixup		= galaxy_fixup,
	.map_io		= galaxy_map_io,
	.init_irq	= msm_init_irq,
	.init_machine	= galaxy_init,
	.timer		= &msm_timer,
MACHINE_END
