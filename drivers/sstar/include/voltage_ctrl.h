/*
 * voltage_ctrl.h- Sigmastar
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
#ifndef __VOLTAGE_CTRL_H
#define __VOLTAGE_CTRL_H

#include "voltage_ctrl_demander.h"

#define VOLTAGE_CORE_850  850
#define VOLTAGE_CORE_900  900
#define VOLTAGE_CORE_950  950
#define VOLTAGE_CORE_1000 1000

#ifdef CONFIG_SS_VOLTAGE_CTRL_WITH_OSC
int sync_core_voltage_with_OSC_and_TEMP(const char *name, int useTT);
#endif

#ifdef CONFIG_SS_VOLTAGE_IDAC_CTRL
int get_lv_voltage(const char *name);
int get_tt_voltage(const char *name);
int sync_voltage_with_OSCandTemp(const char *name, int useTT);
#endif

int set_core_voltage(const char *name, VOLTAGE_DEMANDER_E demander, int mV);
int get_core_voltage(const char *name, int *mV);
int core_voltage_available(const char *name, unsigned int **voltages, unsigned int *num);
int core_voltage_pin(const char *name, unsigned int **pins, unsigned int *num);

#endif //__VOLTAGE_CTRL_H
