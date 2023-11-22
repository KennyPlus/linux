/*
* soc.c- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <linux/sys_soc.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timecounter.h>
#include <clocksource/arm_arch_timer.h>
#include <linux/reboot.h>
#include <linux/gpio.h>
#include <asm/barrier.h>
//#include <asm/mach/arch.h>
#include <asm/memory.h>
#include <asm/io.h>
#include <asm/mach/map.h>
#include "gpio.h"
#include "registers.h"
#include "mcm_id.h"
#include "ms_platform.h"
#include "ms_types.h"
#include "_ms_private.h"
#include <asm/pgtable-prot.h>
#include <asm/mmu.h>

/*
 * Architecture ioremap implementation.
 */
#define MT_DEVICE 0
#define MT_DEVICE_NONSHARED 1
#define MT_DEVICE_CACHED 2
#define MT_DEVICE_WC 3

static struct map_desc_aarch64 mstar_io_desc[] = {
	{ IO_PHYS, IO_VIRT, IO_SIZE },
	{ SPI_PHYS, SPI_VIRT, SPI_SIZE },
	{ GIC_PHYS, GIC_VIRT, GIC_SIZE },
	{ IMI_PHYS, IMI_VIRT, IMI_SIZE },
};

void __init map_device(pgd_t *pgd)
{
	int nr;

	nr = ARRAY_SIZE(mstar_io_desc);
	__map_device(pgd, mstar_io_desc, nr);
}

//static const char *mstar_dt_compat[] __initconst = {
//    "sstar,infinity7",
//    NULL,
//};

extern struct ms_chip *ms_chip_get(void);
extern void __init ms_chip_init_default(void);

static int mcm_rw(int index, int ratio, int write);

/*************************************
*        Sstar chip flush function
*************************************/
//static DEFINE_SPINLOCK(mstar_l2prefetch_lock);

static void mstar_uart_disable_line(int line)
{
	if (line == 0) //for debug, do not change
	{
		// UART0_Mode -> X
		//CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT4 | BIT5);
		//CLRREG16(BASE_REG_PMSLEEP_PA + REG_ID_09, BIT11);
	} else if (line == 1) {
		// UART1_Mode -> X
		CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT8 | BIT9);
	} else if (line == 2) {
		// FUART_Mode -> X
		CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT0 | BIT1);
	}
}

static void mstar_uart_enable_line(int line)
{
	if (line == 0) //for debug, do not change
	{
		// UART0_Mode -> PAD_UART0_TX/RX
		//SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT4);
	} else if (line == 1) {
		// UART1_Mode -> PAD_UART1_TX/RX
		SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT8);
	} else if (line == 2) {
		// FUART_Mode -> PAD_FUART_TX/RX
		SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT0);
	}
}

static int mstar_get_device_id(void)
{
	return (int)(INREG16(BASE_REG_PMTOP_PA) & 0x00FF);
	;
}

static int mstar_get_revision(void)
{
	u16 tmp = 0;
	tmp = INREG16((unsigned int)(BASE_REG_PMTOP_PA + REG_ID_67));
	tmp = ((tmp >> 8) & 0x00FF);

	return (tmp + 1);
}

static void mstar_chip_flush_miu_pipe(void)
{
	// I7 don't need to flush the miu pipe
	/*unsigned long dwLockFlag = 0;
	unsigned short dwReadData = 0;

	spin_lock_irqsave(&mstar_l2prefetch_lock, dwLockFlag);
	//toggle the flush miu pipe fire bit
	*(volatile unsigned short *)(0x1F204414 + IO_OFFSET) = 0x10;
	*(volatile unsigned short *)(0x1F204414 + IO_OFFSET) = 0x11;

	do {
		dwReadData =
			*(volatile unsigned short *)(0x1F204440 + IO_OFFSET);
		dwReadData &= BIT12; //Check Status of Flush Pipe Finish

	} while (dwReadData == 0);

	spin_unlock_irqrestore(&mstar_l2prefetch_lock, dwLockFlag);*/
}

static void mstar_chip_flush_STB_and_miu_pipe(void)
{
	mb();
	mstar_chip_flush_miu_pipe();
}

static u64 mstar_phys_to_MIU(u64 x)
{
	return ((x)-MIU0_BASE);
}

static u64 mstar_MIU_to_phys(u64 x)
{
	return ((x) + MIU0_BASE);
}

static u64 mstar_phys_to_IMI(u64 x)
{
	return ((x)-IMI_PHYS);
}

static u64 mstar_IMI_to_phys(u64 x)
{
	return ((x) + IMI_PHYS);
}

static int mstar_phys_check_range_in_MIU(u64 x)
{
	return 1; //need to completed if needed
}

static int mstar_phys_check_range_in_IMI(u64 x)
{
	if ((IMI_PHYS <= x) && (x <= (IMI_PHYS + IMI_SIZE))) {
		return 1;
	} else {
		return 0;
	}
}

struct soc_device_attribute mstar_soc_dev_attr;

extern const struct of_device_id of_default_bus_match_table[];

static int mstar_get_storage_type(void)
{
//check DIDKEY bank, offset 0x70
#define STORAGE_SPI_NONE 0x00
#define STORAGE_SPI_NAND_SKIP_SD BIT2
#define STORAGE_SPI_NAND BIT4
#define STORAGE_SPI_NOR BIT5
#define STORAGE_SPI_NOR_SKIP_SD BIT1
#define STORAGE_USB BIT12
#define STORAGE_EMMC BIT3 //Boot from eMMC
#define STORAGE_EMMC_4 STORAGE_EMMC
#define STORAGE_EMMC_8                                                         \
	STORAGE_EMMC | BIT7 //emmc_8bit_mode (0: 4-bit mode, 1: 8-bit mode)
#define STORAGE_EMMC_4_SD1                                                     \
	STORAGE_EMMC | BIT10 //emmc 4-bit source (0: SD0, 1: SD1)
#define STORAGE_BOOT_TYPES                                                     \
	(BIT12 | BIT10 | BIT7 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1)

	u16 boot_type =
		(INREG16(BASE_REG_DIDKEY_PA + 0x70 * 4) & STORAGE_BOOT_TYPES);

	if (boot_type == STORAGE_SPI_NAND ||
	    boot_type == STORAGE_SPI_NAND_SKIP_SD) {
		return (int)MS_STORAGE_SPINAND_ECC;
	} else if ((boot_type & STORAGE_EMMC) == STORAGE_EMMC) {
		return (int)MS_STORAGE_EMMC;
	} else if (boot_type == STORAGE_SPI_NOR ||
		   boot_type == STORAGE_SPI_NOR_SKIP_SD) {
		return (int)MS_STORAGE_NOR;
	} else {
		return (int)MS_STORAGE_UNKNOWN;
	}
}

static int mstar_get_package_type(void)
{
#if 0 // FIXME
    if(!strncmp(&mstar_soc_dev_attr.machine[11], "SSC012B-S01A", 12))
        return MS_I6E_PACKAGE_QFN_DDR3;
    else if(!strncmp(&mstar_soc_dev_attr.machine[11], "SSC013A-S01A", 12))
        return MS_I6E_PACKAGE_BGA_LPDDR2;
    else if(!strncmp(&mstar_soc_dev_attr.machine[11], "SSC015A-S01A", 12))
        return MS_I6E_PACKAGE_BGA2_DDR3;
    else if(!strncmp(&mstar_soc_dev_attr.machine[11], "FPGA", 4))
        return MS_I6E_PACKAGE_FPGA_128MB;
    else
#endif
	{
		printk(KERN_ERR "!!!!! Machine name [%s] \n",
		       mstar_soc_dev_attr.machine);
		return MS_I6_PACKAGE_UNKNOWN;
	}
}
static char mstar_platform_name[] = CONFIG_SSTAR_SHORT_NAME;

char *mstar_get_platform_name(void)
{
	return mstar_platform_name;
}

static unsigned long long mstar_chip_get_riu_phys(void)
{
	return IO_PHYS;
}

static int mstar_chip_get_riu_size(void)
{
	return IO_SIZE;
}

static int mstar_ir_enable(int param)
{
	printk(KERN_ERR "NOT YET IMPLEMENTED!![%s]", __FUNCTION__);
	return 0;
}

static int mstar_usb_vbus_control(int param)
{
	int ret;
	//int package = mstar_get_package_type();
	int power_en_gpio = -1;

	struct device_node *np;
	int pin_data;
	int port_num = param >> 16;
	int vbus_enable = param & 0xFF;
	if ((vbus_enable < 0 || vbus_enable > 1) &&
	    (port_num < 0 || port_num > 1)) {
		printk(KERN_ERR "[%s] param invalid:%d %d\n", __FUNCTION__,
		       port_num, vbus_enable);
		return -EINVAL;
	}

	if (power_en_gpio < 0) {
		if (0 == port_num) {
			np = of_find_node_by_path("/soc/Sstar-ehci-1");
		} else {
			np = of_find_node_by_path("/soc/Sstar-ehci-2");
		}

		if (!of_property_read_u32(np, "power-enable-pad", &pin_data)) {
			printk(KERN_ERR
			       "Get power-enable-pad from DTS GPIO(%d)\n",
			       pin_data);
			power_en_gpio = (unsigned char)pin_data;
		} else {
			printk(KERN_ERR
			       "Can't get power-enable-pad from DTS, set default GPIO(%d)\n",
			       pin_data);
			power_en_gpio = PAD_SR_GPIO2;
		}
		ret = gpio_request(power_en_gpio, "USB0-power-enable");
		if (ret < 0) {
			printk(KERN_INFO
			       "Failed to request USB0-power-enable GPIO(%d)\n",
			       power_en_gpio);
			power_en_gpio = -1;
			return ret;
		}
	}

	if (0 == vbus_enable) //disable vbus
	{
		gpio_direction_output(power_en_gpio, 0);
		printk(KERN_INFO "[%s] Disable USB VBUS GPIO(%d)\n",
		       __FUNCTION__, power_en_gpio);
	} else if (1 == vbus_enable) {
		gpio_direction_output(power_en_gpio, 1);
		printk(KERN_INFO "[%s] Enable USB VBUS GPIO(%d)\n",
		       __FUNCTION__, power_en_gpio);
	}
	return 0;
}
static u64 us_ticks_cycle_offset = 0;
static u64 us_ticks_factor = 1;

static u64 mstar_chip_get_us_ticks(void)
{
	u64 cycles = arch_timer_read_counter();
	u64 usticks = div64_u64(cycles, us_ticks_factor);
	return usticks;
}

void mstar_reset_us_ticks_cycle_offset(void)
{
	us_ticks_cycle_offset = arch_timer_read_counter();
}

static int mstar_chip_function_set(int function_id, int param)
{
	int res = -1;

	printk("CHIP_FUNCTION SET. ID=%d, param=%d\n", function_id, param);
	switch (function_id) {
	case CHIP_FUNC_UART_ENABLE_LINE:
		mstar_uart_enable_line(param);
		break;
	case CHIP_FUNC_UART_DISABLE_LINE:
		mstar_uart_disable_line(param);
		break;
	case CHIP_FUNC_IR_ENABLE:
		mstar_ir_enable(param);
		break;
	case CHIP_FUNC_USB_VBUS_CONTROL:
		mstar_usb_vbus_control(param);
		break;
	case CHIP_FUNC_MCM_DISABLE_ID:
		mcm_rw(param, 0, 1);
		break;
	case CHIP_FUNC_MCM_ENABLE_ID:
		mcm_rw(param, 15, 1);
		break;
	default:
		printk(KERN_ERR "Unsupport CHIP_FUNCTION!! ID=%d\n",
		       function_id);
	}

	return res;
}

static int __init mstar_init_early(void)
{
	struct ms_chip *chip = NULL;
	ms_chip_init_default();
	chip = ms_chip_get();

	//enable axi exclusive access
	*(volatile unsigned short *)(0x1F204414 + IO_OFFSET) = 0x10;

	chip->chip_flush_miu_pipe = mstar_chip_flush_STB_and_miu_pipe; //dsb
	chip->chip_flush_miu_pipe_nodsb =
		mstar_chip_flush_miu_pipe; //nodsbchip->phys_to_miu=mstar_phys_to_MIU;
	chip->phys_to_miu = mstar_phys_to_MIU;
	chip->miu_to_phys = mstar_MIU_to_phys;
	chip->phys_to_imi = mstar_phys_to_IMI;
	chip->imi_to_phys = mstar_IMI_to_phys;
	chip->phys_check_range_in_miu = mstar_phys_check_range_in_MIU;
	chip->phys_check_range_in_imi = mstar_phys_check_range_in_IMI;
	chip->chip_get_device_id = mstar_get_device_id;
	chip->chip_get_revision = mstar_get_revision;
	chip->chip_get_platform_name = mstar_get_platform_name;
	chip->chip_get_riu_phys = mstar_chip_get_riu_phys;
	chip->chip_get_riu_size = mstar_chip_get_riu_size;

	chip->chip_function_set = mstar_chip_function_set;
	chip->chip_get_storage_type = mstar_get_storage_type;
	chip->chip_get_package_type = mstar_get_package_type;
	chip->chip_get_us_ticks = mstar_chip_get_us_ticks;
	return 0;
}
early_initcall(mstar_init_early);

extern char *LX_VERSION;
struct device *parent = NULL;
static int __init mstar_init_machine(void)
{
	struct soc_device *soc_dev;

	pr_info("\n\nVersion : %s\n\n", LX_VERSION);

	mstar_reset_us_ticks_cycle_offset();
	us_ticks_factor = div64_u64(arch_timer_get_rate(), 1000000);

	mstar_soc_dev_attr.family = kasprintf(GFP_KERNEL, mstar_platform_name);
	mstar_soc_dev_attr.revision =
		kasprintf(GFP_KERNEL, "%d", mstar_get_revision());
	mstar_soc_dev_attr.soc_id =
		kasprintf(GFP_KERNEL, "%u", mstar_get_device_id());
	mstar_soc_dev_attr.api_version =
		kasprintf(GFP_KERNEL, ms_chip_get()->chip_get_API_version());
	mstar_soc_dev_attr.machine =
		kasprintf(GFP_KERNEL, of_flat_dt_get_machine_name());

	soc_dev = soc_device_register(&mstar_soc_dev_attr);
	if (IS_ERR(soc_dev)) {
		pr_info("%s:%d:soc device register fail.\n", __func__,
			__LINE__);
		kfree((void *)mstar_soc_dev_attr.family);
		kfree((void *)mstar_soc_dev_attr.revision);
		kfree((void *)mstar_soc_dev_attr.soc_id);
		kfree((void *)mstar_soc_dev_attr.machine);
		goto out;
	}

	parent = soc_device_to_device(soc_dev);
	if (NULL == parent) {
		pr_info("%s:%d:soc device parent is null.\n", __func__,
			__LINE__);
	}

out:
	//write log_buf address to mailbox
	OUTREG16(BASE_REG_MAILBOX_PA + BK_REG(0x08),
		 (long)log_buf_addr_get() & 0xFFFF);
	OUTREG16(BASE_REG_MAILBOX_PA + BK_REG(0x09),
		 ((long)log_buf_addr_get() >> 16) & 0xFFFF);
	return 0;
}
device_initcall(mstar_init_machine);

struct mcm_client {
	char *name;
	short index;
	unsigned int reg;
	short slow_down_ratio;
};

static struct mcm_client mcm_clients[] = {
	{ "MCU51", MCM_ID_MCU51, OFFSET_MCM_ID_MCU51, 0 },
	{ "URDMA", MCM_ID_URDMA, OFFSET_MCM_ID_URDMA, 0 },
	{ "BDMA", MCM_ID_BDMA, OFFSET_MCM_ID_BDMA, 0 },
	{ "MFE", MCM_ID_MFE, OFFSET_MCM_ID_MFE, 0 },
	{ "JPE", MCM_ID_JPE, OFFSET_MCM_ID_JPE, 0 },
	{ "BACH", MCM_ID_BACH, OFFSET_MCM_ID_BACH, 0 },
	{ "FILE", MCM_ID_FILE, OFFSET_MCM_ID_FILE, 0 },
	{ "UHC0", MCM_ID_UHC0, OFFSET_MCM_ID_UHC0, 0 },
	{ "EMAC", MCM_ID_EMAC, OFFSET_MCM_ID_EMAC, 0 },
	{ "CMDQ", MCM_ID_CMDQ, OFFSET_MCM_ID_CMDQ, 0 },
	{ "ISP_DNR", MCM_ID_ISP_DNR, OFFSET_MCM_ID_ISP_DNR, 0 },
	{ "ISP_DMA", MCM_ID_ISP_DMA, OFFSET_MCM_ID_ISP_DMA, 0 },
	{ "GOP0", MCM_ID_GOP0, OFFSET_MCM_ID_GOP0, 0 },
	{ "SC_DNR", MCM_ID_SC_DNR, OFFSET_MCM_ID_SC_DNR, 0 },
	{ "SC_DNR_SAD", MCM_ID_SC_DNR_SAD, OFFSET_MCM_ID_SC_DNR_SAD, 0 },
	{ "SC_CROP", MCM_ID_SC_CROP, OFFSET_MCM_ID_SC_CROP, 0 },
	{ "SC1_FRM", MCM_ID_SC1_FRM, OFFSET_MCM_ID_SC1_FRM, 0 },
	{ "SC1_SNP", MCM_ID_SC1_SNP, OFFSET_MCM_ID_SC1_SNP, 0 },
	{ "SC1_DBG", MCM_ID_SC1_DBG, OFFSET_MCM_ID_SC1_DBG, 0 },
	{ "SC2_FRM", MCM_ID_SC2_FRM, OFFSET_MCM_ID_SC2_FRM, 0 },
	{ "SC3_FRM", MCM_ID_SC3_FRM, OFFSET_MCM_ID_SC3_FRM, 0 },
	{ "FCIE", MCM_ID_FCIE, OFFSET_MCM_ID_FCIE, 0 },
	{ "SDIO", MCM_ID_SDIO, OFFSET_MCM_ID_SDIO, 0 },
	{ "SC1_SNPI", MCM_ID_SC1_SNPI, OFFSET_MCM_ID_SC1_SNPI, 0 },
	{ "SC2_SNPI", MCM_ID_SC2_SNPI, OFFSET_MCM_ID_SC2_SNPI, 0 },
	{ "CMDQ1", MCM_ID_CMDQ1, OFFSET_MCM_ID_CMDQ1, 0 },
	{ "CMDQ2", MCM_ID_CMDQ2, OFFSET_MCM_ID_CMDQ2, 0 },
	{ "GOP1", MCM_ID_GOP1, OFFSET_MCM_ID_GOP1, 0 },
	{ "GOP2", MCM_ID_GOP2, OFFSET_MCM_ID_GOP2, 0 },
	{ "UHC1", MCM_ID_UHC1, OFFSET_MCM_ID_UHC1, 0 },
	{ "IVE", MCM_ID_IVE, OFFSET_MCM_ID_IVE, 0 },
	{ "VHE", MCM_ID_VHE, OFFSET_MCM_ID_VHE, 0 },
	{ "*ALL_CLIENTS", MCM_ID_ALL, 0, 0 } //use carefully
};

struct device mcm_dev;

static struct bus_type mcm_subsys = {
	.name = "mcm",
	.dev_name = "mcm",
};

static int mcm_rw(int index, int ratio, int write)
{
	int i, addr;

	if (index == MCM_ID_ALL && write) {
		for (i = 0;
		     i < (sizeof(mcm_clients) / sizeof(mcm_clients[0])) - 1;
		     i++)
			mcm_rw(i, ratio, write);

		return 0;
	}

	if ((index >= MCM_ID_START) && (index < MCM_ID_END)) {
		addr = mcm_clients[index].reg;
	} else {
		printk(KERN_ERR "mcm_clients[%d] not exists\n", index);
		return -1;
	}

	if (write)
		OUTREG8(addr, (ratio << 4));
	else
		mcm_clients[index].slow_down_ratio = (INREG8(addr) >> 4);

	return 0;
}

static ssize_t mcm_slow_ratio_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t n)
{
	int index, ratio;
	sscanf(buf, "%d %d", &index, &ratio);

	if (0 > ratio || ratio > 15) {
		printk(KERN_ERR "MCM slow down ratio should be 0~15\n");
		return -1;
	}

	return mcm_rw(index, ratio, 1) ? -1 : n;
}

static ssize_t mcm_slow_ratio_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	char *str = buf;
	char *end = buf + PAGE_SIZE;
	int i = 0;

	for (i = 0; i < (sizeof(mcm_clients) / sizeof(mcm_clients[0])) - 1;
	     i++) {
		mcm_rw(i, 0, 0);
		str += scnprintf(str, end - str, "[%d] %s: %d\n",
				 mcm_clients[i].index, mcm_clients[i].name,
				 mcm_clients[i].slow_down_ratio);
	}
	str += scnprintf(str, end - str, "[%d] %s\n", mcm_clients[i].index,
			 mcm_clients[i].name);

	return (str - buf);
}

DEVICE_ATTR(mcm_slow_ratio, 0644, mcm_slow_ratio_show, mcm_slow_ratio_store);

static void __init mstar_create_MCM_node(void)
{
	int ret;

	mcm_dev.kobj.name = "mcm";
	mcm_dev.bus = &mcm_subsys;

	ret = subsys_system_register(&mcm_subsys, NULL);
	if (ret) {
		printk(KERN_ERR "Failed to register MCM sub system!! %d\n",
		       ret);
		return;
	}

	ret = device_register(&mcm_dev);
	if (ret) {
		printk(KERN_ERR "Failed to register MCM device!! %d\n", ret);
		return;
	}

	device_create_file(&mcm_dev, &dev_attr_mcm_slow_ratio);
}

extern void mstar_create_MIU_node(void);
extern int mstar_pm_init(void);
static inline int __init mstar_init_late(void)
{
#ifdef CONFIG_PM_SLEEP
	mstar_pm_init();
#endif
	mstar_create_MCM_node();
	return 0;
}
late_initcall(mstar_init_late);

extern void msys_set_rebootType(const char *cmd);
static void mstar_global_reset(enum reboot_mode mode, const char *cmd)
{
	U16 i = 0;

	//riu_isp
	OUTREG16(0x1f001000, 0xAAAA);
	//password
	do {
		i++;
		OUTREG16(0x1f001004, 0x0005); //cmd
		OUTREG16(0x1f001030, 0x0001); //trigger
		while ((INREG16(0x1f001054) & 0x1) !=
		       0x1) //check read data ready
			;

		if (Chip_Get_Storage_Type() != MS_STORAGE_NOR) //if no nor-flash
			break;
	} while ((INREG16(0x1f001014) & 0x1) != 0x0); //check WIP=0

	msys_set_rebootType(cmd);

	while (1) {
		OUTREG8(0x1f221000, 0x30 + i);
		mdelay(5);
		OUTREG8(0x1f001cb8,
			0x79); //reg_top_sw_rst,fire reset for global chip, active by password
	}
}

static int mstar_reset_restart_handler(struct notifier_block *nb,
				       unsigned long action, void *data)
{
	mstar_global_reset(action, data);
	return NOTIFY_DONE;
}

static struct notifier_block mstar_restart_nb = {
	.notifier_call = mstar_reset_restart_handler,
	.priority = 128,
};

static int __init mstar_reset_init(void)
{
	return register_restart_handler(&mstar_restart_nb);
}

/*
 * RCU is system core entity which is in Always On Domain whose clocks
 * or resource initialization happens in system core initialization.
 * Also, it is required for most of the platform or architecture
 * specific devices to perform reset operation as part of initialization.
 * So perform RCU as post core initialization.
 */
postcore_initcall(mstar_reset_init);
