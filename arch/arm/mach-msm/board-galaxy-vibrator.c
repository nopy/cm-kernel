/*
** =========================================================================
** File:
**     vtmdrv.h
**
** Description: 
**     Constants and type definitions for the VibeTonz Kernel Module.
**
** Portions Copyright (c) 2008 Immersion Corporation. All Rights Reserved. 
**
** This file contains Original Code and/or Modifications of Original Code 
** as defined in and that are subject to the GNU Public License v2 - 
** (the 'License'). You may not use this file except in compliance with the 
** License. You should have received a copy of the GNU General Public License 
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or contact 
** VibeTonzSales@immersion.com.
**
** The Original Code and all software distributed under the License are 
** distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESS OR IMPLIED, AND IMMERSION HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
** INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
** FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see 
** the License for the specific language governing rights and limitations 
** under the License.
** =========================================================================
*/
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/gpio.h>

#include <linux/mutex.h>
#include <linux/clk.h>
#include <mach/vreg.h>

#include <linux/delay.h>
#include <linux/hrtimer.h>

#include <mach/msm_iomap.h>

#include <../../../drivers/staging/android/timed_gpio.h>
#include <../../../drivers/staging/android/timed_output.h>

/* Device name and version information */
#define VERSION_STR " v2.0.92.3\n"                  /* PLEASE DO NOT CHANGE - this is auto-generated */
#define VERSION_STR_LEN 16                          /* account extra space for future extra digits in version number */

#define VIBE_MAX_DEVICE_NAME_LENGTH			64
#define VIB_ON                              95

/* Type definition */
#ifdef __KERNEL__
typedef int8_t		VibeInt8;
typedef u_int8_t	VibeUInt8;
typedef int16_t		VibeInt16;
typedef u_int16_t	VibeUInt16;
typedef int32_t		VibeInt32;
typedef u_int32_t	VibeUInt32;
typedef u_int8_t	VibeBool;
typedef VibeInt32	VibeStatus;
#endif

/* Error and Return value codes */
#define VIBE_S_SUCCESS                  0	/*!< Success */
#define VIBE_E_FAIL			-4	/*!< Generic error */

#define __inp(port)         (*((volatile unsigned char *) (port)))
#define __inpw(port)        (*((volatile unsigned short *) (port)))
#define __inpdw(port)       (*((volatile unsigned int *) (port)))

#define __outp(port, val)   (*((volatile unsigned char *) (port)) = ((unsigned char) (val)))
#define __outpw(port, val)  (*((volatile unsigned short *) (port)) = ((unsigned short) (val)))
#define __outpdw(port, val) (*((volatile unsigned int *) (port)) = ((unsigned int) (val)))


#define in_dword(addr)              (__inpdw(addr))
#define in_dword_masked(addr, mask) (__inpdw(addr) & (mask))
#define out_dword(addr, val)        __outpdw(addr,val)
#define out_dword_masked(io, mask, val, shadow)  \
   shadow = (shadow & (unsigned int)(~(mask))) | ((unsigned int)((val) & (mask))); \
   (void) out_dword( io, shadow); 
#define out_dword_masked_ns(io, mask, val, current_reg_content) \
  (void) out_dword( io, ((current_reg_content & (unsigned int)(~(mask))) | ((unsigned int)((val) & (mask)))) )

#define HWIO_GP_MD_REG_ADDR                                                 (MSM_CLK_CTL_BASE      + 0x00000058)
#define HWIO_GP_MD_REG_PHYS                                                 (MSM_CLK_CTL_PHYS + 0x00000058)
#define HWIO_GP_MD_REG_RMSK                                                 0xffffffff
#define HWIO_GP_MD_REG_SHFT                                                          0
#define HWIO_GP_MD_REG_IN                                                   \
        in_dword_masked(HWIO_GP_MD_REG_ADDR, HWIO_GP_MD_REG_RMSK)
#define HWIO_GP_MD_REG_INM(m)                                               \
        in_dword_masked(HWIO_GP_MD_REG_ADDR, m)
#define HWIO_GP_MD_REG_OUT(v)                                               \
        out_dword(HWIO_GP_MD_REG_ADDR,v)
#define HWIO_GP_MD_REG_OUTM(m,v)                                            \
	out_dword_masked_ns(HWIO_GP_MD_REG_ADDR,m,v,HWIO_GP_MD_REG_IN)

#define HWIO_GP_MD_REG_M_VAL_BMSK                                           0xffff0000
#define HWIO_GP_MD_REG_M_VAL_SHFT                                                 0x10
#define HWIO_GP_MD_REG_D_VAL_BMSK                                               0xffff
#define HWIO_GP_MD_REG_D_VAL_SHFT                                                    0

#define HWIO_GP_NS_REG_ADDR                                                 (MSM_CLK_CTL_BASE      + 0x0000005C)
#define HWIO_GP_NS_REG_PHYS                                                 (MSM_CLK_CTL_PHYS + 0x0000005C)
#define HWIO_GP_NS_REG_RMSK                                                 0xffffffff
#define HWIO_GP_NS_REG_SHFT                                                          0
#define HWIO_GP_NS_REG_IN                                                   \
        in_dword_masked(HWIO_GP_NS_REG_ADDR, HWIO_GP_NS_REG_RMSK)
#define HWIO_GP_NS_REG_INM(m)                                               \
        in_dword_masked(HWIO_GP_NS_REG_ADDR, m)
#define HWIO_GP_NS_REG_OUT(v)                                               \
        out_dword(HWIO_GP_NS_REG_ADDR,v)
#define HWIO_GP_NS_REG_OUTM(m,v)                                            \
	out_dword_masked_ns(HWIO_GP_NS_REG_ADDR,m,v,HWIO_GP_NS_REG_IN)
#define HWIO_GP_NS_REG_GP_N_VAL_BMSK                                        0xffff0000
#define HWIO_GP_NS_REG_GP_N_VAL_SHFT                                              0x10
#define HWIO_GP_NS_REG_GP_ROOT_ENA_BMSK                                          0x800
#define HWIO_GP_NS_REG_GP_ROOT_ENA_SHFT                                            0xb
#define HWIO_GP_NS_REG_GP_CLK_INV_BMSK                                           0x400
#define HWIO_GP_NS_REG_GP_CLK_INV_SHFT                                             0xa
#define HWIO_GP_NS_REG_GP_CLK_BRANCH_ENA_BMSK                                    0x200
#define HWIO_GP_NS_REG_GP_CLK_BRANCH_ENA_SHFT                                      0x9
#define HWIO_GP_NS_REG_MNCNTR_EN_BMSK                                            0x100
#define HWIO_GP_NS_REG_MNCNTR_EN_SHFT                                              0x8
#define HWIO_GP_NS_REG_MNCNTR_RST_BMSK                                            0x80
#define HWIO_GP_NS_REG_MNCNTR_RST_SHFT                                             0x7
#define HWIO_GP_NS_REG_MNCNTR_MODE_BMSK                                           0x60
#define HWIO_GP_NS_REG_MNCNTR_MODE_SHFT                                            0x5
#define HWIO_GP_NS_REG_PRE_DIV_SEL_BMSK                                           0x18
#define HWIO_GP_NS_REG_PRE_DIV_SEL_SHFT                                            0x3
#define HWIO_GP_NS_REG_SRC_SEL_BMSK                                                0x7
#define HWIO_GP_NS_REG_SRC_SEL_SHFT                                                  0

#define __msmhwio_outm(hwiosym, mask, val)  HWIO_##hwiosym##_OUTM(mask, val)
#define HWIO_OUTM(hwiosym, mask, val)                    __msmhwio_outm(hwiosym, mask, val)

static char g_szDeviceName[   VIBE_MAX_DEVICE_NAME_LENGTH 
                            + VERSION_STR_LEN];     /* initialized in init_module */
static size_t g_cchDeviceName;                      /* initialized in init_module */

struct clk *android_vib_clk; /* gp_clk */

static struct hrtimer android_timer; // for 3.10 cupcake
static int is_vib_on = 0;
static atomic_t s_vibrator = ATOMIC_INIT(0);

#define GP_CLK_M_DEFAULT			21
#define GP_CLK_N_DEFAULT			18000
#define GP_CLK_D_DEFAULT			9000	/* 50% duty cycle */ 
#define IMM_PWM_MULTIPLIER		    17778	/* Must be integer */

/*
** Global variables for LRA PWM M,N and D values.
*/
VibeInt32 g_nLRA_GP_CLK_M = GP_CLK_M_DEFAULT;
VibeInt32 g_nLRA_GP_CLK_N = GP_CLK_N_DEFAULT;
VibeInt32 g_nLRA_GP_CLK_D = GP_CLK_N_DEFAULT;
VibeInt32 g_nLRA_GP_CLK_PWM_MUL = IMM_PWM_MULTIPLIER;

#if ((LINUX_VERSION_CODE & 0xFFFF00) < KERNEL_VERSION(2,6,0))
#error Unsupported Kernel version
#endif

/* added by gtuo.park */
struct timed_gpio_data {
	struct device *dev;
	struct hrtimer timer;
	spinlock_t lock;
	unsigned    gpio;
	int         max_timeout;
	u8      active_low;
};
struct timed_gpio_data *gpio_data;

/* Variable for setting PWM in Force Out Set */
VibeInt32 g_nForce_32 = 0;

/* High resolution timer funstions */
static enum hrtimer_restart vibetonz_timer_func(struct hrtimer *timer)
{
	unsigned int remain;

	if(hrtimer_active(&android_timer)) {
		ktime_t r = hrtimer_get_remaining(&android_timer);
		remain=r.tv.sec * 1000000 + r.tv.nsec;
		remain = remain / 1000;
//		printk("vibrator time remain:%dsec/%dnsec remain:%d\n",r.tv.sec, r.tv.nsec,remain);
		if(r.tv.sec < 0) {
			remain = 0;
		}
//		if((r.tv.sec > 0) || (r.tv.nsec > 0))
		if(!remain) {
			clk_disable(android_vib_clk);
			gpio_direction_output(101,0);
			is_vib_on = 0;
		} 
	} else {
		printk("hrtimer end!\n");
		clk_disable(android_vib_clk);
		gpio_direction_output(101,0);
		is_vib_on = 0;
	}
//	clk_disable(android_vib_clk);
//	gpio_direction_output(101, 0);
	return HRTIMER_NORESTART;
}


/*
** This function is used to set and re-set the GP_CLK M and N counters
** to output the desired target frequency.
**
*/

static  VibeStatus vibe_set_pwm_freq(VibeInt8 nForce)
{
   /* Put the MND counter in reset mode for programming */
   HWIO_OUTM(GP_NS_REG, HWIO_GP_NS_REG_MNCNTR_EN_BMSK, 0);
   HWIO_OUTM(GP_NS_REG, HWIO_GP_NS_REG_PRE_DIV_SEL_BMSK, 0 << HWIO_GP_NS_REG_PRE_DIV_SEL_SHFT);	/* P: 0 => Freq/1, 1 => Freq/2, 4 => Freq/4 */
   HWIO_OUTM(GP_NS_REG, HWIO_GP_NS_REG_SRC_SEL_BMSK, 0 << HWIO_GP_NS_REG_SRC_SEL_SHFT);	/* S : 0 => TXCO(19.2MHz), 1 => Sleep XTAL(32kHz) */
   HWIO_OUTM(GP_NS_REG, HWIO_GP_NS_REG_MNCNTR_MODE_BMSK, 2 << HWIO_GP_NS_REG_MNCNTR_MODE_SHFT);	/* Dual-edge mode */
   HWIO_OUTM(GP_MD_REG, HWIO_GP_MD_REG_M_VAL_BMSK, g_nLRA_GP_CLK_M << HWIO_GP_MD_REG_M_VAL_SHFT);
   g_nForce_32 = ((nForce * g_nLRA_GP_CLK_PWM_MUL) >> 8) + g_nLRA_GP_CLK_D;
   printk("%s, g_nForce_32 : %d\n",__FUNCTION__,g_nForce_32);
   HWIO_OUTM(GP_MD_REG, HWIO_GP_MD_REG_D_VAL_BMSK, ( ~((VibeInt16)g_nForce_32 << 1) ) << HWIO_GP_MD_REG_D_VAL_SHFT);
   HWIO_OUTM(GP_NS_REG, HWIO_GP_NS_REG_GP_N_VAL_BMSK, ~(g_nLRA_GP_CLK_N - g_nLRA_GP_CLK_M) << HWIO_GP_NS_REG_GP_N_VAL_SHFT);
   HWIO_OUTM(GP_NS_REG, HWIO_GP_NS_REG_MNCNTR_EN_BMSK, 1 << HWIO_GP_NS_REG_MNCNTR_EN_SHFT);			           /* Enable M/N counter */
   printk("%x, %x, %x\n",( ~((VibeInt16)g_nForce_32 << 1) ) << HWIO_GP_MD_REG_D_VAL_SHFT,~(g_nLRA_GP_CLK_N - g_nLRA_GP_CLK_M) << HWIO_GP_NS_REG_GP_N_VAL_SHFT,1 << HWIO_GP_NS_REG_MNCNTR_EN_SHFT);

	return VIBE_S_SUCCESS;
}

static int android_vib_power(int on)
{
	struct vreg *vreg_vibetoz;

	vreg_vibetoz = vreg_get(0, "gp2");

	if(on)
	{
		vreg_set_level(vreg_vibetoz, 3000);
		vreg_enable(vreg_vibetoz);
	}
	else
	{
		vreg_disable(vreg_vibetoz);
	}
	return 0;
}



static int vibrator_pwm;
static int current_pwm;
static void to_vibrator_enable(struct timed_output_dev *sdev, int timeout)
{
//	printk("[VIB] : %d\n",timeout);
	atomic_set(&s_vibrator, timeout);

	hrtimer_cancel(&android_timer);

	if(!timeout) {
		if(is_vib_on) {
			clk_disable(android_vib_clk);
			gpio_direction_output(101, 0);
			is_vib_on = 0;
		}
	} else {
		if(timeout < 50) {
			vibrator_pwm = 140;
			if(current_pwm != vibrator_pwm) {
				vibe_set_pwm_freq(vibrator_pwm);
				current_pwm = vibrator_pwm;
			}
		} else {
			vibrator_pwm = 210;
			if(current_pwm != vibrator_pwm) {
				if(timeout == 10000) {
					vibrator_pwm = 250;
				} else {
					vibrator_pwm = 210;
				}
				vibe_set_pwm_freq(vibrator_pwm);
				current_pwm = vibrator_pwm;
			}
		}
		gpio_direction_output(101,1);
		clk_enable(android_vib_clk);
		hrtimer_start(&android_timer,ktime_set(timeout / 1000, (timeout % 1000) * 1000000),
					HRTIMER_MODE_REL);
		is_vib_on = 1;
	}
}

static int to_vibrator_get_time(struct timed_output_dev *sdev) {
	printk("vibrator_get_time\n");
	return atomic_read(&s_vibrator);
}

static ssize_t show_enable(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", atomic_read(&s_vibrator));
}

static ssize_t store_enable(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct timed_gpio_data *gpio_data = dev_get_drvdata(dev);
	int value;
	unsigned long flags;

	sscanf(buf, "%d", &value);
//	printk("Android Vibetonz, value : %d\n",value);
	spin_lock_irqsave(&gpio_data->lock, flags);
	/* TODO locking... */
	to_vibrator_enable(0, value);
	/* TODO unlocking... */
	spin_unlock_irqrestore(&gpio_data->lock, flags);

	return size;
}

static ssize_t show_pwm(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", vibrator_pwm);
}

static ssize_t store_pwm(
	   struct device *dev, struct device_attribute *attr,
	   const char *buf, size_t size) {

	int value;
	sscanf(buf, "%d", &value);
	printk("PWM Changed to %d\n",value);
	vibrator_pwm = value;
	if(vibrator_pwm < 0) {
		printk("wrong pwm value\n");
		return 0;
	} else if(vibrator_pwm > 255) {
		printk("wrong pwm value\n");
		return 0;
	}

	vibe_set_pwm_freq(vibrator_pwm);

	return vibrator_pwm;

}
static DEVICE_ATTR(enable, 0777, show_enable,store_enable);
static DEVICE_ATTR(pwm, 0777, show_pwm, store_pwm);


static struct timed_output_dev pmic_vibrator = {
	.name = "vibrator",
	.get_time = to_vibrator_get_time,
	.enable = to_vibrator_enable,
};

static int __init vibetonz_init(void)
{
	int nRet;   /* initialized below */

	printk(KERN_INFO "android_vibe: vibetonz_init.\n");
	
	gpio_data = kzalloc(sizeof(struct timed_gpio_data), GFP_KERNEL);
	if (!gpio_data)
		return -ENOMEM;

	hrtimer_init(&android_timer, CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	android_timer.function = vibetonz_timer_func;

	spin_lock_init(&gpio_data->lock);

	nRet = device_create_file(gpio_data->dev, &dev_attr_enable);
	if (nRet)
		return nRet;
	nRet = device_create_file(gpio_data->dev, &dev_attr_pwm);
	if (nRet)
		return nRet;

	/* Append version information and get buffer length */
	strcat(g_szDeviceName, VERSION_STR);
	g_cchDeviceName = strlen(g_szDeviceName);

	/* initialize android viberation mode */
	android_vib_clk = clk_get(NULL,"gp_clk");
	android_vib_power(1);

	/* clock enable and freq set to 210 */
	/* PWM Max is 255 */
	vibrator_pwm = 210;
	current_pwm = vibrator_pwm;
	vibe_set_pwm_freq(vibrator_pwm);

	printk("android viberation initialize OK(base pwm : 180)\n");

	return timed_output_dev_register(&pmic_vibrator);
}

static void __exit vibetonz_exit(void)
{
	printk(KERN_INFO "android_vibe: cleanup_module.\n");

	hrtimer_cancel(&android_timer);
	/* vibrator clock disable */
	if (is_vib_on) {
		clk_disable(android_vib_clk);
		gpio_direction_output(101,0);
	}

	android_vib_power(0);
	device_remove_file(gpio_data->dev, &dev_attr_enable);
	device_remove_file(gpio_data->dev, &dev_attr_pwm);
	kfree(gpio_data);
}

module_init(vibetonz_init);
module_exit(vibetonz_exit);

/* Module info */
MODULE_AUTHOR("Immersion Corporation");
MODULE_DESCRIPTION("VibeTonz Kernel Module");
MODULE_LICENSE("GPL v2");
