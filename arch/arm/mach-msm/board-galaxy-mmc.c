#include <linux/delay.h>
#include <linux/err.h>
#include <asm/gpio.h>
#include <asm/mach/mmc.h>
#include <mach/vreg.h>
#include <linux/platform_device.h>
#include "proc_comm.h"
#include "board-galaxy.h"

int __init msm_add_sdcc(unsigned int controller, struct mmc_platform_data *plat,
						unsigned int stat_irq, unsigned long stat_irq_flags);

static struct vreg *vreg_mmc;

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

//void config_gpio_table(uint32_t *table, int len)
//{
//	int n, rc;
//	for (n = 0; n < len; n++) {
//		rc = gpio_tlmm_config(table[n], GPIO_ENABLE);
//		if (rc) {
//			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
//				__func__, table[n], rc);
//			break;
//		}
//	}
//}

static unsigned long vreg_sts, gpio_sts;
static struct vreg *vreg_mmc, *vreg_movinand;

static void msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int i, rc;

	if (dev_id ==1)
	{
		if (!(test_bit(dev_id, &gpio_sts)^enable)) {
    			//printk("sdcc_setup_gpio test_bit error....\n");
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

static void sdcc_gpio_init(void)
{
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	int rc = 0;
	if (gpio_request(GPIO_FLASH_DETECT, "sdc1_status_irq"))
		pr_err("failed to request gpio sdc1_status_irq\n");
	rc = gpio_tlmm_config(GPIO_CFG(GPIO_FLASH_DETECT, 0, GPIO_INPUT, GPIO_PULL_UP,
				GPIO_2MA), GPIO_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO %d\n",
				__func__, rc);
#endif
	/* SDC1 GPIOs */
	if (gpio_request(51, "sdc1_data_3"))
		pr_err("failed to request gpio sdc1_data_3\n");
	if (gpio_request(52, "sdc1_data_2"))
		pr_err("failed to request gpio sdc1_data_2\n");
	if (gpio_request(53, "sdc1_data_1"))
		pr_err("failed to request gpio sdc1_data_1\n");
	if (gpio_request(54, "sdc1_data_0"))
		pr_err("failed to request gpio sdc1_data_0\n");
	if (gpio_request(55, "sdc1_cmd"))
		pr_err("failed to request gpio sdc1_cmd\n");
	if (gpio_request(56, "sdc1_clk"))
		pr_err("failed to request gpio sdc1_clk\n");

	/* SDC2 GPIOs */
	if (gpio_request(62, "sdc2_clk"))
		pr_err("failed to request gpio sdc2_clk\n");
	if (gpio_request(63, "sdc2_cmd"))
		pr_err("failed to request gpio sdc2_cmd\n");
	if (gpio_request(64, "sdc2_data_3"))
		pr_err("failed to request gpio sdc2_data_3\n");
	if (gpio_request(65, "sdc2_data_2"))
		pr_err("failed to request gpio sdc2_data_2\n");
	if (gpio_request(66, "sdc2_data_1"))
		pr_err("failed to request gpio sdc2_data_1\n");
	if (gpio_request(67, "sdc2_data_0"))
		pr_err("failed to request gpio sdc2_data_0\n");

	/* SDC3 GPIOs */
	if (gpio_request(88, "sdc3_clk"))
		pr_err("failed to request gpio sdc2_clk\n");
	if (gpio_request(89, "sdc3_cmd"))
		pr_err("failed to request gpio sdc2_cmd\n");
	if (gpio_request(90, "sdc3_data_3"))
		pr_err("failed to request gpio sdc2_data_3\n");
	if (gpio_request(91, "sdc3_data_2"))
		pr_err("failed to request gpio sdc2_data_2\n");
	if (gpio_request(92, "sdc3_data_1"))
		pr_err("failed to request gpio sdc2_data_1\n");
	if (gpio_request(93, "sdc3_data_0"))
		pr_err("failed to request gpio sdc2_data_0\n");
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

#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
static unsigned int galaxy_sdcc_slot_status(struct device *dev)
{
	int rc;
	rc = gpio_get_value(O_FLASH_DETECT);

	rc = rc?0:1 ;
	return rc;
}
#endif

static struct mmc_platform_data galaxy_sdcc_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status         = galaxy_sdcc_slot_status,
	.status_irq	= MSM_GPIO_TO_INT(GPIO_FLASH_DETECT),
	.irq_flags      = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
#endif
};

static struct mmc_platform_data movinand_sdcc_data = {
	.ocr_mask	= MMC_VDD_30_31,
	.translate_vdd	= galaxy_movinand_setup_power,
};

void __init galaxy_init_mmc(void)
{
	vreg_mmc = vreg_get(NULL, "mmc");
	if (IS_ERR(vreg_mmc)) {
		//printk(KERN_ERR "%s: vreg get failed ($ld)\n", __func__, PTR_ERR(vreg_mmc));
		return;
	}

	vreg_movinand = vreg_get(NULL, "wlan");
	if (IS_ERR(vreg_movinand)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n", __func__, PTR_ERR(vreg_movinand));
		return;
	}

	/* wake up the system when inserting or removing SD card */
	set_irq_wake(MSM_GPIO_TO_INT(GPIO_FLASH_DETECT), 1);

	sdcc_gpio_init();

	msm_add_sdcc(3, &movinand_sdcc_data, 0, 0);
	msm_add_sdcc(1, &galaxy_sdcc_data, 0, 0);
	//msm_add_sdcc(2, &wifi_data, 0, 0);
}
