/*

	Tomato Firmware
	Copyright (C) 2006-2008 Jonathan Zarate

*/
#include "rc.h"

#include <sys/reboot.h>
#include <wlutils.h>
#include <wlioctl.h>

#if TOMATO_N
#include <linux_gpio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#endif

//	#define DEBUG_TEST


static int gf;

//!!TB - new gpio routines

extern int tgpio_fd_init();
extern void tgpio_fd_cleanup(int fd);
extern void gpio_connect(int fd, int gpio_pin);
extern void gpio_disconnect(int fd, int gpio_pin);
extern uint32_t gpio_read_in(int fd);

static void gpio_connect_mask(uint32_t *mask)
{
	int gpio_pin = 0;
	uint32_t gpio_mask = 1;

	gf = tgpio_fd_init();

	if (gf >= 0) {
		for (; gpio_mask; gpio_mask <<= 1, gpio_pin++) {
			if (*mask & gpio_mask) gpio_connect(gf, gpio_pin);
		}
	}
}

static void gpio_disconnect_mask(uint32_t mask)
{
	int gpio_pin = 0;
	uint32_t gpio_mask = 1;

	if (gf >= 0) {
		for (; gpio_mask; gpio_mask <<= 1, gpio_pin++) {
			if (mask & gpio_mask) gpio_disconnect(gf, gpio_pin);
		}
		tgpio_fd_cleanup(gf);
	}
	gf = -1;
}

static uint32_t _gpio_read(void)
{
	return gpio_read_in(gf);
}
//---------------------------------------------------------------------------------

static int get_btn(const char *name, uint32_t *bit, uint32_t *pushed)
{
	int gpio;
	int inv;

	if (nvget_gpio(name, &gpio, &inv)) {
		*bit = 1 << gpio;
		*pushed = inv ? 0 : *bit;
		return 1;
	}
	return 0;
}

int buttons_main(int argc, char *argv[])
{
	uint32_t gpio;
	uint32_t mask;
	uint32_t last;
	uint32_t ses_mask;
	uint32_t ses_pushed;
	uint32_t reset_mask;
	uint32_t reset_pushed;
	uint32_t brau_mask;
	uint32_t brau_state;
	int count;
	char s[16];
	char *p;
	int n;
	int ses_led;

	ses_mask = ses_pushed = 0;
	reset_pushed = 0;
	brau_mask = 0;
	brau_state = ~0;
	ses_led = LED_DIAG;

	// moveme
	switch (nvram_match("btn_override", "1") ? MODEL_UNKNOWN : get_model()) {
	case MODEL_WRT54G:
	case MODEL_WRTSL54GS:
		reset_mask = 1 << 6;
		ses_mask = 1 << 4;
		ses_led = LED_DMZ;
		break;
/*
	case MODEL_WRH54G:
		reset_mask = 1 << 6;
		break;
*/
	case MODEL_WHRG54S:
	case MODEL_WHRHPG54:
	case MODEL_WHR2A54G54:
	case MODEL_WHR3AG54:
	case MODEL_WHRG125:
		reset_mask = reset_pushed = 1 << 4;
		ses_mask = 1 << 0;
		brau_mask = 1 << 5;
		break;
	case MODEL_WBRG54:
		reset_mask = reset_pushed = 1 << 4;
		break;
	case MODEL_WBR2G54:
		reset_mask = reset_pushed = 1 << 7;
		ses_mask = ses_pushed = 1 << 4;
		ses_led = LED_AOSS;
		break;
	case MODEL_WZRG54:
	case MODEL_WZRHPG54:
	case MODEL_WZRRSG54:
	case MODEL_WZRRSG54HP:
	case MODEL_WVRG54NF:
		reset_mask = reset_pushed = 1 << 4;
		ses_mask = 1 << 0;
		ses_led = LED_AOSS;
		break;
	case MODEL_WR850GV1:
		reset_mask = 1 << 0;
		break;
	case MODEL_WR850GV2:
		reset_mask = 1 << 5;
		break;
	case MODEL_WL500GP:
		reset_mask = reset_pushed = 1 << 0;
		ses_mask = ses_pushed = 1 << 4;
		break;
	case MODEL_WL520GU:
		reset_mask = 1 << 2;
		ses_mask = 1 << 3;
		break;
//	case MODEL_MN700:
//?		reset_mask = reset_pushed = 1 << 7;
//		break;
	default:
		get_btn("btn_ses", &ses_mask, &ses_pushed);
		if (!get_btn("btn_reset", &reset_mask, &reset_pushed)) {
			fprintf(stderr, "Not supported.\n");
			return 1;
		}
		break;
	}
	mask = reset_mask | ses_mask | brau_mask;

#ifdef DEBUG_TEST
	cprintf("reset_mask=0x%X reset_pushed=0x%X\n", reset_mask, reset_pushed);
	cprintf("ses_mask=0x%X\n", ses_mask);
	cprintf("brau_mask=0x%X\n", brau_mask);
	cprintf("ses_led=%d\n", ses_led);
#else
	if (fork() != 0) return 0;
	setsid();
#endif

	signal(SIGCHLD, handle_reap);
/* !!TB
#if TOMATO_N
	// !
#else
	if ((gf = open("/dev/gpio/in", O_RDONLY|O_SYNC)) < 0) return 1;
#endif
*/
	gpio_connect_mask(&mask); //!!TB

	last = 0;
	while (1) {
		if (((gpio = _gpio_read()) == ~0) || (last == (gpio &= mask)) || (check_action() != ACT_IDLE)) {
#ifdef DEBUG_TEST
			cprintf("gpio = %X\n", gpio);
#endif
			sleep(1);
			continue;
		}

		if ((gpio & reset_mask) == reset_pushed) {
#ifdef DEBUG_TEST
			cprintf("reset down\n");
#endif

			led(LED_DIAG, 0);

			count = 0;
			do {
				sleep(1);
				if (++count == 3) led(LED_DIAG, 1);
			} while (((gpio = _gpio_read()) != ~0) && ((gpio & reset_mask) == reset_pushed));

#ifdef DEBUG_TEST
			cprintf("reset count = %d\n", count);
#else
			if (count >= 3) {
				nvram_set("restore_defaults", "1");
				nvram_commit();
				sync();
				reboot(RB_AUTOBOOT);
			}
			else {
				led(LED_DIAG, 1);
				set_action(ACT_REBOOT);
				kill(1, SIGTERM);
			}
			exit(0);
#endif
		}

		if ((ses_mask) && ((gpio & ses_mask) == ses_pushed)) {
			count = 0;
			do {
				led(ses_led, LED_ON);
				usleep(500000);
				led(ses_led, LED_OFF);
				usleep(500000);
				++count;
			} while (((gpio = _gpio_read()) != ~0) && ((gpio & ses_mask) == 0));
			gpio &= mask;

			if ((ses_led == LED_DMZ) && (nvram_match("dmz_enable", "1"))) led(LED_DMZ, 1);

#ifdef DEBUG_TEST
				cprintf("ses count=%d\n", count);
#endif

			if ((count != 3) && (count != 7) && (count != 11)) {
				n = count >> 2;
				if (n > 3) n = 3;
				/*
					0-2  = func0
					4-6  = func1
					8-10 = func2
					12+  = func3
				*/

#ifdef DEBUG_TEST
				cprintf("ses func=%d\n", n);
#else
				sprintf(s, "sesx_b%d", n);
				if ((p = nvram_get(s)) != NULL) {
					switch (*p) {
					case '1':	// toggle wl
						nvram_set("rrules_radio", "-1");
						eval("radio", "toggle");
						break;
					case '2':	// reboot
						kill(1, SIGTERM);
						break;
					case '3':	// shutdown
						kill(1, SIGQUIT);
						break;
					case '4':	// run a script
						sprintf(s, "%d", count);
						run_nvscript("sesx_script", s, 2);
						break;
					}
				}
#endif

			}
		}

		if (brau_mask) {
			last = (gpio & brau_mask);
			if (brau_state != last) {
				brau_state = last;
				p = brau_state ? "auto" : "bridge";
				nvram_set("brau_state", p);
#ifdef DEBUG_TEST
				cprintf("bridge/auto state = %s\n", p);
#else
				run_nvscript("script_brau", p, 2);
#endif
			}
		}

		last = gpio;
	}

	gpio_disconnect_mask(mask); //!!TB
	return 0;
}
