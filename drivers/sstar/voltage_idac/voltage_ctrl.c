/*
 * voltage_ctrl.c- Sigmastar
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
#include <linux/vmalloc.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include "registers.h"
#include "ms_platform.h"
#include "voltage_ctrl.h"
#include "voltage_ctrl_demander.h"
#include "voltage_request_init.h"

#define VOLCTRL_DEBUG 0

#if VOLCTRL_DEBUG
#define VOLCTRL_DBG(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define VOLCTRL_DBG(fmt, arg...)
#endif
#define VOLCTRL_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)

struct voltage_ctrl
{
    const char *            name;
    int                     current_voltage;
    int                     tt_voltage;
    int                     lv_voltage;
    u8                      voltage_gpio_inited;
    struct device           dev_core;
    u8                      enable_scaling_voltage;
    struct platform_device *pdev;
    struct mutex            voltage_mutex;
    u32                     gpio_vid_pin;
    u32                     idac_current_max_code;
    int                     idac_max_voltage;
    int                     idac_base_vol;
    int                     voltage_demander_request_value[VOLTAGE_DEMANDER_MAX];
    struct list_head        list;
};

const char *_gVoltageDemanderName[] = {FOREACH_DEMANDER(GENERATE_STRING)};

static LIST_HEAD(voltage_ctrl_list);

static u8 voltage_subsys_registered;

static struct bus_type voltage_subsys = {
    .name     = "voltage",
    .dev_name = "voltage",
};

static struct voltage_ctrl *get_voltage_ctrl(const char *name)
{
    struct voltage_ctrl *ctrl = NULL;
    list_for_each_entry(ctrl, &voltage_ctrl_list, list)
    {
        if (!strcmp(ctrl->name, name))
        {
            return ctrl;
        }
    }
    return NULL;
}

static int check_voltage_valid(struct voltage_ctrl *ctrl, int mV)
{
    if (mV > ctrl->idac_max_voltage)
    {
        mV = ctrl->idac_max_voltage;
    }
    return mV;
}

int sync_voltage_with_OSCandTemp(const char *name, int useTT)
{
    int                  vcore_max = 0;
    struct voltage_ctrl *ctrl;

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }

    mutex_lock(&ctrl->voltage_mutex);

    if (ctrl->enable_scaling_voltage)
    {
        ctrl->voltage_demander_request_value[VOLTAGE_DEMANDER_TEMPERATURE] =
            (useTT ? ctrl->tt_voltage : ctrl->lv_voltage);

        vcore_max = ctrl->voltage_demander_request_value[VOLTAGE_DEMANDER_TEMPERATURE];

        if (vcore_max < ctrl->voltage_demander_request_value[VOLTAGE_DEMANDER_USER])
        {
            vcore_max = ctrl->voltage_demander_request_value[VOLTAGE_DEMANDER_USER];
        }

        vcore_max = check_voltage_valid(ctrl, vcore_max);

        VOLCTRL_DBG("[Core Voltage] %s: maximum request is %dmV\n", __FUNCTION__, vcore_max);

        idac_set_voltage(ctrl->name, ctrl->idac_base_vol, vcore_max);

        ctrl->current_voltage = vcore_max;
    }
    else
    {
        VOLCTRL_DBG("[Core Voltage] %s: voltage scaling not enable\n", __FUNCTION__);
    }

    VOLCTRL_DBG("[Core Voltage] %s: current core voltage %dmV\n", __FUNCTION__, ctrl->current_voltage);

    mutex_unlock(&ctrl->voltage_mutex);
    return 0;
}
EXPORT_SYMBOL(sync_voltage_with_OSCandTemp);

static void sync_core_voltage(struct voltage_ctrl *ctrl)
{
    int i         = 0;
    int vcore_max = 0;

    mutex_lock(&ctrl->voltage_mutex);

    if (ctrl->enable_scaling_voltage)
    {
        for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
        {
            vcore_max = max(vcore_max, ctrl->voltage_demander_request_value[i]);
        }
        vcore_max = check_voltage_valid(ctrl, vcore_max);
        VOLCTRL_DBG("[Core Voltage] %s: maximum request is %dmV\n", __FUNCTION__, vcore_max);

        idac_set_voltage(ctrl->name, ctrl->idac_base_vol, vcore_max);

        ctrl->current_voltage = vcore_max;
    }
    else
    {
        VOLCTRL_DBG("[Core Voltage] %s: voltage scaling not enable\n", __FUNCTION__);
    }

    VOLCTRL_DBG("[Core Voltage] %s: current core voltage %dmV\n", __FUNCTION__, ctrl->current_voltage);

    mutex_unlock(&ctrl->voltage_mutex);
}

int set_core_voltage(const char *name, VOLTAGE_DEMANDER_E demander, int voloff)
{
    int                  i         = 0;
    int                  vcore_max = 0;
    struct voltage_ctrl *ctrl;

    if (demander >= VOLTAGE_DEMANDER_MAX)
    {
        VOLCTRL_DBG("[Core Voltage] %s: demander number out of range (%d)\n", __FUNCTION__, demander);
        return -EINVAL;
    }

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }

    mutex_lock(&ctrl->voltage_mutex);

    ctrl->voltage_demander_request_value[demander] = voloff;
    VOLCTRL_DBG("[Core Voltage] %s: %s request %dmV\n", __FUNCTION__, _gVoltageDemanderName[demander], voloff);

    if (ctrl->enable_scaling_voltage)
    {
        for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
        {
            vcore_max = max(vcore_max, ctrl->voltage_demander_request_value[i]);
        }
        vcore_max = check_voltage_valid(ctrl, vcore_max);
        VOLCTRL_DBG("[Core Voltage] %s: maximum request is %dmV\n", __FUNCTION__, vcore_max);

        idac_set_voltage(ctrl->name, ctrl->idac_base_vol, vcore_max);

        ctrl->current_voltage = vcore_max;
    }
    else
    {
        VOLCTRL_DBG("[Core Voltage] %s: voltage scaling not enable\n", __FUNCTION__);
    }

    VOLCTRL_DBG("[Core Voltage] %s: current core voltage %dmV\n", __FUNCTION__, ctrl->current_voltage);

    mutex_unlock(&ctrl->voltage_mutex);
    return 0;
}
EXPORT_SYMBOL(set_core_voltage);

int get_core_voltage(const char *name, int *mV)
{
    struct voltage_ctrl *ctrl;

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }
    *mV = ctrl->current_voltage;
    return 0;
}
EXPORT_SYMBOL(get_core_voltage);

int get_tt_voltage(const char *name)
{
    struct voltage_ctrl *ctrl;

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }

    return ctrl->tt_voltage;
}
EXPORT_SYMBOL(get_tt_voltage);

int get_lv_voltage(const char *name)
{
    struct voltage_ctrl *ctrl;

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }

    return ctrl->lv_voltage;
}
EXPORT_SYMBOL(get_lv_voltage);

static ssize_t show_scaling_voltage(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    mutex_lock(&ctrl->voltage_mutex);
    str += scnprintf(str, end - str, "%d\n", ctrl->enable_scaling_voltage);
    mutex_unlock(&ctrl->voltage_mutex);

    return (str - buf);
}

static ssize_t store_scaling_voltage(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u32                  enable;
    struct voltage_ctrl *ctrl;

    if (sscanf(buf, "%d", &enable) <= 0)
        return 0;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        return count;
    }

    mutex_lock(&ctrl->voltage_mutex);
    if (enable)
    {
        ctrl->enable_scaling_voltage = 1;
        VOLCTRL_DBG("[Core Voltage] %s: scaling ON\n", __FUNCTION__);
    }
    else
    {
        ctrl->enable_scaling_voltage = 0;
        VOLCTRL_DBG("[Core Voltage] %s: scaling OFF\n", __FUNCTION__);
    }
    mutex_unlock(&ctrl->voltage_mutex);
    sync_core_voltage(ctrl);

    return count;
}
DEVICE_ATTR(scaling_voltage, 0644, show_scaling_voltage, store_scaling_voltage);

static ssize_t show_voltage_current(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    int                  voltage_mV;
    unsigned int         i;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    if (!get_core_voltage(ctrl->name, &voltage_mV))
    {
        str += scnprintf(str, end - str, "%d\n", voltage_mV);
    }
    else
    {
        str += scnprintf(str, end - str, "Get core voltage fail\n");
    }

    for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
    {
        if (ctrl->voltage_demander_request_value[i])
            str += scnprintf(str, end - str, "    %-32s%d\n", _gVoltageDemanderName[i],
                             ctrl->voltage_demander_request_value[i]);
        else
            str += scnprintf(str, end - str, "    %-32s-\n", _gVoltageDemanderName[i]);
    }

    return (str - buf);
}

static ssize_t store_voltage_current(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u32                  voltage = 0;
    struct voltage_ctrl *ctrl;

    if (sscanf(buf, "%d", &voltage) <= 0)
        return 0;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        return count;
    }

    if (ctrl->enable_scaling_voltage)
        set_core_voltage(ctrl->name, VOLTAGE_DEMANDER_USER, voltage);
    else
        printk("[Core Voltage] voltage scaling not enable\n");

    return count;
}
DEVICE_ATTR(voltage_current, 0644, show_voltage_current, store_voltage_current);

static ssize_t show_vid_gpio_map(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    str += scnprintf(str, end - str, "vid_%d=%d ", 0, ctrl->gpio_vid_pin);
    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}
DEVICE_ATTR(vid_gpio_map, 0444, show_vid_gpio_map, NULL);

static ssize_t show_idac_max_code(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    str += scnprintf(str, end - str, "%d ", ctrl->idac_current_max_code);
    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}
DEVICE_ATTR(idac_max_code, 0444, show_idac_max_code, NULL);

static ssize_t show_voltage_step(struct device *dev, struct device_attribute *attr, char *buf)
{
    int                  voltage_step;
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    voltage_step = idac_get_voltage_reg(ctrl->name, IDAC_CP_VOLTAGE_STEP);
    str += scnprintf(str, end - str, "%d ", voltage_step);
    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t store_voltage_step(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                  step = 0;
    struct voltage_ctrl *ctrl;

    if (sscanf(buf, "%d", &step) <= 0)
        return 0;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        return count;
    }

    step = step & 0xF;
    if (idac_set_voltage_reg(ctrl->name, IDAC_CP_VOLTAGE_STEP, step))
    {
        printk(KERN_ERR "[Core Voltage] %s: set voltage step fail\n", __FUNCTION__);
    }

    return count;
}
DEVICE_ATTR(voltage_step, 0644, show_voltage_step, store_voltage_step);

static ssize_t show_step_current(struct device *dev, struct device_attribute *attr, char *buf)
{
    int                  step_current;
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    step_current = idac_get_voltage_reg(ctrl->name, IDAC_CP_STEP_CURRENT);
    str += scnprintf(str, end - str, "%d ", step_current);
    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t store_step_current(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                  step_current = 0;
    struct voltage_ctrl *ctrl;

    if (sscanf(buf, "%d", &step_current) <= 0)
        return 0;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        return count;
    }

    step_current = (step_current != 0) ? 1 : 0;
    if (idac_set_voltage_reg(ctrl->name, IDAC_CP_STEP_CURRENT, step_current))
    {
        printk(KERN_ERR "[Core Voltage] %s: set step currentfail\n", __FUNCTION__);
    }

    return count;
}
DEVICE_ATTR(step_current, 0644, show_step_current, store_step_current);

static ssize_t show_sink_source(struct device *dev, struct device_attribute *attr, char *buf)
{
    int                  sink_source;
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    sink_source = idac_get_voltage_reg(ctrl->name, IDAC_CP_SINK_SOURCE);
    str += scnprintf(str, end - str, "%d ", sink_source);
    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t store_sink_source(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                  sink_source = 0;
    struct voltage_ctrl *ctrl;

    if (sscanf(buf, "%d", &sink_source) <= 0)
        return 0;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        return count;
    }

    sink_source = (sink_source != 0) ? 1 : 0;
    if (idac_set_voltage_reg(ctrl->name, IDAC_CP_SINK_SOURCE, sink_source))
    {
        printk(KERN_ERR "[Core Voltage] %s: set voltage step fail\n", __FUNCTION__);
    }

    return count;
}
DEVICE_ATTR(sink_source, 0644, show_sink_source, store_sink_source);

static int idac_get_gpio(struct voltage_ctrl *ctrl)
{
    struct device_node *np = ctrl->pdev->dev.of_node;
    char                name[10];
    int                 ret;

    mutex_lock(&ctrl->voltage_mutex);

    if (0 != of_property_read_u32(np, "vid_gpio", &ctrl->gpio_vid_pin))
        goto voltage_get_gpio_err;

    sprintf(name, "vid%d", ctrl->gpio_vid_pin);
    ret = gpio_request(ctrl->gpio_vid_pin, (const char *)name);
    if (ret)
        goto voltage_get_gpio_err;
    gpio_export(ctrl->gpio_vid_pin, 0);
    gpio_direction_input(ctrl->gpio_vid_pin);

    /* idac - set gpio to analog mode for idac control*/
    if (0 != idac_set_gpio_analog_mode(ctrl->gpio_vid_pin))
        goto voltage_get_gpio_err;

    ctrl->voltage_gpio_inited = 1;

    mutex_unlock(&ctrl->voltage_mutex);

    return 0;

voltage_get_gpio_err:
    ctrl->gpio_vid_pin = 0;

    mutex_unlock(&ctrl->voltage_mutex);
    return -1;
}

static int idac_init(struct voltage_ctrl *ctrl)
{
    struct device_node *np = ctrl->pdev->dev.of_node;

    mutex_lock(&ctrl->voltage_mutex);

    if (0 != of_property_read_u32(np, "base_voltage", &ctrl->idac_base_vol))
    {
        VOLCTRL_ERR("[Voltage] %s: base_voltage is null\n", __FUNCTION__);
        goto voltage_get_base_vol_err;
    }

    if (0 != of_property_read_u32(np, "tt_voltage", &ctrl->tt_voltage))
    {
        VOLCTRL_ERR("[Voltage] %s: tt_voltage is null\n", __FUNCTION__);
        goto voltage_get_base_vol_err;
    }

    if (0 != of_property_read_u32(np, "lv_voltage", &ctrl->lv_voltage))
    {
        VOLCTRL_ERR("[Voltage] %s: lv_voltage is null\n", __FUNCTION__);
        goto voltage_get_base_vol_err;
    }

    if (!strcmp(ctrl->name, "core_power"))
    {
        ctrl->idac_current_max_code = idac_get_core_current_max_code();
        ctrl->idac_max_voltage      = ctrl->idac_base_vol + idac_get_core_max_vol_offset();
        idac_init_core();
    }
    else if (!strcmp(ctrl->name, "cpu_power"))
    {
        ctrl->idac_current_max_code = idac_get_cpu_current_max_code();
        ctrl->idac_max_voltage      = ctrl->idac_base_vol + idac_get_cpu_max_vol_offset();
        idac_init_cpu();
    }
    else if (!strcmp(ctrl->name, "dla_power"))
    {
        ctrl->idac_current_max_code = idac_get_dla_current_max_code();
        ctrl->idac_max_voltage      = ctrl->idac_base_vol + idac_get_dla_max_vol_offset();
        idac_init_dla();
    }
    else if (!strcmp(ctrl->name, "dsp_power"))
    {
        ctrl->idac_current_max_code = idac_get_dsp_current_max_code();
        ctrl->idac_max_voltage      = ctrl->idac_base_vol + idac_get_dsp_max_vol_offset();
        idac_init_dsp();
    }
    else
    {
        VOLCTRL_ERR("[Voltage] %s: idac init fail\n", __FUNCTION__);
        goto voltage_idac_init_err;
    }

    mutex_unlock(&ctrl->voltage_mutex);

    printk(KERN_INFO "[Voltage] %s: Add %s\n", __FUNCTION__, ctrl->name);

    return 0;

voltage_get_base_vol_err:
voltage_idac_init_err:

    ctrl->idac_base_vol         = 0;
    ctrl->idac_current_max_code = 0;
    ctrl->idac_max_voltage      = 0;

    mutex_unlock(&ctrl->voltage_mutex);
    return -1;
}

static int sstar_voltage_idac_ctrl_probe(struct platform_device *pdev)
{
    int                  ret = 0;
    struct voltage_ctrl *ctrl;

    ctrl = kzalloc(sizeof(struct voltage_ctrl), GFP_KERNEL);
    if (!ctrl)
    {
        ret = -ENOMEM;
        goto mem_err;
    }

    ctrl->pdev = pdev;
    ctrl->name = pdev->dev.of_node->name;

    ctrl->dev_core.kobj.name = ctrl->name;
    ctrl->dev_core.bus       = &voltage_subsys;

    mutex_init(&ctrl->voltage_mutex);

    ret = idac_get_gpio(ctrl);
    if (ret)
    {
        printk(KERN_ERR "Failed to get gpio for voltage control!! %d\n", ret);
        goto voltage_control_init_err;
    }

    ret = idac_init(ctrl);
    if (ret)
    {
        printk(KERN_ERR "Failed to do idac_init!! %d\n", ret);
        goto voltage_control_init_err;
    }

    VOLCTRL_DBG("[Core Voltage] %s: register sub system\n", __FUNCTION__);

    if (!voltage_subsys_registered)
    {
        ret = subsys_system_register(&voltage_subsys, NULL);
        if (ret)
        {
            printk(KERN_ERR "Failed to register voltage sub system!! %d\n", ret);
            goto voltage_control_init_err;
        }
        voltage_subsys_registered = 1;
    }

    ret = device_register(&ctrl->dev_core);
    if (ret)
    {
        printk(KERN_ERR "Failed to register voltage core device!! %d\n", ret);
        goto voltage_control_init_err;
    }

    platform_set_drvdata(pdev, ctrl);
    dev_set_drvdata(&ctrl->dev_core, ctrl);

    list_add_tail(&ctrl->list, &voltage_ctrl_list);

    device_create_file(&ctrl->dev_core, &dev_attr_scaling_voltage);
    // device_create_file(&ctrl->dev_core, &dev_attr_voltage_available);
    device_create_file(&ctrl->dev_core, &dev_attr_voltage_current);
    device_create_file(&ctrl->dev_core, &dev_attr_vid_gpio_map);
    device_create_file(&ctrl->dev_core, &dev_attr_idac_max_code);
    device_create_file(&ctrl->dev_core, &dev_attr_voltage_step);
    device_create_file(&ctrl->dev_core, &dev_attr_step_current);
    device_create_file(&ctrl->dev_core, &dev_attr_sink_source);

    // Initialize voltage request for specific IP by chip
    voltage_request_chip(ctrl->name);

    // Enable core voltage scaling
    VOLCTRL_DBG("[Core Voltage] %s: turn-on core voltage scaling\n", __FUNCTION__);
    ctrl->enable_scaling_voltage = 1;

    sync_core_voltage(ctrl);

    return 0;

voltage_control_init_err:
mem_err:
    kfree(ctrl);
    return ret;
}

static const struct of_device_id sstar_voltage_ctrl_of_match_table[] = {{.compatible = "sstar,voltage-idac-ctrl"}, {}};
MODULE_DEVICE_TABLE(of, sstar_voltage_ctrl_of_match_table);

static struct platform_driver sstar_voltage_idac_ctrl_driver = {
    .probe = sstar_voltage_idac_ctrl_probe,
    .driver =
        {
            .name           = "sstar,voltage-idac-ctrl",
            .owner          = THIS_MODULE,
            .of_match_table = sstar_voltage_ctrl_of_match_table,
        },
};

builtin_platform_driver(sstar_voltage_idac_ctrl_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SStar Voltage IDAC Control Driver");
MODULE_LICENSE("GPL v2");
