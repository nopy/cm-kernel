/* linux/arch/arm/mach-msm/board-galaxy-panel.c
**
** Support for toshiba mddi and smd oled client devices which require no
** special initialization code.
**
** Copyright (C) 2008 Samsung Electronics Incorporated
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/delay.h>

#include <mach/msm_fb.h>
#include <mach/gpio.h>
#include <mach/vreg.h>

#include <mach/msm_fb.h>
#include <asm/mach-types.h>

#include "board-galaxy.h"
#include "devices.h"

#define _DEBUG 0
#ifdef _DEBUG
#define dprintk(s, args...) printk("galaxy_panel: %s:%d - " s, __func__, __LINE__,  ##args)
#else
#define dprintk(s, args...)
#endif  /* _DEBUG */

// max level of backlight send by android
#define MAX_BACKLIGHT_BRIGHTNESS 255

#define SMD_AUTO_POWER_CMD
#define SMD_VYSNC_50HZ_CMD
//#define SMD_OLD_GAMMA_CMD

#define MDDI_CLIENT_CORE_BASE  0x108000
#define LCD_CONTROL_BLOCK_BASE 0x110000
#define SPI_BLOCK_BASE         0x120000
#define PWM_BLOCK_BASE         0x140000
#define GPIO_BLOCK_BASE        0x150000
#define SYSTEM_BLOCK1_BASE     0x160000
#define SYSTEM_BLOCK2_BASE     0x170000

#define TTBUSSEL    (MDDI_CLIENT_CORE_BASE|0x18)
#define DPSET0      (MDDI_CLIENT_CORE_BASE|0x1C)
#define DPSET1      (MDDI_CLIENT_CORE_BASE|0x20)
#define DPSUS       (MDDI_CLIENT_CORE_BASE|0x24)
#define DPRUN       (MDDI_CLIENT_CORE_BASE|0x28)
#define SYSCKENA    (MDDI_CLIENT_CORE_BASE|0x2C)

#define BITMAP0     (MDDI_CLIENT_CORE_BASE|0x44)
#define BITMAP1     (MDDI_CLIENT_CORE_BASE|0x48)
#define BITMAP2     (MDDI_CLIENT_CORE_BASE|0x4C)
#define BITMAP3     (MDDI_CLIENT_CORE_BASE|0x50)
#define BITMAP4     (MDDI_CLIENT_CORE_BASE|0x54)

#define SRST        (LCD_CONTROL_BLOCK_BASE|0x00)
#define PORT_ENB    (LCD_CONTROL_BLOCK_BASE|0x04)
#define START       (LCD_CONTROL_BLOCK_BASE|0x08)
#define PORT        (LCD_CONTROL_BLOCK_BASE|0x0C)

#define INTFLG      (LCD_CONTROL_BLOCK_BASE|0x18)
#define INTMSK      (LCD_CONTROL_BLOCK_BASE|0x1C)
#define MPLFBUF     (LCD_CONTROL_BLOCK_BASE|0x20)

#define PXL         (LCD_CONTROL_BLOCK_BASE|0x30)
#define HCYCLE      (LCD_CONTROL_BLOCK_BASE|0x34)
#define HSW         (LCD_CONTROL_BLOCK_BASE|0x38)
#define HDE_START   (LCD_CONTROL_BLOCK_BASE|0x3C)
#define HDE_SIZE    (LCD_CONTROL_BLOCK_BASE|0x40)
#define VCYCLE      (LCD_CONTROL_BLOCK_BASE|0x44)
#define VSW         (LCD_CONTROL_BLOCK_BASE|0x48)
#define VDE_START   (LCD_CONTROL_BLOCK_BASE|0x4C)
#define VDE_SIZE    (LCD_CONTROL_BLOCK_BASE|0x50)
#define WAKEUP      (LCD_CONTROL_BLOCK_BASE|0x54)
#define REGENB      (LCD_CONTROL_BLOCK_BASE|0x5C)
#define VSYNIF      (LCD_CONTROL_BLOCK_BASE|0x60)
#define WRSTB       (LCD_CONTROL_BLOCK_BASE|0x64)
#define RDSTB       (LCD_CONTROL_BLOCK_BASE|0x68)
#define ASY_DATA    (LCD_CONTROL_BLOCK_BASE|0x6C)
#define ASY_DATB    (LCD_CONTROL_BLOCK_BASE|0x70)
#define ASY_DATC    (LCD_CONTROL_BLOCK_BASE|0x74)
#define ASY_DATD    (LCD_CONTROL_BLOCK_BASE|0x78)
#define ASY_DATE    (LCD_CONTROL_BLOCK_BASE|0x7C)
#define ASY_DATF    (LCD_CONTROL_BLOCK_BASE|0x80)
#define ASY_DATG    (LCD_CONTROL_BLOCK_BASE|0x84)
#define ASY_DATH    (LCD_CONTROL_BLOCK_BASE|0x88)
#define ASY_CMDSET  (LCD_CONTROL_BLOCK_BASE|0x8C)
#define MONI        (LCD_CONTROL_BLOCK_BASE|0xB0)
#define VPOS        (LCD_CONTROL_BLOCK_BASE|0xC0)

#define SSICTL      (SPI_BLOCK_BASE|0x00)
#define SSITIME     (SPI_BLOCK_BASE|0x04)
#define SSITX       (SPI_BLOCK_BASE|0x08)
#define SSIINTS     (SPI_BLOCK_BASE|0x14)

#define TIMER0LOAD    (PWM_BLOCK_BASE|0x00)
#define TIMER0CTRL    (PWM_BLOCK_BASE|0x08)
#define PWM0OFF       (PWM_BLOCK_BASE|0x1C)
#define TIMER1LOAD    (PWM_BLOCK_BASE|0x20)
#define TIMER1CTRL    (PWM_BLOCK_BASE|0x28)
#define PWM1OFF       (PWM_BLOCK_BASE|0x3C)
#define TIMER2LOAD    (PWM_BLOCK_BASE|0x40)
#define TIMER2CTRL    (PWM_BLOCK_BASE|0x48)
#define PWM2OFF       (PWM_BLOCK_BASE|0x5C)
#define PWMCR         (PWM_BLOCK_BASE|0x68)

#define GPIODATA    (GPIO_BLOCK_BASE|0x00)
#define GPIODIR     (GPIO_BLOCK_BASE|0x04)
#define GPIOIS      (GPIO_BLOCK_BASE|0x08)
#define GPIOIEV     (GPIO_BLOCK_BASE|0x10)
#define GPIOIC      (GPIO_BLOCK_BASE|0x20)
#define GPIOPC      (GPIO_BLOCK_BASE|0x28)

#define WKREQ       (SYSTEM_BLOCK1_BASE|0x00)
#define CLKENB      (SYSTEM_BLOCK1_BASE|0x04)
#define DRAMPWR     (SYSTEM_BLOCK1_BASE|0x08)
#define INTMASK     (SYSTEM_BLOCK1_BASE|0x0C)
#define CNT_DIS     (SYSTEM_BLOCK1_BASE|0x10)
#define GPIOSEL     (SYSTEM_BLOCK2_BASE|0x00)

#define HVGA_XRES	320
#define HVGA_YRES	480

#define GALAXY_DEFAULT_BACKLIGHT_BRIGHTNESS 255

struct mddi_table {
	uint32_t reg;
	unsigned value[4];
	uint32_t val_len;
};

static struct mddi_table toshiba_mddi_init_table[] = 
{
#ifdef SMD_VYSNC_50HZ_CMD  // change -> 57hz -> 50hz (flicker)
	{ DPSET0,   {0x0A990050},   1   }, 
	{ DPSET1,   {0x00000116},   1   }, 
#else
	{ DPSET0,	{0x4CB60075},	1 	},
	{ DPSET1,	{0x00000112},	1	},
#endif
	{ DPSUS,	{0x00000000},	1 	},
	{ DPRUN,	{0x00000001},	1 	},
	{ 1,		{0x00000000},	500	},
	{ SYSCKENA,	{0x00000001},	1 	},
	{ CLKENB,	{0x000021EF},	1	},
	{ GPIODATA,	{0x03FF0000},	1 	},
	{ GPIODIR,	{0x00000008},	1 	},
	{ GPIOSEL,	{0x00000000},	1 	},
	{ GPIOPC,	{0x00080000},	1 	},
	{ WKREQ,	{0x00000000},	1 	},
	{ GPIODATA,	{0x00080000},	1 	},
	{ 1,		{0x00000000},	10	},
	{ GPIODATA,	{0x00080008},	1 	},
	{ 1,		{0x00000000},	10	},
	{ GPIODATA,	{0x00080000},	1 	},
	{ 1,		{0x00000000},	15	},
	{ GPIODATA,	{0x00080008},	1 	},
	{ DRAMPWR,	{0x00000001},	1 	},
	{ CLKENB,	{0x000021EF},	1 	},
	{ 1,		{0x00000000},	15	},
	{ SSICTL,	{0x00000170},	1 	},
	{ SSITIME,	{0x00000101},	1 	},
	{ CNT_DIS,	{0x00000000},	1 	},
	{ SSICTL,	{0x00000173},	1 	},
	{ BITMAP0,	{0x01E00140},	1 	},
	{ PORT_ENB,	{0x00000001},	1 	},
	{ PORT,		{0x00000008},	1 	},
	{ PXL,		{0x0000003A},	1 	},
	{ MPLFBUF,	{0x00000000},	1 	},
	{ HCYCLE,	{0x000000DF},	1 	},
	{ HSW,		{0x00000000},	1 	},
	{ HDE_START,{0x0000001F},	1 	},
	{ HDE_SIZE,	{0x0000009F},	1 	},
	{ VCYCLE,	{0x000001EF},	1 	},
	{ VSW,		{0x00000001},	1 	},
	{ VDE_START,{0x00000007},	1 	},
	{ VDE_SIZE,	{0x000001DF},	1 	},
	{ CNT_DIS,	{0x00000000},	1 	},
	{ START,	{0x00000001},	1 	},
	{ 1,		{0x00000000},	10	},
};
static struct mddi_table smd_oled_init_table[] = 
{
// Power Setting Sequence - 1 (Analog Setting)
	{ SSITX,	{0x00010100},	1 	},
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00012122},	1	}, 
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00012208}, 	1	},  //   # SPI.SSITX
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00012300}, 	1	},  //   # SPI.SSITX
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00012433},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00012533},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
#ifdef SMD_AUTO_POWER_CMD
	{ SSITX,	{0x00012606},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
#else
	{ SSITX,	{0x00012602},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
#endif
	{ SSITX,	{0x00012742},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00012F02},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
// Power Setting Sequence - 2 (Power Boosting Setting)
#ifdef SMD_AUTO_POWER_CMD
	{ SSITX,	{0x00012001},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	10	},
#else
	{ SSITX,	{0x00012001},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	10	},
	{ SSITX,	{0x00012011},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	20	},
	{ SSITX,	{0x00012031},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	60	},
	{ SSITX,	{0x00012071},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	60	},  //  wait_ms(1);
	{ SSITX,	{0x00012073},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	20	},  //  wait_ms(1);
	{ SSITX,	{0x00012077},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	10	},  //  wait_ms(1);
#endif
// Power Setting Sequence - 3 (AMP On)
	{ SSITX,	{0x00010401},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	10	},  //  wait_ms(1);
// Initial Sequence - 1 (LTPS Setting)
	{ SSITX,	{0x00010644},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00010704},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00010801},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
#ifdef SMD_AUTO_POWER_CMD
	{ SSITX,	{0x00010926},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
#else
	{ SSITX,	{0x00010906},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
#endif
	{ SSITX,	{0x00010A21},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00010C00},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00010D14},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00010E00},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00010F1E},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00011000},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
// Initial Sequence - 2 (RGB I/F Setting)
	{ SSITX, 	{0x00011C08},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00011D05},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
	{ SSITX,	{0x00011F00},	1	},  //   # SPI.SSITX  
	{ 1,		{0x00000000},	1	},
// Gamma Selection Sequence  default 13 level	 
    { SSITX,	{0x00013032},	1 	},
	{ 1,		{0x00000000},	1	},    
	{ SSITX,	{0x00013138},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013236},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x0001334A},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013462},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013562},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x0001361F},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x0001371B},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x0001381C},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013924},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013A20},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013B21},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013C2C},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013D1E},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013E24},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00013F3A},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x0001401C},	1 	},
	{ 1,		{0x00000000},	1	},	
	{ SSITX,	{0x00014128},	1 	},

#if 0 //to_protect_blink
	{ SSITX, 	{0x00010405},	1	},  //   # SPI.SSITX
	{ 2,		{0x00000000},	20  },  //  wait_ms(1);
	{ SSITX,	{0x00010407},	1	},  //   # SPI.SSITX
	{ 2,		{0x00000000},	15  },  //  wait_ms(1);
	{ SSITX,	{0x00010405},	1	},  //   # SPI.SSITX
	{ 2,		{0x00000000},   25  },  //  wait_ms(1);
	{ SSITX,	{0x00010407},	1	},  //   # SPI.SSITX
	{ 2,		{0x00000000},   15  },  //  wait_ms(1);
	{ SSITX,	{0x00010405},	1	},  //   # SPI.SSITX
	{ 2,		{0x00000000},   25  },  //  wait_ms(1);
	{ SSITX,	{0x00010407},	1	},  //   # SPI.SSITX
#else
	{ SSITX,	{0x00010405},	1	},  //   # SPI.SSITX
	{ 1,		{0x00000000},   20	},  //  wait_ms(1);
	{ SSITX,	{0x00010407},	1	},  //   # SPI.SSITX
#endif
};

static struct mddi_table smd_oled_sleep_table[] = 
{
#if 1  // Sleep Sequence
	{ SSITX,	{0x00010403},	1 	},
	{ 1,		{0x00000000},	100 },
	{ SSITX,	{0x00010401},	1 	},
	{ 1,		{0x00000000},	20 	},
	{ SSITX,	{0x00010400},	1 	},
	{ 1,		{0x00000000},	10 },
#ifdef SMD_AUTO_POWER_CMD	
	{ SSITX,	{0x00010500},	1 	},  // power off
#endif
	{ SSITX,	{0x00010302},	1 	},  // Stand by on
	{ 1,		{0x00000000},	10 },
	{ START,	{0x00000000},	1 	},
	{ 1,        {0x00000000},	60	},
#else  // Same Power off sequence Requested By SMD, Because of lcd flicker
   // Power Off Sequence
	{ SSITX,	{0x00010403},	1 	},
	{ 1,		{0x00000000},	100 },
	{ SSITX,	{0x00010401},	1 	},
	{ 1,		{0x00000000},	60 	},
	{ SSITX,	{0x00010400},	1 	},
	{ 1,		{0x00000000},	100 },
#ifdef SMD_AUTO_POWER_CMD	
	{ SSITX,	{0x00010500},	1 	},
	{ 1,        {0x00000000},	100	},
	{ START,	{0x00000000},	1 	},
#else
	{ START,	{0x00000000},	1 	},
	{ 1,        {0x00000000},	100	},
	{ SSITX,	{0x00012000},	1 	},
#endif
#endif
};	

static struct mddi_table mddi_toshiba_sleep_table[] = 
{
	{ PXL,           {0x00000000},   1   },   // LCDC sleep mode
	{ START,         {0x00000000},   1   },   // LCD.START  Sync I/F OFF
	{ REGENB,        {0x00000001},   1   },   // reflect setup by next VSYNC
	{ 1,		     {0x00000000},	 30  },                
	{ CNT_DIS,       {0x00000003},   1   }  , // SYS.CNT_DIS LCD all signal to Low state
	{ CLKENB,        {0x00000000},   1   }, // SYS.CLKENB all module(system) clock disable
	{ DRAMPWR,       {0x00000000},   1   }, // SYS.DRAMPWR
	{ SYSCKENA,      {0x00000000},   1   }, // MDC.SYSCKENA system clock disable
	{ DPSUS,         {0x00000001},   1   },  // MDC.DPSUS -> Suspend
	{ DPRUN,         {0x00000000},   1   },  // MDC.DPRUN -> Reset
	{ 1,		     {0x00000000},	 14   },                
};

static struct mddi_table mddi_toshiba_sleep_first_table[] = 
{
	{ DPSET0,        {0x4CB60075},   1   }, // MDC.DPSET0 
	{ DPSET1,        {0x00000112},   1   }, // MDC.DPSET1 (96MHz PLLCLK)  2008.10.16 DPLL 120MHz -> 96MHz change asked by H/W
	{ DPSUS,         {0x00000000},   1   }, // MDC.DPSUS 
	{ DPRUN,         {0x00000001},   1   }, // MDC.DPRUN reset DPLL
};

static struct mddi_table mddi_toshiba_sleep_second_table[] = 
{
	{ SYSCKENA,      {0x00000001},   1   }, // MDC.SYSCKENA system clock disable
	{ CLKENB,        {0x000000C0},   1   }, // SYS.CLKENB all module(system) clock disable
	{ CNT_DIS,       {0x00000011},   1   }, // SYS.CNT_DIS LCD all signal to Low state	
	{ DRAMPWR,       {0x00000000},   1   }, // SYS.DRAMPWR
	{ GPIODIR,	     {0x00000008},	 1 	 },
	{ GPIOSEL,	     {0x00000000},	 1 	 },
	{ GPIOPC,	     {0x00080000},	 1 	 },
	{ GPIODATA,	     {0x00080000},	 1 	 },
	{ 1,		     {0x00000000},	 10	 },
	{ GPIODATA,	     {0x00080008},	 1 	 },
	{ 1,		     {0x00000000},	 30	 },
	{ SYSCKENA,      {0x00000000},   1   }, // MDC.SYSCKENA system clock disable
	{ DPSET0,        {0x4CB60075},   1   }, // MDC.DPSET0 
	{ DPSET1,        {0x00000112},   1   }, // MDC.DPSET1 (96MHz PLLCLK)  2008.10.16 DPLL 120MHz -> 96MHz change asked by H/W
	{ DPSUS,         {0x00000001},   1   }, // MDC.DPSUS 
	{ DPRUN,         {0x00000000},   1   }, // MDC.DPRUN reset DPLL
};


static struct mddi_table mddi_toshiba_wakeup_first_table[] = 
{	
#ifdef SMD_VYSNC_50HZ_CMD  // change -> 57hz -> 50hz (flicker)
	{ DPSET0,        {0x0A990050},   1   }, // MDC.DPSET0 
	{ DPSET1,        {0x00000116},   1   }, // MDC.DPSET1 (96MHz PLLCLK)  2008.10.16 DPLL 120MHz -> 96MHz change asked by H/W
#else
	{ DPSET0,        {0x4CB60075},   1   }, // MDC.DPSET0 
	{ DPSET1,        {0x00000112},   1   }, // MDC.DPSET1 (96MHz PLLCLK)  2008.10.16 DPLL 120MHz -> 96MHz change asked by H/W
#endif
	{ DPSUS,         {0x00000000},   1   }, // MDC.DPSUS 
	{ DPRUN,         {0x00000001},   1   }, // MDC.DPRUN reset DPLL
}; 

static struct mddi_table mddi_toshiba_wakeup_second_table[] = 
{
	{ SYSCKENA,      {0x00000001},   1   }, // MDC.SYSCKENA 
	{ CLKENB,        {0x000020EF},   1   }, // SYS.CLKENB 
	{ GPIODIR,	     {0x00000008},	 1 	 },
	{ GPIOPC,	     {0x00080008},	 1 	 },
	{ DRAMPWR,       {0x00000001},   1   }, // SYS.DRAMPWR
	{ CLKENB,        {0x000021EF},   1   }, // SYS.CLKENB 
	{ SSICTL,        {0x00000110},   1   }, // SPI operation mode 
	{ SSITIME,       {0x00000101},   1   }, // SPI serial i/f timing
	{ SSICTL,        {0x00000113},   1   }, // Set SPI active mode
#if 1   // LCDC Reset 
	{ SRST,          {0x00000000},   1   }, // LCDC Reset
	{ 1,		     {0x00000000},	 1	 },
	{ SRST,          {0x00000003},   1   },
#endif
	{ BITMAP0,	     {0x01E00140},	 1	 },
	{ CLKENB,        {0x000021EF},   1   }, // SYS.CLKENB 
	{ PORT_ENB,	     {0x00000001},	 1 	 },
	{ PORT,		     {0x00000008},	 1 	 },
	{ PXL,		     {0x0000003A},	 1 	 },
	{ MPLFBUF,	     {0x00000000},	 1 	 },
	{ HCYCLE,	     {0x000000DF},	 1 	 },
	{ HSW,		     {0x00000000},	 1 	 },
	{ HDE_START,     {0x0000001F},	 1 	 },
	{ HDE_SIZE,	     {0x0000009F}, 	 1 	 },
	{ VCYCLE,	     {0x000001EF},	 1 	 },
	{ VSW,		     {0x00000001},	 1 	 },
	{ VDE_START,     {0x00000007},	 1 	 },  
	{ VDE_SIZE,	     {0x000001DF},	 1 	 },
	{ CNT_DIS,        {0x00000000 }, 1 },	
    { START,	    {0x00000001},	1 	},
	{ 1,		     {0x00000000},	 10	 }
};	

static struct mddi_table smd_oled_wakeup_start_table[] = 
{
    { SSITX,	{0x00010300},   1 	},   // Stand by off
#if 0  // dark display problem By SMD
	{ GPIODATA,	{0x00080000},	 1 	 },
	{ 1,		{0x00000000},	 15	 },
	{ GPIODATA,	{0x00080008},	 1 	 },
	{ 1,		{0x00000000},	 5	 },
	{ SSITX,	{0x00012200},	 1 	 },
	{ SSITX,	{0x00012001},	 1 	 },
	{ 1,		{0x00000000},	 10	 },
	{ SSITX,	{0x00012011},	 1 	 },
	{ 1,		{0x00000000},	 20	 },
	{ SSITX,	{0x00012031},	 1 	 },
	{ 1,		{0x00000000},	 60	 },
	{ SSITX,	{0x00010302},	 1 	 },
	{ 1,		{0x00000000},	 10	 },
#endif
#if 1  // Reset Add Requested By SMD, Because of lcd flicker
	{ GPIODATA,	{0x00080000},	 1 	 },
	{ 1,		{0x00000000},	 15	 },
	{ GPIODATA,	{0x00080008},	 1 	 },
	{ 1,		{0x00000000},	 15	 },
#endif

};

static struct mddi_table smd_oled_wakeup_power_table[] = 
{
    // Power Setting Sequence - 1 (Analog Setting)
	{ SSITX,	{0x00010100},	1 	},
	{ 0,        {0x00000000},	2	},
	{ SSITX,	{0x00012133},	1	}, 
	{ 0,        {0x00000000},	2	},
	{ SSITX,	{0x00012208},   1	},	//   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00012300},   1	},	//   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00012433},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00012533},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2	},  //  wait_ms(1);
#ifdef SMD_AUTO_POWER_CMD
	{ SSITX,    {0x00012606},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},	2	},  //  wait_ms(1);
#else
	{ SSITX,    {0x00012602},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},	2	},  //  wait_ms(1);
#endif
	{ SSITX,	{0x00012742},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},	2	},  //  wait_ms(1);
	{ SSITX,    {0x00012F02},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},	2	},  //  wait_ms(1);
    // Power Setting Sequence - 2 (Power Boosting Setting)
#ifdef SMD_AUTO_POWER_CMD
	{ SSITX,    {0x00010501},  1 },  //   # SPI.SSITX  
	{ 1,        {0x00000000},  200 },  //  more wait_ms(200) ;
#else
	{ SSITX,    {0x00012001},	1	},  //   # SPI.SSITX  
	{ 1,        {0x00000000},   10  },  //  wait_ms(1);
	{ SSITX,    {0x00012011},	1	},  //   # SPI.SSITX  
	{ 1,        {0x00000000},   20  },  //  wait_ms(1);
	{ SSITX,    {0x00012031},	1	},  //   # SPI.SSITX  
	{ 1,        {0x00000000},   60  },  //  wait_ms(1);
	{ SSITX,    {0x00012071},	1	},  //   # SPI.SSITX  
	{ 1,        {0x00000000},   60  },  //  wait_ms(1);
	{ SSITX,    {0x00012073},	1	},  //   # SPI.SSITX  
	{ 1,        {0x00000000},   20  },  //  wait_ms(1);
	{ SSITX,    {0x00012077},	1	},  //   # SPI.SSITX  
	{ 1,        {0x00000000},   10  },  //  wait_ms(1);
#endif
	// Power Setting Sequence - 3 (AMP On)
	{ SSITX,    {0x00010401},	1	},  //   # SPI.SSITX  
	{ 1,        {0x00000000},   10  },  //  wait_ms(1);
};

static struct mddi_table smd_oled_wakeup_init_table[] = 
{
	// Initial Sequence - 1 (LTPS Setting)
	{ SSITX,    {0x00010644},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00010704},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00010801},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
#ifdef SMD_VYSNC_50HZ_CMD  // change -> 57hz -> 50hz (flicker)
#ifdef SMD_AUTO_POWER_CMD
	{ SSITX,    {0x00010902},  1 },    //   # SPI.SSITX  
	{ 1,        {0x00000000},  2 },    //   wait_ms(1) ;
#else
	{ SSITX,    {0x00010906},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
#endif
	{ SSITX,    {0x00010A11},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
#else
#ifdef SMD_AUTO_POWER_CMD
	{ SSITX,    {0x00010926},  1 },    //   # SPI.SSITX  
	{ 1,        {0x00000000},  2 },    //   wait_ms(1) ;
#else
	{ SSITX,    {0x00010906},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
#endif
	{ SSITX,    {0x00010A21},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
#endif
	{ SSITX,    {0x00010C00},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00010D14},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00010E00},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00010F1E},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00011000},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	// Initial Sequence - 2 (RGB I/F Setting)
    { SSITX,    {0x00011C08},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
    { SSITX,    {0x00011D05},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00011F00},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
};

static struct mddi_table smd_oled_wakeup_gamma_table[] = 
{
	// Gamma Selection Sequence			 
	{ SSITX,    {0x00013028},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013130},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013230},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013358},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013470},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013576},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x0001361E},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x0001371A},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013819},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013929},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013A23},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013B24},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013C35},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013D25},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013E27},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00013F3F},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00014028},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
	{ SSITX,    {0x00014130},	1	},  //   # SPI.SSITX  
	{ 0,        {0x00000000},   2   },  //  wait_ms(1);
};

static struct mddi_table smd_oled_wakeup_display_on_table[] = 
{
    // Display On Sequence
#if 0 //to_protect_blink
    { SSITX,    {0x00010405},	1	},  //   # SPI.SSITX  
    { 2,        {0x00000000},   20  },  //  wait_ms(1);
    { SSITX,    {0x00010407},	1	},  //   # SPI.SSITX  
    { 2,        {0x00000000},   15  },  //  wait_ms(1);
    { SSITX,    {0x00010405},	1	},  //   # SPI.SSITX      
    { 2,        {0x00000000},   25  },  //  wait_ms(1);	
    { SSITX,    {0x00010407},	1	},  //   # SPI.SSITX  
    { 2,        {0x00000000},   15  },  //  wait_ms(1);
    { SSITX,    {0x00010405},	1	},  //   # SPI.SSITX      
    { 2,        {0x00000000},   25  },  //  wait_ms(1);	
    { SSITX,    {0x00010407},	1	},  //   # SPI.SSITX  
#else
    { SSITX,    {0x00010405},	1	},  //   # SPI.SSITX  
    { 1,        {0x00000000},   20  },  //  wait_ms(1);
    { SSITX,    {0x00010407},	1	},  //   # SPI.SSITX  
#endif
};

static struct mddi_table smd_oled_shutdown_table[] = 
{
   // Power Off Sequence
	{ SSITX,	{0x00010403},	1 	},
	{ 1,		{0x00000000},	100 },
	{ SSITX,	{0x00010401},	1 	},
	{ 1,		{0x00000000},	20 	},
	{ SSITX,	{0x00010400},	1 	},
	{ 1,		{0x00000000},	10 },
#ifdef SMD_AUTO_POWER_CMD	
	{ SSITX,	{0x00010500},	1 	},
	{ SSITX,	{0x00010302},	1 	},	
	{ 1,        {0x00000000},	10	},
	{ START,	{0x00000000},	1 	},
#else
	{ START,	{0x00000000},	1 	},
	{ 1,        {0x00000000},	100	},
	{ SSITX,	{0x00012000},	1 	},
#endif
};

static struct mddi_table mddi_toshiba_shutdown_table[] = 
{
    // Power Off Sequence
	{ PORT,		     {0x00000003},	 1 	 },
	{ REGENB,        {0x00000001},   1   },   // reflect setup by next VSYNC
	{ 1,		     {0x00000000},	 16  },                
	{ PXL,           {0x00000000},   1   },   // LCDC sleep mode
	{ START,         {0x00000000},   1   },   // LCD.START  Sync I/F OFF
	{ REGENB,        {0x00000001},   1   },   // reflect setup by next VSYNC
};

// Dimming -> 50cd
static struct mddi_table smd_oled_gamma_40cd_table[] = 
{
    { SSITX,	{0x00013032},	1 	},
	{ SSITX,	{0x00013133},	1 	},
	{ SSITX,	{0x00013233},	1 	},
	{ SSITX,	{0x00013323},	1 	},
	{ SSITX,	{0x00013431},	1 	},
	{ SSITX,	{0x00013531},	1 	},
	{ SSITX,	{0x00013622},	1 	},
	{ SSITX,	{0x00013720},	1 	},
	{ SSITX,	{0x00013820},	1 	},
	{ SSITX,	{0x00013927},	1 	},
	{ SSITX,	{0x00013A24},	1 	},
	{ SSITX,	{0x00013B25},	1 	},
	{ SSITX,	{0x00013C2A},	1 	},
	{ SSITX,	{0x00013D1E},	1 	},
	{ SSITX,	{0x00013E21},	1 	},
	{ SSITX,	{0x00013F38},	1 	},
	{ SSITX,	{0x00014020},	1 	},
	{ SSITX,	{0x00014128},	1 	},
};	

// level 1 -> 110cd
static struct mddi_table smd_oled_gamma_95cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x00013336},	1 	},
	{ SSITX,	{0x00013448},	1 	},
	{ SSITX,	{0x00013548},	1 	},
	{ SSITX,	{0x00013620},	1 	},
	{ SSITX,	{0x0001371D},	1 	},
	{ SSITX,	{0x0001381F},	1 	},
	{ SSITX,	{0x00013925},	1 	},
	{ SSITX,	{0x00013A22},	1 	},
	{ SSITX,	{0x00013B22},	1 	},
	{ SSITX,	{0x00013C29},	1 	},
	{ SSITX,	{0x00013D1A},	1 	},
	{ SSITX,	{0x00013E1F},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x0001412A},	1 	},
};

// level 2 -> 115cd
static struct mddi_table smd_oled_gamma_100cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x00013338},	1 	},
	{ SSITX,	{0x00013449},	1 	},
	{ SSITX,	{0x0001354B},	1 	},
	{ SSITX,	{0x0001361F},	1 	},
	{ SSITX,	{0x0001371E},	1 	},
	{ SSITX,	{0x0001381D},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A21},	1 	},
	{ SSITX,	{0x00013B21},	1 	},
	{ SSITX,	{0x00013C2A},	1 	},
	{ SSITX,	{0x00013D1C},	1 	},
	{ SSITX,	{0x00013E21},	1 	},
	{ SSITX,	{0x00013F39},	1 	},
	{ SSITX,	{0x00014021},	1 	},
	{ SSITX,	{0x00014129},	1 	},
};

// level 3 -> 120cd 
static struct mddi_table smd_oled_gamma_110cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x00013338},	1 	},
	{ SSITX,	{0x0001344B},	1 	},
	{ SSITX,	{0x0001354B},	1 	},
	{ SSITX,	{0x00013621},	1 	},
	{ SSITX,	{0x0001371D},	1 	},
	{ SSITX,	{0x0001381F},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A21},	1 	},
	{ SSITX,	{0x00013B22},	1 	},
	{ SSITX,	{0x00013C29},	1 	},
	{ SSITX,	{0x00013D1A},	1 	},
	{ SSITX,	{0x00013E1F},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x0001412A},	1 	},
};

// level 4	-> 125cd
static struct mddi_table smd_oled_gamma_120cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x0001333A},	1 	},
	{ SSITX,	{0x0001344C},	1 	},
	{ SSITX,	{0x0001354D},	1 	},
	{ SSITX,	{0x0001361F},	1 	},
	{ SSITX,	{0x0001371D},	1 	},
	{ SSITX,	{0x0001381D},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A21},	1 	},
	{ SSITX,	{0x00013B22},	1 	},
	{ SSITX,	{0x00013C2A},	1 	},
	{ SSITX,	{0x00013D1D},	1 	},
	{ SSITX,	{0x00013E21},	1 	},
	{ SSITX,	{0x00013F39},	1 	},
	{ SSITX,	{0x00014021},	1 	},
	{ SSITX,	{0x00014129},	1 	},
};	

// level 5	
static struct mddi_table smd_oled_gamma_130cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x0001333A},	1 	},
	{ SSITX,	{0x0001344E},	1 	},
	{ SSITX,	{0x0001354E},	1 	},
	{ SSITX,	{0x00013621},	1 	},
	{ SSITX,	{0x0001371D},	1 	},
	{ SSITX,	{0x0001381E},	1 	},
	{ SSITX,	{0x00013925},	1 	},
	{ SSITX,	{0x00013A21},	1 	},
	{ SSITX,	{0x00013B22},	1 	},
	{ SSITX,	{0x00013C2A},	1 	},
	{ SSITX,	{0x00013D1C},	1 	},
	{ SSITX,	{0x00013E20},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x0001412A},	1 	},
};

// level 6	
static struct mddi_table smd_oled_gamma_140cd_table[] = 
{
	{ SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x0001333D},	1 	},
	{ SSITX,	{0x00013450},	1 	},
	{ SSITX,	{0x00013551},	1 	},
	{ SSITX,	{0x00013620},	1 	},
	{ SSITX,	{0x0001371D},	1 	},
	{ SSITX,	{0x0001381D},	1 	},
	{ SSITX,	{0x00013925},	1 	},
	{ SSITX,	{0x00013A22},	1 	},
	{ SSITX,	{0x00013B22},	1 	},
	{ SSITX,	{0x00013C29},	1 	},
	{ SSITX,	{0x00013D1C},	1 	},
	{ SSITX,	{0x00013E20},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x0001412A},	1 	},
};	

// level 7
static struct mddi_table smd_oled_gamma_150cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x0001333F},	1 	},
	{ SSITX,	{0x00013453},	1 	},
	{ SSITX,	{0x00013553},	1 	},
	{ SSITX,	{0x0001361F},	1 	},
	{ SSITX,	{0x0001371D},	1 	},
	{ SSITX,	{0x0001381D},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A21},	1 	},
	{ SSITX,	{0x00013B22},	1 	},
	{ SSITX,	{0x00013C2A},	1 	},
	{ SSITX,	{0x00013D1D},	1 	},
	{ SSITX,	{0x00013E21},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x0001412A},	1 	},
};	

// level 8
static struct mddi_table smd_oled_gamma_160cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x00013341},	1 	},
	{ SSITX,	{0x00013456},	1 	},
	{ SSITX,	{0x00013556},	1 	},
	{ SSITX,	{0x0001361F},	1 	},
	{ SSITX,	{0x0001371C},	1 	},
	{ SSITX,	{0x0001381D},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A21},	1 	},
	{ SSITX,	{0x00013B21},	1 	},
	{ SSITX,	{0x00013C2A},	1 	},
	{ SSITX,	{0x00013D1D},	1 	},
	{ SSITX,	{0x00013E22},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x0001412A},	1 	},
};	

// level 9
static struct mddi_table smd_oled_gamma_170cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x00013343},	1 	},
	{ SSITX,	{0x00013458},	1 	},
	{ SSITX,	{0x00013559},	1 	},
	{ SSITX,	{0x00013620},	1 	},
	{ SSITX,	{0x0001371C},	1 	},
	{ SSITX,	{0x0001381D},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A22},	1 	},
	{ SSITX,	{0x00013B21},	1 	},
	{ SSITX,	{0x00013C29},	1 	},
	{ SSITX,	{0x00013D1D},	1 	},
	{ SSITX,	{0x00013E21},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x0001412A},	1 	},
};

// level 10
static struct mddi_table smd_oled_gamma_180cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013137},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x00013345},	1 	},
	{ SSITX,	{0x0001345B},	1 	},
	{ SSITX,	{0x0001355B},	1 	},
	{ SSITX,	{0x0001361F},	1 	},
	{ SSITX,	{0x0001371B},	1 	},
	{ SSITX,	{0x0001381D},	1 	},
	{ SSITX,	{0x00013923},	1 	},
	{ SSITX,	{0x00013A22},	1 	},
	{ SSITX,	{0x00013B21},	1 	},
	{ SSITX,	{0x00013C2A},	1 	},
	{ SSITX,	{0x00013D1D},	1 	},
	{ SSITX,	{0x00013E22},	1 	},
	{ SSITX,	{0x00013F39},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x00014129},	1 	},
};	

// level 11
static struct mddi_table smd_oled_gamma_190cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013138},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x00013347},	1 	},
	{ SSITX,	{0x0001345D},	1 	},
	{ SSITX,	{0x0001355D},	1 	},
	{ SSITX,	{0x0001361E},	1 	},
	{ SSITX,	{0x0001371B},	1 	},
	{ SSITX,	{0x0001381D},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A21},	1 	},
	{ SSITX,	{0x00013B22},	1 	},
	{ SSITX,	{0x00013C2B},	1 	},
	{ SSITX,	{0x00013D1D},	1 	},
	{ SSITX,	{0x00013E21},	1 	},
	{ SSITX,	{0x00013F39},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x00014129},	1 	},
};

// level 12
static struct mddi_table smd_oled_gamma_200cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013138},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x00013349},	1 	},
	{ SSITX,	{0x0001345F},	1 	},
	{ SSITX,	{0x00013560},	1 	},
	{ SSITX,	{0x0001361F},	1 	},
	{ SSITX,	{0x0001371B},	1 	},
	{ SSITX,	{0x0001381D},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A21},	1 	},
	{ SSITX,	{0x00013B22},	1 	},
	{ SSITX,	{0x00013C2A},	1 	},
	{ SSITX,	{0x00013D1C},	1 	},
	{ SSITX,	{0x00013E20},	1 	},
	{ SSITX,	{0x00013F39},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x00014129},	1 	},
};	

// level 13
static struct mddi_table smd_oled_gamma_210cd_table[] = 
{
    { SSITX,	{0x00013032},	1 	},
	{ SSITX,	{0x00013138},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x0001334A},	1 	},
	{ SSITX,	{0x00013462},	1 	},
	{ SSITX,	{0x00013562},	1 	},
	{ SSITX,	{0x0001361F},	1 	},
	{ SSITX,	{0x0001371B},	1 	},
	{ SSITX,	{0x0001381C},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A20},	1 	},
	{ SSITX,	{0x00013B21},	1 	},
	{ SSITX,	{0x00013C2C},	1 	},
	{ SSITX,	{0x00013D1E},	1 	},
	{ SSITX,	{0x00013E24},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401C},	1 	},
	{ SSITX,	{0x00014128},	1 	},
};

// level 14
static struct mddi_table smd_oled_gamma_220cd_table[] = 
{
    { SSITX,	{0x00013032},	1 	},
	{ SSITX,	{0x00013138},	1 	},
	{ SSITX,	{0x00013236},	1 	},
	{ SSITX,	{0x0001334C},	1 	},
	{ SSITX,	{0x00013463},	1 	},
	{ SSITX,	{0x00013564},	1 	},
	{ SSITX,	{0x0001361F},	1 	},
	{ SSITX,	{0x0001371C},	1 	},
	{ SSITX,	{0x0001381C},	1 	},
	{ SSITX,	{0x00013923},	1 	},
	{ SSITX,	{0x00013A20},	1 	},
	{ SSITX,	{0x00013B21},	1 	},
	{ SSITX,	{0x00013C2B},	1 	},
	{ SSITX,	{0x00013D1D},	1 	},
	{ SSITX,	{0x00013E22},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401C},	1 	},
	{ SSITX,	{0x00014128},	1 	},
};	

// level 15
static struct mddi_table smd_oled_gamma_230cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013139},	1 	},
	{ SSITX,	{0x00013237},	1 	},
	{ SSITX,	{0x0001334E},	1 	},
	{ SSITX,	{0x00013466},	1 	},
	{ SSITX,	{0x00013567},	1 	},
	{ SSITX,	{0x0001361D},	1 	},
	{ SSITX,	{0x0001371A},	1 	},
	{ SSITX,	{0x0001381B},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A20},	1 	},
	{ SSITX,	{0x00013B21},	1 	},
	{ SSITX,	{0x00013C2A},	1 	},
	{ SSITX,	{0x00013D1C},	1 	},
	{ SSITX,	{0x00013E20},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401C},	1 	},
	{ SSITX,	{0x00014128},	1 	},
};	

// level 16
static struct mddi_table smd_oled_gamma_240cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013139},	1 	},
	{ SSITX,	{0x00013237},	1 	},
	{ SSITX,	{0x00013350},	1 	},
	{ SSITX,	{0x00013468},	1 	},
	{ SSITX,	{0x00013569},	1 	},
	{ SSITX,	{0x0001361E},	1 	},
	{ SSITX,	{0x0001371A},	1 	},
	{ SSITX,	{0x0001381B},	1 	},
	{ SSITX,	{0x00013923},	1 	},
	{ SSITX,	{0x00013A20},	1 	},
	{ SSITX,	{0x00013B20},	1 	},
	{ SSITX,	{0x00013C27},	1 	},
	{ SSITX,	{0x00013D1A},	1 	},
	{ SSITX,	{0x00013E1E},	1 	},
	{ SSITX,	{0x00013F39},	1 	},
	{ SSITX,	{0x0001401D},	1 	},
	{ SSITX,	{0x00014128},	1 	},
};	

// level 17
static struct mddi_table smd_oled_gamma_250cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013139},	1 	},
	{ SSITX,	{0x00013237},	1 	},
	{ SSITX,	{0x00013352},	1 	},
	{ SSITX,	{0x0001346A},	1 	},
	{ SSITX,	{0x0001356B},	1 	},
	{ SSITX,	{0x0001361C},	1 	},
	{ SSITX,	{0x0001371A},	1 	},
	{ SSITX,	{0x0001381A},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A1F},	1 	},
	{ SSITX,	{0x00013B21},	1 	},
	{ SSITX,	{0x00013C27},	1 	},
	{ SSITX,	{0x00013D1A},	1 	},
	{ SSITX,	{0x00013E1E},	1 	},
	{ SSITX,	{0x00013F34},	1 	},
	{ SSITX,	{0x0001401E},	1 	},
	{ SSITX,	{0x00014129},	1 	},
};

// level 18
static struct mddi_table smd_oled_gamma_260cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013139},	1 	},
	{ SSITX,	{0x00013237},	1 	},
	{ SSITX,	{0x00013353},	1 	},
	{ SSITX,	{0x0001346C},	1 	},
	{ SSITX,	{0x0001356C},	1 	},
	{ SSITX,	{0x0001361D},	1 	},
	{ SSITX,	{0x0001371A},	1 	},
	{ SSITX,	{0x0001381B},	1 	},
	{ SSITX,	{0x00013923},	1 	},
	{ SSITX,	{0x00013A1F},	1 	},
	{ SSITX,	{0x00013B20},	1 	},
	{ SSITX,	{0x00013C27},	1 	},
	{ SSITX,	{0x00013D1A},	1 	},
	{ SSITX,	{0x00013E1E},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401E},	1 	},
	{ SSITX,	{0x00014129},	1 	},
};	

// level 19
static struct mddi_table smd_oled_gamma_270cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013139},	1 	},
	{ SSITX,	{0x00013237},	1 	},
	{ SSITX,	{0x00013354},	1 	},
	{ SSITX,	{0x0001346E},	1 	},
	{ SSITX,	{0x0001356E},	1 	},
	{ SSITX,	{0x0001361D},	1 	},
	{ SSITX,	{0x00013719},	1 	},
	{ SSITX,	{0x0001381B},	1 	},
	{ SSITX,	{0x00013924},	1 	},
	{ SSITX,	{0x00013A1F},	1 	},
	{ SSITX,	{0x00013B20},	1 	},
	{ SSITX,	{0x00013C27},	1 	},
	{ SSITX,	{0x00013D1A},	1 	},
	{ SSITX,	{0x00013E1E},	1 	},
	{ SSITX,	{0x00013F3A},	1 	},
	{ SSITX,	{0x0001401E},	1 	},
	{ SSITX,	{0x00014129},	1 	},
};

// level 20	
static struct mddi_table smd_oled_gamma_280cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013139},	1 	},
	{ SSITX,	{0x00013237},	1 	},
	{ SSITX,	{0x00013355},	1 	},
	{ SSITX,	{0x00013470},	1 	},
	{ SSITX,	{0x00013570},	1 	},
	{ SSITX,	{0x0001361E},	1 	},
	{ SSITX,	{0x00013719},	1 	},
	{ SSITX,	{0x0001381B},	1 	},
	{ SSITX,	{0x00013923},	1 	},
	{ SSITX,	{0x00013A1F},	1 	},
	{ SSITX,	{0x00013B1F},	1 	},
	{ SSITX,	{0x00013C28},	1 	},
	{ SSITX,	{0x00013D1B},	1 	},
	{ SSITX,	{0x00013E1F},	1 	},
	{ SSITX,	{0x00013F3B},	1 	},
	{ SSITX,	{0x0001401F},	1 	},
	{ SSITX,	{0x0001412A},	1 	},
};	

// level 21	
static struct mddi_table smd_oled_gamma_290cd_table[] = 
{
    { SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013139},	1 	},
	{ SSITX,	{0x00013237},	1 	},
	{ SSITX,	{0x00013357},	1 	},
	{ SSITX,	{0x00013471},	1 	},
	{ SSITX,	{0x00013572},	1 	},
	{ SSITX,	{0x0001361E},	1 	},
	{ SSITX,	{0x0001371A},	1 	},
	{ SSITX,	{0x0001381B},	1 	},
	{ SSITX,	{0x00013922},	1 	},
	{ SSITX,	{0x00013A1F},	1 	},
	{ SSITX,	{0x00013B1F},	1 	},
	{ SSITX,	{0x00013C28},	1 	},
	{ SSITX,	{0x00013D1B},	1 	},
	{ SSITX,	{0x00013E20},	1 	},
	{ SSITX,	{0x00013F3B},	1 	},
	{ SSITX,	{0x0001401F},	1 	},
	{ SSITX,	{0x0001412A},	1 	},
};

// level 22
static struct mddi_table smd_oled_gamma_300cd_table[] = 
{
	{ SSITX,	{0x00013033},	1 	},
	{ SSITX,	{0x00013139},	1 	},
	{ SSITX,	{0x00013237},	1 	},
	{ SSITX,	{0x00013359},	1 	},
	{ SSITX,	{0x00013474},	1 	},
	{ SSITX,	{0x00013574},	1 	},
	{ SSITX,	{0x0001361C},	1 	},
	{ SSITX,	{0x00013719},	1 	},
	{ SSITX,	{0x0001381A},	1 	},
	{ SSITX,	{0x00013923},	1 	},
	{ SSITX,	{0x00013A1F},	1 	},
	{ SSITX,	{0x00013B20},	1 	},
	{ SSITX,	{0x00013C29},	1 	},
	{ SSITX,	{0x00013D1B},	1 	},
	{ SSITX,	{0x00013E20},	1 	},
	{ SSITX,	{0x00013F39},	1 	},
	{ SSITX,	{0x00014021},	1 	},
	{ SSITX,	{0x00014129},	1 	},
};	

// calculate refresh rate  <hg2395.kim>
/* Timing variables for tracking vsync */
/* dot_clock = 13.332MHz
 * horizontal count = 448
 * vertical count = 496
 * refresh rate = 13332000/(448*496) = 60Hz
 */

//static uint32 mddi_toshiba_rows_per_second = 29758;  /* 13332000/448 */
//static uint32 mddi_toshiba_usecs_per_refresh = 16667; /* (448*496) / 13332000 */
//static uint32 mddi_toshiba_rows_per_refresh = 496;

static int galaxy_backlight_off = 1;
static int galaxy_backlight_brightness = GALAXY_DEFAULT_BACKLIGHT_BRIGHTNESS;
static int galaxy_backlight_last_level = 4;
struct msm_mddi_client_data *g_client_data = NULL;
static DEFINE_MUTEX(galaxy_backlight_lock);

#define GPIOSEL_VWAKEINT (1U << 0)
#define INTMASK_VWAKEOUT (1U << 0)
	    
static void galaxy_process_mddi_table(
				struct msm_mddi_client_data *client_data,
				struct mddi_table *table,
				size_t count)
{
	int i;
	uint32_t reg, val_len;
	
	for(i = 0; i < count; i++) 
	{
		reg = table[i].reg;
		val_len = table[i].val_len;

		if (reg == 0)
			udelay(val_len);
		else if (reg == 1)
			msleep(val_len);
		//to_protect_blink
		else if (reg == 2)
			msleep(val_len);
		else {
			//write_client_reg_multi( reg,  (unsigned *)value, val_len);
			if (val_len == 1) {
				uint32_t value = table[i].value[0];
				client_data->remote_write(client_data, value, reg);
			} else {
				uint8_t* value = (uint8_t*)table[i].value;
				printk("galaxy_process_mddi_table: multiple write on reg(%x)\n", reg);
				client_data->remote_write_vals(client_data, value, reg, val_len);
			}
		}

	}
}

static void smd_hvga_oled_power_wakeup(struct msm_mddi_client_data *client_data)
{
	galaxy_process_mddi_table(client_data, smd_oled_wakeup_power_table, ARRAY_SIZE(smd_oled_wakeup_power_table));
}

static void smd_hvga_oled_start_wakeup(struct msm_mddi_client_data *client_data)
{
	printk("[LCD] smd_hvga_oled_start_wakeup\n");
	galaxy_process_mddi_table(client_data, smd_oled_wakeup_start_table, ARRAY_SIZE(smd_oled_wakeup_start_table));
}

static void smd_hvga_oled_init_wakeup(struct msm_mddi_client_data *client_data)
{
	galaxy_process_mddi_table(client_data, smd_oled_wakeup_init_table, ARRAY_SIZE(smd_oled_wakeup_init_table));
}

static void smd_hvga_oled_display_on_wakeup(struct msm_mddi_client_data *client_data)
{
	printk("[LCD] smd_hvga_oled_display_on_wakeup\n");
	galaxy_process_mddi_table(client_data, smd_oled_wakeup_display_on_table, ARRAY_SIZE(smd_oled_wakeup_display_on_table));
}

static void smd_hvga_oled_sleep(struct msm_mddi_client_data *client_data)
{
	galaxy_process_mddi_table(client_data, smd_oled_sleep_table, ARRAY_SIZE(smd_oled_sleep_table));
}

static void smd_hvga_oled_shutdown(struct msm_mddi_client_data *client_data)
{
	galaxy_process_mddi_table(client_data, smd_oled_shutdown_table, ARRAY_SIZE(smd_oled_shutdown_table));
}

static void toshiba_bridge_wakeup(struct msm_mddi_client_data *client_data)
{
	printk("[LCD] toshiba_bridge_wakeup\n");
	galaxy_process_mddi_table(client_data, mddi_toshiba_wakeup_first_table, ARRAY_SIZE(mddi_toshiba_wakeup_first_table));
	//mddi_wait(200);
	msleep(200);
	galaxy_process_mddi_table(client_data, mddi_toshiba_wakeup_second_table, ARRAY_SIZE(mddi_toshiba_wakeup_second_table));
}

static void toshiba_bridge_sleep(struct msm_mddi_client_data *client_data)
{
	galaxy_process_mddi_table(client_data, mddi_toshiba_sleep_table, ARRAY_SIZE(mddi_toshiba_sleep_table));
}

static void toshiba_bridge_shutdown(struct msm_mddi_client_data *client_data)
{
	galaxy_process_mddi_table(client_data, mddi_toshiba_shutdown_table, ARRAY_SIZE(mddi_toshiba_shutdown_table));
}

static void galaxy_set_backlight_level(uint8_t level)
{
	unsigned int brightness_level;
	struct msm_mddi_client_data *client_data = g_client_data;

	if (galaxy_backlight_off) {
		pr_warn("galaxy_panel: ignoring backlight set, panel is off !\n");
		return;
	}

	if (!client_data) {
		pr_warn("galaxy_panel: ignoring backlight set, no client data !\n");
		return;
	}

	brightness_level = (2 * level * 22 + MAX_BACKLIGHT_BRIGHTNESS)
				/(2 * MAX_BACKLIGHT_BRIGHTNESS);

	dprintk("setting brightness level to %d\n", brightness_level);
	switch (brightness_level)
	{
	case 0:  // Dimming Not Saved
		galaxy_process_mddi_table(client_data, smd_oled_gamma_40cd_table, ARRAY_SIZE(smd_oled_gamma_40cd_table));
		break;
	case 1:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_95cd_table, ARRAY_SIZE(smd_oled_gamma_95cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 2:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_100cd_table, ARRAY_SIZE(smd_oled_gamma_100cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 3:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_110cd_table, ARRAY_SIZE(smd_oled_gamma_110cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 4:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_120cd_table, ARRAY_SIZE(smd_oled_gamma_120cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 5:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_130cd_table, ARRAY_SIZE(smd_oled_gamma_130cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 6:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_140cd_table, ARRAY_SIZE(smd_oled_gamma_140cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 7:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_150cd_table, ARRAY_SIZE(smd_oled_gamma_150cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 8:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_160cd_table, ARRAY_SIZE(smd_oled_gamma_160cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 9:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_170cd_table, ARRAY_SIZE(smd_oled_gamma_170cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 10:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_180cd_table, ARRAY_SIZE(smd_oled_gamma_180cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 11:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_190cd_table, ARRAY_SIZE(smd_oled_gamma_190cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 12:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_200cd_table, ARRAY_SIZE(smd_oled_gamma_200cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 13:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_210cd_table, ARRAY_SIZE(smd_oled_gamma_210cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 14:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_220cd_table, ARRAY_SIZE(smd_oled_gamma_220cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 15:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_230cd_table, ARRAY_SIZE(smd_oled_gamma_230cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 16:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_240cd_table, ARRAY_SIZE(smd_oled_gamma_240cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 17:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_250cd_table, ARRAY_SIZE(smd_oled_gamma_250cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 18:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_260cd_table, ARRAY_SIZE(smd_oled_gamma_260cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 19:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_270cd_table, ARRAY_SIZE(smd_oled_gamma_270cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 20:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_280cd_table, ARRAY_SIZE(smd_oled_gamma_280cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 21:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_290cd_table, ARRAY_SIZE(smd_oled_gamma_290cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	case 22:
		galaxy_process_mddi_table(client_data, smd_oled_gamma_300cd_table, ARRAY_SIZE(smd_oled_gamma_300cd_table));
		galaxy_backlight_last_level = brightness_level;
		break;
	default:
		break;
	}
}

static void galaxy_brightness_set(struct led_classdev *led_cdev, enum led_brightness brightness)
{
	dprintk("%s: brightness %d\n", __func__, brightness);
	if (brightness>=0  && brightness<=MAX_BACKLIGHT_BRIGHTNESS)
	{
		mutex_lock(&galaxy_backlight_lock);
		galaxy_backlight_brightness = brightness;
		if (galaxy_backlight_brightness)
			galaxy_set_backlight_level(galaxy_backlight_brightness);
		mutex_unlock(&galaxy_backlight_lock);
	}
}

static void galaxy_mddi_power_client(struct msm_mddi_client_data *client_data,
				    int on)
{
	dprintk("%s(%d): entered\n", __func__, on);
//	dprintk(KERN_INFO "galaxy_mddi_power_client:%d\r\n", on);
	if (on) {
		// Bridge Wake Up
		//toshiba_bridge_wakeup(client_data);
		// OLED Wake Up
		//smd_hvga_oled_start_wakeup(client_data);
		//smd_hvga_oled_power_wakeup(client_data);
	} else {
		//smd_hvga_oled_shutdown(client_data);
		//toshiba_bridge_shutdown(client_data);
	}
}

#define MFR_NAME	0xd263
#define PRODUCT_CODE 	0x8722

static void galaxy_panel_fixup(uint16_t * mfr_name, uint16_t * product_code)
{
	dprintk(KERN_DEBUG "%s: enter.\n", __func__);
	*mfr_name = MFR_NAME ;
	*product_code= PRODUCT_CODE ;
}

static int galaxy_mddi_toshiba_client_init(
			struct msm_mddi_bridge_platform_data *bridge_data,
			struct msm_mddi_client_data *client_data)
{
	dprintk("%s: entered\n", __func__);

	mutex_lock(&galaxy_backlight_lock);
	// FIXME: May not be required
	client_data->auto_hibernate(client_data, 0);
	// Bridge Wake Up
	toshiba_bridge_wakeup(client_data);
	// OLED Wake Up


	//client_data->remote_write(client_data, GPIOSEL_VWAKEINT, GPIOSEL);
	//client_data->remote_write(client_data, INTMASK_VWAKEOUT, INTMASK);

	client_data->auto_hibernate(client_data, 1);
	mutex_unlock(&galaxy_backlight_lock);
	return 0;
}

static int galaxy_mddi_toshiba_client_uninit(
			struct msm_mddi_bridge_platform_data *bridge_data,
			struct msm_mddi_client_data *client_data)
{
	dprintk("%s: entered\n", __func__);

	mutex_lock(&galaxy_backlight_lock);
	client_data->auto_hibernate(client_data, 0);

	toshiba_bridge_sleep(client_data);

	client_data->auto_hibernate(client_data, 1);
	//galaxy_set_backlight_level(0);
	galaxy_backlight_off = 1;
	mutex_unlock(&galaxy_backlight_lock);
	g_client_data = NULL;

	return 0;
}

static int galaxy_mddi_panel_unblank(
			struct msm_mddi_bridge_platform_data *bridge_data,
			struct msm_mddi_client_data *client_data)
{
	int ret = 0;
	dprintk("%s: entered\n", __func__);
	// Restore backlight level
	mutex_lock(&galaxy_backlight_lock);

	smd_hvga_oled_start_wakeup(client_data);
	smd_hvga_oled_power_wakeup(client_data);
	smd_hvga_oled_init_wakeup(client_data);

	galaxy_backlight_off = 0;
	g_client_data = client_data;
	galaxy_set_backlight_level(galaxy_backlight_brightness);

	smd_hvga_oled_display_on_wakeup(client_data);

	mutex_unlock(&galaxy_backlight_lock);
	return ret;
}

static int galaxy_mddi_panel_blank(
			struct msm_mddi_bridge_platform_data *bridge_data,
			struct msm_mddi_client_data *client_data)
{
	int ret = 0;
	dprintk("%s: entered\n", __func__);

	mutex_lock(&galaxy_backlight_lock);
	smd_hvga_oled_sleep(client_data);
	mutex_unlock(&galaxy_backlight_lock);
	return ret;
}

static struct platform_device galaxy_backlight =
{
	.name       = "galaxy-backlight",
};

static struct led_classdev galaxy_backlight_led = {
	.name           = "lcd-backlight",
	.brightness = GALAXY_DEFAULT_BACKLIGHT_BRIGHTNESS,
	.brightness_set = galaxy_brightness_set,
};

static int galaxy_backlight_probe(struct platform_device *pdev)
{
	return led_classdev_register(&pdev->dev, &galaxy_backlight_led);
}

static int galaxy_backlight_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&galaxy_backlight_led);
	return 0;
}

static struct platform_driver galaxy_backlight_driver =
{
	.probe      = galaxy_backlight_probe,
	.remove     = galaxy_backlight_remove,
	.driver     =
	{
		.name       = "galaxy-backlight",
		.owner      = THIS_MODULE,
	},
};

static struct resource resources_msm_fb[] = {
	{
		.start = MSM_FB_BASE,
		.end = MSM_FB_BASE + MSM_FB_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	}
};

static struct msm_mddi_bridge_platform_data toshiba_client_data = {
	.init = galaxy_mddi_toshiba_client_init,
	.uninit = galaxy_mddi_toshiba_client_uninit,
	.blank = galaxy_mddi_panel_blank,
	.unblank = galaxy_mddi_panel_unblank,
	.fb_data = {
		.xres = 320,
		.yres = 480,
		.width = 45,
		.height = 67,
		.output_format = MSM_MDP_OUT_IF_FMT_RGB565,
	},
};

static struct msm_mddi_platform_data mddi_pdata = {
	.clk_rate = 122880000,
	.power_client = galaxy_mddi_power_client,
	.fixup = galaxy_panel_fixup,
	.vsync_irq = MSM_GPIO_TO_INT(GPIO_LCD_VSYNC),
	.fb_resource = resources_msm_fb,
	.num_clients = 1,
	.client_platform_data = {
		{
			.product_id = 0xd2638722,
			.name = "mddi_c_d263_0000",
			.id = 0,
			.client_data = &toshiba_client_data,
			.clk_rate = 0,
		}
	},
};

static int __init galaxy_init_panel(void) {
	int rc = -1;

	/* checking board as soon as possible */
	printk("galaxy_init_panel:machine_is_galaxy=%d, machine_arch_type=%d, MACH_TYPE_GALAXY=%d\r\n", machine_is_galaxy(), machine_arch_type, MACH_TYPE_GALAXY);
	if (!machine_is_galaxy())
		return 0;

	/* configure GPIO */
	rc = gpio_request(GPIO_LCD_VSYNC, "vsync");
	if (rc) {
		pr_err("galaxy_init_panel:gpio_request failed\n");
		return rc;
	}
	rc = gpio_direction_input(GPIO_LCD_VSYNC);
	if (rc) {
		pr_err("galaxy_init_panel:gpio_direction_input failed\n");
		return rc;
	}
	rc = platform_device_register(&msm_device_mdp);
	if (rc) {
		pr_err("galaxy_init_panel:platform_device_register msm_device_mdp failed\n");
		return rc;
	}

	msm_device_mddi0.dev.platform_data = &mddi_pdata;
	rc = platform_device_register(&msm_device_mddi0);
	if (rc) {
		pr_err("galaxy_init_panel:platform_device_register msm_device_mddi0 failed\n");
		return rc;
	}
	platform_device_register(&galaxy_backlight);
	return platform_driver_register(&galaxy_backlight_driver);
}

device_initcall(galaxy_init_panel);
