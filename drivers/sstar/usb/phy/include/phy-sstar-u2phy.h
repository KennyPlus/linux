/*
 * phy-sstar-u2phy.h- Sigmastar
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

#ifndef __PHY_SSTAR_USB20_H__
#define __PHY_SSTAR_USB20_H__

#include <linux/usb/phy.h>
#include <linux/phy/phy.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>

struct usb2_phy_data
{
    const char *label;
    u32         has_utmi2_bank : 1;
    u32         has_phy_trim_val : 1;
    u32         has_dm_dp_swap : 1;
};

struct sstar_u2phy
{
    struct device *dev;
    struct phy *   phy;
    struct usb_phy usb_phy;
    spinlock_t     lock;

    struct regmap *       utmi;
    struct regmap *       bc;
    struct regmap *       usbc;
    struct regmap *       ehc;
    struct regmap *       otp;
    struct usb2_phy_data *phy_data;

    struct dentry *root;
};

#endif
