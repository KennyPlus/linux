/*
 * phy-sstar-u2phy.c - Sigmastar
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
#include <linux/debugfs.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include "phy-sstar-u2phy.h"
//#include <io.h>

// 0x00: 550mv, 0x20: 575, 0x40: 600, 0x60: 625

#define UTMI_DISCON_LEVEL_2A (0x62)
static uint tx_swing = 0x0;
module_param(tx_swing, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tx_swing, "driver current, in decimal format");

static uint dem_cur = 0x0;
module_param(dem_cur, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dem_cur, "dem current, in decimal format");

static uint cm_cur = 0x0;
module_param(cm_cur, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(cm_cur, "VCM current, in decimal format");

static int sstar_u2phy_utmi_cm_cur_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;

    unsigned long flags;
    u32           cm_cur;
    int           bit_masks;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x17 << 2), &cm_cur);
    spin_unlock_irqrestore(&priv->lock, flags);

    bit_masks = BIT(3) | BIT(4) | BIT(5);
    cm_cur    = (bit_masks & cm_cur) >> 3;
    switch (cm_cur)
    {
        case 0x03:
            cm_cur = 105;
            break;
        case 0x02:
            cm_cur = 110;
            break;
        case 0x01:
        case 0x07:
            cm_cur = 115;
            break;
        case 0x00:
        case 0x06:
            cm_cur = 120;
            break;
        case 0x05:
            cm_cur = 125;
            break;
        case 0x04:
            cm_cur = 130;
            break;
        default:
            seq_printf(s, "cm_current: unknown\r\n");
            return 0;
    }
    seq_printf(s, "cm_current: %d%%\r\n", cm_cur);
    return 0;
}

static int sstar_u2phy_utmi_cm_cur_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_cm_cur_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_cm_cur_set(struct sstar_u2phy *priv, unsigned int cm_cur)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;
    int            bit_masks;

    switch (cm_cur)
    {
        case 105:
            cm_cur = 0x03;
            break;
        case 110:
            cm_cur = 0x02;
            break;
        case 115:
            cm_cur = 0x01;
            break;
        case 120:
            cm_cur = 0x00;
            break;
        case 125:
            cm_cur = 0x05;
            break;
        case 130:
            cm_cur = 0x04;
            break;
        default:
            dev_err(&priv->phy->dev, "Unsupported option: %d\n", cm_cur);
            dev_err(&priv->phy->dev, "CM Current option should be: 105/110/115/120/125/130\n");
            return -EINVAL;
    }

    spin_lock_irqsave(&priv->lock, flags);
    bit_masks = BIT(3) | BIT(4) | BIT(5);
    regmap_update_bits(utmi, (0x17 << 2), bit_masks, (cm_cur << 3));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_cm_cur_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 cm_cur;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &cm_cur);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_cm_cur_set(priv, cm_cur);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_cm_cur_fops = {
    .open    = sstar_u2phy_utmi_cm_cur_open,
    .write   = sstar_u2phy_utmi_cm_cur_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_dem_cur_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;

    unsigned long flags;
    u32           dem_cur;
    int           bit_masks;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x16 << 2), &dem_cur);
    spin_unlock_irqrestore(&priv->lock, flags);

    bit_masks = (BIT(7) | BIT(8) | BIT(9));
    dem_cur   = (bit_masks & dem_cur) >> 7;
    switch (dem_cur)
    {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            dem_cur = 100;
            break;
        case 0x04:
            dem_cur = 105;
            break;
        case 0x05:
            dem_cur = 110;
            break;
        case 0x06:
            dem_cur = 115;
            break;
        case 0x07:
            dem_cur = 120;
            break;
        default:
            seq_printf(s, "de_emphasis_current: unknown\r\n");
            return 0;
    }
    seq_printf(s, "de_emphasis_current: %d%%\r\n", dem_cur);
    return 0;
}

static int sstar_u2phy_utmi_dem_cur_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_dem_cur_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_dem_cur_set(struct sstar_u2phy *priv, unsigned int dem_cur)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;
    int            bit_masks;

    switch (dem_cur)
    {
        case 100:
            dem_cur = 0x00;
            break;
        case 105:
            dem_cur = 0x04;
            break;
        case 110:
            dem_cur = 0x05;
            break;
        case 115:
            dem_cur = 0x06;
            break;
        case 120:
            dem_cur = 0x07;
            break;
        default:
            dev_err(&priv->phy->dev, "Unsupported option: %d\n", dem_cur);
            dev_err(&priv->phy->dev, "De-emphasis Current option should be: 100/105/110/115/120\n");
            return -EINVAL;
    }

    spin_lock_irqsave(&priv->lock, flags);
    bit_masks = (BIT(7) | BIT(8) | BIT(9));
    regmap_update_bits(utmi, (0x16 << 2), bit_masks, (dem_cur << 7));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_dem_cur_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 dem_cur;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &dem_cur);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_dem_cur_set(priv, dem_cur);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_dem_cur_fops = {
    .open    = sstar_u2phy_utmi_dem_cur_open,
    .write   = sstar_u2phy_utmi_dem_cur_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_tx_swing_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;

    unsigned long flags;
    u32           tx_swing;
    int           bit_masks;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x16 << 2), &tx_swing);
    spin_unlock_irqrestore(&priv->lock, flags);
    bit_masks = (BIT(4) | BIT(5) | BIT(6));
    tx_swing  = (bit_masks & tx_swing) >> 4;

    switch (tx_swing)
    {
        case 0x04:
            tx_swing = 80;
            break;
        case 0x05:
            tx_swing = 85;
            break;
        case 0x06:
            tx_swing = 90;
            break;
        case 0x07:
            tx_swing = 95;
            break;
        case 0x00:
            tx_swing = 100;
            break;
        case 0x01:
            tx_swing = 105;
            break;
        case 0x02:
            tx_swing = 110;
            break;
        case 0x03:
            tx_swing = 115;
            break;
        default:
            seq_printf(s, "tx_swing: unknown\r\n");
            return 0;
    }

    seq_printf(s, "tx_swing: %d%%\r\n", tx_swing);
    return 0;
}

static int sstar_u2phy_utmi_tx_swing_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_tx_swing_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_tx_swing_set(struct sstar_u2phy *priv, unsigned int tx_swing)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;
    int            bit_masks;

    switch (tx_swing)
    {
        case 80:
            tx_swing = 0x04;
            break;
        case 85:
            tx_swing = 0x05;
            break;
        case 90:
            tx_swing = 0x06;
            break;
        case 95:
            tx_swing = 0x07;
            break;
        case 100:
            tx_swing = 0x00;
            break;
        case 105:
            tx_swing = 0x01;
            break;
        case 110:
            tx_swing = 0x02;
            break;
        case 115:
            tx_swing = 0x03;
            break;
        default:
            dev_err(&priv->phy->dev, "Unsupported option: %d\n", tx_swing);
            dev_err(&priv->phy->dev, "Main Current(TX swing) option should be: 80/85/90/95/100/105/110/115\n");
            return -EINVAL;
    }

    spin_lock_irqsave(&priv->lock, flags);
    bit_masks = (BIT(4) | BIT(5) | BIT(6));
    regmap_update_bits(utmi, (0x16 << 2), bit_masks, (tx_swing << 4));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_tx_swing_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 tx_swing;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &tx_swing);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_tx_swing_set(priv, tx_swing);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_tx_swing_fops = {
    .open    = sstar_u2phy_utmi_tx_swing_open,
    .write   = sstar_u2phy_utmi_tx_swing_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

void sstar_u2phy_utmi_debugfs_init(struct sstar_u2phy *priv)
{
    spin_lock_init(&priv->lock);
    priv->root = debugfs_create_dir(dev_name(&priv->phy->dev), usb_debug_root);

    debugfs_create_file("tx_swing", 0644, priv->root, priv, &sstar_tx_swing_fops);
    debugfs_create_file("de_emphasis_current", 0644, priv->root, priv, &sstar_dem_cur_fops);
    debugfs_create_file("cm_current", 0644, priv->root, priv, &sstar_cm_cur_fops);
}

void sstar_u2phy_utmi_debugfs_exit(struct sstar_u2phy *priv)
{
    debugfs_remove_recursive(priv->root);
}

static void sstar_u2phy_utmi_eye_diagram_setting(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;
    u32                 val[3];

    if (of_property_read_u32_array(phy->dev.of_node, "tx-swing,de-emphasis,cm-current", val, ARRAY_SIZE(val)))
    {
        /* Default UTMI eye diagram parameter setting */
        regmap_write(utmi, (0x16 << 2), 0x0210);
        regmap_write(utmi, (0x17 << 2), 0x8100);
        dev_info(&phy->dev, "Default UTMI eye diagram parameter setting\n");
        return;
    }

    if (tx_swing != 0)
        val[0] = tx_swing;

    if (dem_cur != 0)
        val[1] = dem_cur;

    if (cm_cur != 0)
        val[2] = cm_cur;

    dev_info(&phy->dev, "Tx-swing:%d%% De-emphasis:%d%% CM-current:%d%%\n", val[0], val[1], val[2]);
    sstar_u2phy_utmi_tx_swing_set(priv, val[0]);
    sstar_u2phy_utmi_dem_cur_set(priv, val[1]);
    sstar_u2phy_utmi_cm_cur_set(priv, val[2]);
    regmap_set_bits(utmi, (0x17 << 2), BIT(15) | BIT(8));
    return;
}

static void sstar_u2phy_utmi_avoid_floating(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /* DP_PUEN = 0 DM_PUEN = 0 R_PUMODE = 0 */
    regmap_clear_bits(utmi, (0x0 << 2), (BIT(3) | BIT(4) | BIT(5)));

    /*
     * patch for DM always keep high issue
     * init overwrite register
     */
    regmap_set_bits(utmi, (0x05 << 2), BIT(6));   // hs_txser_en_cb = 1
    regmap_clear_bits(utmi, (0x05 << 2), BIT(7)); // hs_se0_cb = 0

    /* Turn on overwirte mode for D+/D- floating issue when UHC reset
     * Before UHC reset, R_DP_PDEN = 1, R_DM_PDEN = 1, tern_ov = 1 */
    regmap_set_bits(utmi, (0x0 << 2), (BIT(7) | BIT(6) | BIT(1)));
    /* new HW term overwrite: on */
    regmap_set_bits(utmi, (0x29 << 2), (BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0)));
}

static void sstar_u2phy_utmi_etron_enable(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;
    struct regmap *     usbc = priv->usbc;

    /* bit<3> for 240's phase as 120's clock set 1, bit<4> for 240Mhz in mac 0 for faraday 1 for etron */
    regmap_set_bits(utmi, (0x04 << 2), BIT(3));

    /* [7]: reg_etron_en, to enable utmi Preamble function */
    regmap_set_bits(utmi, (0x1f << 2), BIT(15));

    /* [11]: reg_preamble_en, to enable Faraday Preamble */
    regmap_set_bits(usbc, (0x07 << 2), BIT(11));

    /* [0]: reg_preamble_babble_fix, to patch Babble occurs in Preamble */
    /* [1]: reg_preamble_fs_within_pre_en, to patch FS crash problem */
    regmap_set_bits(usbc, (0x08 << 2), BIT(1) | BIT(0));
}

static void sstar_u2phy_utmi_disconnect_window_select(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /* Disconnect window select */
    regmap_update_bits(utmi, (0x01 << 2), BIT(13) | BIT(12) | BIT(11), BIT(13) | BIT(11));
}

static void sstar_u2phy_utmi_ISI_effect_improvement(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /* ISI effect improvement */
    regmap_set_bits(utmi, (0x04 << 2), BIT(8));
}

static void sstar_u2phy_utmi_RX_anti_dead_loc(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /* UTMI RX anti-dead-loc */
    regmap_set_bits(utmi, (0x04 << 2), BIT(15));
}

static void sstar_u2phy_utmi_TX_timing_latch_select(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /* TX timing select latch path */
    regmap_set_bits(utmi, (0x05 << 2), BIT(15));
}

static void sstar_u2phy_utmi_chirp_signal_source_select(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /*
     * [13]: Chirp signal source select
     * [14]: change to 55 interface
     */
    regmap_set_bits(utmi, (0x0a << 2), BIT(14) | BIT(13));
}

static void sstar_u2phy_utmi_CDR_control(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /* RX HS CDR stage control */
    regmap_update_bits(utmi, (0x03 << 2), (BIT(6) | BIT(5)), BIT(6));
    /* Disable improved CDR */
    regmap_clear_bits(utmi, (0x03 << 2), BIT(9));
}

static void sstar_u2phy_utmi_new_hw_chirp(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /* new HW chirp design, default overwrite to reg_2A */
    regmap_clear_bits(utmi, (0x20 << 2), BIT(4));

    /* Init UTMI disconnect level setting (UTMI_DISCON_LEVEL_2A: 0x62) */
    regmap_set_bits(utmi, (0x15 << 2), BIT(6) | BIT(5) | BIT(1));
    dev_info(&phy->dev, "Init UTMI disconnect level setting\r\n");
}

static int sstar_u2phy_utmi_calibrate(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    sstar_u2phy_utmi_disconnect_window_select(phy);
    sstar_u2phy_utmi_CDR_control(phy);

    sstar_u2phy_utmi_chirp_signal_source_select(phy);
    sstar_u2phy_utmi_RX_anti_dead_loc(phy);
    sstar_u2phy_utmi_ISI_effect_improvement(phy);
    sstar_u2phy_utmi_TX_timing_latch_select(phy);
    sstar_u2phy_utmi_new_hw_chirp(phy);

    /* Begin: ECO patch */
    /* [2]: reg_fl_sel_override, to override utmi to have FS drive strength */
    regmap_set_bits(utmi, (0x01 << 2), BIT(10));

    /* Enable deglitch SE0 (low-speed cross point) */
    regmap_set_bits(utmi, (0x02 << 2), BIT(6));

    /* Enable hw auto deassert sw reset(tx/rx reset) */
    regmap_set_bits(utmi, (0x02 << 2), BIT(5));

    sstar_u2phy_utmi_etron_enable(phy);
    /* End: ECO patch */
    return 0;
}

static int sstar_u2phy_utmi_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    // struct regmap *     utmi = priv->utmi;
    struct regmap *usbc = priv->usbc;
    struct regmap *ehc  = priv->ehc;

    switch (mode)
    {
        case PHY_MODE_USB_HOST:
            dev_info(&phy->dev, "Set phy mode -> host\n");
            /* UHC select enable */
            regmap_update_bits(usbc, (0x01 << 2), BIT(1) | BIT(0), BIT(0));

            /* [4]: 0:Vbus On, 1:Vbus off */
            /* [3]: Interrupt signal active high*/
            regmap_clear_bits(ehc, (0x20 << 2), BIT(4));
            udelay(1); // delay 1us
            regmap_set_bits(ehc, (0x20 << 2), BIT(3));

            /* improve the efficiency of USB access MIU when system is busy */
            regmap_set_bits(ehc, (0x40 << 2), BIT(15) | BIT(11) | BIT(10) | BIT(9) | BIT(8));

            /* Init UTMI eye diagram parameter setting */
            /*regmap_write(utmi, (0x16 << 2), 0x0210);
            regmap_write(utmi, (0x17 << 2), 0x8100);
            dev_info(&phy->dev, "Init UTMI eye diagram parameter setting\r\n");*/
            sstar_u2phy_utmi_eye_diagram_setting(phy);

            /* ENABLE_UHC_RUN_BIT_ALWAYS_ON_ECO, Don't close RUN bit when device disconnect */
            regmap_set_bits(ehc, (0x1A << 2), BIT(7));

            /* _USB_MIU_WRITE_WAIT_LAST_DONE_Z_PATCH, Enable PVCI i_miwcplt wait for mi2uh_last_done_z */
            regmap_set_bits(ehc, (0x41 << 2), BIT(12));

            /* ENABLE_UHC_EXTRA_HS_SOF_ECO, Extra HS SOF after bus reset */
            regmap_set_bits(ehc, (0x46 << 2), BIT(0));

            break;
        default:
            return -1;
    }
    return 0;
}

static int sstar_u2phy_utmi_reset(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /* [0]: RX sw reset
     * [1]: Tx sw reset
     * [8]: Tx FSM sw reset
     */
    regmap_set_bits(utmi, (0x03 << 2), BIT(8) | BIT(1) | BIT(0));
    /* [12]: pwr good reset */
    regmap_set_bits(utmi, (0x08 << 2), BIT(12));
    mdelay(1);

    /* Clear reset */
    regmap_clear_bits(utmi, (0x03 << 2), BIT(8) | BIT(1) | BIT(0));
    regmap_clear_bits(utmi, (0x08 << 2), BIT(12));

    dev_info(&phy->dev, "%s\n", __func__);
    return 0;
}

static int sstar_u2phy_utmi_power_off(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;

    /* [0]:  1: Enable USB_XCVR power-down control override
     * [8]:  1: Power down USB_XCVR HS de-serializer block
     * [9]:  1: Power down USB_XCVR pll block
     * [10]: 1: Power down USB_XCVR HS TED block
     * [11]: 1: Power down USB_XCVR HS pre-amplifier block
     * [12]: 1: Power down USB_XCVR FS/LS transceiver block
     * [13]: 1: Power down USB_XCVR VBUS detector block
     * [14]: 1: Power down USB_XCVR HS current reference block
     * [15]: 1: Power down USB_XCVR builtin regulator block
     */
    regmap_set_bits(utmi, (0x00 << 2), BIT(14) | BIT(13) | BIT(12) | BIT(11) | BIT(10) | BIT(9) | BIT(8) | BIT(0));
    mdelay(5);

    dev_info(&phy->dev, "%s\n", __func__);
    return 0;
}

static int sstar_u2phy_utmi_power_on(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;
    // struct regmap *bc = priv->bc;

    /* Turn on all use override mode*/
    regmap_write(utmi, (0x0 << 2), 0x0001);

    /*[6]:	reg_into_host_bc_sw_tri*/
    // regmap_clear_bits(bc, (0x06 << 2), BIT(6));

    /*[14]:  reg_host_bc_en*/
    // regmap_clear_bits(bc, (0x01 << 2), BIT(14));

    // regmap_clear_bits(utmi, (0x0 << 2), BIT(14));

    dev_info(&phy->dev, "%s\n", __func__);
    return 0;
}

int sstar_u2phy_utmi_init(struct phy *phy)
{
    struct sstar_u2phy *priv    = phy_get_drvdata(phy);
    struct regmap *     utmi    = priv->utmi;
    struct regmap *     usbc    = priv->usbc;
    u32                 reg_val = 0;

    regmap_write(utmi, (0x0 << 2), 0x0001);

    if (priv->phy_data && priv->phy_data->has_utmi2_bank)
    {
        struct device_node *node;
        struct regmap *     utmi2;

        node = of_parse_phandle(phy->dev.of_node, "syscon-utmi2", 0);
        if (node)
        {
            utmi2 = syscon_node_to_regmap(node);
            if (IS_ERR(utmi2))
            {
                dev_info(&phy->dev, "No extra utmi setting\n");
            }
            else
            {
                regmap_write(utmi2, (0x0 << 2), 0x7F05);
                dev_info(&phy->dev, "Extra utmi2 setting\n");
            }
            of_node_put(node);
        }
    }
    regmap_write(utmi, (0x4 << 2), 0x0C2F);
    sstar_u2phy_utmi_avoid_floating(phy);

    regmap_set_bits(usbc, (0x0 << 2), BIT(3) | BIT(1)); // Disable MAC initial suspend, Reset UHC
    regmap_clear_bits(usbc, (0x0 << 2), BIT(1));        // Release UHC reset
    regmap_set_bits(usbc, (0x0 << 2), BIT(5));          // enable UHC and OTG XIU function

    /* Init UTMI squelch level setting befor CA */
    regmap_update_bits(utmi, (0x15 << 2), 0xFF, UTMI_DISCON_LEVEL_2A & (BIT(3) | BIT(2) | BIT(1) | BIT(0)));
    regmap_read(utmi, (0x15 << 2), &reg_val);
    dev_info(&phy->dev, "squelch level 0x%08x\n", reg_val);

    /* set CA_START as 1 */
    regmap_set_bits(utmi, (0x1e << 2), BIT(0));
    mdelay(1);
    /* release CA_START */
    regmap_clear_bits(utmi, (0x1e << 2), BIT(0));

    /* polling bit <1> (CA_END) */
    do
    {
        regmap_read(utmi, (0x1e << 2), &reg_val);
    } while ((reg_val & BIT(1)) == 0);

    regmap_read(utmi, (0x1e << 2), &reg_val);
    if (0xFFF0 == reg_val || 0x0000 == (reg_val & 0xFFF0))
        dev_info(&phy->dev, "WARNING: CA Fail !! \n");

    /* Turn on overwirte mode for D+/D- floating issue when UHC reset
     * After UHC reset, disable overwrite bits
     */
    regmap_clear_bits(utmi, (0x0 << 2), (BIT(7) | BIT(6) | BIT(1)));
    /* new HW term overwrite: off */
    regmap_clear_bits(utmi, (0x29 << 2), (BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0)));

    if (device_property_read_bool(&phy->dev, "utmi_dp_dm_swap"))
    {
        /* dp dm swap */
        regmap_set_bits(utmi, (0x5 << 2), BIT(13));
        dev_info(&phy->dev, "DP/DM swap\n");
    }
    else
    {
        dev_info(&phy->dev, "DP/DM no swap\n");
    }

    dev_info(&phy->dev, "%s\n", __func__);
    return 0;
}

int sstar_u2phy_utmi_exit(struct phy *phy)
{
    return 0;
}

static int sstar_u2phy_on_disconnect(struct usb_phy *usb_phy, enum usb_device_speed speed)
{
    struct sstar_u2phy *u2phy = container_of(usb_phy, struct sstar_u2phy, usb_phy);
    struct phy *        phy   = u2phy->phy;
    const char *        s     = usb_speed_string(speed);

    dev_info(&phy->dev, "%s disconnect\n", s);
    if (phy)
    {
        sstar_u2phy_utmi_reset(phy);
        sstar_u2phy_utmi_power_off(phy);
        sstar_u2phy_utmi_power_on(phy);
    }
    return 0;
}

static int sstar_u2phy_on_connect(struct usb_phy *usb_phy, enum usb_device_speed speed)
{
    struct sstar_u2phy *u2phy = container_of(usb_phy, struct sstar_u2phy, usb_phy);
    struct phy *        phy   = u2phy->phy;
    const char *        s     = usb_speed_string(speed);

    dev_info(&phy->dev, "%s connect\n", s);
    return 0;
}

static const struct phy_ops sstar_u2phy_ops = {
    .init      = sstar_u2phy_utmi_init,
    .exit      = sstar_u2phy_utmi_exit,
    .power_on  = sstar_u2phy_utmi_power_on,
    .power_off = sstar_u2phy_utmi_power_off,
    .set_mode  = sstar_u2phy_utmi_set_mode,
    .reset     = sstar_u2phy_utmi_reset,
    .calibrate = sstar_u2phy_utmi_calibrate,
    .owner     = THIS_MODULE,
};

static struct usb2_phy_data m6p_phy_data = {
    .has_utmi2_bank = 1,
};

static const struct of_device_id sstar_u2phy_dt_ids[] = {{
                                                             .compatible = "sstar, u2phy",
                                                         },
                                                         {
                                                             .compatible = "sstar, mercury6p-u2phy",
                                                             .data       = &m6p_phy_data,
                                                         },
                                                         {}};
MODULE_DEVICE_TABLE(of, sstar_u2phy_dt_ids);

static int sstar_u2phy_probe(struct platform_device *pdev)
{
    struct device_node *       np = pdev->dev.of_node;
    struct device_node *       child_np;
    struct device_node *       temp_np;
    struct phy *               phy;
    struct phy_provider *      phy_provider;
    struct device *            dev = &pdev->dev;
    struct regmap *            syscon;
    struct sstar_u2phy *       priv_phy;
    const struct of_device_id *of_match_id;

    of_match_id = of_match_device(sstar_u2phy_dt_ids, dev);
    if (!of_match_id)
    {
        dev_err(dev, "missing match\n");
        return -EINVAL;
    }

    if (!of_match_id->data)
    {
        dev_info(dev, "No extra phy data\n");
    }

    for_each_available_child_of_node(np, child_np)
    {
        priv_phy = devm_kzalloc(dev, sizeof(*priv_phy), GFP_KERNEL);
        if (!priv_phy)
        {
            dev_err(dev, "devm_kzalloc failed for phy private data.\n");
            return -ENOMEM;
        }

        temp_np = of_parse_phandle(child_np, "syscon-utmi", 0);
        if (temp_np)
        {
            syscon = syscon_node_to_regmap(temp_np);
            if (IS_ERR(syscon))
            {
                dev_err(dev, "failed to find utmi syscon regmap\n");
                return PTR_ERR(syscon);
            }
            priv_phy->utmi = syscon;
            of_node_put(temp_np);
        }

        temp_np = of_parse_phandle(child_np, "syscon-bc", 0);
        if (temp_np)
        {
            syscon = syscon_node_to_regmap(temp_np);
            if (IS_ERR(syscon))
            {
                dev_err(dev, "failed to find bc syscon regmap\n");
                return PTR_ERR(syscon);
            }
            priv_phy->bc = syscon;
            of_node_put(temp_np);
        }

        temp_np = of_parse_phandle(child_np, "syscon-usbc", 0);
        if (temp_np)
        {
            syscon = syscon_node_to_regmap(temp_np);
            if (IS_ERR(syscon))
            {
                dev_err(dev, "failed to find usbc syscon regmap\n");
                return PTR_ERR(syscon);
            }
            priv_phy->usbc = syscon;
            of_node_put(temp_np);
        }

        temp_np = of_parse_phandle(child_np, "syscon-uhc", 0);
        if (temp_np)
        {
            syscon = syscon_node_to_regmap(temp_np);
            if (IS_ERR(syscon))
            {
                dev_err(dev, "failed to find uhc syscon regmap\n");
                return PTR_ERR(syscon);
            }
            priv_phy->ehc = syscon;
            of_node_put(temp_np);
        }

        phy = devm_phy_create(dev, child_np, &sstar_u2phy_ops);
        if (IS_ERR(phy))
        {
            dev_err(dev, "failed to create PHY\n");
            return PTR_ERR(phy);
        }

        priv_phy->phy = phy;
        priv_phy->dev = dev;

        priv_phy->usb_phy.dev               = dev;
        priv_phy->usb_phy.notify_connect    = sstar_u2phy_on_connect;
        priv_phy->usb_phy.notify_disconnect = sstar_u2phy_on_disconnect;
        usb_add_phy_dev(&priv_phy->usb_phy);
        ATOMIC_INIT_NOTIFIER_HEAD(&priv_phy->usb_phy.notifier);
        priv_phy->phy_data = (struct usb2_phy_data *)of_match_id->data;
        phy_set_drvdata(phy, priv_phy);
        sstar_u2phy_utmi_debugfs_init(priv_phy);
    }

    phy_provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
    return PTR_ERR_OR_ZERO(phy_provider);
}

static int sstar_u2phy_remove(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    struct device_node *child_np;
    struct usb_phy *    usb_phy = NULL;
    struct sstar_u2phy *priv;

    for_each_available_child_of_node(np, child_np)
    {
        usb_phy = devm_usb_get_phy_by_node(&pdev->dev, child_np, NULL);
        if (usb_phy)
        {
            priv = container_of(usb_phy, struct sstar_u2phy, usb_phy);
            sstar_u2phy_utmi_debugfs_exit(priv);
            usb_phy = NULL;
        }
    }

    return 0;
}

static struct platform_driver sstar_u2phy_driver = {
    .probe  = sstar_u2phy_probe,
    .remove = sstar_u2phy_remove,
    .driver =
        {
            .name           = "sstar-u2phy",
            .of_match_table = sstar_u2phy_dt_ids,
        },
};

module_platform_driver(sstar_u2phy_driver);

MODULE_ALIAS("platform:sstar-usb2-phy");
MODULE_AUTHOR("Zuhuang Zhang <zuhuang.zhang@sigmastar.com.cn>");
MODULE_DESCRIPTION("Sigmastar USB2 PHY driver");
MODULE_LICENSE("GPL v2");
