/*
 * linux/sound/rt5665.h -- Platform data for RT5665
 *
 * Copyright 2013 Realtek Microelectronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LINUX_SND_RT5665_H
#define __LINUX_SND_RT5665_H

enum rt5665_dmic1_data_pin {
	RT5665_DMIC1_NULL,
	RT5665_DMIC1_DATA_GPIO4,
	RT5665_DMIC1_DATA_IN2N,
};

enum rt5665_dmic2_data_pin {
	RT5665_DMIC2_NULL,
	RT5665_DMIC2_DATA_GPIO5,
	RT5665_DMIC2_DATA_IN2P,
};

enum rt5665_jd_src {
	RT5665_JD_NULL,
	RT5665_JD1,
	RT5665_JD1_JD2,
};

struct rt5665_platform_data {
	bool in1_diff;
	bool in2_diff;
	bool in3_diff;
	bool in4_diff;

	const char *regulator_1v8;
	const char *regulator_3v3;
	const char *regulator_5v;

	int ldo1_en; /* GPIO for LDO1_EN */

	enum rt5665_dmic1_data_pin dmic1_data_pin;
	enum rt5665_dmic2_data_pin dmic2_data_pin;
	enum rt5665_jd_src jd_src;

	unsigned int sar_hs_type;
	unsigned int sar_hs_open_gender;
	unsigned int sar_pb_vth0;
	unsigned int sar_pb_vth1;
	unsigned int sar_pb_vth2;
	unsigned int sar_pb_vth3;

	unsigned int offset_comp[16];
	unsigned int offset_comp_r[16];

	int ext_ant_det_gpio;
};

#endif

