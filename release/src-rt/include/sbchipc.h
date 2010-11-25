/*
 * SiliconBackplane Chipcommon core hardware definitions.
 *
 * The chipcommon core provides chip identification, SB control,
 * jtag, 0/1/2 uarts, clock frequency control, a watchdog interrupt timer,
 * gpio interface, extbus, and support for serial and parallel flashes.
 *
 * $Id: sbchipc.h,v 13.115.2.13 2009/09/01 20:11:00 Exp $
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 */

#ifndef	_SBCHIPC_H
#define	_SBCHIPC_H

#ifndef _LANGUAGE_ASSEMBLY

/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif	/* PAD */

typedef volatile struct {
	uint32	chipid;			/* 0x0 */
	uint32	capabilities;
	uint32	corecontrol;		/* corerev >= 1 */
	uint32	bist;

	/* OTP */
	uint32	otpstatus;		/* 0x10, corerev >= 10 */
	uint32	otpcontrol;
	uint32	otpprog;
	uint32	PAD;

	/* Interrupt control */
	uint32	intstatus;		/* 0x20 */
	uint32	intmask;
	uint32	chipcontrol;		/* 0x28, rev >= 11 */
	uint32	chipstatus;		/* 0x2c, rev >= 11 */

	/* Jtag Master */
	uint32	jtagcmd;		/* 0x30, rev >= 10 */
	uint32	jtagir;
	uint32	jtagdr;
	uint32	jtagctrl;

	/* serial flash interface registers */
	uint32	flashcontrol;		/* 0x40 */
	uint32	flashaddress;
	uint32	flashdata;
	uint32	PAD[1];

	/* Silicon backplane configuration broadcast control */
	uint32	broadcastaddress;	/* 0x50 */
	uint32	broadcastdata;

	/* gpio - cleared only by power-on-reset */
	uint32	gpiopullup;		/* 0x58, corerev >= 20 */
	uint32	gpiopulldown;		/* 0x5c, corerev >= 20 */
	uint32	gpioin;			/* 0x60 */
	uint32	gpioout;
	uint32	gpioouten;
	uint32	gpiocontrol;
	uint32	gpiointpolarity;
	uint32	gpiointmask;

	/* GPIO events corerev >= 11 */
	uint32	gpioevent;
	uint32	gpioeventintmask;

	/* Watchdog timer */
	uint32	watchdog;		/* 0x80 */

	/* GPIO events corerev >= 11 */
	uint32	gpioeventintpolarity;

	/* GPIO based LED powersave registers corerev >= 16 */
	uint32  gpiotimerval;		/* 0x88 */
	uint32  gpiotimeroutmask;

	/* clock control */
	uint32	clockcontrol_n;		/* 0x90 */
	uint32	clockcontrol_sb;	/* aka m0 */
	uint32	clockcontrol_pci;	/* aka m1 */
	uint32	clockcontrol_m2;	/* mii/uart/mipsref */
	uint32	clockcontrol_m3;	/* cpu */
	uint32	clkdiv;			/* corerev >= 3 */
	uint32	PAD[2];

	/* pll delay registers (corerev >= 4) */
	uint32	pll_on_delay;		/* 0xb0 */
	uint32	fref_sel_delay;
	uint32	slow_clk_ctl;		/* 5 < corerev < 10 */
	uint32	PAD[1];

	/* Instaclock registers (corerev >= 10) */
	uint32	system_clk_ctl;		/* 0xc0 */
	uint32	clkstatestretch;
	uint32	PAD[13];

	/* In AI chips, pointer to erom */
	uint32	eromptr;

	/* ExtBus control registers (corerev >= 3) */
	uint32	pcmcia_config;		/* 0x100 */
	uint32	pcmcia_memwait;
	uint32	pcmcia_attrwait;
	uint32	pcmcia_iowait;
	uint32	ide_config;
	uint32	ide_memwait;
	uint32	ide_attrwait;
	uint32	ide_iowait;
	uint32	prog_config;
	uint32	prog_waitcount;
	uint32	flash_config;
	uint32	flash_waitcount;
	uint32	PAD[4];

	/* Enhanced Coexistance Interface (ECI) registers (corerev >= 21) */
	uint32	eci_output;		/* 0x140 */
	uint32	eci_control;
	uint32	eci_inputlo;
	uint32	eci_inputmi;
	uint32	eci_inputhi;
	uint32	eci_inputintpolaritylo;
	uint32	eci_inputintpolaritymi;
	uint32	eci_inputintpolarityhi;
	uint32	eci_intmasklo;
	uint32	eci_intmaskmi;
	uint32	eci_intmaskhi;
	uint32	eci_eventlo;
	uint32	eci_eventmi;
	uint32	eci_eventhi;
	uint32	eci_eventmasklo;
	uint32	eci_eventmaskmi;
	uint32	eci_eventmaskhi;
	uint32	PAD[3];

	/* SROM interface (corerev >= 32) */
	uint32	sromcontrol;		/* 0x190 */
	uint32	sromaddress;
	uint32	sromdata;
	uint32	PAD[17];

	/* Clock control and hardware workarounds (corerev >= 20) */
	uint32	clk_ctl_st;		/* 0x1e0 */
	uint32	hw_war;
	uint32	PAD[70];

	/* uarts */
	uint8	uart0data;		/* 0x300 */
	uint8	uart0imr;
	uint8	uart0fcr;
	uint8	uart0lcr;
	uint8	uart0mcr;
	uint8	uart0lsr;
	uint8	uart0msr;
	uint8	uart0scratch;
	uint8	PAD[248];		/* corerev >= 1 */

	uint8	uart1data;		/* 0x400 */
	uint8	uart1imr;
	uint8	uart1fcr;
	uint8	uart1lcr;
	uint8	uart1mcr;
	uint8	uart1lsr;
	uint8	uart1msr;
	uint8	uart1scratch;
	uint32	PAD[126];

	/* PMU registers (corerev >= 20) */
	uint32	pmucontrol;		/* 0x600 */
	uint32	pmucapabilities;
	uint32	pmustatus;
	uint32	res_state;
	uint32	res_pending;
	uint32	pmutimer;
	uint32	min_res_mask;
	uint32	max_res_mask;
	uint32	res_table_sel;
	uint32	res_dep_mask;
	uint32	res_updn_timer;
	uint32	res_timer;
	uint32	clkstretch;
	uint32	pmuwatchdog;
	uint32	gpiosel;		/* 0x638, rev >= 1 */
	uint32	gpioenable;		/* 0x63c, rev >= 1 */
	uint32	res_req_timer_sel;
	uint32	res_req_timer;
	uint32	res_req_mask;
	uint32	PAD;
	uint32	chipcontrol_addr;	/* 0x650 */
	uint32	chipcontrol_data;	/* 0x654 */
	uint32	regcontrol_addr;
	uint32	regcontrol_data;
	uint32	pllcontrol_addr;
	uint32	pllcontrol_data;
	uint32	PAD[102];
	uint16	sromotp[768];
} chipcregs_t;

#endif /* _LANGUAGE_ASSEMBLY */

#define	CC_CHIPID		0
#define	CC_CAPABILITIES		4
#define CC_OTPST		0x10
#define CC_CHIPST		0x2c
#define	CC_JTAGCMD		0x30
#define	CC_JTAGIR		0x34
#define	CC_JTAGDR		0x38
#define	CC_JTAGCTRL		0x3c
#define	CC_WATCHDOG		0x80
#define	CC_CLKC_N		0x90
#define	CC_CLKC_M0		0x94
#define	CC_CLKC_M1		0x98
#define	CC_CLKC_M2		0x9c
#define	CC_CLKC_M3		0xa0
#define	CC_CLKDIV		0xa4
#define	CC_SYS_CLK_CTL		0xc0
#define	CC_EROMPTR		0xfc
#define	CC_CLK_CTL_ST		SI_CLK_CTL_ST
#define	PMU_CTL			0x600
#define	PMU_CAP			0x604
#define	PMU_ST			0x608
#define PMU_RES_STATE		0x60c
#define PMU_TIMER		0x614
#define	PMU_MIN_RES_MASK	0x618
#define	PMU_MAX_RES_MASK	0x61c
#define PMU_REG_CONTROL_ADDR	0x658
#define PMU_REG_CONTROL_DATA	0x65C
#define PMU_PLL_CONTROL_ADDR 	0x660
#define PMU_PLL_CONTROL_DATA 	0x664
#define	CC_SROM_OTP		0x800		/* SROM/OTP address space */

/* chipid */
#define	CID_ID_MASK		0x0000ffff	/* Chip Id mask */
#define	CID_REV_MASK		0x000f0000	/* Chip Revision mask */
#define	CID_REV_SHIFT		16		/* Chip Revision shift */
#define	CID_PKG_MASK		0x00f00000	/* Package Option mask */
#define	CID_PKG_SHIFT		20		/* Package Option shift */
#define	CID_CC_MASK		0x0f000000	/* CoreCount (corerev >= 4) */
#define CID_CC_SHIFT		24
#define	CID_TYPE_MASK		0xf0000000	/* Chip Type */
#define CID_TYPE_SHIFT		28

/* capabilities */
#define	CC_CAP_UARTS_MASK	0x00000003	/* Number of uarts */
#define CC_CAP_MIPSEB		0x00000004	/* MIPS is in big-endian mode */
#define CC_CAP_UCLKSEL		0x00000018	/* UARTs clock select */
#define CC_CAP_UINTCLK		0x00000008	/* UARTs are driven by internal divided clock */
#define CC_CAP_UARTGPIO		0x00000020	/* UARTs own Gpio's 15:12 */
#define CC_CAP_EXTBUS_MASK	0x000000c0	/* External bus mask */
#define CC_CAP_EXTBUS_NONE	0x00000000	/* No ExtBus present */
#define CC_CAP_EXTBUS_FULL	0x00000040	/* ExtBus: PCMCIA, IDE & Prog */
#define CC_CAP_EXTBUS_PROG	0x00000080	/* ExtBus: ProgIf only */
#define	CC_CAP_FLASH_MASK	0x00000700	/* Type of flash */
#define	CC_CAP_PLL_MASK		0x00038000	/* Type of PLL */
#define CC_CAP_PWR_CTL		0x00040000	/* Power control */
#define CC_CAP_OTPSIZE		0x00380000	/* OTP Size (0 = none) */
#define CC_CAP_OTPSIZE_SHIFT	19		/* OTP Size shift */
#define CC_CAP_OTPSIZE_BASE	5		/* OTP Size base */
#define CC_CAP_JTAGP		0x00400000	/* JTAG Master Present */
#define CC_CAP_ROM		0x00800000	/* Internal boot rom active */
#define CC_CAP_BKPLN64		0x08000000	/* 64-bit backplane */
#define	CC_CAP_PMU		0x10000000	/* PMU Present, rev >= 20 */
#define	CC_CAP_ECI		0x20000000	/* ECI Present, rev >= 21 */
#define	CC_CAP_SROM		0x40000000	/* Srom Present, rev >= 32 */

/* PLL type */
#define PLL_NONE		0x00000000
#define PLL_TYPE1		0x00010000	/* 48Mhz base, 3 dividers */
#define PLL_TYPE2		0x00020000	/* 48Mhz, 4 dividers */
#define PLL_TYPE3		0x00030000	/* 25Mhz, 2 dividers */
#define PLL_TYPE4		0x00008000	/* 48Mhz, 4 dividers */
#define PLL_TYPE5		0x00018000	/* 25Mhz, 4 dividers */
#define PLL_TYPE6		0x00028000	/* 100/200 or 120/240 only */
#define PLL_TYPE7		0x00038000	/* 25Mhz, 4 dividers */

/* ILP clock */
#define	ILP_CLOCK		32000

/* ALP clock on pre-PMU chips */
#define	ALP_CLOCK		20000000

/* HT clock */
#define	HT_CLOCK		80000000

/* corecontrol */
#define CC_UARTCLKO		0x00000001	/* Drive UART with internal clock */
#define	CC_SE			0x00000002	/* sync clk out enable (corerev >= 3) */
#define CC_UARTCLKEN		0x00000008	/* enable UART Clock (corerev > = 21 */

/* chipcontrol */
#define CHIPCTRL_4321A0_DEFAULT	0x3a4
#define CHIPCTRL_4321A1_DEFAULT	0x0a4
#define CHIPCTRL_4321_PLL_DOWN	0x800000	/* serdes PLL down override */

/* Fields in the otpstatus register in rev >= 21 */
#define OTPS_OL_MASK		0x000000ff
#define OTPS_OL_MFG		0x00000001	/* manuf row is locked */
#define OTPS_OL_OR1		0x00000002	/* otp redundancy row 1 is locked */
#define OTPS_OL_OR2		0x00000004	/* otp redundancy row 2 is locked */
#define OTPS_OL_GU		0x00000008	/* general use region is locked */
#define OTPS_GUP_MASK		0x00000f00
#define OTPS_GUP_SHIFT		8
#define OTPS_GUP_HW		0x00000100	/* h/w subregion is programmed */
#define OTPS_GUP_SW		0x00000200	/* s/w subregion is programmed */
#define OTPS_GUP_CI		0x00000400	/* chipid/pkgopt subregion is programmed */
#define OTPS_GUP_FUSE		0x00000800	/* fuse subregion is programmed */
#define OTPS_READY		0x00001000
#define OTPS_RV(x)		(1 << (16 + (x)))	/* redundancy entry valid */
#define OTPS_RV_MASK		0x0fff0000

/* Fields in the otpcontrol register in rev >= 21 */
#define OTPC_PROGSEL		0x00000001
#define OTPC_PCOUNT_MASK	0x0000000e
#define OTPC_PCOUNT_SHIFT	1
#define OTPC_VSEL_MASK		0x000000f0
#define OTPC_VSEL_SHIFT		4
#define OTPC_TMM_MASK		0x00000700
#define OTPC_TMM_SHIFT		8
#define OTPC_ODM		0x00000800
#define OTPC_PROGEN		0x80000000

/* Fields in otpprog in rev >= 21 and HND OTP */
#define OTPP_COL_MASK		0x000000ff
#define OTPP_COL_SHIFT		0
#define OTPP_ROW_MASK		0x0000ff00
#define OTPP_ROW_SHIFT		8
#define OTPP_OC_MASK		0x0f000000
#define OTPP_OC_SHIFT		24
#define OTPP_READERR		0x10000000
#define OTPP_VALUE_MASK		0x20000000
#define OTPP_VALUE_SHIFT	29
#define OTPP_START_BUSY		0x80000000
#define	OTPP_READ		0x40000000	/* HND OTP */

/* Opcodes for OTPP_OC field */
#define OTPPOC_READ		0
#define OTPPOC_BIT_PROG		1
#define OTPPOC_VERIFY		3
#define OTPPOC_INIT		4
#define OTPPOC_SET		5
#define OTPPOC_RESET		6
#define OTPPOC_OCST		7
#define OTPPOC_ROW_LOCK		8
#define OTPPOC_PRESCN_TEST	9

/* jtagcmd */
#define JCMD_START		0x80000000
#define JCMD_BUSY		0x80000000
#define JCMD_STATE_MASK		0x60000000
#define JCMD_STATE_TLR		0x00000000	/* Test-logic-reset */
#define JCMD_STATE_PIR		0x20000000	/* Pause IR */
#define JCMD_STATE_PDR		0x40000000	/* Pause DR */
#define JCMD_STATE_RTI		0x60000000	/* Run-test-idle */
#define JCMD0_ACC_MASK		0x0000f000
#define JCMD0_ACC_IRDR		0x00000000
#define JCMD0_ACC_DR		0x00001000
#define JCMD0_ACC_IR		0x00002000
#define JCMD0_ACC_RESET		0x00003000
#define JCMD0_ACC_IRPDR		0x00004000
#define JCMD0_ACC_PDR		0x00005000
#define JCMD0_IRW_MASK		0x00000f00
#define JCMD_ACC_MASK		0x000f0000	/* Changes for corerev 11 */
#define JCMD_ACC_IRDR		0x00000000
#define JCMD_ACC_DR		0x00010000
#define JCMD_ACC_IR		0x00020000
#define JCMD_ACC_RESET		0x00030000
#define JCMD_ACC_IRPDR		0x00040000
#define JCMD_ACC_PDR		0x00050000
#define JCMD_ACC_PIR		0x00060000
#define JCMD_ACC_IRDR_I		0x00070000	/* rev 28: return to run-test-idle */
#define JCMD_ACC_DR_I		0x00080000	/* rev 28: return to run-test-idle */
#define JCMD_IRW_MASK		0x00001f00
#define JCMD_IRW_SHIFT		8
#define JCMD_DRW_MASK		0x0000003f

/* jtagctrl */
#define JCTRL_FORCE_CLK		4		/* Force clock */
#define JCTRL_EXT_EN		2		/* Enable external targets */
#define JCTRL_EN		1		/* Enable Jtag master */

/* Fields in clkdiv */
#define	CLKD_SFLASH		0x0f000000
#define	CLKD_SFLASH_SHIFT	24
#define	CLKD_OTP		0x000f0000
#define	CLKD_OTP_SHIFT		16
#define	CLKD_JTAG		0x00000f00
#define	CLKD_JTAG_SHIFT		8
#define	CLKD_UART		0x000000ff

/* intstatus/intmask */
#define	CI_GPIO			0x00000001	/* gpio intr */
#define	CI_EI			0x00000002	/* extif intr (corerev >= 3) */
#define	CI_TEMP			0x00000004	/* temp. ctrl intr (corerev >= 15) */
#define	CI_SIRQ			0x00000008	/* serial IRQ intr (corerev >= 15) */
#define	CI_ECI			0x00000010	/* eci intr (corerev >= 21) */
#define	CI_PMU			0x00000020	/* pmu intr (corerev >= 21) */
#define	CI_UART			0x00000040	/* uart intr (corerev >= 21) */
#define	CI_WDRESET		0x80000000	/* watchdog reset occurred */

/* slow_clk_ctl */
#define SCC_SS_MASK		0x00000007	/* slow clock source mask */
#define	SCC_SS_LPO		0x00000000	/* source of slow clock is LPO */
#define	SCC_SS_XTAL		0x00000001	/* source of slow clock is crystal */
#define	SCC_SS_PCI		0x00000002	/* source of slow clock is PCI */
#define SCC_LF			0x00000200	/* LPOFreqSel, 1: 160Khz, 0: 32KHz */
#define SCC_LP			0x00000400	/* LPOPowerDown, 1: LPO is disabled,
						 * 0: LPO is enabled
						 */
#define SCC_FS			0x00000800	/* ForceSlowClk, 1: sb/cores running on slow clock,
						 * 0: power logic control
						 */
#define SCC_IP			0x00001000	/* IgnorePllOffReq, 1/0: power logic ignores/honors
						 * PLL clock disable requests from core
						 */
#define SCC_XC			0x00002000	/* XtalControlEn, 1/0: power logic does/doesn't
						 * disable crystal when appropriate
						 */
#define SCC_XP			0x00004000	/* XtalPU (RO), 1/0: crystal running/disabled */
#define SCC_CD_MASK		0xffff0000	/* ClockDivider (SlowClk = 1/(4+divisor)) */
#define SCC_CD_SHIFT		16

/* system_clk_ctl */
#define	SYCC_IE			0x00000001	/* ILPen: Enable Idle Low Power */
#define	SYCC_AE			0x00000002	/* ALPen: Enable Active Low Power */
#define	SYCC_FP			0x00000004	/* ForcePLLOn */
#define	SYCC_AR			0x00000008	/* Force ALP (or HT if ALPen is not set */
#define	SYCC_HR			0x00000010	/* Force HT */
#define SYCC_CD_MASK		0xffff0000	/* ClkDiv  (ILP = 1/(4 * (divisor + 1)) */
#define SYCC_CD_SHIFT		16

/* pcmcia/prog/flash_config */
#define	CF_EN			0x00000001	/* enable */
#define	CF_EM_MASK		0x0000000e	/* mode */
#define	CF_EM_SHIFT		1
#define	CF_EM_FLASH		0		/* flash/asynchronous mode */
#define	CF_EM_SYNC		2		/* synchronous mode */
#define	CF_EM_PCMCIA		4		/* pcmcia mode */
#define	CF_DS			0x00000010	/* destsize:  0=8bit, 1=16bit */
#define	CF_BS			0x00000020	/* byteswap */
#define	CF_CD_MASK		0x000000c0	/* clock divider */
#define	CF_CD_SHIFT		6
#define	CF_CD_DIV2		0x00000000	/* backplane/2 */
#define	CF_CD_DIV3		0x00000040	/* backplane/3 */
#define	CF_CD_DIV4		0x00000080	/* backplane/4 */
#define	CF_CE			0x00000100	/* clock enable */
#define	CF_SB			0x00000200	/* size/bytestrobe (synch only) */

/* pcmcia_memwait */
#define	PM_W0_MASK		0x0000003f	/* waitcount0 */
#define	PM_W1_MASK		0x00001f00	/* waitcount1 */
#define	PM_W1_SHIFT		8
#define	PM_W2_MASK		0x001f0000	/* waitcount2 */
#define	PM_W2_SHIFT		16
#define	PM_W3_MASK		0x1f000000	/* waitcount3 */
#define	PM_W3_SHIFT		24

/* pcmcia_attrwait */
#define	PA_W0_MASK		0x0000003f	/* waitcount0 */
#define	PA_W1_MASK		0x00001f00	/* waitcount1 */
#define	PA_W1_SHIFT		8
#define	PA_W2_MASK		0x001f0000	/* waitcount2 */
#define	PA_W2_SHIFT		16
#define	PA_W3_MASK		0x1f000000	/* waitcount3 */
#define	PA_W3_SHIFT		24

/* pcmcia_iowait */
#define	PI_W0_MASK		0x0000003f	/* waitcount0 */
#define	PI_W1_MASK		0x00001f00	/* waitcount1 */
#define	PI_W1_SHIFT		8
#define	PI_W2_MASK		0x001f0000	/* waitcount2 */
#define	PI_W2_SHIFT		16
#define	PI_W3_MASK		0x1f000000	/* waitcount3 */
#define	PI_W3_SHIFT		24

/* prog_waitcount */
#define	PW_W0_MASK		0x0000001f	/* waitcount0 */
#define	PW_W1_MASK		0x00001f00	/* waitcount1 */
#define	PW_W1_SHIFT		8
#define	PW_W2_MASK		0x001f0000	/* waitcount2 */
#define	PW_W2_SHIFT		16
#define	PW_W3_MASK		0x1f000000	/* waitcount3 */
#define	PW_W3_SHIFT		24

#define PW_W0       		0x0000000c
#define PW_W1       		0x00000a00
#define PW_W2       		0x00020000
#define PW_W3       		0x01000000

/* flash_waitcount */
#define	FW_W0_MASK		0x0000003f	/* waitcount0 */
#define	FW_W1_MASK		0x00001f00	/* waitcount1 */
#define	FW_W1_SHIFT		8
#define	FW_W2_MASK		0x001f0000	/* waitcount2 */
#define	FW_W2_SHIFT		16
#define	FW_W3_MASK		0x1f000000	/* waitcount3 */
#define	FW_W3_SHIFT		24

/* watchdog */
#define WATCHDOG_CLOCK		48000000	/* Hz */
#define WATCHDOG_CLOCK_5354 	32000		/* Hz */

/* When Srom support present, fields in sromcontrol */
#define	SRC_START		0x80000000
#define	SRC_BUSY		0x80000000
#define	SRC_OPCODE		0x60000000
#define	SRC_OP_READ		0x00000000
#define	SRC_OP_WRITE		0x20000000
#define	SRC_OP_WRDIS		0x40000000
#define	SRC_OP_WREN		0x60000000
#define	SRC_OTPSEL		0x00000010
#define	SRC_LOCK		0x00000008
#define	SRC_SIZE_MASK		0x00000006
#define	SRC_SIZE_1K		0x00000000
#define	SRC_SIZE_4K		0x00000002
#define	SRC_SIZE_16K		0x00000004
#define	SRC_SIZE_SHIFT		1
#define	SRC_PRESENT		0x00000001

/* Fields in pmucontrol */
#define	PCTL_ILP_DIV_MASK	0xffff0000
#define	PCTL_ILP_DIV_SHIFT	16
#define PCTL_PLL_PLLCTL_UPD	0x00000400	/* rev 2 */
#define PCTL_NOILP_ON_WAIT	0x00000200	/* rev 1 */
#define	PCTL_HT_REQ_EN		0x00000100
#define	PCTL_ALP_REQ_EN		0x00000080
#define	PCTL_XTALFREQ_MASK	0x0000007c
#define	PCTL_XTALFREQ_SHIFT	2
#define	PCTL_ILP_DIV_EN		0x00000002
#define	PCTL_LPO_SEL		0x00000001

/* gpiotimerval */
#define GPIO_ONTIME_SHIFT	16

/* clockcontrol_n */
#define	CN_N1_MASK		0x3f		/* n1 control */
#define	CN_N2_MASK		0x3f00		/* n2 control */
#define	CN_N2_SHIFT		8
#define	CN_PLLC_MASK		0xf0000		/* pll control */
#define	CN_PLLC_SHIFT		16

/* clockcontrol_sb/pci/uart */
#define	CC_M1_MASK		0x3f		/* m1 control */
#define	CC_M2_MASK		0x3f00		/* m2 control */
#define	CC_M2_SHIFT		8
#define	CC_M3_MASK		0x3f0000	/* m3 control */
#define	CC_M3_SHIFT		16
#define	CC_MC_MASK		0x1f000000	/* mux control */
#define	CC_MC_SHIFT		24

/* N3M Clock control magic field values */
#define	CC_F6_2			0x02		/* A factor of 2 in */
#define	CC_F6_3			0x03		/* 6-bit fields like */
#define	CC_F6_4			0x05		/* N1, M1 or M3 */
#define	CC_F6_5			0x09
#define	CC_F6_6			0x11
#define	CC_F6_7			0x21

#define	CC_F5_BIAS		5		/* 5-bit fields get this added */

#define	CC_MC_BYPASS		0x08
#define	CC_MC_M1		0x04
#define	CC_MC_M1M2		0x02
#define	CC_MC_M1M2M3		0x01
#define	CC_MC_M1M3		0x11

/* Type 2 Clock control magic field values */
#define	CC_T2_BIAS		2		/* n1, n2, m1 & m3 bias */
#define	CC_T2M2_BIAS		3		/* m2 bias */

#define	CC_T2MC_M1BYP		1
#define	CC_T2MC_M2BYP		2
#define	CC_T2MC_M3BYP		4

/* Type 6 Clock control magic field values */
#define	CC_T6_MMASK		1		/* bits of interest in m */
#define	CC_T6_M0		120000000	/* sb clock for m = 0 */
#define	CC_T6_M1		100000000	/* sb clock for m = 1 */
#define	SB2MIPS_T6(sb)		(2 * (sb))

/* Common clock base */
#define	CC_CLOCK_BASE1		24000000	/* Half the clock freq */
#define CC_CLOCK_BASE2		12500000	/* Alternate crystal on some PLL's */

/* Clock control values for 200Mhz in 5350 */
#define	CLKC_5350_N		0x0311
#define	CLKC_5350_M		0x04020009

/* Flash types in the chipcommon capabilities register */
#define FLASH_NONE		0x000		/* No flash */
#define SFLASH_ST		0x100		/* ST serial flash */
#define SFLASH_AT		0x200		/* Atmel serial flash */
#define	PFLASH			0x700		/* Parallel flash */

/* Bits in the ExtBus config registers */
#define	CC_CFG_EN		0x0001		/* Enable */
#define	CC_CFG_EM_MASK		0x000e		/* Extif Mode */
#define	CC_CFG_EM_ASYNC		0x0000		/*   Async/Parallel flash */
#define	CC_CFG_EM_SYNC		0x0002		/*   Synchronous */
#define	CC_CFG_EM_PCMCIA	0x0004		/*   PCMCIA */
#define	CC_CFG_EM_IDE		0x0006		/*   IDE */
#define	CC_CFG_DS		0x0010		/* Data size, 0=8bit, 1=16bit */
#define	CC_CFG_CD_MASK		0x00e0		/* Sync: Clock divisor, rev >= 20 */
#define	CC_CFG_CE		0x0100		/* Sync: Clock enable, rev >= 20 */
#define	CC_CFG_SB		0x0200		/* Sync: Size/Bytestrobe, rev >= 20 */
#define	CC_CFG_IS		0x0400		/* Extif Sync Clk Select, rev >= 20 */

/* ExtBus address space */
#define	CC_EB_BASE		0x1a000000	/* Chipc ExtBus base address */
#define	CC_EB_PCMCIA_MEM	0x1a000000	/* PCMCIA 0 memory base address */
#define	CC_EB_PCMCIA_IO		0x1a200000	/* PCMCIA 0 I/O base address */
#define	CC_EB_PCMCIA_CFG	0x1a400000	/* PCMCIA 0 config base address */
#define	CC_EB_IDE		0x1a800000	/* IDE memory base */
#define	CC_EB_PCMCIA1_MEM	0x1a800000	/* PCMCIA 1 memory base address */
#define	CC_EB_PCMCIA1_IO	0x1aa00000	/* PCMCIA 1 I/O base address */
#define	CC_EB_PCMCIA1_CFG	0x1ac00000	/* PCMCIA 1 config base address */
#define	CC_EB_PROGIF		0x1b000000	/* ProgIF Async/Sync base address */


/* Start/busy bit in flashcontrol */
#define SFLASH_OPCODE		0x000000ff
#define SFLASH_ACTION		0x00000700
#define	SFLASH_CS_ACTIVE	0x00001000	/* Chip Select Active, rev >= 20 */
#define SFLASH_START		0x80000000
#define SFLASH_BUSY		SFLASH_START

/* flashcontrol action codes */
#define	SFLASH_ACT_OPONLY	0x0000		/* Issue opcode only */
#define	SFLASH_ACT_OP1D		0x0100		/* opcode + 1 data byte */
#define	SFLASH_ACT_OP3A		0x0200		/* opcode + 3 address bytes */
#define	SFLASH_ACT_OP3A1D	0x0300		/* opcode + 3 addres & 1 data bytes */
#define	SFLASH_ACT_OP3A4D	0x0400		/* opcode + 3 addres & 4 data bytes */
#define	SFLASH_ACT_OP3A4X4D	0x0500		/* opcode + 3 addres, 4 don't care & 4 data bytes */
#define	SFLASH_ACT_OP3A1X4D	0x0700		/* opcode + 3 addres, 1 don't care & 4 data bytes */

/* flashcontrol action+opcodes for ST flashes */
#define SFLASH_ST_WREN		0x0006		/* Write Enable */
#define SFLASH_ST_WRDIS		0x0004		/* Write Disable */
#define SFLASH_ST_RDSR		0x0105		/* Read Status Register */
#define SFLASH_ST_WRSR		0x0101		/* Write Status Register */
#define SFLASH_ST_READ		0x0303		/* Read Data Bytes */
#define SFLASH_ST_PP		0x0302		/* Page Program */
#define SFLASH_ST_SE		0x02d8		/* Sector Erase */
#define SFLASH_ST_BE		0x00c7		/* Bulk Erase */
#define SFLASH_ST_DP		0x00b9		/* Deep Power-down */
#define SFLASH_ST_RES		0x03ab		/* Read Electronic Signature */
#define SFLASH_ST_CSA		0x1000		/* Keep chip select asserted */

/* Status register bits for ST flashes */
#define SFLASH_ST_WIP		0x01		/* Write In Progress */
#define SFLASH_ST_WEL		0x02		/* Write Enable Latch */
#define SFLASH_ST_BP_MASK	0x1c		/* Block Protect */
#define SFLASH_ST_BP_SHIFT	2
#define SFLASH_ST_SRWD		0x80		/* Status Register Write Disable */

/* flashcontrol action+opcodes for Atmel flashes */
#define SFLASH_AT_READ				0x07e8
#define SFLASH_AT_PAGE_READ			0x07d2
#define SFLASH_AT_BUF1_READ
#define SFLASH_AT_BUF2_READ
#define SFLASH_AT_STATUS			0x01d7
#define SFLASH_AT_BUF1_WRITE			0x0384
#define SFLASH_AT_BUF2_WRITE			0x0387
#define SFLASH_AT_BUF1_ERASE_PROGRAM		0x0283
#define SFLASH_AT_BUF2_ERASE_PROGRAM		0x0286
#define SFLASH_AT_BUF1_PROGRAM			0x0288
#define SFLASH_AT_BUF2_PROGRAM			0x0289
#define SFLASH_AT_PAGE_ERASE			0x0281
#define SFLASH_AT_BLOCK_ERASE			0x0250
#define SFLASH_AT_BUF1_WRITE_ERASE_PROGRAM	0x0382
#define SFLASH_AT_BUF2_WRITE_ERASE_PROGRAM	0x0385
#define SFLASH_AT_BUF1_LOAD			0x0253
#define SFLASH_AT_BUF2_LOAD			0x0255
#define SFLASH_AT_BUF1_COMPARE			0x0260
#define SFLASH_AT_BUF2_COMPARE			0x0261
#define SFLASH_AT_BUF1_REPROGRAM		0x0258
#define SFLASH_AT_BUF2_REPROGRAM		0x0259

/* Status register bits for Atmel flashes */
#define SFLASH_AT_READY				0x80
#define SFLASH_AT_MISMATCH			0x40
#define SFLASH_AT_ID_MASK			0x38
#define SFLASH_AT_ID_SHIFT			3

/* 
 * These are the UART port assignments, expressed as offsets from the base
 * register.  These assignments should hold for any serial port based on
 * a 8250, 16450, or 16550(A).
 */

#define UART_RX		0	/* In:  Receive buffer (DLAB=0) */
#define UART_TX		0	/* Out: Transmit buffer (DLAB=0) */
#define UART_DLL	0	/* Out: Divisor Latch Low (DLAB=1) */
#define UART_IER	1	/* In/Out: Interrupt Enable Register (DLAB=0) */
#define UART_DLM	1	/* Out: Divisor Latch High (DLAB=1) */
#define UART_IIR	2	/* In: Interrupt Identity Register  */
#define UART_FCR	2	/* Out: FIFO Control Register */
#define UART_LCR	3	/* Out: Line Control Register */
#define UART_MCR	4	/* Out: Modem Control Register */
#define UART_LSR	5	/* In:  Line Status Register */
#define UART_MSR	6	/* In:  Modem Status Register */
#define UART_SCR	7	/* I/O: Scratch Register */
#define UART_LCR_DLAB	0x80	/* Divisor latch access bit */
#define UART_LCR_WLEN8	0x03	/* Wordlength: 8 bits */
#define UART_MCR_OUT2	0x08	/* MCR GPIO out 2 */
#define UART_MCR_LOOP	0x10	/* Enable loopback test mode */
#define UART_LSR_THRE	0x20	/* Transmit-hold-register empty */
#define UART_LSR_RXRDY	0x01	/* Receiver ready */
#define UART_FCR_FIFO_ENABLE 1	/* FIFO control register bit controlling FIFO enable/disable */

/* Interrupt Identity Register (IIR) bits */
#define UART_IIR_FIFO_MASK	0xc0	/* IIR FIFO disable/enabled mask */
#define UART_IIR_INT_MASK	0xf	/* IIR interrupt ID source */
#define UART_IIR_MDM_CHG	0x0	/* Modem status changed */
#define UART_IIR_NOINT		0x1	/* No interrupt pending */
#define UART_IIR_THRE		0x2	/* THR empty */
#define UART_IIR_RCVD_DATA	0x4	/* Received data available */
#define UART_IIR_RCVR_STATUS 	0x6	/* Receiver status */
#define UART_IIR_CHAR_TIME 	0xc	/* Character time */

/* Interrupt Enable Register (IER) bits */
#define UART_IER_EDSSI	8	/* enable modem status interrupt */
#define UART_IER_ELSI	4	/* enable receiver line status interrupt */
#define UART_IER_ETBEI  2	/* enable transmitter holding register empty interrupt */
#define UART_IER_ERBFI	1	/* enable data available interrupt */

/* pmustatus */
#define	PST_INTPEND	0x0040
#define	PST_SBCLKST	0x0030
#define	PST_SBCLKST_ILP	0x0010
#define	PST_SBCLKST_ALP	0x0020
#define	PST_SBCLKST_HT	0x0030
#define	PST_ALPAVAIL	0x0008
#define	PST_HTAVAIL	0x0004
#define	PST_RESINIT	0x0003

/* pmucapabilities */
#define PCAP_REV_MASK	0x000000ff
#define PCAP_RC_MASK	0x00001f00
#define PCAP_RC_SHIFT	8
#define PCAP_TC_MASK	0x0001e000
#define PCAP_TC_SHIFT	13
#define PCAP_PC_MASK	0x001e0000
#define PCAP_PC_SHIFT	17
#define PCAP_VC_MASK	0x01e00000
#define PCAP_VC_SHIFT	21
#define PCAP_CC_MASK	0x1e000000
#define PCAP_CC_SHIFT	25
#define PCAP5_PC_MASK	0x003e0000	/* PMU corerev >= 5 */
#define PCAP5_PC_SHIFT	17
#define PCAP5_VC_MASK	0x07c00000
#define PCAP5_VC_SHIFT	22
#define PCAP5_CC_MASK	0xf8000000
#define PCAP5_CC_SHIFT	27

/* PMU Resource Request Timer registers */
/* This is based on PmuRev0 */
#define	PRRT_TIME_MASK	0x03ff
#define	PRRT_INTEN	0x0400
#define	PRRT_REQ_ACTIVE	0x0800
#define	PRRT_ALP_REQ	0x1000
#define	PRRT_HT_REQ	0x2000

/* PMU resource bit position */
#define PMURES_BIT(bit)	(1 << (bit))

/* PMU resource number limit */
#define PMURES_MAX_RESNUM	30

/* PMU chip control0 register */
#define	PMU_CHIPCTL0		0

/* PMU chip control1 register */
#define	PMU_CHIPCTL1		1
#define	PMU_CC1_RXC_DLL_BYPASS	0x00010000

/* PMU corerev and chip specific PLL controls.
 * PMU<rev>_PLL<num>_XXXX where <rev> is PMU corerev and <num> is an arbitary number
 * to differentiate different PLLs controlled by the same PMU rev.
 */
/* pllcontrol registers */
/* PDIV, div_phy, div_arm, div_adc, dith_sel, ioff, kpd_scale, lsb_sel, mash_sel, lf_c & lf_r */
#define	PMU0_PLL0_PLLCTL0		0
#define	PMU0_PLL0_PC0_PDIV_MASK		1
#define	PMU0_PLL0_PC0_PDIV_FREQ		25000
#define PMU0_PLL0_PC0_DIV_ARM_MASK	0x00000038
#define PMU0_PLL0_PC0_DIV_ARM_SHIFT	3
#define PMU0_PLL0_PC0_DIV_ARM_BASE	8

/* PC0_DIV_ARM for PLLOUT_ARM */
#define PMU0_PLL0_PC0_DIV_ARM_110MHZ	0
#define PMU0_PLL0_PC0_DIV_ARM_97_7MHZ	1
#define PMU0_PLL0_PC0_DIV_ARM_88MHZ	2
#define PMU0_PLL0_PC0_DIV_ARM_80MHZ	3 /* Default */
#define PMU0_PLL0_PC0_DIV_ARM_73_3MHZ	4
#define PMU0_PLL0_PC0_DIV_ARM_67_7MHZ	5
#define PMU0_PLL0_PC0_DIV_ARM_62_9MHZ	6
#define PMU0_PLL0_PC0_DIV_ARM_58_6MHZ	7

/* Wildcard base, stop_mod, en_lf_tp, en_cal & lf_r2 */
#define	PMU0_PLL0_PLLCTL1		1
#define	PMU0_PLL0_PC1_WILD_INT_MASK	0xf0000000
#define	PMU0_PLL0_PC1_WILD_INT_SHIFT	28
#define	PMU0_PLL0_PC1_WILD_FRAC_MASK	0x0fffff00
#define	PMU0_PLL0_PC1_WILD_FRAC_SHIFT	8
#define	PMU0_PLL0_PC1_STOP_MOD		0x00000040

/* Wildcard base, vco_calvar, vco_swc, vco_var_selref, vso_ical & vco_sel_avdd */
#define	PMU0_PLL0_PLLCTL2		2
#define	PMU0_PLL0_PC2_WILD_INT_MASK	0xf
#define	PMU0_PLL0_PC2_WILD_INT_SHIFT	4

/* Chip specific PMU resources. */
#define RES4328_EXT_SWITCHER_PWM	0	/* 0x00001 */
#define RES4328_BB_SWITCHER_PWM		1	/* 0x00002 */
#define RES4328_BB_SWITCHER_BURST	2	/* 0x00004 */
#define RES4328_BB_EXT_SWITCHER_BURST	3	/* 0x00008 */
#define RES4328_ILP_REQUEST		4	/* 0x00010 */
#define RES4328_RADIO_SWITCHER_PWM	5	/* 0x00020 */
#define RES4328_RADIO_SWITCHER_BURST	6	/* 0x00040 */
#define RES4328_ROM_SWITCH		7	/* 0x00080 */
#define RES4328_PA_REF_LDO		8	/* 0x00100 */
#define RES4328_RADIO_LDO		9	/* 0x00200 */
#define RES4328_AFE_LDO			10	/* 0x00400 */
#define RES4328_PLL_LDO			11	/* 0x00800 */
#define RES4328_BG_FILTBYP		12	/* 0x01000 */
#define RES4328_TX_FILTBYP		13	/* 0x02000 */
#define RES4328_RX_FILTBYP		14	/* 0x04000 */
#define RES4328_XTAL_PU			15	/* 0x08000 */
#define RES4328_XTAL_EN			16	/* 0x10000 */
#define RES4328_BB_PLL_FILTBYP		17	/* 0x20000 */
#define RES4328_RF_PLL_FILTBYP		18	/* 0x40000 */
#define RES4328_BB_PLL_PU		19	/* 0x80000 */

#define RES5354_EXT_SWITCHER_PWM	0	/* 0x00001 */
#define RES5354_BB_SWITCHER_PWM		1	/* 0x00002 */
#define RES5354_BB_SWITCHER_BURST	2	/* 0x00004 */
#define RES5354_BB_EXT_SWITCHER_BURST	3	/* 0x00008 */
#define RES5354_ILP_REQUEST		4	/* 0x00010 */
#define RES5354_RADIO_SWITCHER_PWM	5	/* 0x00020 */
#define RES5354_RADIO_SWITCHER_BURST	6	/* 0x00040 */
#define RES5354_ROM_SWITCH		7	/* 0x00080 */
#define RES5354_PA_REF_LDO		8	/* 0x00100 */
#define RES5354_RADIO_LDO		9	/* 0x00200 */
#define RES5354_AFE_LDO			10	/* 0x00400 */
#define RES5354_PLL_LDO			11	/* 0x00800 */
#define RES5354_BG_FILTBYP		12	/* 0x01000 */
#define RES5354_TX_FILTBYP		13	/* 0x02000 */
#define RES5354_RX_FILTBYP		14	/* 0x04000 */
#define RES5354_XTAL_PU			15	/* 0x08000 */
#define RES5354_XTAL_EN			16	/* 0x10000 */
#define RES5354_BB_PLL_FILTBYP		17	/* 0x20000 */
#define RES5354_RF_PLL_FILTBYP		18	/* 0x40000 */
#define RES5354_BB_PLL_PU		19	/* 0x80000 */

/* pllcontrol registers */
/* ndiv_pwrdn, pwrdn_ch<x>, refcomp_pwrdn, dly_ch<x>, p1div, p2div, _bypsss_sdmod */
#define PMU1_PLL0_PLLCTL0		0
#define PMU1_PLL0_PC0_P1DIV_MASK	0x00f00000
#define PMU1_PLL0_PC0_P1DIV_SHIFT	20
#define PMU1_PLL0_PC0_P2DIV_MASK	0x0f000000
#define PMU1_PLL0_PC0_P2DIV_SHIFT	24

/* m<x>div */
#define PMU1_PLL0_PLLCTL1		1
#define PMU1_PLL0_PC1_M1DIV_MASK	0x000000ff
#define PMU1_PLL0_PC1_M1DIV_SHIFT	0
#define PMU1_PLL0_PC1_M2DIV_MASK	0x0000ff00
#define PMU1_PLL0_PC1_M2DIV_SHIFT	8
#define PMU1_PLL0_PC1_M3DIV_MASK	0x00ff0000
#define PMU1_PLL0_PC1_M3DIV_SHIFT	16
#define PMU1_PLL0_PC1_M4DIV_MASK	0xff000000
#define PMU1_PLL0_PC1_M4DIV_SHIFT	24

#define DOT11MAC_880MHZ_CLK_DIVISOR_SHIFT 8
#define DOT11MAC_880MHZ_CLK_DIVISOR_MASK (0xFF << DOT11MAC_880MHZ_CLK_DIVISOR_SHIFT)
#define DOT11MAC_880MHZ_CLK_DIVISOR_VAL  (0xE << DOT11MAC_880MHZ_CLK_DIVISOR_SHIFT)

/* m<x>div, ndiv_dither_mfb, ndiv_mode, ndiv_int */
#define PMU1_PLL0_PLLCTL2		2
#define PMU1_PLL0_PC2_M5DIV_MASK	0x000000ff
#define PMU1_PLL0_PC2_M5DIV_SHIFT	0
#define PMU1_PLL0_PC2_M6DIV_MASK	0x0000ff00
#define PMU1_PLL0_PC2_M6DIV_SHIFT	8
#define PMU1_PLL0_PC2_NDIV_MODE_MASK	0x000e0000
#define PMU1_PLL0_PC2_NDIV_MODE_SHIFT	17
#define PMU1_PLL0_PC2_NDIV_MODE_MFB	2	/* recommended for 4319 */
#define PMU1_PLL0_PC2_NDIV_INT_MASK	0x1ff00000
#define PMU1_PLL0_PC2_NDIV_INT_SHIFT	20

/* ndiv_frac */
#define PMU1_PLL0_PLLCTL3		3
#define PMU1_PLL0_PC3_NDIV_FRAC_MASK	0x00ffffff
#define PMU1_PLL0_PC3_NDIV_FRAC_SHIFT	0

/* pll_ctrl */
#define PMU1_PLL0_PLLCTL4		4

/* pll_ctrl, vco_rng, clkdrive_ch<x> */
#define PMU1_PLL0_PLLCTL5		5
#define PMU1_PLL0_PC5_CLK_DRV_MASK 0xffffff00
#define PMU1_PLL0_PC5_CLK_DRV_SHIFT 8

/* PMU rev 2 control words */
#define PMU2_PHY_PLL_PLLCTL		4
#define PMU2_SI_PLL_PLLCTL		10

/* PMU rev 2 */
/* pllcontrol registers */
/* ndiv_pwrdn, pwrdn_ch<x>, refcomp_pwrdn, dly_ch<x>, p1div, p2div, _bypsss_sdmod */
#define PMU2_PLL_PLLCTL0		0
#define PMU2_PLL_PC0_P1DIV_MASK 	0x00f00000
#define PMU2_PLL_PC0_P1DIV_SHIFT	20
#define PMU2_PLL_PC0_P2DIV_MASK 	0x0f000000
#define PMU2_PLL_PC0_P2DIV_SHIFT	24

/* m<x>div */
#define PMU2_PLL_PLLCTL1		1
#define PMU2_PLL_PC1_M1DIV_MASK 	0x000000ff
#define PMU2_PLL_PC1_M1DIV_SHIFT	0
#define PMU2_PLL_PC1_M2DIV_MASK 	0x0000ff00
#define PMU2_PLL_PC1_M2DIV_SHIFT	8
#define PMU2_PLL_PC1_M3DIV_MASK 	0x00ff0000
#define PMU2_PLL_PC1_M3DIV_SHIFT	16
#define PMU2_PLL_PC1_M4DIV_MASK 	0xff000000
#define PMU2_PLL_PC1_M4DIV_SHIFT	24

/* m<x>div, ndiv_dither_mfb, ndiv_mode, ndiv_int */
#define PMU2_PLL_PLLCTL2		2
#define PMU2_PLL_PC2_M5DIV_MASK 	0x000000ff
#define PMU2_PLL_PC2_M5DIV_SHIFT	0
#define PMU2_PLL_PC2_M6DIV_MASK 	0x0000ff00
#define PMU2_PLL_PC2_M6DIV_SHIFT	8
#define PMU2_PLL_PC2_NDIV_MODE_MASK	0x000e0000
#define PMU2_PLL_PC2_NDIV_MODE_SHIFT	17
#define PMU2_PLL_PC2_NDIV_INT_MASK	0x1ff00000
#define PMU2_PLL_PC2_NDIV_INT_SHIFT	20

/* ndiv_frac */
#define PMU2_PLL_PLLCTL3		3
#define PMU2_PLL_PC3_NDIV_FRAC_MASK	0x00ffffff
#define PMU2_PLL_PC3_NDIV_FRAC_SHIFT	0

/* pll_ctrl */
#define PMU2_PLL_PLLCTL4		4

/* pll_ctrl, vco_rng, clkdrive_ch<x> */
#define PMU2_PLL_PLLCTL5		5
#define PMU2_PLL_PC5_CLKDRIVE_CH1_MASK	0x00000f00
#define PMU2_PLL_PC5_CLKDRIVE_CH1_SHIFT	8
#define PMU2_PLL_PC5_CLKDRIVE_CH2_MASK	0x0000f000
#define PMU2_PLL_PC5_CLKDRIVE_CH2_SHIFT	12
#define PMU2_PLL_PC5_CLKDRIVE_CH3_MASK	0x000f0000
#define PMU2_PLL_PC5_CLKDRIVE_CH3_SHIFT	16
#define PMU2_PLL_PC5_CLKDRIVE_CH4_MASK	0x00f00000
#define PMU2_PLL_PC5_CLKDRIVE_CH4_SHIFT	20
#define PMU2_PLL_PC5_CLKDRIVE_CH5_MASK	0x0f000000
#define PMU2_PLL_PC5_CLKDRIVE_CH5_SHIFT	24
#define PMU2_PLL_PC5_CLKDRIVE_CH6_MASK	0xf0000000
#define PMU2_PLL_PC5_CLKDRIVE_CH6_SHIFT	28

/* PMU rev 5 (& 6) */
#define	PMU5_PLL_P1P2_OFF		0
#define	PMU5_PLL_P1_MASK		0x0f000000
#define	PMU5_PLL_P1_SHIFT		24
#define	PMU5_PLL_P2_MASK		0x00f00000
#define	PMU5_PLL_P2_SHIFT		20
#define	PMU5_PLL_M14_OFF		1
#define	PMU5_PLL_MDIV_MASK		0x000000ff
#define	PMU5_PLL_MDIV_WIDTH		8
#define	PMU5_PLL_NM5_OFF		2
#define	PMU5_PLL_NDIV_MASK		0xfff00000
#define	PMU5_PLL_NDIV_SHIFT		20
#define	PMU5_PLL_FMAB_OFF		3
#define	PMU5_PLL_MRAT_MASK		0xf0000000
#define	PMU5_PLL_MRAT_SHIFT		28
#define	PMU5_PLL_ABRAT_MASK		0x08000000
#define	PMU5_PLL_ABRAT_SHIFT		27
#define	PMU5_PLL_FDIV_MASK		0x07ffffff
#define	PMU5_PLL_PLLCTL_OFF		4
#define	PMU5_PLL_PCHI_OFF		5
#define	PMU5_PLL_PCHI_MASK		0x0000003f

/* Divider allocation in 4716/47162/5356 */
#define	PMU5_MAINPLL_CPU		1
#define	PMU5_MAINPLL_MEM		2
#define	PMU5_MAINPLL_SI			3

/* PLL usage in 4716/47162 */
#define	PMU4716_MAINPLL_PLL0		12

/* 4716/162 resources */
#define RES4716_PROC_PLL_ON		0x00000040
#define RES4716_PROC_HT_AVAIL		0x00000080

/* PLL usage in 5356 */
#define	PMU5356_MAINPLL_PLL0		0

/* 4325 A0/A1 resources */
#define RES4325_BUCK_BOOST_BURST	0	/* 0x00000001 */
#define RES4325_CBUCK_BURST		1	/* 0x00000002 */
#define RES4325_CBUCK_PWM		2	/* 0x00000004 */
#define RES4325_CLDO_CBUCK_BURST	3	/* 0x00000008 */
#define RES4325_CLDO_CBUCK_PWM		4	/* 0x00000010 */
#define RES4325_BUCK_BOOST_PWM		5	/* 0x00000020 */
#define RES4325_ILP_REQUEST		6	/* 0x00000040 */
#define RES4325_ABUCK_BURST		7	/* 0x00000080 */
#define RES4325_ABUCK_PWM		8	/* 0x00000100 */
#define RES4325_LNLDO1_PU		9	/* 0x00000200 */
#define RES4325_LNLDO2_PU		10	/* 0x00000400 */
#define RES4325_LNLDO3_PU		11	/* 0x00000800 */
#define RES4325_LNLDO4_PU		12	/* 0x00001000 */
#define RES4325_XTAL_PU			13	/* 0x00002000 */
#define RES4325_ALP_AVAIL		14	/* 0x00004000 */
#define RES4325_RX_PWRSW_PU		15	/* 0x00008000 */
#define RES4325_TX_PWRSW_PU		16	/* 0x00010000 */
#define RES4325_RFPLL_PWRSW_PU		17	/* 0x00020000 */
#define RES4325_LOGEN_PWRSW_PU		18	/* 0x00040000 */
#define RES4325_AFE_PWRSW_PU		19	/* 0x00080000 */
#define RES4325_BBPLL_PWRSW_PU		20	/* 0x00100000 */
#define RES4325_HT_AVAIL		21	/* 0x00200000 */

/* 4325 B0/C0 resources */
#define RES4325B0_CBUCK_LPOM		1	/* 0x00000002 */
#define RES4325B0_CBUCK_BURST		2	/* 0x00000004 */
#define RES4325B0_CBUCK_PWM		3	/* 0x00000008 */
#define RES4325B0_CLDO_PU		4	/* 0x00000010 */

/* 4325 C1 resources */
#define RES4325C1_OTP_PWRSW_PU		10	/* 0x00000400 */
#define RES4325C1_LNLDO2_PU		12	/* 0x00001000 */

/* 4325 Chip specific ChipStatus register bits */
#define CST4325_SPROM_OTP_SEL_MASK	0x00000003
#define CST4325_DEFCIS_SEL		0	/* OTP is powered up, use def. CIS, no SPROM */
#define CST4325_SPROM_SEL		1	/* OTP is powered up, SPROM is present */
#define CST4325_OTP_SEL			2	/* OTP is powered up, no SPROM */
#define CST4325_OTP_PWRDN		3	/* OTP is powered down, SPROM is present */
#define CST4325_SDIO_USB_MODE_MASK	0x00000004
#define CST4325_SDIO_USB_MODE_SHIFT	2
#define CST4325_RCAL_VALID_MASK		0x00000008
#define CST4325_RCAL_VALID_SHIFT	3
#define CST4325_RCAL_VALUE_MASK		0x000001f0
#define CST4325_RCAL_VALUE_SHIFT	4
#define CST4325_PMUTOP_2B_MASK 		0x00000200	/* 1 for 2b, 0 for to 2a */
#define CST4325_PMUTOP_2B_SHIFT   	9


/* 4312 Chip specific ChipStatus register bits */
#define CST4312_SPROM_OTP_SEL_MASK	0x00000003
#define CST4312_DEFCIS_SEL		0	/* OTP is powered up, use def. CIS, no SPROM */
#define CST4312_SPROM_SEL		1	/* OTP is powered up, SPROM is present */
#define CST4312_OTP_SEL			2	/* OTP is powered up, no SPROM */
#define CST4312_OTP_BAD			3	/* OTP is broken, SPROM is present */

/* all PMU chips with little memory constraint */
#define RES4312_SWITCHER_BURST		0	/* 0x00000001 */
#define RES4312_SWITCHER_PWM    	1	/* 0x00000002 */
#define RES4312_PA_REF_LDO		2	/* 0x00000004 */
#define RES4312_CORE_LDO_BURST		3	/* 0x00000008 */
#define RES4312_CORE_LDO_PWM		4	/* 0x00000010 */
#define RES4312_RADIO_LDO		5	/* 0x00000020 */
#define RES4312_ILP_REQUEST		6	/* 0x00000040 */
#define RES4312_BG_FILTBYP		7	/* 0x00000080 */
#define RES4312_TX_FILTBYP		8	/* 0x00000100 */
#define RES4312_RX_FILTBYP		9	/* 0x00000200 */
#define RES4312_XTAL_PU			10	/* 0x00000400 */
#define RES4312_ALP_AVAIL		11	/* 0x00000800 */
#define RES4312_BB_PLL_FILTBYP		12	/* 0x00001000 */
#define RES4312_RF_PLL_FILTBYP		13	/* 0x00002000 */
#define RES4312_HT_AVAIL		14	/* 0x00004000 */

/* 4322 resources */
#define RES4322_RF_LDO			0
#define RES4322_ILP_REQUEST		1
#define RES4322_XTAL_PU			2
#define RES4322_ALP_AVAIL		3
#define RES4322_SI_PLL_ON		4
#define RES4322_HT_SI_AVAIL		5
#define RES4322_PHY_PLL_ON		6
#define RES4322_HT_PHY_AVAIL		7
#define RES4322_OTP_PU			8

/* 4322 Chip specific ChipStatus register bits */
#define CST4322_XTAL_FREQ_20_40MHZ	0x00000020
#define CST4322_SPROM_OTP_SEL_MASK	0x000000c0
#define CST4322_SPROM_OTP_SEL_SHIFT	6
#define CST4322_NO_SPROM_OTP		0	/* no OTP, no SPROM */
#define CST4322_SPROM_PRESENT		1	/* SPROM is present */
#define CST4322_OTP_PRESENT		2	/* OTP is present */
#define CST4322_PCI_OR_USB		0x00000100
#define CST4322_BOOT_MASK		0x00000600
#define CST4322_BOOT_SHIFT		9
#define CST4322_BOOT_FROM_SRAM		0	/* boot from SRAM, ARM in reset */
#define CST4322_BOOT_FROM_ROM		1	/* boot from ROM */
#define CST4322_BOOT_FROM_FLASH		2	/* boot from FLASH */
#define CST4322_BOOT_FROM_INVALID	3
#define CST4322_ILP_DIV_EN		0x00000800
#define CST4322_FLASH_TYPE_MASK		0x00001000
#define CST4322_FLASH_TYPE_SHIFT	12
#define CST4322_FLASH_TYPE_SHIFT_ST	0	/* ST serial FLASH */
#define CST4322_FLASH_TYPE_SHIFT_ATMEL	1	/* ATMEL flash */
#define CST4322_ARM_TAP_SEL		0x00002000
#define CST4322_RES_INIT_MODE_MASK	0x0000c000
#define CST4322_RES_INIT_MODE_SHIFT	14
#define CST4322_RES_INIT_MODE_ILPAVAIL	0	/* resinitmode: ILP available */
#define CST4322_RES_INIT_MODE_ILPREQ	1	/* resinitmode: ILP request */
#define CST4322_RES_INIT_MODE_ALPAVAIL	2	/* resinitmode: ALP available */
#define CST4322_RES_INIT_MODE_HTAVAIL	3	/* resinitmode: HT avilable */
#define CST4322_PCIPLLCLK_GATING	0x00010000
#define CST4322_CLK_SWITCH_PCI_TO_ALP	0x00020000
#define CST4322_PCI_CARDBUS_MODE	0x00040000

/* 43224 Chip specific ChipControl register bits */
#define CCTRL43224_GPIO_TOGGLE		0x8000 /* gpio[3:0] pins as btcoex or s/w gpio */
#define CCTRL_43224A0_12MA_LED_DRIVE	0x00F000F0 /* 12 mA drive strength */
#define CCTRL_43224B0_12MA_LED_DRIVE	0xF0	/* 12 mA drive strengh for later 43224s */



#define RES4319_CBUCK_LPOM		1	/* 0x00000002 */
#define RES4319_CBUCK_BURST		2	/* 0x00000004 */
#define RES4319_CBUCK_PWM		3	/* 0x00000008 */
#define RES4319_CLDO_PU			4	/* 0x00000010 */
#define RES4319_PALDO_PU		5	/* 0x00000020 */
#define RES4319_ILP_REQUEST		6	/* 0x00000040 */
#define RES4319_LNLDO1_PU		9	/* 0x00000200 */
#define RES4319_OTP_PU			10	/* 0x00000400 */
#define RES4319_LNLDO2_PU		12	/* 0x00001000 */
#define RES4319_XTAL_PU			13	/* 0x00002000 */
#define RES4319_ALP_AVAIL		14	/* 0x00004000 */
#define RES4319_RX_PWRSW_PU		15	/* 0x00008000 */
#define RES4319_TX_PWRSW_PU		16	/* 0x00010000 */
#define RES4319_RFPLL_PWRSW_PU		17	/* 0x00020000 */
#define RES4319_LOGEN_PWRSW_PU		18	/* 0x00040000 */
#define RES4319_AFE_PWRSW_PU		19	/* 0x00080000 */
#define RES4319_BBPLL_PWRSW_PU		20	/* 0x00100000 */
#define RES4319_HT_AVAIL		21	/* 0x00200000 */

#define	CST4319_SPI_CPULESSUSB		0x00000001
#define	CST4319_SPI_CLK_POL		0x00000002
#define	CST4319_SPI_CLK_PH		0x00000008
#define	CST4319_SPROM_OTP_SEL_MASK	0x000000c0	/* gpio [7:6], SDIO CIS selection */
#define	CST4319_SPROM_OTP_SEL_SHIFT	6
#define CST4319_DEFCIS_SEL		0x00000000	/* use default CIS, OTP is powered up */
#define	CST4319_SPROM_SEL		0x00000040	/* use SPROM, OTP is powered up */
#define	CST4319_OTP_SEL			0x00000080      /* use OTP, OTP is powered up */
#define	CST4319_OTP_PWRDN		0x000000c0      /* use SPROM, OTP is powered down */
#define	CST4319_SDIO_USB_MODE		0x00000100	/* gpio [8], sdio/usb mode */
#define	CST4319_REMAP_SEL_MASK		0x00000600
#define	CST4319_ILPDIV_EN		0x00000800
#define	CST4319_XTAL_PD_POL		0x00001000
#define	CST4319_LPO_SEL			0x00002000
#define	CST4319_RES_INIT_MODE		0x0000c000
#define	CST4319_PALDO_EXTPNP		0x00010000	/* PALDO is configured with external PNP */
#define	CST4319_CBUCK_MODE_MASK		0x00060000
#define CST4319_CBUCK_MODE_BURST	0x00020000
#define CST4319_CBUCK_MODE_LPBURST	0x00060000
#define	CST4319_RCAL_VALID		0x01000000
#define	CST4319_RCAL_VALUE_MASK		0x3e000000
#define	CST4319_RCAL_VALUE_SHIFT	25
#define PMU1_PLL0_CHIPCTL0		0
#define PMU1_PLL0_CHIPCTL1		1
#define PMU1_PLL0_CHIPCTL2		2
#define CCTL_4319USB_XTAL_SEL_MASK	0x00180000
#define CCTL_4319USB_XTAL_SEL_SHIFT	19
#define CCTL_4319USB_48MHZ_PLL_SEL	1
#define CCTL_4319USB_24MHZ_PLL_SEL	2
/*
* Maximum delay for the PMU state transition in us.
* This is an upper bound intended for spinwaits etc.
*/
#define PMU_MAX_TRANSITION_DLY	15000

/* PMU resource up transition time in ILP cycles */
#define PMURES_UP_TRANSITION	2

/*
* Infromation from BT to WLAN over eci_inputlo, eci_inputmi &
* eci_inputhi register.  Rev >=21
*/
/* Fields in eci_inputlo register - [0:31] */
#define	ECI_INLO_TASKTYPE_MASK	0x0000000f /* [3:0] - 4 bits */
#define ECI_INLO_TASKTYPE_SHIFT 0
#define	ECI_INLO_PKTDUR_MASK	0x000000f0 /* [7:4] - 4 bits */
#define ECI_INLO_PKTDUR_SHIFT	4
#define	ECI_INLO_ROLE_MASK	0x00000100 /* [8] - 1 bits */
#define ECI_INLO_ROLE_SHIFT	8
#define	ECI_INLO_MLP_MASK	0x00000e00 /* [11:9] - 3 bits */
#define ECI_INLO_MLP_SHIFT	9
#define	ECI_INLO_TXPWR_MASK	0x000ff000 /* [19:12] - 8 bits */
#define ECI_INLO_TXPWR_SHIFT	12
#define	ECI_INLO_RSSI_MASK	0x0ff00000 /* [27:20] - 8 bits */
#define ECI_INLO_RSSI_SHIFT	20
#define	ECI_INLO_VAD_MASK	0x10000000 /* [28] - 1 bits */
#define ECI_INLO_VAD_SHIFT	28

/*
* Register eci_inputlo bitfield values.
* - BT packet type information bits [7:0]
*/
/*  [3:0] - Task (link) type */
#define BT_ACL				0x00
#define BT_SCO				0x01
#define BT_eSCO				0x02
#define BT_A2DP				0x03
#define BT_SNIFF			0x04
#define BT_PAGE_SCAN			0x05
#define BT_INQUIRY_SCAN			0x06
#define BT_PAGE				0x07
#define BT_INQUIRY			0x08
#define BT_MSS				0x09
#define BT_PARK				0x0a
#define BT_RSSISCAN			0x0b
#define BT_MD_ACL			0x0c
#define BT_MD_eSCO			0x0d
#define BT_SCAN_WITH_SCO_LINK		0x0e
#define BT_SCAN_WITHOUT_SCO_LINK	0x0f
/* [7:4] = packet duration code */
/* [8] - Master / Slave */
#define BT_MASTER			0
#define BT_SLAVE			1
/* [11:9] - multi-level priority */
#define BT_LOWEST_PRIO			0x0
#define BT_HIGHEST_PRIO			0x3
/* [19:12] - BT transmit power */
/* [27:20] - BT RSSI */
/* [28] - VAD silence */
/* [31:29] - Undefined */
/* Register eci_inputmi values - [32:63] - none defined */
/* [63:32] - Undefined */

/* Infromation from WLAN to BT over eci_output register. */
/* Fields in eci_output register - [0:31] */
#define	ECI_OUT_CHANNEL_MASK		0x0000000f  /* [3:0] - 4 bits */
#define ECI_OUT_CHANNEL_SHIFT		0
#define ECI_OUT_BW_MASK			0x00000070  /* [6:4] - 3 bits */
#define ECI_OUT_BW_SHIFT		4
#define	ECI_OUT_ANTENNA_MASK		0x00000080  /* [7] - 1 bits */
#define ECI_OUT_ANTENNA_SHIFT		7
#define	ECI_OUT_CUR_TXPOWER_MASK	0x00000f00  /* [11:8] - 4 bits */
#define ECI_OUT_CUR_TXPOWER_SHIFT	8
#define	ECI_OUT_CUR_RSSI_MASK		0x0000f000  /* [15:12] - 4 bits */
#define ECI_OUT_CUR_RSSI_SHIFT		12
#define	ECI_OUT_SIMUL_TXRX_MASK		0x00010000  /* [16] - 1 bits */
#define ECI_OUT_SIMUL_TXRX_SHIFT	15

/* Indicate controll of bits between s/w and dot11mac.
 * Current assignment :
 * 7:0 - wl
 * 15:8 - dot11mac/ucode
 * 31:16 - unassigned (presumed to be with do11mac/ucode)
 */
#define ECI_WL_BITS			0xffffff00

/* WLAN channel numbers - used from wifi.h */

/* WLAN BW */
#define BW_20	0x0
#define BW_40	0x4

/* WLAN - number of antenna */
#define WLAN_NUM_ANT1 TXANT_0
#define WLAN_NUM_ANT2 TXANT_1

#endif	/* _SBCHIPC_H */
