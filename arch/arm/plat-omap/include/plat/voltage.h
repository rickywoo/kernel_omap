/*
 * OMAP Voltage Management Routines
 *
 * Author: Thara Gopinath	<thara@ti.com>
 *
 * Copyright (C) 2009 Texas Instruments, Inc.
 * Thara Gopinath <thara@ti.com>
 *
 * Copyright (C) 2010 Motorola
 * Lun Chang <l.chang@motorola.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ARCH_ARM_MACH_PLAT_OMAP_VOLTAGE_H
#define __ARCH_ARM_MACH_PLAT_OMAP_VOLTAGE_H

#include <linux/notifier.h>
#include <linux/err.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/spinlock.h>
#include <linux/plist.h>
#include <linux/slab.h>

//#include <plat/omap-pm.h>
#include <plat/omap34xx.h>
//#include <plat/opp.h>
#include <plat/clock.h>
#include <plat/common.h>
#include <plat/voltage.h>
#include <plat/control.h>

extern u32 enable_sr_vp_debug;
#ifdef CONFIG_PM_DEBUG
extern struct dentry *pm_dbg_main_dir;
#endif

#define VOLTSCALE_VPFORCEUPDATE		1
#define VOLTSCALE_VCBYPASS		2

#define VOLTAGE_PRECHANGE	0
#define VOLTAGE_POSTCHANGE	1

struct omap_volt_pmic_info {
	char *name;
	int slew_rate;
	int step_size;
	unsigned char i2c_addr;
	unsigned char i2c_vreg;
	unsigned char i2c_cmdreg;
	unsigned long (*vsel_to_uv)(unsigned char vsel);
	unsigned char (*uv_to_vsel)(unsigned long uv);
	unsigned char (*onforce_cmd)(unsigned char vsel);
	unsigned char (*on_cmd)(unsigned char vsel);
	unsigned char (*sleepforce_cmd)(unsigned char vsel);
	unsigned char (*sleep_cmd)(unsigned char vsel);
	unsigned char vp_config_erroroffset;
	unsigned char vp_vstepmin_vstepmin;
	unsigned char vp_vstepmax_vstepmax;
	unsigned short vp_vlimitto_timeout_us;
	unsigned char vp_vlimitto_vddmin;
	unsigned char vp_vlimitto_vddmax;
};

/**
 * voltagedomain - omap voltage domain global structure
 * @name       : Name of the voltage domain which can be used as a unique
 *               identifier.
 */
struct voltagedomain {
       char *name;
};

#define OMAP3PLUS_DYNAMIC_NOMINAL_MARGIN_UV	50000

/**
 * omap_volt_data - Omap voltage specific data.
 * @voltage_nominal	: The possible voltage value in uV
 * @sr_nvalue		: Smartreflex N target value at voltage <voltage>
 * @sr_errminlimit	: Error min limit value for smartreflex. This value
 *			  differs at differnet opp and thus is linked
 *			  with voltage.
 * @vp_errorgain	: Error gain value for the voltage processor. This
 *			  field also differs according to the voltage/opp.
 * @abb_type		: Bitfield OPP_SEL.PRM_LDO_ABB_CTRL.
 */
struct omap_volt_data {
	u32	volt_nominal;
	u32	volt_calibrated;
	u32	volt_dynamic_nominal;
	u32	sr_nvalue;
	u8	sr_errminlimit;
	u8	vp_errgain;
	u8	abb_type;
};


/* Various voltage controller related info */
struct omap_volt_vc_data {
	u16 clksetup;
	u16 voltsetup_time1;
	u16 voltsetup_time2;
	u16 voltoffset;
	u16 voltsetup2;
/* PRM_VC_CMD_VAL_0 specific bits */
	u32 vdd0_on;
	u32 vdd0_onlp;
	u32 vdd0_ret;
	u32 vdd0_off;
/* PRM_VC_CMD_VAL_1 specific bits */
	u32 vdd1_on;
	u32 vdd1_onlp;
	u32 vdd1_ret;
	u32 vdd1_off;
/* PRM_VC_CMD_VAL_2 specific bits */
	u32 vdd2_on;
	u32 vdd2_onlp;
	u32 vdd2_ret;
	u32 vdd2_off;
};

/* Voltage change notifier structure */
struct omap_volt_change_info {
	struct omap_vdd_info *vdd_info;
	struct omap_volt_data *curr_volt;
	struct omap_volt_data *target_volt;
};


/* Voltage processor register offsets */
struct vp_reg_offs {
	u8 vpconfig;
	u8 vstepmin;
	u8 vstepmax;
	u8 vlimitto;
	u8 vstatus;
	u8 voltage;
};

/* Voltage Processor bit field values, shifts and masks */
struct vp_reg_val {
	/* VPx_VPCONFIG */
	u32 vpconfig_erroroffset;
	u16 vpconfig_errorgain;
	u32 vpconfig_errorgain_mask;
	u8 vpconfig_errorgain_shift;
	u32 vpconfig_initvoltage_mask;
	u8 vpconfig_initvoltage_shift;
	u32 vpconfig_timeouten;
	u32 vpconfig_initvdd;
	u32 vpconfig_forceupdate;
	u32 vpconfig_vpenable;
	/* VPx_VSTEPMIN */
	u8 vstepmin_stepmin;
	u16 vstepmin_smpswaittimemin;
	u8 vstepmin_stepmin_shift;
	u8 vstepmin_smpswaittimemin_shift;
	/* VPx_VSTEPMAX */
	u8 vstepmax_stepmax;
	u16 vstepmax_smpswaittimemax;
	u8 vstepmax_stepmax_shift;
	u8 vstepmax_smpswaittimemax_shift;
	/* VPx_VLIMITTO */
	u16 vlimitto_vddmin;
	u16 vlimitto_vddmax;
	u16 vlimitto_timeout;
	u16 vlimitto_vddmin_shift;
	u16 vlimitto_vddmax_shift;
	u16 vlimitto_timeout_shift;
	/* PRM_IRQSTATUS*/
	u32 tranxdone_status;
};

/**
 * omap_vdd_dep_volt - Table containing the parent vdd voltage and the
 *			dependent vdd voltage corresponding to it.
 *
 * @main_vdd_volt	: The main vdd voltage
 * @dep_vdd_volt	: The voltage at which the dependent vdd should be
 *			  when the main vdd is at <main_vdd_volt> voltage
 */
struct omap_vdd_dep_volt {
	u32 main_vdd_volt;
	u32 dep_vdd_volt;
};

/**
 *  ABB Register offsets and masks
 *
 * @prm_abb_ldo_setup_idx : PRM_LDO_ABB_SETUP Register specific to MPU/IVA
 * @prm_abb_ldo_ctrl_idx  : PRM_LDO_ABB_CTRL Register specific to MPU/IVA
 * @prm_irqstatus_mpu	  : PRM_IRQSTATUS_MPU_A9/PRM_IRQSTATUS_MPU_A9_2
 * @abb_done_st_shift	  : ABB_DONE_ST shift
 * @abb_done_st_mask	  : ABB_DONE_ST_MASK bit mask
 *
 */
struct abb_reg_val {
	u16 prm_abb_ldo_setup_idx;
	u16 prm_abb_ldo_ctrl_idx;
	u16 prm_irqstatus_mpu;
	u32 abb_done_st_shift;
	u32 abb_done_st_mask;
};

/**
 * omap_vdd_dep_info - Dependent vdd info
 *
 * @name		: Dependent vdd name
 * @voltdm		: Dependent vdd pointer
 * @dep_table		: Table containing the dependent vdd voltage
 *			  corresponding to every main vdd voltage.
 */
struct omap_vdd_dep_info{
	char *name;
	struct voltagedomain *voltdm;
	struct omap_vdd_dep_volt *dep_table;
};

/**
 * omap_vdd_user_list	- The per vdd user list
 *
 * @dev		: The device asking for the vdd to be set at a particular
 *		  voltage
 * @node	: The list head entry
 * @volt	: The voltage requested by the device <dev>
 */
struct omap_vdd_user_list {
	struct device *dev;
	struct plist_node node;
	u32 volt;
};

/**
 * omap_vdd_info - Per Voltage Domain info
 *
 * @volt_data		: voltage table having the distinct voltages supported
 *			  by the domain and other associated per voltage data.
 * @vp_offs		: structure containing the offsets for various
 *			  vp registers
 * @vp_reg		: the register values, shifts, masks for various
 *			  vp registers
 * @volt_clk		: the clock associated with the vdd.
 * @opp_dev		: the 'struct device' associated with this vdd.
 * @user_lock		: the lock to be used by the plist user_list
 * @user_list		: the list head maintaining the various users
 *			  of this vdd with the voltage requested by each user.
 * @volt_data_count	: Number of distinct voltages supported by this vdd.
 * @nominal_volt	: Nominal voltaged for this vdd.
 * cmdval_reg		: Voltage controller cmdval register.
 * @vdd_sr_reg		: The smartreflex register associated with this VDD.
 */
struct omap_vdd_info{
	struct omap_volt_data *volt_data;
	struct vp_reg_offs vp_offs;
	struct vp_reg_val vp_reg;
	struct clk *volt_clk;
	struct device *opp_dev;
	struct voltagedomain voltdm;
	struct abb_reg_val omap_abb_reg_val;
	struct omap_vdd_dep_info *dep_vdd_info;
	spinlock_t user_lock;
	struct plist_head user_list;
	struct srcu_notifier_head volt_change_notify_list;
	int volt_data_count;
	int nr_dep_vdd;
	struct device **dev_list;
	int dev_count;
	struct omap_volt_data *nominal_volt;
	struct omap_volt_data *curr_volt;
	u8 cmdval_reg;
	u8 vdd_sr_reg;
	u16 ocp_mod;
	u8 prm_irqst_reg;
	struct omap_volt_pmic_info *pmic;
	struct device vdd_device;
};



struct voltagedomain *omap_voltage_domain_get(char *name);
unsigned long omap_vp_get_curr_volt(struct voltagedomain *voltdm);
void omap_vp_enable(struct voltagedomain *voltdm);
void omap_vp_disable(struct voltagedomain *voltdm);
int omap_voltage_scale_vdd(struct voltagedomain *voltdm,
		struct omap_volt_data *target_volt);
void omap_voltage_reset(struct voltagedomain *voltdm);
int omap_voltage_get_volttable(struct voltagedomain *voltdm,
		struct omap_volt_data **volt_data);
struct omap_volt_data *omap_voltage_get_voltdata(struct voltagedomain *voltdm,
		unsigned long volt);
void omap_voltage_register_pmic(struct omap_volt_pmic_info *pmic_info,
		char *vdmname);
struct omap_volt_data *omap_voltage_get_nom_volt(struct voltagedomain *voltdm);
int omap_voltage_add_userreq(struct voltagedomain *voltdm, struct device *dev,
		unsigned long *volt);
int omap_voltage_scale(struct voltagedomain *voltdm);
bool omap_vp_is_transdone(struct voltagedomain *voltdm);
bool omap_vp_clear_transdone(struct voltagedomain *voltdm);

int omap_voltage_calib_reset(struct voltagedomain *voltdm);
int omap_vscale_pause(struct voltagedomain *voltdm, bool trylock);
int omap_vscale_unpause(struct voltagedomain *voltdm);

#ifdef CONFIG_PM
void omap_voltage_init_vc(struct omap_volt_vc_data *setup_vc);
void omap_change_voltscale_method(int voltscale_method);
int omap_voltage_register_notifier(struct voltagedomain *voltdm,
		struct notifier_block *nb);
int omap_voltage_unregister_notifier(struct voltagedomain *voltdm,
		struct notifier_block *nb);
#else
static inline void omap_voltage_init_vc(struct omap_volt_vc_data *setup_vc) {}
static inline  void omap_change_voltscale_method(int voltscale_method) {}
static inline int omap_voltage_register_notifier(
		struct voltagedomain *voltdm, struct notifier_block *nb)
{
	return 0;
}

static inline int omap_voltage_unregister_notifier(
		 struct voltagedomain *voltdm, struct notifier_block *nb)
{
	return 0;
}
#endif

/* convert volt data to the voltage for the voltage data */
static inline unsigned long omap_get_operation_voltage(
		struct omap_volt_data *vdata)
{
	if (IS_ERR_OR_NULL(vdata))
		return 0;
	return (vdata->volt_calibrated) ? vdata->volt_calibrated :
		(vdata->volt_dynamic_nominal) ? vdata->volt_dynamic_nominal :
			vdata->volt_nominal;
}

/* what is my dynamic nominal? */
static inline unsigned long omap_get_dyn_nominal(struct omap_volt_data *vdata)
{
	if (IS_ERR_OR_NULL(vdata))
		return 0;
	if (vdata->volt_calibrated) {
		unsigned long v = vdata->volt_calibrated +
			OMAP3PLUS_DYNAMIC_NOMINAL_MARGIN_UV;
		if (v > vdata->volt_nominal)
			return vdata->volt_nominal;
		return v;
	}
	return vdata->volt_nominal;
}
#endif
