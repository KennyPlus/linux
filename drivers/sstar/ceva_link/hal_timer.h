/*
 * hal_timer.h- Sigmastar
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
#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "ms_platform.h"
#include <linux/kernel.h>
int dsp_timer_hal_init(void);

void dump_bank_register(unsigned int Bank);
#endif
