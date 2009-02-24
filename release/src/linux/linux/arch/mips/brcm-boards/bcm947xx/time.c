/*
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/serial_reg.h>
#include <linux/interrupt.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/time.h>

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <sbconfig.h>
#include <sbextif.h>
#include <sbchipc.h>
#include <sbutils.h>
#include <hndmips.h>
#include <mipsinc.h>
#include <hndcpu.h>
#include <bcmdevs.h>

/* Global SB handle */
extern void *bcm947xx_sbh;
extern spinlock_t bcm947xx_sbh_lock;

/* Convenience */
#define sbh bcm947xx_sbh
#define sbh_lock bcm947xx_sbh_lock

extern int panic_timeout;
static int watchdog = 0;
static u8 *mcr = NULL;

/* Add functions to blink WLAN LED and implement reset button */
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/delay.h>
#include "wlioctl.h"

#define WLAN_IF_NAME            "eth0"
#define WLAN_LED_BLINK_RATE     10 
#define WLAN_LED_GPIO           5
#define TEST_GPIO               6
#define RESET_BUTTON_GPIO       7
#define UDELAY_COUNT            30

static sb_t *gpio_sbh;
static int wlan_led_init(void)
{
    if (!(gpio_sbh = sb_kattach(SB_OSH))) {
        printk("%s failed!\n", __FUNCTION__);
        return -ENODEV;
    }

//    sb_gpioouten(gpio_sbh, ((uint32) 1 << RESET_BUTTON_GPIO), 0, GPIO_APP_PRIORITY);
    
    /* init WLAN LED (gpio=5) */
//    sb_gpioreserve(gpio_sbh, 1 << WLAN_LED_GPIO, GPIO_APP_PRIORITY);
//    sb_gpioouten(gpio_sbh, 1 << WLAN_LED_GPIO, 1 << WLAN_LED_GPIO, GPIO_APP_PRIORITY);
    return 0;
}

static int gpio_led_on_off(int gpio_num, int on_off)
{
    sb_gpioout(gpio_sbh, 1 << gpio_num, on_off << gpio_num, GPIO_APP_PRIORITY);
    return 0;
}

/* Used for controling WLAN LED when "ath0" interface does not exist" */
#if 0
static int wlan_led_state = 0;
void wlan_led_control(int on)
{
    wlan_led_state = on;
}
EXPORT_SYMBOL(wlan_led_control);
#endif

/* API to blink WLAN LED */
static int blink_wlan_led(void)
{
    struct net_device *wlan_dev = NULL;
    static int interrupt_count = 0;
    static int pkt_cnt_old = 0;
    static int led_state = 0;
    struct ifreq ifr;
    wl_ioctl_t ioc;    
    unsigned char buffer[200];
    unsigned char blank_bssid[6] = "\0\0\0\0\0\0";
    int ret;

    interrupt_count++;
    if (interrupt_count == WLAN_LED_BLINK_RATE/2)
        gpio_led_on_off(WLAN_LED_GPIO, led_state);
    if (interrupt_count < WLAN_LED_BLINK_RATE)
        return 0;
        
    interrupt_count = 0;
    
    wlan_dev = dev_get_by_name(WLAN_IF_NAME);
    if (wlan_dev) {

        /* Check radio status */
        ioc.cmd = WLC_GET_BSSID;
        ioc.buf = buffer;
        ioc.len = sizeof(buffer);
        strcpy(ifr.ifr_name, WLAN_IF_NAME);
        ifr.ifr_data = (caddr_t) &ioc;
        ret = wlan_dev->do_ioctl(wlan_dev, &ifr, SIOCDEVPRIVATE);
        if (ret != 0 || (memcmp(buffer, blank_bssid, 6) == 0) ) {
            /* Radio is off, turn off LED */
            gpio_led_on_off(WLAN_LED_GPIO, 0);
            return 0;
        }

        if (wlan_dev->flags & IFF_UP) {
            /* Interface is up and running, check device stats */
            struct net_device_stats *wlan_dev_stats = NULL;
            int    pkt_cnt;

            led_state = 1;

            /* Check total packet count, if different from before, blink LED. */
            wlan_dev_stats = wlan_dev->get_stats(wlan_dev);
            if (wlan_dev_stats) {
                pkt_cnt = wlan_dev_stats->tx_packets + wlan_dev_stats->rx_packets;
                if (pkt_cnt != pkt_cnt_old) {
                    pkt_cnt_old = pkt_cnt;
                    gpio_led_on_off(WLAN_LED_GPIO, 0);
                }
            }
        } else {
            /* Interface is down, turn LED off */
            led_state = 0;
        }
        dev_put(wlan_dev);
#if 0
    } else if (wlan_led_state) {
        /* If user wanted to turn this LED on */
        led_state = 1;
#endif
    } else {
        /* Interface not ready, turn LED off */
        led_state = 0;
    }

    return 0;
}

#if 0
static void send_reboot_msg(int data)
{
    struct sk_buff *skb = NULL; 
    struct sock *nl_sk = NULL;
    struct nlmsghdr *nlh;

    nl_sk = netlink_kernel_create(NETLINK_USERSOCK, NULL);
    if (!nl_sk) {
        printk(KERN_ERR "Unable to create netlink socket!\n");
        return;
    }

    skb = alloc_skb(1024, GFP_ATOMIC);
    
    skb_put(skb, sizeof(int));
    memcpy(skb->data, (char *)&data, sizeof(int));

    /*multicast the message to all listening processes*/
    netlink_broadcast(nl_sk, skb, 0, 1, GFP_ATOMIC);

    /* Don't care about releasing netlink socket, 
     * since we are going to reboot anyway.
     */
}

void check_reset_button(void)
{
    static int reset_cnt = 0;
    int val;
    int is_reset;

    /* disable output bit first */
    sb_gpioout(gpio_sbh, (unsigned long)1 << RESET_BUTTON_GPIO, (unsigned long)1 << RESET_BUTTON_GPIO, GPIO_HI_PRIORITY); /* pull high first for old hardware revision */
    sb_gpioouten(gpio_sbh, ((unsigned long) 1 << RESET_BUTTON_GPIO), ((unsigned long) 0 << RESET_BUTTON_GPIO), GPIO_HI_PRIORITY);

    val = sb_gpioin(gpio_sbh);
    
    /* enable the output bit and pull low the status to turn on the power led (hardware revision L1) */
    sb_gpioouten(gpio_sbh, ((unsigned long) 1 << RESET_BUTTON_GPIO), ((unsigned long) 1 << RESET_BUTTON_GPIO), GPIO_HI_PRIORITY);
    sb_gpioout(gpio_sbh, (unsigned long)1 << RESET_BUTTON_GPIO, (unsigned long)0 << RESET_BUTTON_GPIO, GPIO_HI_PRIORITY);
    
    is_reset = !(val & (1 << RESET_BUTTON_GPIO)); /* S/W reset: low */

    if (!is_reset && !reset_cnt)
        return;

    /* Change Power LED to amber, when reset button is first pressed */
    if (is_reset && reset_cnt == 0) {
        gpio_led_on_off(TEST_GPIO, 1);
    }

    reset_cnt++;
    
    /* 5 sec passed, load default */
    if (reset_cnt > 500) {
        if (is_reset) {
            /* User has not release reset button yet, 
             * Blink Power LED every sec, 0.25 sec on, 0.75 sec off
             */
            if ((reset_cnt % 100) == 25) {
                /* Turn off @ 0.25s */
                gpio_led_on_off(TEST_GPIO, 0);
            } else if ((reset_cnt % 100) == 0) {
                /* Turn on at sec interval */
                gpio_led_on_off(TEST_GPIO, 1);
            }
        } else {
            printk(KERN_DEBUG "Load Default\n");

            /* Turn off power/test LED */            
            gpio_led_on_off(TEST_GPIO, 0);

            /* Send reboot message to user-space swresetd */
            send_reboot_msg(1);
            reset_cnt = 0;
            return;
        }
    }

    if (!is_reset) {
        printk(KERN_DEBUG "Reset \n");
        
        /* Turn off power/test LED */
        gpio_led_on_off(TEST_GPIO, 0);

        send_reboot_msg(0);
        reset_cnt = 0;
    }
}
#endif

void __init
bcm947xx_time_init(void)
{
	unsigned int hz;
	extifregs_t *eir;

	/*
	 * Use deterministic values for initial counter interrupt
	 * so that calibrate delay avoids encountering a counter wrap.
	 */
	write_c0_count(0);
	write_c0_compare(0xffff);

	if (!(hz = sb_cpu_clock(sbh)))
		hz = 100000000;

	printk("CPU: BCM%04x rev %d at %d MHz\n", sb_chip(sbh), sb_chiprev(sbh),
	       (hz + 500000) / 1000000);

	/* Set MIPS counter frequency for fixed_rate_gettimeoffset() */
	// !!TB - fix for WL-520GU clock rate
	if (sb_chip(sbh) == BCM5354_CHIP_ID && nvram_match("t_fix1", "WL-520GU"))
		mips_counter_frequency = 100000000; // Fix WL520GUGC clock
	else
		mips_counter_frequency = hz / 2;

	/* Set watchdog interval in ms */
	watchdog = simple_strtoul(nvram_safe_get("watchdog"), NULL, 0);

	/* Please set the watchdog to 3 sec if it is less than 3 but not equal to 0 */
	if (watchdog > 0) {
		if (watchdog < 3000)
			watchdog = 3000;
	}

	/* Set panic timeout in seconds */
	panic_timeout = watchdog / 1000;

	/* Setup blink */
	if ((eir = sb_setcore(sbh, SB_EXTIF, 0))) {
		sbconfig_t *sb = (sbconfig_t *)((unsigned int) eir + SBCONFIGOFF);
		unsigned long base = EXTIF_CFGIF_BASE(sb_base(readl(&sb->sbadmatch1)));
		mcr = (u8 *) ioremap_nocache(base + UART_MCR, 1);
	}

    /* Init WLAN LED */
    wlan_led_init();
}

#ifdef CONFIG_HND_BMIPS3300_PROF
extern bool hndprofiling;
#ifdef CONFIG_MIPS64
typedef u_int64_t sbprof_pc;
#else
typedef u_int32_t sbprof_pc;
#endif
extern void sbprof_cpu_intr(sbprof_pc restartpc);
#endif	/* CONFIG_HND_BMIPS3300_PROF */

static void
bcm947xx_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
#ifdef CONFIG_HND_BMIPS3300_PROF
	/*
	 * Are there any ExcCode or other mean(s) to determine what has caused
	 * the timer interrupt? For now simply stop the normal timer proc if
	 * count register is less than compare register.
	 */
	if (hndprofiling) {
		sbprof_cpu_intr(regs->cp0_epc +
		                ((regs->cp0_cause >> (CAUSEB_BD - 2)) & 4));
		if (read_c0_count() < read_c0_compare())
			return;
	}
#endif	/* CONFIG_HND_BMIPS3300_PROF */

	/* Generic MIPS timer code */
	timer_interrupt(irq, dev_id, regs);

	/* Set the watchdog timer to reset after the specified number of ms */
	if (watchdog > 0) {
		if (sb_chip(sbh) == BCM5354_CHIP_ID)
			sb_watchdog(sbh, WATCHDOG_CLOCK_5354 / 1000 * watchdog);
		else
			sb_watchdog(sbh, WATCHDOG_CLOCK / 1000 * watchdog);
	}

#ifdef	CONFIG_HWSIM
	(*((int *)0xa0000f1c))++;
#else
	/* Blink one of the LEDs in the external UART */
	if (mcr && !(jiffies % (HZ/2)))
		writeb(readb(mcr) ^ UART_MCR_OUT2, mcr);
#endif
    
}

static struct irqaction bcm947xx_timer_irqaction = {
	bcm947xx_timer_interrupt,
	SA_INTERRUPT,
	0,
	"timer",
	NULL,
	NULL
};

void __init
bcm947xx_timer_setup(struct irqaction *irq)
{
	/* Enable the timer interrupt */
	setup_irq(7, &bcm947xx_timer_irqaction);
}
