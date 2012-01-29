/*
 * OMAP2plus display device setup / initialization.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *	Senthilvadivu Guruswamy
 *	Sumit Semwal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>

#include <video/omapdss.h>
#include <plat/omap_hwmod.h>
#include <plat/omap_device.h>
#include <plat/omap-pm.h>

/*---------------------------------------------------------------------------*/
#ifdef CONFIG_OMAP2_DSS

#define MAX_OMAP_DSS_HWMOD_NAME_LEN 16
static const char dssname[] = "dss";
struct omap_device *od;

static struct platform_device omap_display_device = {
	.name		= "omapdss",
	.id		= -1,
	.dev		= {
		.platform_data = NULL,
	},
};

static struct omap_device_pm_latency omap_dss_latency[] = {
	[0] = {
		.deactivate_func	= omap_device_idle_hwmods,
		.activate_func		= omap_device_enable_hwmods,
		.flags			= OMAP_DEVICE_LATENCY_AUTO_ADJUST,
	},
};

struct omap_dss_hwmod_data {
	const char *oh_name;
	const char *dev_name;
	const int id;
};

static const struct omap_dss_hwmod_data omap2_dss_hwmod_data[] __initdata = {
	{ "dss_core", "omapdss_dss", -1 },
	{ "dss_dispc", "omapdss_dispc", -1 },
	{ "dss_rfbi", "omapdss_rfbi", -1 },
	{ "dss_venc", "omapdss_venc", -1 },
};

static const struct omap_dss_hwmod_data omap3_dss_hwmod_data[] __initdata = {
	{ "dss_core", "omapdss_dss", -1 },
	{ "dss_dispc", "omapdss_dispc", -1 },
	{ "dss_rfbi", "omapdss_rfbi", -1 },
	{ "dss_venc", "omapdss_venc", -1 },
	{ "dss_dsi1", "omapdss_dsi1", -1 },
};

static const struct omap_dss_hwmod_data omap4_dss_hwmod_data[] __initdata = {
	{ "dss_core", "omapdss_dss", -1 },
	{ "dss_dispc", "omapdss_dispc", -1 },
	{ "dss_rfbi", "omapdss_rfbi", -1 },
	{ "dss_venc", "omapdss_venc", -1 },
	{ "dss_dsi1", "omapdss_dsi1", -1 },
	{ "dss_dsi2", "omapdss_dsi2", -1 },
	{ "dss_hdmi", "omapdss_hdmi", -1 },
};

int __init omap_display_init(struct omap_dss_board_info *board_data)
{
#if 0
	struct omap_hwmod *oh;
	char oh_name_omap4[7][MAX_OMAP_DSS_HWMOD_NAME_LEN] = {"dss",
			"dss_dispc",
			"dss_dsi1",
			"dss_dsi2",
			"dss_hdmi",
			"dss_rfbi",
			"dss_venc"};
	char oh_name_omap3[5][MAX_OMAP_DSS_HWMOD_NAME_LEN] = {"dss",
			"dss_dispc",
			"dss_dsi1",
			"dss_rfbi",
			"dss_venc"
			};

	char (*oh_name)[MAX_OMAP_DSS_HWMOD_NAME_LEN];
	int l, i, idx = 0, count = 0;
	struct omap_display_platform_data pdata;
	idx = 1;

	if (cpu_is_omap44xx()) {
		oh_name =  &oh_name_omap4[0];
		count = 7;
	} else {
		oh_name =  &oh_name_omap3[0];
		count = 5;
	}
	for (i = 0; i < count; i++)	{
		l = snprintf(oh_name[i],
			MAX_OMAP_DSS_HWMOD_NAME_LEN, oh_name[i]);
		WARN(l >= MAX_OMAP_DSS_HWMOD_NAME_LEN,
		"String buffer overflow in DSS device setup\n");

		oh = omap_hwmod_lookup(oh_name[i]);

		if (!oh) {
			pr_err("Could not look up %s\n", oh_name[i]);
			return;
		}

		/* FIXME-HASH: CHECK TO BE SURE WE NEED THIS */
		strcpy(pdata.name, oh_name[i]);
		pdata.hwmod_count	= 2;
		pdata.board_data	= board_data;
		pdata.board_data->get_last_off_on_transaction_id = NULL;
		/* FIXME-HASH: added context_loss_count for new dss driver */
		pdata.board_data->get_context_loss_count = omap_pm_get_dev_context_loss_count; // omap_device_get_context_loss_count;

		pdata.device_enable	= omap_device_enable;
		pdata.device_idle	= omap_device_idle;
		pdata.device_shutdown	= omap_device_shutdown;

		od = omap_device_build(oh_name[i], -1, oh, &pdata,
			sizeof(struct omap_display_platform_data),
			omap_dss_latency,
			ARRAY_SIZE(omap_dss_latency), 0);
		WARN((IS_ERR(od)), "Could not build omap_device for %s %s\n",
			dssname, oh_name[i]);
	}
	omap_display_device.dev.platform_data = board_data;
	if (platform_device_register(&omap_display_device) < 0)
		printk(KERN_ERR "Unable to register OMAP-Display device\n");

	return;
#endif
	int r = 0;
	struct omap_hwmod *oh;
	struct omap_device *od;
	int i, oh_count;
	struct omap_display_platform_data pdata;
	const struct omap_dss_hwmod_data *curr_dss_hwmod;

	memset(&pdata, 0, sizeof(pdata));

	if (cpu_is_omap24xx()) {
		curr_dss_hwmod = omap2_dss_hwmod_data;
		oh_count = ARRAY_SIZE(omap2_dss_hwmod_data);
	} else if (cpu_is_omap34xx()) {
		curr_dss_hwmod = omap3_dss_hwmod_data;
		oh_count = ARRAY_SIZE(omap3_dss_hwmod_data);
	} else {
		curr_dss_hwmod = omap4_dss_hwmod_data;
		oh_count = ARRAY_SIZE(omap4_dss_hwmod_data);
	}

	pdata.board_data = board_data;

	for (i = 0; i < oh_count; i++) {
		oh = omap_hwmod_lookup(curr_dss_hwmod[i].oh_name);
		if (!oh) {
			pr_err("Could not look up %s\n",
				curr_dss_hwmod[i].oh_name);
			return -ENODEV;
		}

		od = omap_device_build(curr_dss_hwmod[i].dev_name,
				curr_dss_hwmod[i].id, oh, &pdata,
				sizeof(struct omap_display_platform_data),
				omap_dss_latency,
				ARRAY_SIZE(omap_dss_latency), 0);

		if (WARN((IS_ERR(od)), "Could not build omap_device for %s\n",
				curr_dss_hwmod[i].oh_name))
			return -ENODEV;
	}
	omap_display_device.dev.platform_data = board_data;

	r = platform_device_register(&omap_display_device);
	if (r < 0)
		printk(KERN_ERR "Unable to register OMAP-Display device\n");

	return r;
}
#else
int __init omap_display_init(struct omap_dss_board_info *board_data)
{
}
#endif
