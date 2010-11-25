/*
 * Broadcom 53xx RoboSwitch device driver.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmrobo.c,v 1.16.2.8 2009/07/22 15:00:48 Exp $
 */


#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmparams.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <bcmrobo.h>
#include <proto/ethernet.h>


#define	ET_ERROR(args)
#define	ET_MSG(args)

#define VARG(var, len) (((len) == 1) ? *((uint8 *)(var)) : \
		        ((len) == 2) ? *((uint16 *)(var)) : \
		        *((uint32 *)(var)))

/*
 * Switch can be programmed through SPI interface, which
 * has a rreg and a wreg functions to read from and write to
 * registers.
 */

/* MII access registers */
#define PSEUDO_PHYAD	0x1E	/* MII Pseudo PHY address */
#define REG_MII_CTRL    0x00    /* 53115 MII control register */
#define REG_MII_PAGE	0x10	/* MII Page register */
#define REG_MII_ADDR	0x11	/* MII Address register */
#define REG_MII_DATA0	0x18	/* MII Data register 0 */
#define REG_MII_DATA1	0x19	/* MII Data register 1 */
#define REG_MII_DATA2	0x1a	/* MII Data register 2 */
#define REG_MII_DATA3	0x1b	/* MII Data register 3 */
#define REG_MII_BRCM_TEST	0x1f	/* Broadcom test register */
#define REG_MII_AUX_STATUS2	0x1b	/* Auxiliary status 2 register */
#define REG_MII_AUTO_PWRDOWN	0x1C	/* 53115 Auto power down register */

/* Page numbers */
#define PAGE_CTRL	0x00	/* Control page */
#define PAGE_STATUS	0x01	/* Status page */
#define PAGE_MMR	0x02	/* 5397 Management/Mirroring page */
#define PAGE_VTBL	0x05	/* ARL/VLAN Table access page */
#define PAGE_VLAN	0x34	/* VLAN page */
#define PAGE_JUMBO	0x40	/* JUMBO frame page */

/* Control page registers */
#define REG_CTRL_PORT0	0x00	/* Port 0 traffic control register */
#define REG_CTRL_PORT1	0x01	/* Port 1 traffic control register */
#define REG_CTRL_PORT2	0x02	/* Port 2 traffic control register */
#define REG_CTRL_PORT3	0x03	/* Port 3 traffic control register */
#define REG_CTRL_PORT4	0x04	/* Port 4 traffic control register */
#define REG_CTRL_PORT5	0x05	/* Port 5 traffic control register */
#define REG_CTRL_PORT6	0x06	/* Port 6 traffic control register */
#define REG_CTRL_PORT7	0x07	/* Port 7 traffic control register */
#define REG_CTRL_IMP	0x08	/* IMP port traffic control register */
#define REG_CTRL_MODE	0x0B	/* Switch Mode register */
#define REG_CTRL_MIIPO	0x0E	/* 5325: MII Port Override register */
#define REG_CTRL_PWRDOWN 0x0F   /* 5325: Power Down Mode register */
#define REG_CTRL_SRST	0x79	/* Software reset control register */

/* Status Page Registers */
#define REG_STATUS_LINK	0x00	/* Link Status Summary */

#define REG_DEVICE_ID	0x30	/* 539x Device id: */

/* JUMBO Control Register */
#define	REG_JUMBO_CTRL	0x01
#define	REG_JUMBO_SIZE	0x05

/* Status Page Registers */
#define REG_STATUS_LINK	0x00	/* Link Status Summary */

/* VLAN page registers */
#define REG_VLAN_CTRL0	0x00	/* VLAN Control 0 register */
#define REG_VLAN_CTRL1	0x01	/* VLAN Control 1 register */
#define REG_VLAN_CTRL4	0x04	/* VLAN Control 4 register */
#define REG_VLAN_CTRL5	0x05	/* VLAN Control 5 register */
#define REG_VLAN_ACCESS	0x06	/* VLAN Table Access register */
#define REG_VLAN_ACCESS_5365	0x08	/* 5365 VLAN Table Access register */
#define REG_VLAN_WRITE	0x08	/* VLAN Write register */
#define REG_VLAN_WRITE_5365	0x0A	/* 5365 VLAN Write register */
#define REG_VLAN_READ	0x0C	/* VLAN Read register */
#define REG_VLAN_PTAG0	0x10	/* VLAN Default Port Tag register - port 0 */
#define REG_VLAN_PTAG1	0x12	/* VLAN Default Port Tag register - port 1 */
#define REG_VLAN_PTAG2	0x14	/* VLAN Default Port Tag register - port 2 */
#define REG_VLAN_PTAG3	0x16	/* VLAN Default Port Tag register - port 3 */
#define REG_VLAN_PTAG4	0x18	/* VLAN Default Port Tag register - port 4 */
#define REG_VLAN_PTAG5	0x1a	/* VLAN Default Port Tag register - port 5 */
#define REG_VLAN_PTAG6	0x1c	/* VLAN Default Port Tag register - port 6 */
#define REG_VLAN_PTAG7	0x1e	/* VLAN Default Port Tag register - port 7 */
#define REG_VLAN_PTAG8	0x20	/* 539x: VLAN Default Port Tag register - IMP port */
#define REG_VLAN_PMAP	0x20	/* 5325: VLAN Priority Re-map register */

#define VLAN_NUMVLANS	16	/* # of VLANs */


/* ARL/VLAN Table Access page registers */
#define REG_VTBL_CTRL		0x00	/* ARL Read/Write Control */
#define REG_VTBL_MINDX		0x02	/* MAC Address Index */
#define REG_VTBL_VINDX		0x08	/* VID Table Index */
#define REG_VTBL_ARL_E0		0x10	/* ARL Entry 0 */
#define REG_VTBL_ARL_E1		0x18	/* ARL Entry 1 */
#define REG_VTBL_DAT_E0		0x18	/* ARL Table Data Entry 0 */
#define REG_VTBL_SCTRL		0x20	/* ARL Search Control */
#define REG_VTBL_SADDR		0x22	/* ARL Search Address */
#define REG_VTBL_SRES		0x24	/* ARL Search Result */
#define REG_VTBL_SREXT		0x2c	/* ARL Search Result */
#define REG_VTBL_VID_E0		0x30	/* VID Entry 0 */
#define REG_VTBL_VID_E1		0x32	/* VID Entry 1 */
#define REG_VTBL_PREG		0xFF	/* Page Register */
#define REG_VTBL_ACCESS		0x60	/* VLAN table access register */
#define REG_VTBL_INDX		0x61	/* VLAN table address index register */
#define REG_VTBL_ENTRY		0x63	/* VLAN table entry register */
#define REG_VTBL_ACCESS_5395	0x80	/* VLAN table access register */
#define REG_VTBL_INDX_5395	0x81	/* VLAN table address index register */
#define REG_VTBL_ENTRY_5395	0x83	/* VLAN table entry register */

/* SPI registers */
#define REG_SPI_PAGE	0xff	/* SPI Page register */

/* Access switch registers through GPIO/SPI */

/* Minimum timing constants */
#define SCK_EDGE_TIME	2	/* clock edge duration - 2us */
#define MOSI_SETUP_TIME	1	/* input setup duration - 1us */
#define SS_SETUP_TIME	1 	/* select setup duration - 1us */

/* misc. constants */
#define SPI_MAX_RETRY	100

/* Enable GPIO access to the chip */
static void
gpio_enable(robo_info_t *robo)
{
	/* Enable GPIO outputs with SCK and MOSI low, SS high */
	si_gpioout(robo->sih, robo->ss | robo->sck | robo->mosi, robo->ss, GPIO_DRV_PRIORITY);
	si_gpioouten(robo->sih, robo->ss | robo->sck | robo->mosi,
	             robo->ss | robo->sck | robo->mosi, GPIO_DRV_PRIORITY);
}

/* Disable GPIO access to the chip */
static void
gpio_disable(robo_info_t *robo)
{
	/* Disable GPIO outputs with all their current values */
	si_gpioouten(robo->sih, robo->ss | robo->sck | robo->mosi, 0, GPIO_DRV_PRIORITY);
}

/* Write a byte stream to the chip thru SPI */
static int
spi_write(robo_info_t *robo, uint8 *buf, uint len)
{
	uint i;
	uint8 mask;

	/* Byte bang from LSB to MSB */
	for (i = 0; i < len; i++) {
		/* Bit bang from MSB to LSB */
		for (mask = 0x80; mask; mask >>= 1) {
			/* Clock low */
			si_gpioout(robo->sih, robo->sck, 0, GPIO_DRV_PRIORITY);
			OSL_DELAY(SCK_EDGE_TIME);

			/* Sample on rising edge */
			if (mask & buf[i])
				si_gpioout(robo->sih, robo->mosi, robo->mosi, GPIO_DRV_PRIORITY);
			else
				si_gpioout(robo->sih, robo->mosi, 0, GPIO_DRV_PRIORITY);
			OSL_DELAY(MOSI_SETUP_TIME);

			/* Clock high */
			si_gpioout(robo->sih, robo->sck, robo->sck, GPIO_DRV_PRIORITY);
			OSL_DELAY(SCK_EDGE_TIME);
		}
	}

	return 0;
}

/* Read a byte stream from the chip thru SPI */
static int
spi_read(robo_info_t *robo, uint8 *buf, uint len)
{
	uint i, timeout;
	uint8 rack, mask, byte;

	/* Timeout after 100 tries without RACK */
	for (i = 0, rack = 0, timeout = SPI_MAX_RETRY; i < len && timeout;) {
		/* Bit bang from MSB to LSB */
		for (mask = 0x80, byte = 0; mask; mask >>= 1) {
			/* Clock low */
			si_gpioout(robo->sih, robo->sck, 0, GPIO_DRV_PRIORITY);
			OSL_DELAY(SCK_EDGE_TIME);

			/* Sample on falling edge */
			if (si_gpioin(robo->sih) & robo->miso)
				byte |= mask;

			/* Clock high */
			si_gpioout(robo->sih, robo->sck, robo->sck, GPIO_DRV_PRIORITY);
			OSL_DELAY(SCK_EDGE_TIME);
		}
		/* RACK when bit 0 is high */
		if (!rack) {
			rack = (byte & 1);
			timeout--;
			continue;
		}
		/* Byte bang from LSB to MSB */
		buf[i] = byte;
		i++;
	}

	if (timeout == 0) {
		ET_ERROR(("spi_read: timeout"));
		return -1;
	}

	return 0;
}

/* Enable/disable SPI access */
static void
spi_select(robo_info_t *robo, uint8 spi)
{
	if (spi) {
		/* Enable SPI access */
		si_gpioout(robo->sih, robo->ss, 0, GPIO_DRV_PRIORITY);
	} else {
		/* Disable SPI access */
		si_gpioout(robo->sih, robo->ss, robo->ss, GPIO_DRV_PRIORITY);
	}
	OSL_DELAY(SS_SETUP_TIME);
}


/* Select chip and page */
static void
spi_goto(robo_info_t *robo, uint8 page)
{
	uint8 reg8 = REG_SPI_PAGE;	/* page select register */
	uint8 cmd8;

	/* Issue the command only when we are on a different page */
	if (robo->page == page)
		return;

	robo->page = page;

	/* Enable SPI access */
	spi_select(robo, 1);

	/* Select new page with CID 0 */
	cmd8 = ((6 << 4) |		/* normal SPI */
	        1);			/* write */
	spi_write(robo, &cmd8, 1);
	spi_write(robo, &reg8, 1);
	spi_write(robo, &page, 1);

	/* Disable SPI access */
	spi_select(robo, 0);
}

/* Write register thru SPI */
static int
spi_wreg(robo_info_t *robo, uint8 page, uint8 addr, void *val, int len)
{
	int status = 0;
	uint8 cmd8;
	union {
		uint8 val8;
		uint16 val16;
		uint32 val32;
	} bytes;

	/* validate value length and buffer address */
	ASSERT(len == 1 || (len == 2 && !((int)val & 1)) ||
	       (len == 4 && !((int)val & 3)));

	/* Select chip and page */
	spi_goto(robo, page);

	/* Enable SPI access */
	spi_select(robo, 1);

	/* Write with CID 0 */
	cmd8 = ((6 << 4) |		/* normal SPI */
	        1);			/* write */
	spi_write(robo, &cmd8, 1);
	spi_write(robo, &addr, 1);
	switch (len) {
	case 1:
		bytes.val8 = *(uint8 *)val;
		break;
	case 2:
		bytes.val16 = htol16(*(uint16 *)val);
		break;
	case 4:
		bytes.val32 = htol32(*(uint32 *)val);
		break;
	}
	spi_write(robo, (uint8 *)val, len);

	ET_MSG(("%s: [0x%x-0x%x] := 0x%x (len %d)\n", __FUNCTION__, page, addr,
	        *(uint16 *)val, len));
	/* Disable SPI access */
	spi_select(robo, 0);
	return status;
}

/* Read register thru SPI in fast SPI mode */
static int
spi_rreg(robo_info_t *robo, uint8 page, uint8 addr, void *val, int len)
{
	int status = 0;
	uint8 cmd8;
	union {
		uint8 val8;
		uint16 val16;
		uint32 val32;
	} bytes;

	/* validate value length and buffer address */
	ASSERT(len == 1 || (len == 2 && !((int)val & 1)) ||
	       (len == 4 && !((int)val & 3)));

	/* Select chip and page */
	spi_goto(robo, page);

	/* Enable SPI access */
	spi_select(robo, 1);

	/* Fast SPI read with CID 0 and byte offset 0 */
	cmd8 = (1 << 4);		/* fast SPI */
	spi_write(robo, &cmd8, 1);
	spi_write(robo, &addr, 1);
	status = spi_read(robo, (uint8 *)&bytes, len);
	switch (len) {
	case 1:
		*(uint8 *)val = bytes.val8;
		break;
	case 2:
		*(uint16 *)val = ltoh16(bytes.val16);
		break;
	case 4:
		*(uint32 *)val = ltoh32(bytes.val32);
		break;
	}

	ET_MSG(("%s: [0x%x-0x%x] => 0x%x (len %d)\n", __FUNCTION__, page, addr,
	        *(uint16 *)val, len));

	/* Disable SPI access */
	spi_select(robo, 0);
	return status;
}

/* SPI/gpio interface functions */
static dev_ops_t spigpio = {
	gpio_enable,
	gpio_disable,
	spi_wreg,
	spi_rreg,
	"SPI (GPIO)"
};


/* Access switch registers through MII (MDC/MDIO) */

#define MII_MAX_RETRY	100

/* Write register thru MDC/MDIO */
static int
mii_wreg(robo_info_t *robo, uint8 page, uint8 reg, void *val, int len)
{
	uint16 cmd16, val16;
	void *h = robo->h;
	int i;
	uint8 *ptr = (uint8 *)val;

	/* validate value length and buffer address */
	ASSERT(len == 1 || len == 6 || len == 8 ||
	       ((len == 2) && !((int)val & 1)) || ((len == 4) && !((int)val & 3)));

	ET_MSG(("%s: [0x%x-0x%x] := 0x%x (len %d)\n", __FUNCTION__, page, reg,
	       VARG(val, len), len));

	/* set page number - MII register 0x10 */
	if (robo->page != page) {
		cmd16 = ((page << 8) |		/* page number */
		         1);			/* mdc/mdio access enable */
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_PAGE, cmd16);
		robo->page = page;
	}

	switch (len) {
	case 8:
		val16 = ptr[7];
		val16 = ((val16 << 8) | ptr[6]);
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_DATA3, val16);
		/* FALLTHRU */

	case 6:
		val16 = ptr[5];
		val16 = ((val16 << 8) | ptr[4]);
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_DATA2, val16);
		val16 = ptr[3];
		val16 = ((val16 << 8) | ptr[2]);
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_DATA1, val16);
		val16 = ptr[1];
		val16 = ((val16 << 8) | ptr[0]);
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_DATA0, val16);
		break;

	case 4:
		val16 = (uint16)((*(uint32 *)val) >> 16);
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_DATA1, val16);
		val16 = (uint16)(*(uint32 *)val);
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_DATA0, val16);
		break;

	case 2:
		val16 = *(uint16 *)val;
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_DATA0, val16);
		break;

	case 1:
		val16 = *(uint8 *)val;
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_DATA0, val16);
		break;
	}

	/* set register address - MII register 0x11 */
	cmd16 = ((reg << 8) |		/* register address */
	         1);		/* opcode write */
	robo->miiwr(h, PSEUDO_PHYAD, REG_MII_ADDR, cmd16);

	/* is operation finished? */
	for (i = MII_MAX_RETRY; i > 0; i --) {
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_ADDR);
		if ((val16 & 3) == 0)
			break;
	}

	/* timed out */
	if (!i) {
		ET_ERROR(("mii_wreg: timeout"));
		return -1;
	}
	return 0;
}

/* Read register thru MDC/MDIO */
static int
mii_rreg(robo_info_t *robo, uint8 page, uint8 reg, void *val, int len)
{
	uint16 cmd16, val16;
	void *h = robo->h;
	int i;
	uint8 *ptr = (uint8 *)val;

	/* validate value length and buffer address */
	ASSERT(len == 1 || len == 6 || len == 8 ||
	       ((len == 2) && !((int)val & 1)) || ((len == 4) && !((int)val & 3)));

	/* set page number - MII register 0x10 */
	if (robo->page != page) {
		cmd16 = ((page << 8) |		/* page number */
		         1);			/* mdc/mdio access enable */
		robo->miiwr(h, PSEUDO_PHYAD, REG_MII_PAGE, cmd16);
		robo->page = page;
	}

	/* set register address - MII register 0x11 */
	cmd16 = ((reg << 8) |		/* register address */
	         2);			/* opcode read */
	robo->miiwr(h, PSEUDO_PHYAD, REG_MII_ADDR, cmd16);

	/* is operation finished? */
	for (i = MII_MAX_RETRY; i > 0; i --) {
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_ADDR);
		if ((val16 & 3) == 0)
			break;
	}
	/* timed out */
	if (!i) {
		ET_ERROR(("mii_rreg: timeout"));
		return -1;
	}

	switch (len) {
	case 8:
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_DATA3);
		ptr[7] = (val16 >> 8);
		ptr[6] = (val16 & 0xff);
		/* FALLTHRU */

	case 6:
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_DATA2);
		ptr[5] = (val16 >> 8);
		ptr[4] = (val16 & 0xff);
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_DATA1);
		ptr[3] = (val16 >> 8);
		ptr[2] = (val16 & 0xff);
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_DATA0);
		ptr[1] = (val16 >> 8);
		ptr[0] = (val16 & 0xff);
		break;

	case 4:
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_DATA1);
		*(uint32 *)val = (((uint32)val16) << 16);
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_DATA0);
		*(uint32 *)val |= val16;
		break;

	case 2:
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_DATA0);
		*(uint16 *)val = val16;
		break;

	case 1:
		val16 = robo->miird(h, PSEUDO_PHYAD, REG_MII_DATA0);
		*(uint8 *)val = (uint8)(val16 & 0xff);
		break;
	}

	ET_MSG(("%s: [0x%x-0x%x] => 0x%x (len %d)\n", __FUNCTION__, page, reg,
	       VARG(val, len), len));

	return 0;
}

/* MII interface functions */
static dev_ops_t mdcmdio = {
	NULL,
	NULL,
	mii_wreg,
	mii_rreg,
	"MII (MDC/MDIO)"
};

/* High level switch configuration functions. */

/* Get access to the RoboSwitch */
robo_info_t *
bcm_robo_attach(si_t *sih, void *h, char *vars, miird_f miird, miiwr_f miiwr)
{
	robo_info_t *robo;
	uint32 reset, idx;
	char *et1port, *et1phyaddr;
	int mdcport = 0, phyaddr = 0;

	/* Allocate and init private state */
	if (!(robo = MALLOC(si_osh(sih), sizeof(robo_info_t)))) {
		ET_ERROR(("robo_attach: out of memory, malloced %d bytes",
		          MALLOCED(si_osh(sih))));
		return NULL;
	}
	bzero(robo, sizeof(robo_info_t));

	robo->h = h;
	robo->sih = sih;
	robo->vars = vars;
	robo->miird = miird;
	robo->miiwr = miiwr;
	robo->page = -1;

	/* Trigger external reset by nvram variable existance */
	if ((reset = getgpiopin(robo->vars, "robo_reset", GPIO_PIN_NOTDEFINED)) !=
	    GPIO_PIN_NOTDEFINED) {
		/*
		 * Reset sequence: RESET low(50ms)->high(20ms)
		 *
		 * We have to perform a full sequence for we don't know how long
		 * it has been from power on till now.
		 */
		ET_MSG(("%s: Using external reset in gpio pin %d\n", __FUNCTION__, reset));
		reset = 1 << reset;

		/* Keep RESET low for 50 ms */
		si_gpioout(sih, reset, 0, GPIO_DRV_PRIORITY);
		si_gpioouten(sih, reset, reset, GPIO_DRV_PRIORITY);
		bcm_mdelay(50);

		/* Keep RESET high for at least 20 ms */
		si_gpioout(sih, reset, reset, GPIO_DRV_PRIORITY);
		bcm_mdelay(20);
	} else {
		/* In case we need it */
		idx = si_coreidx(sih);

		if (si_setcore(sih, ROBO_CORE_ID, 0)) {
			/* If we have an internal robo core, reset it using si_core_reset */
			ET_MSG(("%s: Resetting internal robo core\n", __FUNCTION__));
			si_core_reset(sih, 0, 0);
			robo->corerev = si_corerev(sih);
		}
		else if (sih->chip == BCM5356_CHIP_ID) {
			/* Testing chipid is a temporary hack. We need to really
			 * figure out how to treat non-cores in ai chips.
			 */
			robo->corerev = 3;
		}
		si_setcoreidx(sih, idx);
		ET_MSG(("%s: Internal robo rev %d\n", __FUNCTION__, robo->corerev));
	}

	if (miird && miiwr) {
		uint16 tmp;
		int rc, retry_count = 0;

		/* Read the PHY ID */
		tmp = miird(h, PSEUDO_PHYAD, 2);

		/* WAR: Enable mdc/mdio access to the switch registers. Unless
		 * a write to bit 0 of pseudo phy register 16 is done we are
		 * unable to talk to the switch on a customer ref design.
		 */
		if (tmp == 0xffff) {
			miiwr(h, PSEUDO_PHYAD, 16, 1);
			tmp = miird(h, PSEUDO_PHYAD, 2);
		}

		if (tmp != 0xffff) {
			do {
				rc = mii_rreg(robo, PAGE_MMR, REG_DEVICE_ID,
				              &robo->devid, sizeof(uint16));
				if (rc != 0)
					break;
				retry_count++;
			} while ((robo->devid == 0) && (retry_count < 10));

			ET_MSG(("%s: devid read %ssuccesfully via mii: 0x%x\n",
			        __FUNCTION__, rc ? "un" : "", robo->devid));
			ET_MSG(("%s: mii access to switch works\n", __FUNCTION__));
			robo->ops = &mdcmdio;
			if ((rc != 0) || (robo->devid == 0)) {
				ET_MSG(("%s: error reading devid, assuming 5325e\n",
				        __FUNCTION__));
				robo->devid = DEVID5325;
			}
		}
		ET_MSG(("%s: devid: 0x%x\n", __FUNCTION__, robo->devid));
	}

	if ((robo->devid == DEVID5395) ||
	    (robo->devid == DEVID5397) ||
	    (robo->devid == DEVID5398)) {
		uint8 srst_ctrl;

		/* If it is a 539x switch, use the soft reset register */
		ET_MSG(("%s: Resetting 539x robo switch\n", __FUNCTION__));

		/* Reset the 539x switch core and register file */
		srst_ctrl = 0x83;
		mii_wreg(robo, PAGE_CTRL, REG_CTRL_SRST, &srst_ctrl, sizeof(uint8));
		bcm_mdelay(500); /* Gemtek: for reset issue */
		srst_ctrl = 0x00;
		mii_wreg(robo, PAGE_CTRL, REG_CTRL_SRST, &srst_ctrl, sizeof(uint8));
	}

	if (!robo->ops) {
		int mosi, miso, ss, sck;

		robo->ops = &spigpio;
		robo->devid = DEVID5325;

		/* Init GPIO mapping. Default 2, 3, 4, 5 */
		ss = getgpiopin(vars, "robo_ss", 2);
		if (ss == GPIO_PIN_NOTDEFINED) {
			ET_ERROR(("robo_attach: robo_ss gpio fail: GPIO 2 in use"));
			goto error;
		}
		robo->ss = 1 << ss;
		sck = getgpiopin(vars, "robo_sck", 3);
		if (sck == GPIO_PIN_NOTDEFINED) {
			ET_ERROR(("robo_attach: robo_sck gpio fail: GPIO 3 in use"));
			goto error;
		}
		robo->sck = 1 << sck;
		mosi = getgpiopin(vars, "robo_mosi", 4);
		if (mosi == GPIO_PIN_NOTDEFINED) {
			ET_ERROR(("robo_attach: robo_mosi gpio fail: GPIO 4 in use"));
			goto error;
		}
		robo->mosi = 1 << mosi;
		miso = getgpiopin(vars, "robo_miso", 5);
		if (miso == GPIO_PIN_NOTDEFINED) {
			ET_ERROR(("robo_attach: robo_miso gpio fail: GPIO 5 in use"));
			goto error;
		}
		robo->miso = 1 << miso;
		ET_MSG(("%s: ss %d sck %d mosi %d miso %d\n", __FUNCTION__,
		        ss, sck, mosi, miso));
	}

	/* sanity check */
	ASSERT(robo->ops);
	ASSERT(robo->ops->write_reg);
	ASSERT(robo->ops->read_reg);
	ASSERT((robo->devid == DEVID5325) ||
	       (robo->devid == DEVID5395) ||
	       (robo->devid == DEVID5397) ||
	       (robo->devid == DEVID5398) ||
	       (robo->devid == DEVID53115));

	/* nvram variable switch_mode controls the power save mode on the switch
	 * set the default value in the beginning
	 */
	robo->pwrsave_mode_manual = getintvar(robo->vars, "switch_mode");
	robo->pwrsave_mode_auto = getintvar(robo->vars, "switch_mode_auto");

	/* Determining what all phys need to be included in
	 * power save operation
	 */
	et1port = getvar(vars, "et1mdcport");
	if (et1port)
		mdcport = bcm_atoi(et1port);

	et1phyaddr = getvar(vars, "et1phyaddr");
	if (et1phyaddr)
		phyaddr = bcm_atoi(et1phyaddr);

	if ((mdcport == 0) && (phyaddr == 4))
		/* For 5325F switch we need to do only phys 0-3 */
		robo->pwrsave_phys = 0xf;
	else
		/* By default all 5 phys are put into power save if there is no link */
		robo->pwrsave_phys = 0x1f;

	return robo;

error:
	bcm_robo_detach(robo);
	return NULL;
}

/* Release access to the RoboSwitch */
void
bcm_robo_detach(robo_info_t *robo)
{
	MFREE(si_osh(robo->sih), robo, sizeof(robo_info_t));
}

/* Enable the device and set it to a known good state */
int
bcm_robo_enable_device(robo_info_t *robo)
{
	uint8 reg_offset, reg_val;
	int ret = 0;

	/* Enable management interface access */
	if (robo->ops->enable_mgmtif)
		robo->ops->enable_mgmtif(robo);

	if (robo->devid == DEVID5398) {
		/* Disable unused ports: port 6 and 7 */
		for (reg_offset = REG_CTRL_PORT6; reg_offset <= REG_CTRL_PORT7; reg_offset ++) {
			/* Set bits [1:0] to disable RX and TX */
			reg_val = 0x03;
			robo->ops->write_reg(robo, PAGE_CTRL, reg_offset, &reg_val,
			                     sizeof(reg_val));
		}
	}

	if (robo->devid == DEVID5325) {
		/* Must put the switch into Reverse MII mode! */

		/* MII port state override (page 0 register 14) */
		robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_MIIPO, &reg_val, sizeof(reg_val));

		/* Bit 4 enables reverse MII mode */
		if (!(reg_val & (1 << 4))) {
			/* Enable RvMII */
			reg_val |= (1 << 4);
			robo->ops->write_reg(robo, PAGE_CTRL, REG_CTRL_MIIPO, &reg_val,
			                     sizeof(reg_val));

			/* Read back */
			robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_MIIPO, &reg_val,
			                    sizeof(reg_val));
			if (!(reg_val & (1 << 4))) {
				ET_ERROR(("robo_enable_device: enabling RvMII mode failed\n"));
				ret = -1;
			}
		}
	}

	/* Disable management interface access */
	if (robo->ops->disable_mgmtif)
		robo->ops->disable_mgmtif(robo);

	return ret;
}

/* Port flags */
#define FLAG_TAGGED	't'	/* output tagged (external ports only) */
#define FLAG_UNTAG	'u'	/* input & output untagged (CPU port only, for OS (linux, ...) */
#define FLAG_LAN	'*'	/* input & output untagged (CPU port only, for CFE */

/* port descriptor */
typedef	struct {
	uint32 untag;	/* untag enable bit (Page 0x05 Address 0x63-0x66 Bit[17:9]) */
	uint32 member;	/* vlan member bit (Page 0x05 Address 0x63-0x66 Bit[7:0]) */
	uint8 ptagr;	/* port tag register address (Page 0x34 Address 0x10-0x1F) */
	uint8 cpu;	/* is this cpu port? */
} pdesc_t;

pdesc_t pdesc97[] = {
	/* 5395/5397/5398/53115S is 0 ~ 7.  port 8 is IMP port. */
	/* port 0 */ {1 << 9, 1 << 0, REG_VLAN_PTAG0, 0},
	/* port 1 */ {1 << 10, 1 << 1, REG_VLAN_PTAG1, 0},
	/* port 2 */ {1 << 11, 1 << 2, REG_VLAN_PTAG2, 0},
	/* port 3 */ {1 << 12, 1 << 3, REG_VLAN_PTAG3, 0},
	/* port 4 */ {1 << 13, 1 << 4, REG_VLAN_PTAG4, 0},
	/* port 5 */ {1 << 14, 1 << 5, REG_VLAN_PTAG5, 0},
	/* port 6 */ {1 << 15, 1 << 6, REG_VLAN_PTAG6, 0},
	/* port 7 */ {1 << 16, 1 << 7, REG_VLAN_PTAG7, 0},
	/* mii port */ {1 << 17, 1 << 8, REG_VLAN_PTAG8, 1},
};

pdesc_t pdesc25[] = {
	/* port 0 */ {1 << 6, 1 << 0, REG_VLAN_PTAG0, 0},
	/* port 1 */ {1 << 7, 1 << 1, REG_VLAN_PTAG1, 0},
	/* port 2 */ {1 << 8, 1 << 2, REG_VLAN_PTAG2, 0},
	/* port 3 */ {1 << 9, 1 << 3, REG_VLAN_PTAG3, 0},
	/* port 4 */ {1 << 10, 1 << 4, REG_VLAN_PTAG4, 0},
	/* mii port */ {1 << 11, 1 << 5, REG_VLAN_PTAG5, 1},
};

/* Configure the VLANs */
int
bcm_robo_config_vlan(robo_info_t *robo, uint8 *mac_addr)
{
	uint8 val8;
	uint16 val16;
	uint32 val32;
	pdesc_t *pdesc;
	int pdescsz;
	uint16 vid, vid0;
	uint8 arl_entry[8] = { 0 }, arl_entry1[8] = { 0 };

	/* Enable management interface access */
	if (robo->ops->enable_mgmtif)
		robo->ops->enable_mgmtif(robo);

	/* setup global vlan configuration */
	/* VLAN Control 0 Register (Page 0x34, Address 0) */
	robo->ops->read_reg(robo, PAGE_VLAN, REG_VLAN_CTRL0, &val8, sizeof(val8));
	val8 |= ((1 << 7) |		/* enable 802.1Q VLAN */
	         (3 << 5));		/* individual VLAN learning mode */
	if (robo->devid == DEVID5325)
		val8 &= ~(1 << 1);	/* must clear reserved bit 1 */
	robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL0, &val8, sizeof(val8));

	/* VLAN Control 1 Register (Page 0x34, Address 1) */
	robo->ops->read_reg(robo, PAGE_VLAN, REG_VLAN_CTRL1, &val8, sizeof(val8));
	val8 |= ((1 << 2) |		/* enable RSV multicast V Fwdmap */
		 (1 << 3));		/* enable RSV multicast V Untagmap */
	if (robo->devid == DEVID5325)
		val8 |= (1 << 1);	/* enable RSV multicast V Tagging */
	robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL1, &val8, sizeof(val8));

	/* Jumbo Frame control refister (Page 0x40, Address 0x01) */
	/* Added by Yen */
	robo->ops->read_reg(robo, PAGE_JUMBO, REG_JUMBO_CTRL, &val8, sizeof(val8));
	val8 &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));
	if (nvram_match("jumbo_frame_enable", "1"))
		val8 |= ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));
	robo->ops->write_reg(robo, PAGE_JUMBO, REG_JUMBO_CTRL, &val8, sizeof(val8));

	arl_entry[0] = mac_addr[5];
	arl_entry[1] = mac_addr[4];
	arl_entry[2] = mac_addr[3];
	arl_entry[3] = mac_addr[2];
	arl_entry[4] = mac_addr[1];
	arl_entry[5] = mac_addr[0];

	if (robo->devid == DEVID5325) {
		/* Init the entry 1 of the bin */
		robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_ARL_E1,
		                     arl_entry1, sizeof(arl_entry1));
		robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_VID_E1,
		                     arl_entry1, 1);

		/* Init the entry 0 of the bin */
		arl_entry[6] = 0x8;		/* Port Id: MII */
		arl_entry[7] = 0xc0;	/* Static Entry, Valid */

		robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_ARL_E0,
		                     arl_entry, sizeof(arl_entry));
		robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_MINDX,
		                     arl_entry, ETHER_ADDR_LEN);

		/* VLAN Control 4 Register (Page 0x34, Address 4) */
		val8 = (1 << 6);		/* drop frame with VID violation */
		robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL4, &val8, sizeof(val8));

		/* VLAN Control 5 Register (Page 0x34, Address 5) */
		val8 = (1 << 3);		/* drop frame when miss V table */
		robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL5, &val8, sizeof(val8));

		pdesc = pdesc25;
		pdescsz = sizeof(pdesc25) / sizeof(pdesc_t);
	} else {
		/* Initialize the MAC Addr Index Register */
		robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_MINDX,
		                     arl_entry, ETHER_ADDR_LEN);

		pdesc = pdesc97;
		pdescsz = sizeof(pdesc97) / sizeof(pdesc_t);
	}

	vid0 = getintvar(robo->vars, "vlan0tag");

	/* setup each vlan. max. 16 vlans. */
	/* force vlan id to be equal to vlan number */
	for (vid = 0; vid < VLAN_NUMVLANS; vid ++) {
		char vlanports[] = "vlanXXXXports";
		char port[] = "XXXX", *ports, *next, *cur;
		uint32 untag = 0;
		uint32 member = 0;
		int pid, len;

		/* no members if VLAN id is out of limitation */
		if (vid > VLAN_MAXVID)
			goto vlan_setup;

		/* get vlan member ports from nvram */
		sprintf(vlanports, "vlan%dports", vid);
		ports = getvar(robo->vars, vlanports);

		/* In 539x vid == 0 us invalid?? */
		if ((robo->devid != DEVID5325) && (robo->devid != DEVID5397) && (vid == 0)) {
			if (ports)
				ET_ERROR(("VID 0 is set in nvram, Ignoring\n"));
			continue;
		}

		/* Gemtek: to distinguish switch 5397 and 5395 */
		if ((robo->devid == DEVID5395) && (vid == 1)) {
			sprintf(vlanports, "vlan0ports");
			ports = getvar(robo->vars, vlanports);
			if (!ports || (*ports == 0) || !strcmp(ports, " ")) {
				ET_ERROR(("BCM5395: already booted, use internal fixup\n"));
				sprintf(vlanports, "vlan1ports");
				ports = getvar(robo->vars, vlanports);
			}
			else {
				ET_ERROR(("Configure vlan1 (%s) instead of vlan0 for BCM5395\n", ports));
			}
		}

		/* disable this vlan if not defined */
		if (!ports)
			goto vlan_setup;

		/*
		 * setup each port in the vlan. cpu port needs special handing
		 * (with or without output tagging) to support linux/pmon/cfe.
		 */
		for (cur = ports; cur; cur = next) {
			/* tokenize the port list */
			while (*cur == ' ')
				cur ++;
			next = bcmstrstr(cur, " ");
			len = next ? next - cur : strlen(cur);
			if (!len)
				break;
			if (len > sizeof(port) - 1)
				len = sizeof(port) - 1;
			strncpy(port, cur, len);
			port[len] = 0;

			/* make sure port # is within the range */
			pid = bcm_atoi(port);
			if (pid >= pdescsz) {
				ET_ERROR(("robo_config_vlan: port %d in vlan%dports is out "
				          "of range[0-%d]\n", pid, vid, pdescsz));
				continue;
			}

			/* build VLAN registers values */
#ifndef	_CFE_
			if ((!pdesc[pid].cpu && !strchr(port, FLAG_TAGGED)) ||
			    (pdesc[pid].cpu && strchr(port, FLAG_UNTAG)))
#endif
				untag |= pdesc[pid].untag;

			member |= pdesc[pid].member;

			/* set port tag - applies to untagged ingress frames */
			/* Default Port Tag Register (Page 0x34, Address 0x10-0x1D) */
#ifdef	_CFE_
#define	FL	FLAG_LAN
#else
#define	FL	FLAG_UNTAG
#endif /* _CFE_ */
			if ((!pdesc[pid].cpu && !strchr(port, FLAG_TAGGED)) || 
			    strchr(port, FL)) {
				val16 = ((0 << 13) |		/* priority - always 0 */
				         vid0 | vid);	/* vlan id */
				robo->ops->write_reg(robo, PAGE_VLAN, pdesc[pid].ptagr,
				                     &val16, sizeof(val16));
			}
		}

		/* Add static ARL entries */
		if (robo->devid == DEVID5325) {
			val8 = vid;
			robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_VID_E0,
			                     &val8, sizeof(val8));
			robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_VINDX,
			                     &val8, sizeof(val8));

			/* Write the entry */
			val8 = 0x80;
			robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_CTRL,
			                     &val8, sizeof(val8));
			/* Wait for write to complete */
			SPINWAIT((robo->ops->read_reg(robo, PAGE_VTBL, REG_VTBL_CTRL,
			         &val8, sizeof(val8)), ((val8 & 0x80) != 0)),
			         100 /* usec */);
		} else {
			/* Set the VLAN Id in VLAN ID Index Register */
			val8 = vid;
			robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_VINDX,
			                     &val8, sizeof(val8));

			/* Set the MAC addr and VLAN Id in ARL Table MAC/VID Entry 0
			 * Register.
			 */
			arl_entry[6] = vid;
			arl_entry[7] = 0x0;
			robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_ARL_E0,
			                     arl_entry, sizeof(arl_entry));

			/* Set the Static bit , Valid bit and Port ID fields in
			 * ARL Table Data Entry 0 Register
			 */
			val16 = 0xc008;
			robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_DAT_E0,
			                     &val16, sizeof(val16));

			/* Clear the ARL_R/W bit and set the START/DONE bit in
			 * the ARL Read/Write Control Register.
			 */
			val8 = 0x80;
			robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_CTRL,
			                     &val8, sizeof(val8));
			/* Wait for write to complete */
			SPINWAIT((robo->ops->read_reg(robo, PAGE_VTBL, REG_VTBL_CTRL,
			         &val8, sizeof(val8)), ((val8 & 0x80) != 0)),
			         100 /* usec */);
		}

vlan_setup:
		/* setup VLAN ID and VLAN memberships */

		val32 = (untag |			/* untag enable */
		         member);			/* vlan members */
		if (robo->sih->chip == BCM5365_CHIP_ID) {
			/* VLAN Write Register (Page 0x34, Address 0x0A) */
			val32 = ((1 << 14) |	/* valid write */
				 (untag << 1) |	/* untag enable */
				 member);		/* vlan members */
			robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_WRITE_5365, &val32,
			                     sizeof(val32));
			/* VLAN Table Access Register (Page 0x34, Address 0x08) */
			val16 = ((1 << 13) | 	/* start command */
				 (1 << 12) |	/* write state */
				 vid0 | vid);	/* vlan id */
			robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_ACCESS_5365, &val16,
			                     sizeof(val16));
		} else if (robo->devid == DEVID5325) {
			val32 |= ((1 << 20) |			/* valid write */
				  (((vid0 | vid) >> 4) << 12));	/* vlan id bit[11:4] */

			/* VLAN Write Register (Page 0x34, Address 0x08-0x0B) */
			robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_WRITE, &val32,
			                     sizeof(val32));
			/* VLAN Table Access Register (Page 0x34, Address 0x06-0x07) */
			val16 = ((1 << 13) |	/* start command */
			         (1 << 12) |	/* write state */
			         vid0 | vid);	/* vlan id */
			robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_ACCESS, &val16,
			                     sizeof(val16));
		} else {
			uint8 vtble, vtbli, vtbla;

			if ((robo->devid == DEVID5395) || (robo->devid == DEVID53115)) {
				vtble = REG_VTBL_ENTRY_5395;
				vtbli = REG_VTBL_INDX_5395;
				vtbla = REG_VTBL_ACCESS_5395;
			} else {
				vtble = REG_VTBL_ENTRY;
				vtbli = REG_VTBL_INDX;
				vtbla = REG_VTBL_ACCESS;
			}

			/* VLAN Table Entry Register (Page 0x05, Address 0x63-0x66/0x83-0x86) */
			robo->ops->write_reg(robo, PAGE_VTBL, vtble, &val32,
			                     sizeof(val32));
			/* VLAN Table Address Index Reg (Page 0x05, Address 0x61-0x62/0x81-0x82) */
			val16 = vid0 | vid;     /* vlan id */
			robo->ops->write_reg(robo, PAGE_VTBL, vtbli, &val16,
			                     sizeof(val16));

			/* VLAN Table Access Register (Page 0x34, Address 0x60/0x80) */
			val8 = ((1 << 7) | 	/* start command */
			        0);	        /* write */
			robo->ops->write_reg(robo, PAGE_VTBL, vtbla, &val8,
			                     sizeof(val8));
		}
	}

	if (robo->devid == DEVID5325) {
		/* setup priority mapping - applies to tagged ingress frames */
		/* Priority Re-map Register (Page 0x34, Address 0x20-0x23) */
		val32 = ((0 << 0) |	/* 0 -> 0 */
		         (1 << 3) |	/* 1 -> 1 */
		         (2 << 6) |	/* 2 -> 2 */
		         (3 << 9) |	/* 3 -> 3 */
		         (4 << 12) |	/* 4 -> 4 */
		         (5 << 15) |	/* 5 -> 5 */
		         (6 << 18) |	/* 6 -> 6 */
		         (7 << 21));	/* 7 -> 7 */
		robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_PMAP, &val32, sizeof(val32));
	}

	/* Disable management interface access */
	if (robo->ops->disable_mgmtif)
		robo->ops->disable_mgmtif(robo);

	return 0;
}

/* Enable switching/forwarding */
int
bcm_robo_enable_switch(robo_info_t *robo)
{
	int i, max_port_ind, ret = 0;
	uint8 val8;

	/* Enable management interface access */
	if (robo->ops->enable_mgmtif)
		robo->ops->enable_mgmtif(robo);

	/* Switch Mode register (Page 0, Address 0x0B) */
	robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_MODE, &val8, sizeof(val8));

	/* Bit 1 enables switching/forwarding */
	if (!(val8 & (1 << 1))) {
		/* Set unmanaged mode */
		val8 &= (~(1 << 0));

		/* Enable forwarding */
		val8 |= (1 << 1);
		robo->ops->write_reg(robo, PAGE_CTRL, REG_CTRL_MODE, &val8, sizeof(val8));

		/* Read back */
		robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_MODE, &val8, sizeof(val8));
		if (!(val8 & (1 << 1))) {
			ET_ERROR(("robo_enable_switch: enabling forwarding failed\n"));
			ret = -1;
		}

		/* No spanning tree for unmanaged mode */
		val8 = 0;
		max_port_ind = ((robo->devid == DEVID5398) ? REG_CTRL_PORT7 : REG_CTRL_PORT4);
		for (i = REG_CTRL_PORT0; i <= max_port_ind; i++) {
			robo->ops->write_reg(robo, PAGE_CTRL, i, &val8, sizeof(val8));
		}

		/* No spanning tree on IMP port too */
		robo->ops->write_reg(robo, PAGE_CTRL, REG_CTRL_IMP, &val8, sizeof(val8));
	}

	/* Enable WAN port (#0) on the asus wl-500g deluxe boxes */
	if (robo->sih->chip == BCM5365_CHIP_ID) {
		val8 = 0;
		robo->ops->write_reg(robo, PAGE_CTRL, REG_CTRL_PORT0, &val8, sizeof(val8));
	}

	/* Disable management interface access */
	if (robo->ops->disable_mgmtif)
		robo->ops->disable_mgmtif(robo);

	return ret;
}


/*
 * Update the power save configuration for ports that changed link status.
 */
void
robo_power_save_mode_update(robo_info_t *robo, bool allports)
{
	uint phy;
	uint16 link_status, update = 0;

	/* read the link status of all ports */
	robo->ops->read_reg(robo, PAGE_STATUS, REG_STATUS_LINK,
	                    &link_status, sizeof(uint16));
	link_status &= 0x1f;

	if (!allports) {
		/* return if no change in link status */
		if (link_status == robo->prev_status)
			return;

		ET_MSG(("%s: old link status %x new link status %x\n",
		        __FUNCTION__, robo->prev_status, link_status));

		/* get the link status bits that changed */
		update = robo->prev_status ^ link_status;
	}

	robo->prev_status = link_status;

	/* when link status changes update the power save configuration
	 * for the corresponding ports. ports that moved to down state
	 * are put in to power save mode. ports that moved to up state
	 * are taken out of power save mode.
	 */
	for (phy = 0; phy < MAX_NO_PHYS; phy++) {
		if (!allports && !((update & (1 << phy)) &&
		                   (robo->pwrsave_phys & (1 << phy))))
			continue;

		if (link_status & (1 << phy)) {
			/* link is up, put the phy in normal mode */
			ET_MSG(("%s: link up, set port %d to normal mode\n",
			        __FUNCTION__, phy));
			robo_power_save_mode(robo, ROBO_PWRSAVE_NORMAL, phy);
		} else {
			/* link is down, put phys in auto, manual
			 * or auto+manual mode based on the config.
			 */
			if (robo->pwrsave_mode_auto & (1 << phy)) {
				ET_MSG(("%s: link down, set port %d to auto mode\n",
				        __FUNCTION__, phy));
				robo_power_save_mode(robo, ROBO_PWRSAVE_AUTO, phy);
			}
			if (robo->pwrsave_mode_manual & (1 << phy)) {
				ET_MSG(("%s: link down, set port %d to man mode\n",
				        __FUNCTION__, phy));
				robo_power_save_mode(robo, ROBO_PWRSAVE_MANUAL, phy);
			}
		}
	}

	return;
}

static int32
robo_power_save_mode_clear_auto(robo_info_t *robo, int32 phy)
{
	uint16 val16;

	if (robo->devid == DEVID53115) {
		/* For 53115 0x1C is the MII address of the auto power
		 * down register. Bit 5 is enabling the mode
		 * bits has the following purpose
		 * 15 - write enable 10-14 shadow register select 01010 for 
		 * auto power 6-9 reserved 5 auto power mode enable
		 * 4 sleep timer select : 1 means 5.4 sec
		 * 0-3 wake up timer select: 0xF 1.26 sec
		 */ 
		val16 = 0xa800;
		robo->miiwr(robo->h, phy, REG_MII_AUTO_PWRDOWN, val16);
	} else if (robo->sih->chip == BCM5356_CHIP_ID) {
		/* To disable auto power down mode
		 * clear bit 5 of Aux Status 2 register
		 * (Shadow reg 0x1b). Shadow register
		 * access is enabled by writing
		 * 1 to bit 7 of MII register 0x1f.
		 */
		val16 = robo->miird(robo->h, phy, REG_MII_BRCM_TEST);
		robo->miiwr(robo->h, phy, REG_MII_BRCM_TEST,
		            (val16 | (1 << 7)));

		/* Disable auto power down by clearing
		 * bit 5 of to Aux Status 2 reg.
		 */
		val16 = robo->miird(robo->h, phy, REG_MII_AUX_STATUS2);
		robo->miiwr(robo->h, phy, REG_MII_AUX_STATUS2,
		            (val16 & ~(1 << 5)));

		/* Undo shadow access */
		val16 = robo->miird(robo->h, phy, REG_MII_BRCM_TEST);
		robo->miiwr(robo->h, phy, REG_MII_BRCM_TEST,
		            (val16 & ~(1 << 7)));
	} else
		return -1;

	robo->pwrsave_mode_phys[phy] &= ~ROBO_PWRSAVE_AUTO;

	return 0;
}

static int32
robo_power_save_mode_clear_manual(robo_info_t *robo, int32 phy)
{
	uint8 val8;
	uint16 val16;

	if ((robo->devid == DEVID53115) ||
	    (robo->sih->chip == BCM5356_CHIP_ID)) {
		/* For 53115 0x0 is the MII control register
		 * Bit 11 is the power down mode bit 
		 */
		val16 = robo->miird(robo->h, phy, REG_MII_CTRL);
		val16 &= 0xf7ff;
		robo->miiwr(robo->h, phy, REG_MII_CTRL, val16);
	} else if (robo->devid == DEVID5325) {
		if (phy == 0)
			return -1;
		/* For 5325 page 0x00 address 0x0F is the power down
		 * mode register. Bits 1-4 determines which of the
		 * phys are enabled for this mode
		 */ 
		robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_PWRDOWN,
		                    &val8, sizeof(val8));
		val8 &= ~(0x1  << phy);
		robo->ops->write_reg(robo, PAGE_CTRL, REG_CTRL_PWRDOWN,
		                     &val8, sizeof(val8));
	} else
		return -1;

	robo->pwrsave_mode_phys[phy] &= ~ROBO_PWRSAVE_MANUAL;

	return 0;
}

/*
 * Function which periodically checks the power save mode on the switch
 */
int32
robo_power_save_toggle(robo_info_t *robo, int32 normal)
{
	int32 phy;

	/* Put the phys into the normal mode first so that link status
	 * can be checked. Once in the normal mode check the link status
	 * and if any of the link is up do not put that phy into
	 * manual power save mode
	 */ 
	for (phy = 0; phy < MAX_NO_PHYS; phy++) {
		/* When auto+manual modes are enabled we toggle between
		 * manual and auto modes. When only manual mode is enabled
		 * we toggle between manual and normal modes. When only
		 * auto mode is enabled there is no need to do anything
		 * here since auto mode is one time config.
		 */
		if ((robo->pwrsave_phys & (1 << phy)) &&
		    (robo->pwrsave_mode_manual & (1 << phy))) {
			if (!normal) {
				if (robo->pwrsave_mode_auto & (1 << phy))
					/* auto+manual -> auto */
					robo_power_save_mode_clear_manual(robo, phy);
				else
					/* manual -> normal */
					robo_power_save_mode(robo, ROBO_PWRSAVE_NORMAL, phy);
			} else
				/* normal -> manual / auto+manual */
				robo_power_save_mode(robo, ROBO_PWRSAVE_MANUAL, phy);
		}
	}

	return 0;
}

/*
 * Switch the ports to normal mode.
 */
static int32
robo_power_save_mode_normal(robo_info_t *robo, int32 phy)
{
	int32 error = 0;

	/* If the phy in the power save mode come out of it */ 
	switch (robo->pwrsave_mode_phys[phy]) {
		case ROBO_PWRSAVE_AUTO_MANUAL:
		case ROBO_PWRSAVE_AUTO:
			error = robo_power_save_mode_clear_auto(robo, phy);
			if ((error == -1) ||
			    (robo->pwrsave_mode_phys[phy] == ROBO_PWRSAVE_AUTO))
				break;

		case ROBO_PWRSAVE_MANUAL:
			error = robo_power_save_mode_clear_manual(robo, phy);
			break;

		default:
			break;
	}

	return error;
}

/*
 * Switch all the inactive ports to auto power down mode.
 */
static int32
robo_power_save_mode_auto(robo_info_t *robo, int32 phy)
{
	uint16 val16;

	/* For both 5325 and 53115 the link status register
	 * is the same
	 */
	robo->ops->read_reg(robo, PAGE_STATUS, REG_STATUS_LINK,
	                    &val16, sizeof(val16));
	if (val16 & (0x1 << phy))
		return 0;

	/* If the switch supports auto power down enable that */  
	if (robo->devid ==  DEVID53115) {
		/* For 53115 0x1C is the MII address of the auto power
		 * down register. Bit 5 is enabling the mode 	
		 * bits has the following purpose
		 * 15 - write enable 10-14 shadow register select 01010 for 
		 * auto power 6-9 reserved 5 auto power mode enable
		 * 4 sleep timer select : 1 means 5.4 sec
		 * 0-3 wake up timer select: 0xF 1.26 sec
		 */ 
		robo->miiwr(robo->h, phy, REG_MII_AUTO_PWRDOWN, 0xA83F);
	} else if (robo->sih->chip == BCM5356_CHIP_ID) {
		/* To enable auto power down mode set bit 5 of
		 * Auxillary Status 2 register (Shadow reg 0x1b)
		 * Shadow register access is enabled by writing
		 * 1 to bit 7 of MII register 0x1f.
		 */
		val16 = robo->miird(robo->h, phy, REG_MII_BRCM_TEST);
		robo->miiwr(robo->h, phy, REG_MII_BRCM_TEST,
		            (val16 | (1 << 7)));

		/* Enable auto power down by writing to Auxillary
		 * Status 2 reg.
		 */
		val16 = robo->miird(robo->h, phy, REG_MII_AUX_STATUS2);
		robo->miiwr(robo->h, phy, REG_MII_AUX_STATUS2,
		            (val16 | (1 << 5)));

		/* Undo shadow access */
		val16 = robo->miird(robo->h, phy, REG_MII_BRCM_TEST);
		robo->miiwr(robo->h, phy, REG_MII_BRCM_TEST,
		            (val16 & ~(1 << 7)));
	} else
		return -1;

	robo->pwrsave_mode_phys[phy] |= ROBO_PWRSAVE_AUTO;

	return 0;
}

/*
 * Switch all the inactive ports to manual power down mode.
 */
static int32
robo_power_save_mode_manual(robo_info_t *robo, int32 phy)
{
	uint8 val8;
	uint16 val16;

	/* For both 5325 and 53115 the link status register is the same */
	robo->ops->read_reg(robo, PAGE_STATUS, REG_STATUS_LINK,
	                    &val16, sizeof(val16));
	if (val16 & (0x1 << phy))
		return 0;

	/* If the switch supports manual power down enable that */  
	if (robo->devid ==  DEVID53115) {
		/* For 53115 0x0 is the MII control register bit 11 is the
		 * power down mode bit 
		 */
		val16 = robo->miird(robo->h, phy, REG_MII_CTRL);
		robo->miiwr(robo->h, phy, REG_MII_CTRL, val16 | 0x800);
	} else  if (robo->devid == DEVID5325) {
		if (phy == 0)
			return -1;
		/* For 5325 page 0x00 address 0x0F is the power down mode
		 * register. Bits 1-4 determines which of the phys are enabled
		 * for this mode 
		 */ 
		robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_PWRDOWN, &val8,
		                    sizeof(val8));
		val8 |= (1 << phy);
		robo->ops->write_reg(robo, PAGE_CTRL, REG_CTRL_PWRDOWN, &val8,
		                     sizeof(val8));
	} else
		return -1;

	robo->pwrsave_mode_phys[phy] |= ROBO_PWRSAVE_MANUAL;

	return 0;
}

/*
 * Set power save modes on the robo switch
 */
int32
robo_power_save_mode(robo_info_t *robo, int32 mode, int32 phy)
{
	int32 error = -1;

	if (phy > MAX_NO_PHYS) {
		ET_ERROR(("Passed parameter phy is out of range\n"));
		return -1;
	}

	/* Enable management interface access */
	if (robo->ops->enable_mgmtif)
		robo->ops->enable_mgmtif(robo);

	switch (mode) {
		case ROBO_PWRSAVE_NORMAL:
			/* If the phy in the power save mode come out of it */ 
			error = robo_power_save_mode_normal(robo, phy);
			break;

		case ROBO_PWRSAVE_AUTO_MANUAL:
			/* If the switch supports auto and manual power down 
			 * enable both of them 
			 */  
		case ROBO_PWRSAVE_AUTO:
			error = robo_power_save_mode_auto(robo, phy);
			if ((error == -1) || (mode == ROBO_PWRSAVE_AUTO))
				break;

		case ROBO_PWRSAVE_MANUAL:
			error = robo_power_save_mode_manual(robo, phy);
			break;

		default:
			break;
	}

	/* Disable management interface access */
	if (robo->ops->disable_mgmtif)
		robo->ops->disable_mgmtif(robo);

	return error;
}

/*
 * Get the current power save mode of the switch ports.
 */
int32
robo_power_save_mode_get(robo_info_t *robo, int32 phy)
{
	ASSERT(robo);

	if (phy >= MAX_NO_PHYS)
		return -1;

	return robo->pwrsave_mode_phys[phy];
}

/*
 * Configure the power save mode for the switch ports.
 */
int32
robo_power_save_mode_set(robo_info_t *robo, int32 mode, int32 phy)
{
	int32 error;

	ASSERT(robo);

	if (phy >= MAX_NO_PHYS)
		return -1;

	error = robo_power_save_mode(robo, mode, phy);

	if (error)
		return error;

	if (mode == ROBO_PWRSAVE_NORMAL) {
		robo->pwrsave_mode_manual &= ~(1 << phy);
		robo->pwrsave_mode_auto &= ~(1 << phy);
	} else if (mode == ROBO_PWRSAVE_AUTO) {
		robo->pwrsave_mode_auto |= (1 << phy);
		robo->pwrsave_mode_manual &= ~(1 << phy);
		robo_power_save_mode_clear_manual(robo, phy);
	} else if (mode == ROBO_PWRSAVE_MANUAL) {
		robo->pwrsave_mode_manual |= (1 << phy);
		robo->pwrsave_mode_auto &= ~(1 << phy);
		robo_power_save_mode_clear_auto(robo, phy);
	} else {
		robo->pwrsave_mode_auto |= (1 << phy);
		robo->pwrsave_mode_manual |= (1 << phy);
	}

	return 0;
}
