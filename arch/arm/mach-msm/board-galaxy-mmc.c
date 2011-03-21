#include <linux/delay.h>
#include <linux/err.h>
#include <asm/gpio.h>
#include <asm/mach/mmc.h>
#include <mach/vreg.h>
#include <linux/platform_device.h>
#include <linux/mmc/sdio_ids.h>

#include "proc_comm.h"
#include "board-galaxy.h"

int __init msm_add_sdcc(unsigned int controller, struct mmc_platform_data *plat,
						unsigned int stat_irq, unsigned long stat_irq_flags);

static unsigned sdcc_cfg_data[][6] = {
	/* SDC1 configs */
	{
	GPIO_CFG(51, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA),
	GPIO_CFG(52, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA),
	GPIO_CFG(53, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA),
	GPIO_CFG(54, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA),
	GPIO_CFG(55, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA),
	GPIO_CFG(56, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_4MA),
	},
	/* SDC2 configs */
	{
	GPIO_CFG(62, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA),
	GPIO_CFG(63, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(64, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(65, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(66, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(67, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	},
	{
	/* SDC3 configs */
	GPIO_CFG(88, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA),
	GPIO_CFG(89, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(90, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(91, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(92, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(93, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	},
};

extern int bcm4325_set_core_power(unsigned bcm_core, unsigned pow_on);
static unsigned long vreg_sts, gpio_sts;
static struct vreg *vreg_mmc, *vreg_movinand;

static void msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int i, rc;

	if (dev_id ==1)
	{
		if (!(test_bit(dev_id, &gpio_sts)^enable)) {
    			printk("sdcc_setup_gpio test_bit error....\n");
			return;
		}

		if (enable)
			set_bit(dev_id, &gpio_sts);
		else
			clear_bit(dev_id, &gpio_sts);
	}

	for (i = 0; i < ARRAY_SIZE(sdcc_cfg_data[dev_id - 1]); i++) {
		rc = gpio_tlmm_config(sdcc_cfg_data[dev_id - 1][i],
			enable ? GPIO_ENABLE : GPIO_DISABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, sdcc_cfg_data[dev_id - 1][i], rc);
		}
	}
}

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

//	printk("[MMC] %s @@@@@@\n", __func__);
	pdev = container_of(dv, struct platform_device, dev);
	msm_sdcc_setup_gpio(pdev->id, !!vdd);

	if (vdd == 0) {
		if (!vreg_sts)
			return 0;

		clear_bit(pdev->id, &vreg_sts);

		if (!vreg_sts) {
			vreg_disable(vreg_mmc);
		}
		return 0;
	}

	if (!vreg_sts) {
		rc = vreg_set_level(vreg_mmc, 2850);
		if (!rc)
			rc = vreg_enable(vreg_mmc);
		if (rc)
			printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
	}
	set_bit(pdev->id, &vreg_sts);
	return 0;
}

static uint32_t galaxy_movinand_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;


	pdev = container_of(dv, struct platform_device, dev);
	msm_sdcc_setup_gpio(pdev->id, !!vdd);

	if (vdd == 0) {

		rc = vreg_disable(vreg_movinand);
		if (rc)
			printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
		return 0;
	}

	rc = vreg_set_level(vreg_movinand, 3050);
	if (!rc)
		rc = vreg_enable(vreg_movinand);
	else
		printk(KERN_ERR "%s: return val: %d \n",
				__func__, rc);
	return 0;
}

static unsigned int galaxy_sdcc_slot_status(struct device *dev)
{
	unsigned int status;

	status = (unsigned) gpio_get_value(GPIO_FLASH_DETECT);
	return !status;
}

static struct mmc_platform_data galaxy_sdcc_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.status         = galaxy_sdcc_slot_status,
};

static struct mmc_platform_data movinand_sdcc_data = {
	.ocr_mask	= MMC_VDD_30_31,
	.translate_vdd	= galaxy_movinand_setup_power,
};

static uint32_t wifi_on_gpio_table[] = {			// needed to set GPIO. but...
	GPIO_CFG(62, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), /* CLK */
	GPIO_CFG(63, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), /* CMD */
	GPIO_CFG(64, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), /* DAT3 */
	GPIO_CFG(65, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), /* DAT2 */
	GPIO_CFG(66, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), /* DAT1 */
	GPIO_CFG(67, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), /* DAT0 */
	GPIO_CFG(GPIO_WLAN_HOST_WAKE, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),  /* WLAN_HOST_WAKE	*/
#if !defined(CONFIG_MACH_GALAXY_REV05)
	GPIO_CFG(20, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_4MA),  /* WLAN_HOST_WAKE	*/
#endif
};

static uint32_t wifi_off_gpio_table[] = {
	GPIO_CFG(62, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), /* CLK */
	GPIO_CFG(63, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), /* CMD */
	GPIO_CFG(64, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), /* DAT3 */
	GPIO_CFG(65, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), /* DAT2 */
	GPIO_CFG(66, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), /* DAT1 */
	GPIO_CFG(67, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), /* DAT0 */
	GPIO_CFG(GPIO_WLAN_HOST_WAKE, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),  /* WLAN_HOST_WAKE */
#if !defined(CONFIG_MACH_GALAXY_REV05)
	GPIO_CFG(20, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_4MA),  /* WLAN IRQ */
#endif
};

static int wifi_cd = 0;           /* WIFI virtual 'card detect' status */
//static struct vreg *vreg_wifi_osc;      /* WIFI 32khz oscilator */

// dh0421.hwang for wifi
static unsigned int wifi_status(struct device *dev)
{
	return wifi_cd;			// wifi 'card detect' status return
}

static void (*wifi_status_cb)(int card_present, void *dev_id);

static void *wifi_status_cb_devid;

static int wifi_status_register(void (*callback)(int card_present, void *dev_id), void *dev_id)
{
//	printk("%s:%d is called.....\n", __func__, __LINE__);
	if(wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static struct sdio_embedded_func wifi_func[] = {
         {SDIO_CLASS_WLAN, 64},
	 {SDIO_CLASS_WLAN, 64},
};

static struct embedded_sdio_data wifi_emb_data = {
        .cis    = {
                .vendor         = 0x02d0,
                .device         = 0x4c6,
                .blksize        = 64,
		/*.max_dtr        = 11000000,*/
                .max_dtr        =   48000000,
        },
        .cccr   = {
		//.sdio_vsn	= 2,
		.multi_block	= 1,
		//.low_speed	= 0,
                .low_speed      = 1,
		//.wide_bus	= 0,
                .wide_bus       = 1,
		.high_power	= 1,
		.high_speed	= 1,
                //.high_speed     = 0,
        },
        .funcs  = &wifi_func,
        .num_funcs = 2,
};

static struct mmc_platform_data wifi_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.status		= wifi_status,
	.register_status_notify	= wifi_status_register,
	//.embedded_sdio	= &wifi_emb_data,
	.built_in = 1,
};

int galaxy_wifi_set_carddetect(int val)
{
	wifi_cd = val;
	if(wifi_status_cb)
	{
		wifi_status_cb(val, wifi_status_cb_devid);
	}
	else
	{
		printk("%s: Nobody to notify\n", __func__);
	}
	return 0;
}
EXPORT_SYMBOL(galaxy_wifi_set_carddetect);

static int wifi_power_state;


#define BCM4325_BT 0
#define BCM4325_WLAN 1
int galaxy_wifi_card_power(int on)
{
	printk("%s: %d\n", __func__, on);
    
    if(on) {
		config_gpio_table(wifi_on_gpio_table, ARRAY_SIZE(wifi_on_gpio_table));
		bcm4325_set_core_power(BCM4325_WLAN, 1);  /* dgahn.bcm_mutex */
    }
    else
    {
		config_gpio_table(wifi_off_gpio_table, ARRAY_SIZE(wifi_off_gpio_table));
		bcm4325_set_core_power(BCM4325_WLAN, 0);  /* dgahn.bcm_mutex */
    }
    
    return 0;
}
EXPORT_SYMBOL(galaxy_wifi_card_power);

int wifi_power(int on)
{

	printk("%s: %d\n", __func__, on);

	/* Power on the BCM4325 */
	galaxy_wifi_card_power(on);

	/* Do the mmc_rescan */
	galaxy_wifi_set_carddetect(on);

	return 0;
}
EXPORT_SYMBOL(wifi_power);

int galaxy_wifi_reset(int on) {
  // nothing
  return 0;
}

void __init galaxy_init_mmc(void)
{
	int rc = 0;

	vreg_mmc = vreg_get(NULL, "mmc");
	if (IS_ERR(vreg_mmc)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n", __func__, PTR_ERR(vreg_mmc));
		return;
	}

	vreg_movinand = vreg_get(NULL, "wlan");
	if (IS_ERR(vreg_movinand)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n", __func__, PTR_ERR(vreg_movinand));
		return;
	}

	/*if (gpio_request(GPIO_FLASH_DETECT, "sdc1_status_irq"))
		pr_err("failed to request gpio sdc1_status_irq\n");
	rc = gpio_tlmm_config(GPIO_CFG(GPIO_FLASH_DETECT, 0, GPIO_INPUT, GPIO_PULL_UP,
				GPIO_2MA), GPIO_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO %d\n",
			__func__, rc);*/

	/* wake up the system when inserting or removing SD card */
	//set_irq_wake(MSM_GPIO_TO_INT(GPIO_FLASH_DETECT), 1);

	msm_add_sdcc(1, &galaxy_sdcc_data, /*MSM_GPIO_TO_INT(GPIO_FLASH_DETECT)*/ 0 , 0);
	msm_add_sdcc(2, &wifi_data, 0, 0);
	msm_add_sdcc(3, &movinand_sdcc_data, 0, 0);
}
