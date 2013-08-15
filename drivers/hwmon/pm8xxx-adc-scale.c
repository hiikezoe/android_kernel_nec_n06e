/*
 * Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/
 












#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#define KELVINMIL_DEGMIL	273160

/* Units for temperature below (on x axis) is in 0.1DegC as
   required by the battery driver. Note the resolution used
   here to compute the table was done for DegC to milli-volts.
   In consideration to limit the size of the table for the given
   temperature range below, the result is linearly interpolated
   and provided to the battery driver in the units desired for
   their framework which is 0.1DegC. True resolution of 0.1DegC
   will result in the below table size to increase by 10 times */






static struct pm8xxx_adc_map_pt ncm_adcmap_btm_threshold[] = {
	{ -400, 0x69D},
	{ -390, 0x697},
	{ -380, 0x691},
	{ -370, 0x68B},
	{ -360, 0x685},
	{ -350, 0x67E},
	{ -340, 0x677},
	{ -330, 0x670},
	{ -320, 0x668},
	{ -310, 0x660},
	{ -300, 0x658},
	{ -290, 0x650},
	{ -280, 0x648},
	{ -270, 0x63F},
	{ -260, 0x635},
	{ -250, 0x62C},
	{ -240, 0x622},
	{ -230, 0x618},
	{ -220, 0x60E},
	{ -210, 0x603},
	{ -200, 0x5F9},
	{ -190, 0x5ED},
	{ -180, 0x5E2},
	{ -170, 0x5D6},
	{ -160, 0x5CA},
	{ -150, 0x5BE},
	{ -140, 0x5B2},
	{ -130, 0x5A5},
	{ -120, 0x598},
	{ -110, 0x58B},
	{ -100, 0x57D},
	{  -90, 0x570},
	{  -80, 0x562},
	{  -70, 0x554},
	{  -60, 0x545},
	{  -50, 0x537},
	{  -40, 0x528},
	{  -30, 0x519},
	{  -20, 0x50A},
	{  -10, 0x4FB},
	{    0, 0x4EC},
	{   10, 0x4DC},
	{   20, 0x4CD},
	{   30, 0x4BE},
	{   40, 0x4AE},
	{   50, 0x49E},
	{   60, 0x48F},
	{   70, 0x47F},
	{   80, 0x46F},
	{   90, 0x45F},
	{  100, 0x44F},
	{  110, 0x440},
	{  120, 0x430},
	{  130, 0x420},
	{  140, 0x410},
	{  150, 0x401},
	{  160, 0x3F1},
	{  170, 0x3E2},
	{  180, 0x3D3},
	{  190, 0x3C3},
	{  200, 0x3B4},
	{  210, 0x3A5},
	{  220, 0x396},
	{  230, 0x388},
	{  240, 0x379},
	{  250, 0x36B},
	{  260, 0x35D},
	{  270, 0x34F},
	{  280, 0x341},
	{  290, 0x333},
	{  300, 0x325},
	{  310, 0x318},
	{  320, 0x30B},
	{  330, 0x2FE},
	{  340, 0x2F2},
	{  350, 0x2E5},
	{  360, 0x2D9},
	{  370, 0x2CD},
	{  380, 0x2C1},
	{  390, 0x2B5},
	{  400, 0x2AA},
	{  410, 0x29F},
	{  420, 0x294},
	{  430, 0x289},
	{  440, 0x27F},
	{  450, 0x274},
	{  460, 0x26A},
	{  470, 0x260},
	{  480, 0x257},
	{  490, 0x24D},
	{  500, 0x244},
	{  510, 0x23B},
	{  520, 0x232},
	{  530, 0x229},
	{  540, 0x221},
	{  550, 0x219},
	{  560, 0x211},
	{  570, 0x209},
	{  580, 0x201},
	{  590, 0x1FA},
	{  600, 0x1F2},
	{  610, 0x1EB},
	{  620, 0x1E5},
	{  630, 0x1DE},
	{  640, 0x1D7},
	{  650, 0x1D1},
	{  660, 0x1CB},
	{  670, 0x1C5},
	{  680, 0x1BF},
	{  690, 0x1B9},
	{  700, 0x1B4},
	{  710, 0x1AE},
	{  720, 0x1A9},
	{  730, 0x1A4},
	{  740, 0x19F},
	{  750, 0x19A},
	{  760, 0x195},
	{  770, 0x191},
	{  780, 0x18C},
	{  790, 0x188},
	{  800, 0x184},
	{  810, 0x17F},
	{  820, 0x17B},
	{  830, 0x178},
	{  840, 0x174},
	{  850, 0x170},
	{  860, 0x16C},
	{  870, 0x169},
	{  880, 0x165},
	{  890, 0x162},
	{  900, 0x15F},
	{  910, 0x15C},
	{  920, 0x159},
	{  930, 0x156},
	{  940, 0x153},
	{  950, 0x150},
	{  960, 0x14D},
	{  970, 0x14A},
	{  980, 0x148},
	{  990, 0x145},
	{ 1000, 0x143},
	{ 1010, 0x140},
	{ 1020, 0x13E},
	{ 1030, 0x13C},
	{ 1040, 0x139},
	{ 1050, 0x137},
	{ 1060, 0x135},
	{ 1070, 0x133},
	{ 1080, 0x131},
	{ 1090, 0x12F},
	{ 1100, 0x12D},
	{ 1110, 0x12B},
	{ 1120, 0x12A},
	{ 1130, 0x128},
	{ 1140, 0x126},
	{ 1150, 0x125},
	{ 1160, 0x123},
	{ 1170, 0x121},
	{ 1180, 0x120},
	{ 1190, 0x11E},
	{ 1200, 0x11D},
	{ 1210, 0x11B},
	{ 1220, 0x11A},
	{ 1230, 0x119},
	{ 1240, 0x117},
	{ 1250, 0x116},
};
















































































































































































static struct pm8xxx_adc_map_pt ncm_adcmap_pa_therm[] = {
	{ 0x6DF, -40},
	{ 0x6DD, -39},
	{ 0x6D9, -38},
	{ 0x6D6, -37},
	{ 0x6D3, -36},
	{ 0x6CF, -35},
	{ 0x6CB, -34},
	{ 0x6C7, -33},
	{ 0x6C3, -32},
	{ 0x6BE, -31},
	{ 0x6B9, -30},
	{ 0x6B4, -29},
	{ 0x6AE, -28},
	{ 0x6A9, -27},
	{ 0x6A3, -26},
	{ 0x69C, -25},
	{ 0x696, -24},
	{ 0x68F, -23},
	{ 0x687, -22},
	{ 0x680, -21},
	{ 0x677, -20},
	{ 0x66F, -19},
	{ 0x666, -18},
	{ 0x65D, -17},
	{ 0x653, -16},
	{ 0x649, -15},
	{ 0x63F, -14},
	{ 0x634, -13},
	{ 0x628, -12},
	{ 0x61D, -11},
	{ 0x610, -10},
	{ 0x604,  -9},
	{ 0x5F6,  -8},
	{ 0x5E9,  -7},
	{ 0x5DB,  -6},
	{ 0x5CC,  -5},
	{ 0x5BD,  -4},
	{ 0x5AE,  -3},
	{ 0x59E,  -2},
	{ 0x58E,  -1},
	{ 0x57D,   0},
	{ 0x56C,   1},
	{ 0x55B,   2},
	{ 0x549,   3},
	{ 0x536,   4},
	{ 0x524,   5},
	{ 0x511,   6},
	{ 0x4FD,   7},
	{ 0x4EA,   8},
	{ 0x4D6,   9},
	{ 0x4C2,  10},
	{ 0x4AD,  11},
	{ 0x499,  12},
	{ 0x484,  13},
	{ 0x46F,  14},
	{ 0x45A,  15},
	{ 0x444,  16},
	{ 0x42F,  17},
	{ 0x419,  18},
	{ 0x404,  19},
	{ 0x3EE,  20},
	{ 0x3D9,  21},
	{ 0x3C3,  22},
	{ 0x3AE,  23},
	{ 0x398,  24},
	{ 0x383,  25},
	{ 0x36E,  26},
	{ 0x359,  27},
	{ 0x344,  28},
	{ 0x32F,  29},
	{ 0x31B,  30},
	{ 0x306,  31},
	{ 0x2F2,  32},
	{ 0x2DE,  33},
	{ 0x2CB,  34},
	{ 0x2B8,  35},
	{ 0x2A5,  36},
	{ 0x292,  37},
	{ 0x280,  38},
	{ 0x26E,  39},
	{ 0x25C,  40},
	{ 0x24B,  41},
	{ 0x23A,  42},
	{ 0x22A,  43},
	{ 0x219,  44},
	{ 0x209,  45},
	{ 0x1FA,  46},
	{ 0x1EB,  47},
	{ 0x1DC,  48},
	{ 0x1CE,  49},
	{ 0x1C0,  50},
	{ 0x1B2,  51},
	{ 0x1A5,  52},
	{ 0x198,  53},
	{ 0x18B,  54},
	{ 0x17F,  55},
	{ 0x173,  56},
	{ 0x167,  57},
	{ 0x15C,  58},
	{ 0x151,  59},
	{ 0x146,  60},
	{ 0x13C,  61},
	{ 0x132,  62},
	{ 0x128,  63},
	{ 0x11F,  64},
	{ 0x116,  65},
	{ 0x10D,  66},
	{ 0x105,  67},
	{ 0x0FC,  68},
	{ 0x0F4,  69},
	{ 0x0EC,  70},
	{ 0x0E5,  71},
	{ 0x0DE,  72},
	{ 0x0D7,  73},
	{ 0x0D0,  74},
	{ 0x0C9,  75},
	{ 0x0C3,  76},
	{ 0x0BD,  77},
	{ 0x0B7,  78},
	{ 0x0B1,  79},
	{ 0x0AB,  80},
	{ 0x0A6,  81},
	{ 0x0A1,  82},
	{ 0x09C,  83},
	{ 0x097,  84},
	{ 0x092,  85},
	{ 0x08D,  86},
	{ 0x089,  87},
	{ 0x085,  88},
	{ 0x081,  89},
	{ 0x07D,  90},
	{ 0x079,  91},
	{ 0x075,  92},
	{ 0x071,  93},
	{ 0x06E,  94},
	{ 0x06B,  95},
	{ 0x067,  96},
	{ 0x064,  97},
	{ 0x061,  98},
	{ 0x05E,  99},
	{ 0x05B, 100},
	{ 0x059, 101},
	{ 0x056, 102},
	{ 0x053, 103},
	{ 0x051, 104},
	{ 0x04E, 105},
	{ 0x04C, 106},
	{ 0x04A, 107},
	{ 0x048, 108},
	{ 0x046, 109},
	{ 0x044, 110},
	{ 0x042, 111},
	{ 0x040, 112},
	{ 0x03E, 113},
	{ 0x03C, 114},
	{ 0x03A, 115},
	{ 0x039, 116},
	{ 0x037, 117},
	{ 0x035, 118},
	{ 0x034, 119},
	{ 0x032, 120},
	{ 0x031, 121},
	{ 0x030, 122},
	{ 0x02E, 123},
	{ 0x02D, 124},
	{ 0x02C, 125}
};


static struct pm8xxx_adc_map_pt ncm_adcmap_ntcg_104ef_104fb[] = {
	{ 0x6DF, -40000},
	{ 0x6DD, -39000},
	{ 0x6D9, -38000},
	{ 0x6D6, -37000},
	{ 0x6D3, -36000},
	{ 0x6CF, -35000},
	{ 0x6CB, -34000},
	{ 0x6C7, -33000},
	{ 0x6C3, -32000},
	{ 0x6BE, -31000},
	{ 0x6B9, -30000},
	{ 0x6B4, -29000},
	{ 0x6AF, -28000},
	{ 0x6A9, -27000},
	{ 0x6A3, -26000},
	{ 0x69C, -25000},
	{ 0x696, -24000},
	{ 0x68F, -23000},
	{ 0x687, -22000},
	{ 0x680, -21000},
	{ 0x678, -20000},
	{ 0x66F, -19000},
	{ 0x666, -18000},
	{ 0x65D, -17000},
	{ 0x653, -16000},
	{ 0x649, -15000},
	{ 0x63F, -14000},
	{ 0x634, -13000},
	{ 0x629, -12000},
	{ 0x61D, -11000},
	{ 0x611, -10000},
	{ 0x604,  -9000},
	{ 0x5F7,  -8000},
	{ 0x5E9,  -7000},
	{ 0x5DB,  -6000},
	{ 0x5CD,  -5000},
	{ 0x5BE,  -4000},
	{ 0x5AE,  -3000},
	{ 0x59F,  -2000},
	{ 0x58E,  -1000},
	{ 0x57E,      0},
	{ 0x56D,   1000},
	{ 0x55B,   2000},
	{ 0x549,   3000},
	{ 0x537,   4000},
	{ 0x524,   5000},
	{ 0x511,   6000},
	{ 0x4FE,   7000},
	{ 0x4EA,   8000},
	{ 0x4D6,   9000},
	{ 0x4C2,  10000},
	{ 0x4AE,  11000},
	{ 0x499,  12000},
	{ 0x484,  13000},
	{ 0x46F,  14000},
	{ 0x45A,  15000},
	{ 0x445,  16000},
	{ 0x42F,  17000},
	{ 0x41A,  18000},
	{ 0x404,  19000},
	{ 0x3EF,  20000},
	{ 0x3D9,  21000},
	{ 0x3C4,  22000},
	{ 0x3AE,  23000},
	{ 0x399,  24000},
	{ 0x384,  25000},
	{ 0x36E,  26000},
	{ 0x359,  27000},
	{ 0x344,  28000},
	{ 0x330,  29000},
	{ 0x31B,  30000},
	{ 0x307,  31000},
	{ 0x2F3,  32000},
	{ 0x2DF,  33000},
	{ 0x2CC,  34000},
	{ 0x2B8,  35000},
	{ 0x2A5,  36000},
	{ 0x293,  37000},
	{ 0x280,  38000},
	{ 0x26F,  39000},
	{ 0x25D,  40000},
	{ 0x24C,  41000},
	{ 0x23B,  42000},
	{ 0x22A,  43000},
	{ 0x21A,  44000},
	{ 0x20A,  45000},
	{ 0x1FB,  46000},
	{ 0x1EB,  47000},
	{ 0x1DD,  48000},
	{ 0x1CE,  49000},
	{ 0x1C0,  50000},
	{ 0x1B2,  51000},
	{ 0x1A5,  52000},
	{ 0x198,  53000},
	{ 0x18B,  54000},
	{ 0x17F,  55000},
	{ 0x173,  56000},
	{ 0x168,  57000},
	{ 0x15C,  58000},
	{ 0x151,  59000},
	{ 0x147,  60000},
	{ 0x13C,  61000},
	{ 0x132,  62000},
	{ 0x129,  63000},
	{ 0x11F,  64000},
	{ 0x116,  65000},
	{ 0x10D,  66000},
	{ 0x105,  67000},
	{ 0x0FD,  68000},
	{ 0x0F5,  69000},
	{ 0x0ED,  70000},
	{ 0x0E5,  71000},
	{ 0x0DE,  72000},
	{ 0x0D7,  73000},
	{ 0x0D0,  74000},
	{ 0x0C9,  75000},
	{ 0x0C3,  76000},
	{ 0x0BD,  77000},
	{ 0x0B7,  78000},
	{ 0x0B1,  79000},
	{ 0x0AC,  80000},
	{ 0x0A6,  81000},
	{ 0x0A1,  82000},
	{ 0x09C,  83000},
	{ 0x097,  84000},
	{ 0x092,  85000},
	{ 0x08E,  86000},
	{ 0x089,  87000},
	{ 0x085,  88000},
	{ 0x081,  89000},
	{ 0x07D,  90000},
	{ 0x079,  91000},
	{ 0x075,  92000},
	{ 0x072,  93000},
	{ 0x06E,  94000},
	{ 0x06B,  95000},
	{ 0x067,  96000},
	{ 0x064,  97000},
	{ 0x061,  98000},
	{ 0x05E,  99000},
	{ 0x05B, 100000},
	{ 0x059, 101000},
	{ 0x056, 102000},
	{ 0x053, 103000},
	{ 0x051, 104000},
	{ 0x04F, 105000},
	{ 0x04C, 106000},
	{ 0x04A, 107000},
	{ 0x048, 108000},
	{ 0x046, 109000},
	{ 0x044, 110000},
	{ 0x042, 111000},
	{ 0x040, 112000},
	{ 0x03E, 113000},
	{ 0x03C, 114000},
	{ 0x03A, 115000},
	{ 0x039, 116000},
	{ 0x037, 117000},
	{ 0x035, 118000},
	{ 0x034, 119000},
	{ 0x033, 120000},
	{ 0x031, 121000},
	{ 0x030, 122000},
	{ 0x02E, 123000},
	{ 0x02D, 124000},
	{ 0x02C, 125000}
};



































































































































































































































































































































































































































static int32_t pm8xxx_adc_map_linear(const struct pm8xxx_adc_map_pt *pts,
		uint32_t tablesize, int32_t input, int64_t *output)
{
	bool descending = 1;
	uint32_t i = 0;

	if ((pts == NULL) || (output == NULL))
		return -EINVAL;

	/* Check if table is descending or ascending */
	if (tablesize > 1) {
		if (pts[0].x < pts[1].x)
			descending = 0;
	}

	while (i < tablesize) {
		if ((descending == 1) && (pts[i].x < input)) {
			/* table entry is less than measured
				value and table is descending, stop */
			break;
		} else if ((descending == 0) &&
				(pts[i].x > input)) {
			/* table entry is greater than measured
				value and table is ascending, stop */
			break;
		} else {
			i++;
		}
	}

	if (i == 0)
		*output = pts[0].y;
	else if (i == tablesize)
		*output = pts[tablesize-1].y;
	else {
		/* result is between search_index and search_index-1 */
		/* interpolate linearly */
		*output = (((int32_t) ((pts[i].y - pts[i-1].y)*
			(input - pts[i-1].x))/
			(pts[i].x - pts[i-1].x))+
			pts[i-1].y);
	}

	return 0;
}

static int32_t pm8xxx_adc_map_batt_therm(const struct pm8xxx_adc_map_pt *pts,
		uint32_t tablesize, int32_t input, int64_t *output)
{
	bool descending = 1;
	uint32_t i = 0;

	if ((pts == NULL) || (output == NULL))
		return -EINVAL;

	/* Check if table is descending or ascending */
	if (tablesize > 1) {
		if (pts[0].y < pts[1].y)
			descending = 0;
	}

	while (i < tablesize) {
		if ((descending == 1) && (pts[i].y < input)) {
			/* table entry is less than measured
				value and table is descending, stop */
			break;
		} else if ((descending == 0) && (pts[i].y > input)) {
			/* table entry is greater than measured
				value and table is ascending, stop */
			break;
		} else {
			i++;
		}
	}

	if (i == 0) {
		*output = pts[0].x;
	} else if (i == tablesize) {
		*output = pts[tablesize-1].x;
	} else {
		/* result is between search_index and search_index-1 */
		/* interpolate linearly */
		*output = (((int32_t) ((pts[i].x - pts[i-1].x)*
			(input - pts[i-1].y))/
			(pts[i].y - pts[i-1].y))+
			pts[i-1].x);
	}

	return 0;
}

int32_t pm8xxx_adc_scale_default(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	bool negative_rawfromoffset = 0, negative_offset = 0;
	int64_t scale_voltage = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;

	scale_voltage = (adc_code -
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].adc_gnd)
		* chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;
	if (scale_voltage < 0) {
		negative_offset = 1;
		scale_voltage = -scale_voltage;
	}
	do_div(scale_voltage,
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dy);
	if (negative_offset)
		scale_voltage = -scale_voltage;
	scale_voltage += chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;

	if (scale_voltage < 0) {
		if (adc_properties->bipolar) {
			scale_voltage = -scale_voltage;
			negative_rawfromoffset = 1;
		} else {
			scale_voltage = 0;
		}
	}

	adc_chan_result->measurement = scale_voltage *
				chan_properties->offset_gain_denominator;

	/* do_div only perform positive integer division! */
	do_div(adc_chan_result->measurement,
				chan_properties->offset_gain_numerator);

	if (negative_rawfromoffset)
		adc_chan_result->measurement = -adc_chan_result->measurement;

	/* Note: adc_chan_result->measurement is in the unit of
	 * adc_properties.adc_reference. For generic channel processing,
	 * channel measurement is a scale/ratio relative to the adc
	 * reference input */
	adc_chan_result->physical = adc_chan_result->measurement;

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_default);

static int64_t pm8xxx_adc_scale_ratiometric_calib(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties)
{
	int64_t adc_voltage = 0;
	bool negative_offset = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties)
		return -EINVAL;

	adc_voltage = (adc_code -
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].adc_gnd)
		* adc_properties->adc_vdd_reference;
	if (adc_voltage < 0) {
		negative_offset = 1;
		adc_voltage = -adc_voltage;
	}
	do_div(adc_voltage,
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].dy);
	if (negative_offset)
		adc_voltage = -adc_voltage;

	return adc_voltage;
}

int32_t pm8xxx_adc_scale_batt_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t bat_voltage = 0;



	int ret;
	struct pm8xxx_adc_chan_result scale_default_result;

	ret = pm8xxx_adc_scale_default(adc_code,
			adc_properties, chan_properties, &scale_default_result);
	if (ret) {
		pr_err("%s: ret = %d\n", __func__, ret);
		return ret;
	}

	bat_voltage = (int)scale_default_result.measurement / 1000;

	return pm8xxx_adc_map_batt_therm(
			ncm_adcmap_btm_threshold,
			ARRAY_SIZE(ncm_adcmap_btm_threshold),
			bat_voltage,
			&adc_chan_result->physical);













}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_batt_therm);

int32_t pm8xxx_adc_scale_pa_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t pa_voltage = 0;

	pa_voltage = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);



    return pm8xxx_adc_map_linear(
            ncm_adcmap_pa_therm,
            ARRAY_SIZE(ncm_adcmap_pa_therm),
            pa_voltage,
            &adc_chan_result->physical);









}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_pa_therm);

int32_t pm8xxx_adc_scale_batt_id(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t batt_id_voltage = 0;

	batt_id_voltage = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);
	adc_chan_result->physical = batt_id_voltage;
	adc_chan_result->physical = adc_chan_result->measurement;

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_batt_id);

int32_t pm8xxx_adc_scale_pmic_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t pmic_voltage = 0;
	bool negative_offset = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;

	pmic_voltage = (adc_code -
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].adc_gnd)
		* chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;
	if (pmic_voltage < 0) {
		negative_offset = 1;
		pmic_voltage = -pmic_voltage;
	}
	do_div(pmic_voltage,
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dy);
	if (negative_offset)
		pmic_voltage = -pmic_voltage;
	pmic_voltage += chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;

	if (pmic_voltage > 0) {
		/* 2mV/K */
		adc_chan_result->measurement = pmic_voltage*
			chan_properties->offset_gain_denominator;

		do_div(adc_chan_result->measurement,
			chan_properties->offset_gain_numerator * 2);
	} else {
		adc_chan_result->measurement = 0;
	}
	/* Change to .001 deg C */
	adc_chan_result->measurement -= KELVINMIL_DEGMIL;
	adc_chan_result->physical = (int32_t)adc_chan_result->measurement;

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_pmic_therm);

/* Scales the ADC code to 0.001 degrees C using the map
 * table for the XO thermistor.
 */
int32_t pm8xxx_adc_tdkntcg_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t xo_thm = 0;



	int ret;
	struct pm8xxx_adc_chan_result scale_default_result;


	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;


	ret = pm8xxx_adc_scale_default(adc_code,
			adc_properties, chan_properties, &scale_default_result);
	if (ret) {
		pr_err("%s: ret = %d\n", __func__, ret);
		return ret;
	}

	xo_thm = (int)scale_default_result.measurement / 1000;

	pm8xxx_adc_map_linear(ncm_adcmap_ntcg_104ef_104fb,
		ARRAY_SIZE(ncm_adcmap_ntcg_104ef_104fb),
		xo_thm, &adc_chan_result->physical);












	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_tdkntcg_therm);

int32_t pm8xxx_adc_batt_scaler(struct pm8xxx_adc_arb_btm_param *btm_param,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties)
{
	int rc;



    rc = pm8xxx_adc_map_linear(
        ncm_adcmap_btm_threshold,
        ARRAY_SIZE(ncm_adcmap_btm_threshold),
        (btm_param->low_thr_temp),
        &btm_param->low_thr_voltage);









	if (rc)
		return rc;

	btm_param->low_thr_voltage *=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].dy;
	do_div(btm_param->low_thr_voltage, adc_properties->adc_vdd_reference);
	btm_param->low_thr_voltage +=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].adc_gnd;



    rc = pm8xxx_adc_map_linear(
        ncm_adcmap_btm_threshold,
        ARRAY_SIZE(ncm_adcmap_btm_threshold),
        (btm_param->high_thr_temp),
        &btm_param->high_thr_voltage);









	if (rc)
		return rc;

	btm_param->high_thr_voltage *=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].dy;
	do_div(btm_param->high_thr_voltage, adc_properties->adc_vdd_reference);
	btm_param->high_thr_voltage +=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].adc_gnd;


	return rc;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_batt_scaler);
