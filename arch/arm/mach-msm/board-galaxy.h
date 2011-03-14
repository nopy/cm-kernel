
#ifndef __ARCH_ARM_MACH_MSM_BOARD_GALAXY_H
#define __ARCH_ARM_MACH_MSM_BOARD_GALAXY_H

// SMI Bank

#define MSM_SMI_BASE			0x00100000
#define MSM_SMI_SIZE			0x00800000

#define MSM_PMEM_GPU0_BASE		MSM_SMI_BASE
#define MSM_PMEM_GPU0_SIZE		(7*SZ_1M)

#define MSM_FB_BASE			0x00800000
#define MSM_FB_SIZE			0x96000 // 320x480

#define MSM_RAM_CONSOLE_BASE		(MSM_FB_BASE + MSM_FB_SIZE)
#define MSM_RAM_CONSOLE_SIZE		(1*SZ_1M - MSM_FB_SIZE)

// SMI2 Bank

#define MSM_SMI2_BASE			0x02000000

#define MSM_PMEM_MDP_BASE		MSM_SMI2_BASE
#define MSM_PMEM_MDP_SIZE		(8*SZ_1M)

#define MSM_PMEM_ADSP_BASE 		(MSM_PMEM_MDP_BASE + MSM_PMEM_MDP_SIZE)
#define MSM_PMEM_ADSP_SIZE		(13*SZ_1M)

#define MSM_PMEM_CAMERA_BASE		(MSM_PMEM_ADSP_BASE + MSM_PMEM_ADSP_SIZE)
#define MSM_PMEM_CAMERA_SIZE		(11*SZ_1M)

// EBI Bank

#define MSM_EBI_BASE			0x10000000
#define MSM_EBI_SIZE			0x06D00000

#define MSM_LINUX_BASE			MSM_EBI_BASE
#define MSM_LINUX_SIZE			(109*SZ_1M - MSM_PMEM_GPU1_SIZE)

#define MSM_PMEM_GPU1_BASE		(MSM_LINUX_BASE + MSM_LINUX_SIZE)
#define MSM_PMEM_GPU1_SIZE 		(8*SZ_1M)

#define MSM_KERNEL_PANIC_DUMP_SIZE	0x8000 /* 32kbytes */
#define MSM_PMEM_KERNEL_EBI1_SIZE	0x200000

struct msm_pmem_setting{
	resource_size_t pmem_start;
	resource_size_t pmem_size;
	resource_size_t pmem_adsp_start;
	resource_size_t pmem_adsp_size;
	resource_size_t pmem_gpu0_start;
	resource_size_t pmem_gpu0_size;
	resource_size_t pmem_gpu1_start;
	resource_size_t pmem_gpu1_size;
	resource_size_t pmem_camera_start;
	resource_size_t pmem_camera_size;
	resource_size_t pmem_kernel_ebi1_start;
	resource_size_t pmem_kernel_ebi1_size;
	resource_size_t ram_console_start;
	resource_size_t ram_console_size;
};

#define GPIO_FLASH_DETECT	20
#define GPIO_LCD_VSYNC		97
#define GPIO_PS_HOLD		25
#define GPIO_USB_PHY_RST	149

/* I2C Bus Num */
#define I2C_BUS_NUM_SENSORS		1
#define I2C_BUS_NUM_AMP			2
#define I2C_BUS_NUM_TOUCH		5
#define I2C_BUS_NUM_CAMERA_PM		4
#define I2C_BUS_NUM_CAMERA		3

/*
#define I2C_BUS_NUM_BACKLIGHT		2
#define I2C_BUS_NUM_CAMERA		3
#define I2C_BUS_NUM_PROX		6
#define I2C_BUS_NUM_COMPASS		7
*/

/* Sensors */
#define GPIO_SENSORS_I2C_SDA		3
#define GPIO_SENSORS_I2C_SCL		2
#define GPIO_PROXIMITY_IRQ		57

/* Amp */
#define GPIO_AMP_I2C_SDA		83
#define GPIO_AMP_I2C_SCL		82

/* Touchscreen */
#define GPIO_TOUCH_I2C_SDA		30
#define GPIO_TOUCH_I2C_SCL		29
#define GPIO_TOUCH_IRQ			19

/* camera */
#define GPIO_CAM_PM_I2C_SDA		37
#define GPIO_CAM_PM_I2C_SCL		36
#define GPIO_CAM_5M_I2C_SDA		61
#define GPIO_CAM_5M_I2C_SCL		60

/* bluetooth */
#define GPIO_BT_REG_ON	 		85
#define GPIO_BT_WAKE			77
#define GPIO_BT_HOST_WAKE		94
#define GPIO_BT_RESET 			109

#define GPIO_BT_UART_RTS		43
#define GPIO_BT_UART_CTS		44
#define GPIO_BT_UART_RXD		45
#define GPIO_BT_UART_TXD		46

#define GPIO_BT_PCM_DOUT		68
#define GPIO_BT_PCM_DIN			69
#define GPIO_BT_PCM_SYNC		70
#define GPIO_BT_PCM_CLK			71

/* wifi */
#define GPIO_WLAN_RESET			81
#define GPIO_WLAN_HOST_WAKE		28
#define GALAXY_GPIO_WIFI_IRQ		GPIO_WLAN_HOST_WAKE

/* USB */
#define GPIO_USB_DET			49


#define GPIO_CHK_BOARD_REV 		99


#define GPIO_SEND_END 			38



#define KEY_CAMERA_AF			247

#endif //__ARCH_ARM_MACH_MSM_BOARD_GALAXY_H
