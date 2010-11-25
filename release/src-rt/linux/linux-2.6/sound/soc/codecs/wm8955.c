/*
 * wm8955.c -- WM8955 ALSA SoC audio driver
 *
 * Copyright 
 *
 * Author: 
 *
 * Based on WM8750.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>

#include "wm8955.h"

#define AUDIO_NAME "WM8955"
#define WM8955_VERSION "0.12"

/*
 * Debug
 */

#define WM8955_DEBUG 0

#ifdef WM8955_DEBUG
#define dbg(format, arg...) \
	printk(KERN_DEBUG AUDIO_NAME ": " format "\n" , ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif
#define err(format, arg...) \
	printk(KERN_ERR AUDIO_NAME ": " format "\n" , ## arg)
#define info(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": " format "\n" , ## arg)
#define warn(format, arg...) \
	printk(KERN_WARNING AUDIO_NAME ": " format "\n" , ## arg)

/* codec private data */
struct wm8955_priv {
	unsigned int sysclk;
};

/*
 * wm8955 register cache
 * We can't read the WM8955 register space when we
 * are using 2 wire for device control, so we cache them instead.
 */
static const u16 wm8955_reg[] = {
	0x0000, 0x0000, 0x0079, 0x0079,  /*  0 */
	0x0000, 0x0008, 0x0000, 0x000a,  /*  4 */
	0x0000, 0x0000, 0x00ff, 0x00ff,  /*  8 */
	0x000f, 0x000f, 0x0000, 0x0000,  /* 12 */
	0x0000, 0x0000, 0x0000, 0x0000,  /* 16 */
	0x0000, 0x0000, 0x0000, 0x00c1,  /* 20 */
	0x0000, 0x0000, 0x0000, 0x0000,  /* 24 */
	0x0000, 0x0000, 0x0000, 0x0000,  /* 28 */
	0x0000, 0x0000, 0x0050, 0x0050,  /* 32 */
	0x0050, 0x0050, 0x0050, 0x0050,  /* 36 */
	0x0079, 0x0079, 0x0079, 0x0000,  /* 40 */
	0x0103, 0x0024, 0x01ba, 0x0000,  /* 44 */
};

/*
 * read wm8955 register cache
 */
static inline unsigned int wm8955_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u16 *cache = codec->reg_cache;
	if (reg > WM8955_CACHE_REGNUM)
		return -1;
	return cache[reg];
}

/*
 * write wm8955 register cache
 */
static inline void wm8955_write_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value)
{
	u16 *cache = codec->reg_cache;
	if (reg > WM8955_CACHE_REGNUM)
		return;
	cache[reg] = value;
}

static int wm8955_write(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value)
{
	u8 data[2];

	/* data is
	 *   D15..D9 WM8955 register offset
	 *   D8...D0 register data
	 */
	data[0] = (reg << 1) | ((value >> 8) & 0x0001);
	data[1] = value & 0x00ff;

	wm8955_write_reg_cache (codec, reg, value);
	if (codec->hw_write(codec->control_data, data, 2) == 2)
		return 0;
	else
		return -EIO;
}

#define wm8955_reset(c)	wm8955_write(c, WM8955_RESET, 0)

/*
 * WM8955 Controls
 */
static const char *wm8955_bass[] = {"Linear Control", "Adaptive Boost"};
static const char *wm8955_bass_filter[] = { "130Hz @ 48kHz", "200Hz @ 48kHz" };
static const char *wm8955_treble[] = {"8kHz", "4kHz"};
static const char *wm8955_line_mux[] = {"Line 1", "Line 2", "Line 3", "PGA",
	"Differential"};
static const char *wm8955_out3[] = {"VREF", "ROUT1 + Vol", "MonoOut",
	"ROUT1"};
static const char *wm8955_deemph[] = {"None", "32Khz", "44.1Khz", "48Khz"};

static const struct soc_enum wm8955_enum[] = {
SOC_ENUM_SINGLE(WM8955_BASS, 7, 2, wm8955_bass),
SOC_ENUM_SINGLE(WM8955_BASS, 6, 2, wm8955_bass_filter),
SOC_ENUM_SINGLE(WM8955_TREBLE, 6, 2, wm8955_treble),
SOC_ENUM_SINGLE(WM8955_LOUTM1, 0, 5, wm8955_line_mux),
SOC_ENUM_SINGLE(WM8955_ROUTM1, 0, 5, wm8955_line_mux),
SOC_ENUM_SINGLE(WM8955_ADCTL2, 7, 4, wm8955_out3),
SOC_ENUM_SINGLE(WM8955_DACCTL, 1, 4, wm8955_deemph),
};

static const struct snd_kcontrol_new wm8955_snd_controls[] = {

SOC_DOUBLE_R("Headphone Playback ZC Switch", WM8955_LOUT1V,
	WM8955_ROUT1V, 7, 1, 0),
SOC_DOUBLE_R("Speaker Playback ZC Switch", WM8955_LOUT2V,
	WM8955_ROUT2V, 7, 1, 0),

SOC_ENUM("Playback De-emphasis", wm8955_enum[6]),

SOC_SINGLE("Playback 6dB Attenuate", WM8955_DACCTL, 7, 1, 0),

SOC_DOUBLE_R("PCM Volume", WM8955_LDAC, WM8955_RDAC, 0, 255, 0),

SOC_ENUM("Bass Boost", wm8955_enum[0]),
SOC_ENUM("Bass Filter", wm8955_enum[1]),
SOC_SINGLE("Bass Volume", WM8955_BASS, 0, 15, 1),

SOC_SINGLE("Treble Volume", WM8955_TREBLE, 0, 15, 0),
SOC_ENUM("Treble Cut-off", wm8955_enum[2]),

SOC_SINGLE("ZC Timeout Switch", WM8955_ADCTL1, 0, 1, 0),
SOC_SINGLE("Playback Invert Switch", WM8955_ADCTL1, 1, 1, 0),

SOC_SINGLE("Right Speaker Playback Invert Switch", WM8955_ADCTL2, 4, 1, 0),

/* Unimplemented */
/* ADCTL1 Bit 4,5 - DMONOMIX */
/* ADCTL1 Bit 6,7 - VSEL */
/* ADCTL2 Bit 3 - HPSWZC */
/* ADCTL3 Bit 6 - VROI */

SOC_DOUBLE_R("Bypass Left Playback Volume", WM8955_LOUTM1,
	WM8955_LOUTM2, 4, 7, 1),
SOC_DOUBLE_R("Bypass Right Playback Volume", WM8955_ROUTM1,
	WM8955_ROUTM2, 4, 7, 1),
SOC_DOUBLE_R("Bypass Mono Playback Volume", WM8955_MOUTM1,
	WM8955_MOUTM2, 4, 7, 1),

SOC_SINGLE("Mono Playback ZC Switch", WM8955_MOUTV, 7, 1, 0),

SOC_DOUBLE_R("Headphone Playback Volume", WM8955_LOUT1V, WM8955_ROUT1V,
	0, 127, 0),
SOC_DOUBLE_R("Speaker Playback Volume", WM8955_LOUT2V, WM8955_ROUT2V,
	0, 127, 0),

SOC_SINGLE("Mono Playback Volume", WM8955_MOUTV, 0, 127, 0),

};

/* add non dapm controls */
static int wm8955_add_controls(struct snd_soc_codec *codec)
{
	int err, i;

	for (i = 0; i < ARRAY_SIZE(wm8955_snd_controls); i++) {
		err = snd_ctl_add(codec->card,
				snd_soc_cnew(&wm8955_snd_controls[i],codec, NULL));
		if (err < 0)
			return err;
	}
	return 0;
}

/*
 * DAPM Controls
 */

/* Left Mixer */
static const struct snd_kcontrol_new wm8955_left_mixer_controls[] = {
SOC_DAPM_SINGLE("Playback Switch", WM8955_LOUTM1, 8, 1, 0),
SOC_DAPM_SINGLE("Left Bypass Switch", WM8955_LOUTM1, 7, 1, 0),
SOC_DAPM_SINGLE("Right Playback Switch", WM8955_LOUTM2, 8, 1, 0),
SOC_DAPM_SINGLE("Right Bypass Switch", WM8955_LOUTM2, 7, 1, 0),
};

/* Right Mixer */
static const struct snd_kcontrol_new wm8955_right_mixer_controls[] = {
SOC_DAPM_SINGLE("Left Playback Switch", WM8955_ROUTM1, 8, 1, 0),
SOC_DAPM_SINGLE("Left Bypass Switch", WM8955_ROUTM1, 7, 1, 0),
SOC_DAPM_SINGLE("Playback Switch", WM8955_ROUTM2, 8, 1, 0),
SOC_DAPM_SINGLE("Right Bypass Switch", WM8955_ROUTM2, 7, 1, 0),
};

/* Mono Mixer */
static const struct snd_kcontrol_new wm8955_mono_mixer_controls[] = {
SOC_DAPM_SINGLE("Left Playback Switch", WM8955_MOUTM1, 8, 1, 0),
SOC_DAPM_SINGLE("Left Bypass Switch", WM8955_MOUTM1, 7, 1, 0),
SOC_DAPM_SINGLE("Right Playback Switch", WM8955_MOUTM2, 8, 1, 0),
SOC_DAPM_SINGLE("Right Bypass Switch", WM8955_MOUTM2, 7, 1, 0),
};

/* Left Line Mux */
static const struct snd_kcontrol_new wm8955_left_line_controls =
SOC_DAPM_ENUM("Route", wm8955_enum[3]);

/* Right Line Mux */
static const struct snd_kcontrol_new wm8955_right_line_controls =
SOC_DAPM_ENUM("Route", wm8955_enum[4]);

/* Out 3 Mux */
static const struct snd_kcontrol_new wm8955_out3_controls =
SOC_DAPM_ENUM("Route", wm8955_enum[5]);

static const struct snd_soc_dapm_widget wm8955_dapm_widgets[] = {
	SND_SOC_DAPM_MIXER("Left Mixer", SND_SOC_NOPM, 0, 0,
		&wm8955_left_mixer_controls[0],
		ARRAY_SIZE(wm8955_left_mixer_controls)),
	SND_SOC_DAPM_MIXER("Right Mixer", SND_SOC_NOPM, 0, 0,
		&wm8955_right_mixer_controls[0],
		ARRAY_SIZE(wm8955_right_mixer_controls)),
	SND_SOC_DAPM_MIXER("Mono Mixer", WM8955_PWR2, 2, 0,
		&wm8955_mono_mixer_controls[0],
		ARRAY_SIZE(wm8955_mono_mixer_controls)),

	SND_SOC_DAPM_PGA("Right Out 2", WM8955_PWR2, 3, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Left Out 2", WM8955_PWR2, 4, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Right Out 1", WM8955_PWR2, 5, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Left Out 1", WM8955_PWR2, 6, 0, NULL, 0),
	SND_SOC_DAPM_DAC("Right DAC", "Right Playback", WM8955_PWR2, 7, 0),
	SND_SOC_DAPM_DAC("Left DAC", "Left Playback", WM8955_PWR2, 8, 0),

	SND_SOC_DAPM_MICBIAS("Mic Bias", WM8955_PWR1, 1, 0),
	SND_SOC_DAPM_ADC("Right ADC", "Right Capture", WM8955_PWR1, 2, 0),
	SND_SOC_DAPM_ADC("Left ADC", "Left Capture", WM8955_PWR1, 3, 0),

	SND_SOC_DAPM_MUX("Left Line Mux", SND_SOC_NOPM, 0, 0,
		&wm8955_left_line_controls),
	SND_SOC_DAPM_MUX("Right Line Mux", SND_SOC_NOPM, 0, 0,
		&wm8955_right_line_controls),

	SND_SOC_DAPM_MUX("Out3 Mux", SND_SOC_NOPM, 0, 0, &wm8955_out3_controls),
	SND_SOC_DAPM_PGA("Out 3", WM8955_PWR2, 1, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Mono Out 1", WM8955_PWR2, 2, 0, NULL, 0),

	SND_SOC_DAPM_OUTPUT("LOUT1"),
	SND_SOC_DAPM_OUTPUT("ROUT1"),
	SND_SOC_DAPM_OUTPUT("LOUT2"),
	SND_SOC_DAPM_OUTPUT("ROUT2"),
	SND_SOC_DAPM_OUTPUT("MONO"),
	SND_SOC_DAPM_OUTPUT("OUT3"),

	SND_SOC_DAPM_INPUT("LINPUT1"),
	SND_SOC_DAPM_INPUT("LINPUT2"),
	SND_SOC_DAPM_INPUT("LINPUT3"),
	SND_SOC_DAPM_INPUT("RINPUT1"),
	SND_SOC_DAPM_INPUT("RINPUT2"),
	SND_SOC_DAPM_INPUT("RINPUT3"),
};

static const char *audio_map[][3] = {
	/* left mixer */
	{"Left Mixer", "Playback Switch", "Left DAC"},
	{"Left Mixer", "Left Bypass Switch", "Left Line Mux"},
	{"Left Mixer", "Right Playback Switch", "Right DAC"},
	{"Left Mixer", "Right Bypass Switch", "Right Line Mux"},

	/* right mixer */
	{"Right Mixer", "Left Playback Switch", "Left DAC"},
	{"Right Mixer", "Left Bypass Switch", "Left Line Mux"},
	{"Right Mixer", "Playback Switch", "Right DAC"},
	{"Right Mixer", "Right Bypass Switch", "Right Line Mux"},

	/* left out 1 */
	{"Left Out 1", NULL, "Left Mixer"},
	{"LOUT1", NULL, "Left Out 1"},

	/* left out 2 */
	{"Left Out 2", NULL, "Left Mixer"},
	{"LOUT2", NULL, "Left Out 2"},

	/* right out 1 */
	{"Right Out 1", NULL, "Right Mixer"},
	{"ROUT1", NULL, "Right Out 1"},

	/* right out 2 */
	{"Right Out 2", NULL, "Right Mixer"},
	{"ROUT2", NULL, "Right Out 2"},

	/* mono mixer */
	{"Mono Mixer", "Left Playback Switch", "Left DAC"},
	{"Mono Mixer", "Left Bypass Switch", "Left Line Mux"},
	{"Mono Mixer", "Right Playback Switch", "Right DAC"},
	{"Mono Mixer", "Right Bypass Switch", "Right Line Mux"},

	/* mono out */
	{"Mono Out 1", NULL, "Mono Mixer"},
	{"MONO1", NULL, "Mono Out 1"},

	/* out 3 */
	{"Out3 Mux", "VREF", "VREF"},
	{"Out3 Mux", "ROUT1 + Vol", "ROUT1"},
	{"Out3 Mux", "ROUT1", "Right Mixer"},
	{"Out3 Mux", "MonoOut", "MONO1"},
	{"Out 3", NULL, "Out3 Mux"},
	{"OUT3", NULL, "Out 3"},

	/* Left Line Mux */
	{"Left Line Mux", "Line 1", "LINPUT1"},
	{"Left Line Mux", "Line 2", "LINPUT2"},
	{"Left Line Mux", "Line 3", "LINPUT3"},
	{"Left Line Mux", "PGA", "Left PGA Mux"},
	{"Left Line Mux", "Differential", "Differential Mux"},

	/* Right Line Mux */
	{"Right Line Mux", "Line 1", "RINPUT1"},
	{"Right Line Mux", "Line 2", "RINPUT2"},
	{"Right Line Mux", "Line 3", "RINPUT3"},
	{"Right Line Mux", "PGA", "Right PGA Mux"},
	{"Right Line Mux", "Differential", "Differential Mux"},

	/* Left PGA Mux */
	{"Left PGA Mux", "Line 1", "LINPUT1"},
	{"Left PGA Mux", "Line 2", "LINPUT2"},
	{"Left PGA Mux", "Line 3", "LINPUT3"},
	{"Left PGA Mux", "Differential", "Differential Mux"},

	/* Right PGA Mux */
	{"Right PGA Mux", "Line 1", "RINPUT1"},
	{"Right PGA Mux", "Line 2", "RINPUT2"},
	{"Right PGA Mux", "Line 3", "RINPUT3"},
	{"Right PGA Mux", "Differential", "Differential Mux"},

	/* Differential Mux */
	{"Differential Mux", "Line 1", "LINPUT1"},
	{"Differential Mux", "Line 1", "RINPUT1"},
	{"Differential Mux", "Line 2", "LINPUT2"},
	{"Differential Mux", "Line 2", "RINPUT2"},

	/* Left ADC Mux */
	{"Left ADC Mux", "Stereo", "Left PGA Mux"},
	{"Left ADC Mux", "Mono (Left)", "Left PGA Mux"},
	{"Left ADC Mux", "Digital Mono", "Left PGA Mux"},

	/* Right ADC Mux */
	{"Right ADC Mux", "Stereo", "Right PGA Mux"},
	{"Right ADC Mux", "Mono (Right)", "Right PGA Mux"},
	{"Right ADC Mux", "Digital Mono", "Right PGA Mux"},

	/* ADC */
	{"Left ADC", NULL, "Left ADC Mux"},
	{"Right ADC", NULL, "Right ADC Mux"},

	/* terminator */
	{NULL, NULL, NULL},
};

static int wm8955_add_widgets(struct snd_soc_codec *codec)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(wm8955_dapm_widgets); i++) {
		snd_soc_dapm_new_control(codec, &wm8955_dapm_widgets[i]);
	}

	/* set up audio path audio_mapnects */
	for(i = 0; audio_map[i][0] != NULL; i++) {
		snd_soc_dapm_connect_input(codec, audio_map[i][0],
			audio_map[i][1], audio_map[i][2]);
	}

	snd_soc_dapm_new_widgets(codec);
	return 0;
}

/* PLL divisors */
struct _pll_div {
	u32 div2:1;
	u32 n:4;
	u32 k:24;
};

/* The size in bits of the pll divide multiplied by 10
 * to allow rounding later */
#define FIXED_PLL_SIZE ((1 << 22) * 10)

static void pll_factors(struct _pll_div *pll_div, unsigned int target,
	unsigned int source)
{
	u64 Kpart;
	unsigned int K, Ndiv, Nmod;

	Ndiv = target / source;
	if (Ndiv < 6) {
		source >>= 1;
		pll_div->div2 = 1;
		Ndiv = target / source;
	} else
		pll_div->div2 = 0;

	if ((Ndiv < 6) || (Ndiv > 12))
		printk(KERN_WARNING
			"WM8955 N value outwith recommended range! N = %d\n",Ndiv);

	pll_div->n = Ndiv;
	Nmod = target % source;
	Kpart = FIXED_PLL_SIZE * (long long)Nmod;

	do_div(Kpart, source);

	K = Kpart & 0xFFFFFFFF;

	/* Check if we need to round */
	if ((K % 10) >= 5)
		K += 5;

	/* Move down to proper range now rounding is done */
	K /= 10;

	pll_div->k = K;
}

static int wm8955_set_dai_pll(struct snd_soc_codec_dai *codec_dai,
		int pll_id, unsigned int freq_in, unsigned int freq_out)
{
	u16 reg, enable = 0x1F8;
	struct snd_soc_codec *codec = codec_dai->codec;

	/* read and mask off PLLEN bit */
	reg = wm8955_read_reg_cache(codec, WM8955_CLOCK) & 0xfff7;

	if (!freq_in || !freq_out) {
		/* disable PLL  */
		wm8955_write(codec, WM8955_CLOCK, reg);
		return 0;
	} else {
		u16 value = 0;
		struct _pll_div pll_div;

		pll_factors(&pll_div, freq_out * 8, freq_in);

		/* set up N and K PLL divisor ratios */
		/* bits 8:5 = PLL_N, bits 3:0 = PLL_K[21:18] */
		value = (pll_div.n << 5) + ((pll_div.k & 0x3c0000) >> 18);
		wm8955_write(codec, WM8955_PLLCTL1, value);

		/* bits 8:0 = PLL_K[17:9] */
		value = (pll_div.k & 0x03fe00) >> 9;
		wm8955_write(codec, WM8955_PLLCTL2, value);

		/* bits 8:0 = PLL_K[8:0] */
		value = pll_div.k & 0x0001ff;
		wm8955_write(codec, WM8955_PLLCTL3, value);

		/* use fractional "K" */
		wm8955_write(codec, WM8955_PLLCTL4, 0x80);

		/* Enable the PLL */
		enable |= (pll_div.div2 << 5);
		wm8955_write(codec, WM8955_CLOCK, reg | enable);
	}
	return 0;
}

struct _coeff_div {
	u32 mclk;
	u32 rate;
	u16 fs;
	u8 sr:5;
	u8 usb:1;
};

/* codec hifi mclk clock divider coefficients */
static const struct _coeff_div coeff_div[] = {
	/* 8k */
	{12288000, 8000, 1536, 0x2, 0x0},
	{11289600, 8000, 1408, 0x12, 0x0},
	{18432000, 8000, 2304, 0x3, 0x0},
	{16934400, 8000, 2112, 0x13, 0x0},
	{12000000, 8000, 1500, 0x2, 0x1},

	/* 11.025k */
	{11289600, 11025, 1024, 0x18, 0x0},
	{16934400, 11025, 1536, 0x19, 0x0},
	{12000000, 11025, 1088, 0x19, 0x1},

	/* 16k */
	{12288000, 16000, 768, 0xa, 0x0},
	{18432000, 16000, 1152, 0xb, 0x0},
	{12000000, 16000, 750, 0xa, 0x1},

	/* 22.05k */
	{11289600, 22050, 512, 0x1a, 0x0},
	{16934400, 22050, 768, 0x1b, 0x0},
	{12000000, 22050, 544, 0x1b, 0x1},

	/* 32k */
	{12288000, 32000, 384, 0xc, 0x0},
	{18432000, 32000, 576, 0xd, 0x0},
	{12000000, 32000, 375, 0xc, 0x1},

	/* 44.1k */
	{11289600, 44100, 256, 0x10, 0x0},
	{16934400, 44100, 384, 0x11, 0x0},
	{12000000, 44100, 272, 0x11, 0x1},

	/* 48k */
	{12288000, 48000, 256, 0x0, 0x0},
	{18432000, 48000, 384, 0x1, 0x0},
	{12000000, 48000, 250, 0x0, 0x1},

	/* 88.2k */
	{11289600, 88200, 128, 0x1e, 0x0},
	{16934400, 88200, 192, 0x1f, 0x0},
	{12000000, 88200, 136, 0x1f, 0x1},

	/* 96k */
	{12288000, 96000, 128, 0xe, 0x0},
	{18432000, 96000, 192, 0xf, 0x0},
	{12000000, 96000, 125, 0xe, 0x1},
};

static inline int get_coeff(int mclk, int rate)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(coeff_div); i++) {
		if (coeff_div[i].rate == rate && coeff_div[i].mclk == mclk)
			return i;
	}

	printk(KERN_ERR "wm8955: could not get coeff for mclk %d @ rate %d\n",
		mclk, rate);
	return -EINVAL;
}

static int wm8955_set_dai_sysclk(struct snd_soc_codec_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct wm8955_priv *wm8955 = codec->private_data;

	switch (freq) {
	case 11289600:
	case 12000000:
	case 12288000:
	case 16934400:
	case 18432000:
		wm8955->sysclk = freq;
		return 0;
	}
	return -EINVAL;
}

static int wm8955_set_dai_fmt(struct snd_soc_codec_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 iface = 0;

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		iface = 0x0040;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		return -EINVAL;
	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface |= 0x0002;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface |= 0x0001;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface |= 0x0003;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		iface |= 0x0013;
		break;
	default:
		return -EINVAL;
	}

	/* clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_IF:
		iface |= 0x0090;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface |= 0x0080;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		iface |= 0x0010;
		break;
	default:
		return -EINVAL;
	}

	wm8955_write(codec, WM8955_IFACE, iface);
	return 0;
}

static int wm8955_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->codec;
	struct wm8955_priv *wm8955 = codec->private_data;
	u16 iface = wm8955_read_reg_cache(codec, WM8955_IFACE) & 0x1f3;
	u16 srate = wm8955_read_reg_cache(codec, WM8955_SRATE) & 0x1c0;
	int coeff = get_coeff(wm8955->sysclk, params_rate(params));

	/* bit size */
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		iface |= 0x0004;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		iface |= 0x0008;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		iface |= 0x000c;
		break;
	}

	/* set iface & srate */
	wm8955_write(codec, WM8955_IFACE, iface);
	if (coeff >= 0)

		wm8955_write(codec, WM8955_SRATE, srate |
			(coeff_div[coeff].sr << 1) | coeff_div[coeff].usb | 0x40);

	return 0;
}

static int wm8955_mute(struct snd_soc_codec_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
	u16 mute_reg = wm8955_read_reg_cache(codec, WM8955_DACCTL) & 0xfff7;

	if (mute)
		wm8955_write(codec, WM8955_DACCTL, mute_reg | 0x8);
	else
		wm8955_write(codec, WM8955_DACCTL, mute_reg);
	return 0;
}

static int wm8955_dapm_event(struct snd_soc_codec *codec, int event)
{
	u16 pwr_reg = wm8955_read_reg_cache(codec, WM8955_PWR1) & 0xfe3e;

	switch (event) {
	case SNDRV_CTL_POWER_D0: /* full On */
		/* Turn On LDAC/RDAC, LOUT2/ROUT2 */
		wm8955_write(codec, WM8955_PWR2, 0x198);
		/* set vmid to 50k and unmute dac */
		wm8955_write(codec, WM8955_PWR1, pwr_reg | 0x00c0);
		break;
	case SNDRV_CTL_POWER_D1: /* partial On */
	case SNDRV_CTL_POWER_D2: /* partial On */
		/* set vmid to 5k for quick power up */
		wm8955_write(codec, WM8955_PWR1, pwr_reg | 0x01c1);
		break;
	case SNDRV_CTL_POWER_D3hot: /* Off, with power */
		/* mute dac and set vmid to 500k, enable VREF */
		wm8955_write(codec, WM8955_PWR1, pwr_reg | 0x0141);
		break;
	case SNDRV_CTL_POWER_D3cold: /* Off, without power */
		wm8955_write(codec, WM8955_PWR1, 0x0001);
		wm8955_write(codec, WM8955_PWR2, 0x0);
		break;
	}
	codec->dapm_state = event;
	return 0;
}

#define WM8955_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
		SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_44100 | \
		SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

#define WM8955_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
	SNDRV_PCM_FMTBIT_S24_LE)

struct snd_soc_codec_dai wm8955_dai = {
	.name = "WM8955",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WM8955_RATES,
		.formats = WM8955_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WM8955_RATES,
		.formats = WM8955_FORMATS,},
	.ops = {
		.hw_params = wm8955_pcm_hw_params,
	},
	.dai_ops = {
		.digital_mute = wm8955_mute,
		.set_fmt = wm8955_set_dai_fmt,
		.set_pll = wm8955_set_dai_pll,
		.set_sysclk = wm8955_set_dai_sysclk,
	},
};
EXPORT_SYMBOL_GPL(wm8955_dai);

static void wm8955_work(struct work_struct *work)
{
	struct snd_soc_codec *codec =
		container_of(work, struct snd_soc_codec, delayed_work.work);
	wm8955_dapm_event(codec, codec->dapm_state);
}

static int wm8955_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->codec;

	wm8955_dapm_event(codec, SNDRV_CTL_POWER_D3cold);
	return 0;
}

static int wm8955_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->codec;
	int i;
	u8 data[2];
	u16 *cache = codec->reg_cache;

	/* Sync reg_cache with the hardware */
	for (i = 0; i < ARRAY_SIZE(wm8955_reg); i++) {
		if (i == WM8955_RESET)
			continue;
		data[0] = (i << 1) | ((cache[i] >> 8) & 0x0001);
		data[1] = cache[i] & 0x00ff;
		codec->hw_write(codec->control_data, data, 2);
	}

	wm8955_dapm_event(codec, SNDRV_CTL_POWER_D3hot);

	/* charge wm8955 caps */
	if (codec->suspend_dapm_state == SNDRV_CTL_POWER_D0) {
		wm8955_dapm_event(codec, SNDRV_CTL_POWER_D2);
		codec->dapm_state = SNDRV_CTL_POWER_D0;
		schedule_delayed_work(&codec->delayed_work, msecs_to_jiffies(1000));
	}

	return 0;
}

/*
 * initialise the WM8955 driver
 * register the mixer and dsp interfaces with the kernel
 */
static int wm8955_init(struct snd_soc_device *socdev)
{
	struct snd_soc_codec *codec = socdev->codec;
	int reg, ret = 0;

	codec->name = "WM8955";
	codec->owner = THIS_MODULE;
	codec->read = wm8955_read_reg_cache;
	codec->write = wm8955_write;
	codec->dapm_event = wm8955_dapm_event;
	codec->dai = &wm8955_dai;
	codec->num_dai = 1;
	codec->reg_cache_size = sizeof(wm8955_reg);
	codec->reg_cache = kmemdup(wm8955_reg, sizeof(wm8955_reg), GFP_KERNEL);
	if (codec->reg_cache == NULL)
		return -ENOMEM;

	wm8955_reset(codec);

	/* register pcms */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		printk(KERN_ERR "wm8955: failed to create pcms\n");
		goto pcm_err;
	}

	/* charge output caps */
	wm8955_dapm_event(codec, SNDRV_CTL_POWER_D2);
	codec->dapm_state = SNDRV_CTL_POWER_D3hot;
	schedule_delayed_work(&codec->delayed_work, msecs_to_jiffies(1000));

	/* set the update bits */
	reg = wm8955_read_reg_cache(codec, WM8955_LDAC);
	wm8955_write(codec, WM8955_LDAC, reg | 0x0100);
	reg = wm8955_read_reg_cache(codec, WM8955_RDAC);
	wm8955_write(codec, WM8955_RDAC, reg | 0x0100);
	reg = wm8955_read_reg_cache(codec, WM8955_LOUT1V);
	wm8955_write(codec, WM8955_LOUT1V, reg | 0x0100);
	reg = wm8955_read_reg_cache(codec, WM8955_ROUT1V);
	wm8955_write(codec, WM8955_ROUT1V, reg | 0x0100);
	/* Read LOUT2V/ROUT2V and strip volume bits */
	reg = wm8955_read_reg_cache(codec, WM8955_LOUT2V) & 0x180;
	wm8955_write(codec, WM8955_LOUT2V, reg | 0x015f);
	reg = wm8955_read_reg_cache(codec, WM8955_ROUT2V) * 0x180;
	wm8955_write(codec, WM8955_ROUT2V, reg | 0x015f);

	/* Enable L & R Digital Inputs for the L & R Outputs of the mixer */
	reg = wm8955_read_reg_cache(codec, WM8955_LOUTM1);
	wm8955_write(codec, WM8955_LOUTM1, reg | 0x0100);
	reg = wm8955_read_reg_cache(codec, WM8955_ROUTM1);
	wm8955_write(codec, WM8955_ROUTM2, reg | 0x0100);

	wm8955_add_controls(codec);
	wm8955_add_widgets(codec);
	ret = snd_soc_register_card(socdev);
	if (ret < 0) {
		printk(KERN_ERR "wm8955: failed to register card\n");
		goto card_err;
	}
	return ret;

card_err:
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
pcm_err:
	kfree(codec->reg_cache);
	return ret;
}

/* If the i2c layer weren't so broken, we could pass this kind of data
   around */
static struct snd_soc_device *wm8955_socdev;

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)

/*
 * WM8955 2 wire address is determined by GPIO5
 * state during powerup.
 *    low  = 0x1a
 *    high = 0x1b
 */
static unsigned short normal_i2c[] = { 0, I2C_CLIENT_END };

/* Magic definition of all other variables and things */
I2C_CLIENT_INSMOD;

static struct i2c_driver wm8955_i2c_driver;
static struct i2c_client client_template;

static int wm8955_codec_probe(struct i2c_adapter *adap, int addr, int kind)
{
	struct snd_soc_device *socdev = wm8955_socdev;
	struct wm8955_setup_data *setup = socdev->codec_data;
	struct snd_soc_codec *codec = socdev->codec;
	struct i2c_client *i2c;
	int ret;

	if (addr != setup->i2c_address)
		return -ENODEV;

	client_template.adapter = adap;
	client_template.addr = addr;

	i2c = kmemdup(&client_template, sizeof(client_template), GFP_KERNEL);
	if (i2c == NULL) {
		kfree(codec);
		return -ENOMEM;
	}
	i2c_set_clientdata(i2c, codec);
	codec->control_data = i2c;

	ret = i2c_attach_client(i2c);
	if (ret < 0) {
		err("failed to attach codec at addr %x\n", addr);
		goto err;
	}

	ret = wm8955_init(socdev);
	if (ret < 0) {
	err("failed to initialise WM8955\n");
		goto err;
	}
	return ret;

err:
	kfree(codec);
	kfree(i2c);
	return ret;
}

static int wm8955_i2c_detach(struct i2c_client *client)
{
	struct snd_soc_codec *codec = i2c_get_clientdata(client);
	i2c_detach_client(client);
	kfree(codec->reg_cache);
	kfree(client);
	return 0;
}

static int wm8955_i2c_attach(struct i2c_adapter *adap)
{
	return i2c_probe(adap, &addr_data, wm8955_codec_probe);
}

/* corgi i2c codec control layer */
static struct i2c_driver wm8955_i2c_driver = {
	.driver = {
		.name = "WM8955 I2C Codec",
		.owner = THIS_MODULE,
	},
	.id =             I2C_DRIVERID_WM8955,
	.attach_adapter = wm8955_i2c_attach,
	.detach_client =  wm8955_i2c_detach,
	.command =        NULL,
};

static struct i2c_client client_template = {
	.name =   "WM8955",
	.driver = &wm8955_i2c_driver,
};
#endif

static int wm8955_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct wm8955_setup_data *setup = socdev->codec_data;
	struct snd_soc_codec *codec;
	struct wm8955_priv *wm8955;
	int ret = 0;

	info("WM8955 Audio Codec %s", WM8955_VERSION);
	codec = kzalloc(sizeof(struct snd_soc_codec), GFP_KERNEL);
	if (codec == NULL)
		return -ENOMEM;

	wm8955 = kzalloc(sizeof(struct wm8955_priv), GFP_KERNEL);
	if (wm8955 == NULL) {
		kfree(codec);
		return -ENOMEM;
	}

	codec->private_data = wm8955;
	socdev->codec = codec;
	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);
	wm8955_socdev = socdev;
	INIT_DELAYED_WORK(&codec->delayed_work, wm8955_work);
	
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	if (setup->i2c_address) {
		normal_i2c[0] = setup->i2c_address;
		codec->hw_write = (hw_write_t)i2c_master_send;
		ret = i2c_add_driver(&wm8955_i2c_driver);
		if (ret != 0)
			printk(KERN_ERR "can't add i2c driver");
	}
#else
		/* Add other interfaces here */
#endif

	return ret;
}

/*
 * This function forces any delayed work to be queued and run.
 */
static int run_delayed_work(struct delayed_work *dwork)
{
	int ret;

	/* cancel any work waiting to be queued. */
	ret = cancel_delayed_work(dwork);

	/* if there was any work waiting then we run it now and
	 * wait for it's completion */
	if (ret) {
		schedule_delayed_work(dwork, 0);
		flush_scheduled_work();
	}
	return ret;
}

/* power down chip */
static int wm8955_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->codec;

	if (codec->control_data)
		wm8955_dapm_event(codec, SNDRV_CTL_POWER_D3cold);
	run_delayed_work(&codec->delayed_work);
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	i2c_del_driver(&wm8955_i2c_driver);
#endif
	kfree(codec->private_data);
	kfree(codec);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_wm8955 = {
	.probe = 	wm8955_probe,
	.remove = 	wm8955_remove,
	.suspend = 	wm8955_suspend,
	.resume =	wm8955_resume,
};

EXPORT_SYMBOL_GPL(soc_codec_dev_wm8955);

MODULE_DESCRIPTION("ASoC WM8955 driver");
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
