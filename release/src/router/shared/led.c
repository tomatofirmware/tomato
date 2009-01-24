/*

	Tomato Firmware
	Copyright (C) 2006-2008 Jonathan Zarate

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>

//!!TB
#include <sys/ioctl.h>
#include <linux_gpio.h>

#include "utils.h"
#include "shared.h"
#include "shutils.h"

const char *led_names[] = { "wlan", "diag", "white", "amber", "dmz", "aoss", "bridge", "mystery" };

// --- move begin ---

//!!TB - changed gpio_write and gpio_read to work with newer driver

/* Global containing information about each GPIO pin */

/* GPIO registers */
#define BCMGPIO_REG_IN          0
#define BCMGPIO_REG_OUT         1
#define BCMGPIO_REG_OUTEN       2
#define BCMGPIO_REG_RESERVE     3
#define BCMGPIO_REG_RELEASE     4

/* Generic functions to read/write Chip Common core's GPIO registers on the AP */

int tgpio_fd_init()
{
	return open("/dev/gpio", O_RDWR);
}

void tgpio_fd_cleanup(int fd)
{
	if (fd != -1) close (fd);
}

uint32_t tgpio_ioctl(int fd, int gpioreg, uint32_t mask, uint32_t val)
{
        struct gpio_ioctl gpio;
        int type;

        gpio.val = val;
        gpio.mask = mask;

        switch (gpioreg) {
                case BCMGPIO_REG_IN:
                        type = GPIO_IOC_IN;
                        break;
                case BCMGPIO_REG_OUT:
                        type = GPIO_IOC_OUT;
                        break;
                case BCMGPIO_REG_OUTEN:
                        type = GPIO_IOC_OUTEN;
                        break;
                case BCMGPIO_REG_RESERVE:
                        type = GPIO_IOC_RESERVE;
                        break;
                case BCMGPIO_REG_RELEASE:
                        type = GPIO_IOC_RELEASE;
                        break;
                default:
                        //printf("invalid gpioreg %d\n", gpioreg);
                        return ~0;
        }
        if (ioctl(fd, type, &gpio) < 0) {
                //printf("invalid gpioreg %d\n", gpioreg);
                return ~0;
        }
        return (gpio.val);
}

void __gpio_write(char *dev, uint32_t gpio, int val)
{
	unsigned int gpio_type = 0;

	if (strcmp(dev, "/dev/gpio/in") == 0)
		gpio_type = BCMGPIO_REG_IN;
	else if (strcmp(dev, "/dev/gpio/out") == 0)
		gpio_type = BCMGPIO_REG_OUT;
	else if (strcmp(dev, "/dev/gpio/outen") == 0)
		gpio_type = BCMGPIO_REG_OUTEN;
	//else
	//	printf("ERROR GPIO NAME %s\n", dev);

	int fd = tgpio_fd_init();

	if (fd >= 0) {
		tgpio_ioctl(fd, BCMGPIO_REG_RESERVE, gpio, gpio);

		if (val > 0)
			tgpio_ioctl(fd, gpio_type, gpio, gpio);
		else
			tgpio_ioctl(fd, gpio_type, gpio, 0);

		tgpio_fd_cleanup(fd);
	}
}

void gpio_write(uint32_t bit, int en)
{
	__gpio_write("/dev/gpio/out", bit, en);
}

uint32_t __gpio_read(char *dev, uint32_t gpio)
{
	unsigned int gpio_type = 0;
	uint32_t ret;

	if (strcmp(dev, "/dev/gpio/in") == 0)
		gpio_type = BCMGPIO_REG_IN;
	else if (strcmp(dev, "/dev/gpio/out") == 0)
		gpio_type = BCMGPIO_REG_OUT;
	else if (strcmp(dev, "/dev/gpio/outen") == 0)
		gpio_type = BCMGPIO_REG_OUTEN;
	//else
	//	printf("ERROR GPIO NAME %s\n", dev);

	int fd = tgpio_fd_init();
	if (fd >= 0) {
		/* read the value of GPIO */
		ret = tgpio_ioctl(fd, gpio_type, gpio, 0);
		tgpio_fd_cleanup(fd);
		return (ret & gpio);
	}

	return ~0;
}

uint32_t gpio_read_bit(int bit)
{
	return __gpio_read("/dev/gpio/in", 1 << bit);
}

uint32_t gpio_read_in(int fd)
{
	if (fd >= 0)
		return tgpio_ioctl(fd, BCMGPIO_REG_IN, 0, 0);
	else
		return ~0;
}

uint32_t gpio_read(void)
{
	int i;
	uint32_t v, val = 0;

	for (i = 15; i >= 0; --i) {
		v = __gpio_read("/dev/gpio/in", 1 << i);
		val |= (v) ? (1 << i) : 0;
	}
	return val;
}

void gpio_connect(int fd, int gpio_pin)
{
	uint32_t bitmask = ((unsigned int) 1 << gpio_pin);

	/* reserve the pin */
	tgpio_ioctl(fd, BCMGPIO_REG_RESERVE, bitmask, bitmask);

	tgpio_ioctl(fd, BCMGPIO_REG_OUTEN, bitmask, 0);
}

void gpio_disconnect(int fd, int gpio_pin)
{
	uint32_t bitmask = ((unsigned int) 1 << gpio_pin);

	/* release the pin */
	tgpio_ioctl(fd, BCMGPIO_REG_RELEASE, bitmask, 0);
}

#if 0 // old functions
void gpio_write(uint32_t bit, int en)
{
	int f;
	uint32_t r;

	if ((f = open("/dev/gpio/control", O_RDWR)) < 0) return;
	read(f, &r, sizeof(r));
	r &= ~bit;
	write(f, &r, sizeof(r));
	close(f);

	if ((f = open("/dev/gpio/outen", O_RDWR)) < 0) return;
	read(f, &r, sizeof(r));
	r |= bit;
	write(f, &r, sizeof(r));
	close(f);

	if ((f = open("/dev/gpio/out", O_RDWR)) < 0) return;
	read(f, &r, sizeof(r));
	if (en) r |= bit;
		else r &= ~bit;
	write(f, &r, sizeof(r));
	close(f);
}

uint32_t gpio_read(void)
{
	int f;
	uint32_t r;

	if ((f = open("/dev/gpio/in", O_RDONLY)) < 0) return ~0;
	if (read(f, &r, sizeof(r)) != sizeof(r)) r = ~0;
	close(f);
	return r;
}
#endif //0

int nvget_gpio(const char *name, int *gpio, int *inv)
{
	char *p;
	uint32_t n;

	if (((p = nvram_get(name)) != NULL) && (*p)) {
		n = strtoul(p, NULL, 0);
		if ((n & 0xFFFFFF70) == 0) {
			*gpio = (n & 15);
			*inv = ((n & 0x80) != 0);
			return 1;
		}
	}
	return 0;
}
// --- move end ---


int led(int which, int mode)
{
//								WLAN  DIAG  WHITE AMBER DMZ   AOSS  BRIDG MYST
//								----- ----- ----- ----- ----- ----- ----- -----
	static int wrt54g[]		= { 0,    1,    2,    3,    7,    255,  255,  255	};
	static int wrtsl[]		= { 255,  1,    5,    7,    0,    255,  255,  255	};
	static int whrg54[]		= { 2,    7,    255,  255,  255,  6,    1,    3		};
	static int wbr2g54[]	= { 255,  -1,   255,  255,  255,  -6,   255,  255	};
	static int wzrg54[]		= { 2,    7,    255,  255,  255,  6,    255,  255	};
	static int wr850g1[]	= { 7,    3,    255,  255,  255,  255,  255,  255	};
	static int wr850g2[]	= { 0,    1,    255,  255,  255,  255,  255,  255	};
	char s[16];
	int n;
	int b;

	if ((which < 0) || (which >= LED_COUNT)) return 0;

	switch (nvram_match("led_override", "1") ? MODEL_UNKNOWN : get_model()) {
	case MODEL_WRT54G:
		if (check_hw_type() == HW_BCM4702) {
			// G v1.x
			if ((which != LED_DIAG) && (which != LED_DMZ)) return 0;
			if (mode != LED_PROBE) {
				if (f_read_string("/proc/sys/diag", s, sizeof(s)) > 0) {
					b = (which == LED_DMZ) ? 1 : 4;
					n = atoi(s);
					sprintf(s, "%u", mode ? (n | b) : (n & ~b));
					f_write_string("/proc/sys/diag", s, 0, 0);
				}
			}
			return 1;
		}
		switch (which) {
		case LED_AMBER:
		case LED_WHITE:
			if (!supports(SUP_WHAM_LED)) return 0;
			break;
		}
		b = wrt54g[which];
		break;
	case MODEL_WRTSL54GS:
		b = wrtsl[which];
		break;
	case MODEL_WHRG54S:
	case MODEL_WHRHPG54:
	case MODEL_WHRG125:
		b = whrg54[which];
		break;
	case MODEL_WZRG54:
	case MODEL_WZRHPG54:
	case MODEL_WZRRSG54:
	case MODEL_WZRRSG54HP:
	case MODEL_WVRG54NF:
	case MODEL_WHR2A54G54:
	case MODEL_WHR3AG54:
		b = wzrg54[which];
		break;
/*
	case MODEL_WHR2A54G54:
		if (which != LED_DIAG) return 0;
		b = 7;
		break;
*/
	case MODEL_WBRG54:
		if (which != LED_DIAG) return 0;
		b = 7;
		break;
	case MODEL_WBR2G54:
		b = wbr2g54[which];
		break;
	case MODEL_WR850GV1:
		b = wr850g1[which];
		break;
	case MODEL_WR850GV2:
		b = wr850g2[which];
		break;
	case MODEL_WL500GP:
		if (which != LED_DIAG) return 0;
		b = -1;	// power light
		break;
	case MODEL_WL520GU:
		if (which != LED_DIAG) return 0;
		b = 0;	// Invert power light as diag indicator
		mode = !mode;
		break;
/*
	case MODEL_RT390W:
		break;
*/
	case MODEL_MN700:
		if (which != LED_DIAG) return 0;
		b = 6;
		break;
	default:
		sprintf(s, "led_%s", led_names[which]);
		if (nvget_gpio(s, &b, &n)) {
			if ((mode != LED_PROBE) && (n)) mode = !mode;
			goto SET;
		}
		return 0;
	}

	if (b < 0) {
		if (b == -99) b = 1; // -0 substitute
			else b = -b;
	}
	else if (mode != LED_PROBE) {
		mode = !mode;
	}

SET:
	if (b < 16) {
		if (mode != LED_PROBE) {
			gpio_write(1 << b, mode);
		}
		return 1;
	}

	return 0;
}
