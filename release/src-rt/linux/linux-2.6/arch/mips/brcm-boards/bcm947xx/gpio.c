/*
 * GPIO char driver
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: gpio.c,v 1.5 2008/04/03 03:49:45 Exp $
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif

#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmdevs.h>

#define GPIO_CHAR_MAJOR	127

static si_t *gpio_sih;
#ifndef CONFIG_DEVFS_FS
static struct class *gpio_class;
#endif

static struct {
	char *name;
#ifdef CONFIG_DEVFS_FS
} gpio_file[] = {
	{ "in" },
	{ "out" },
	{ "outen" },
	{ "control" }
#else
	struct class *handle;
} gpio_file[] = {
	{ "gpioin", NULL },
	{ "gpioout", NULL },
	{ "gpioouten", NULL },
	{ "gpiocontrol", NULL }
#endif
};

static int
gpio_open(struct inode *inode, struct file * file)
{
	if (MINOR(inode->i_rdev) > ARRAYSIZE(gpio_file))
		return -ENODEV;

	MOD_INC_USE_COUNT;
	return 0;
}

static int
gpio_release(struct inode *inode, struct file * file)
{
	MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t
gpio_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	u32 val;

	switch (MINOR(file->f_dentry->d_inode->i_rdev)) {
	case 0:
		val = si_gpioin(gpio_sih);
		break;
	case 1:
		val = si_gpioout(gpio_sih, 0, 0, GPIO_DRV_PRIORITY);
		break;
	case 2:
		val = si_gpioouten(gpio_sih, 0, 0, GPIO_DRV_PRIORITY);
		break;
	case 3:
		val = si_gpiocontrol(gpio_sih, 0, 0, GPIO_DRV_PRIORITY);
		break;
	default:
		return -ENODEV;
	}

	if (put_user(val, (u32 *) buf))
		return -EFAULT;

	return sizeof(val);
}

static ssize_t
gpio_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	u32 val;

	if (get_user(val, (u32 *) buf))
		return -EFAULT;

	switch (MINOR(file->f_dentry->d_inode->i_rdev)) {
	case 0:
		return -EACCES;
	case 1:
		si_gpioout(gpio_sih, ~0, val, GPIO_DRV_PRIORITY);
		break;
	case 2:
		si_gpioouten(gpio_sih, ~0, val, GPIO_DRV_PRIORITY);
		break;
	case 3:
		si_gpiocontrol(gpio_sih, ~0, val, GPIO_DRV_PRIORITY);
		break;
	default:
		return -ENODEV;
	}

	return sizeof(val);
}

static struct file_operations gpio_fops = {
	owner:		THIS_MODULE,
	open:		gpio_open,
	release:	gpio_release,
	read:		gpio_read,
	write:		gpio_write,
};

static int __init
gpio_init(void)
{
	int i;

	if (!(gpio_sih = si_kattach(SI_OSH)))
		return -ENODEV;

	si_gpiosetcore(gpio_sih);

	if (register_chrdev(GPIO_CHAR_MAJOR, "gpio", &gpio_fops)) {
		printk(KERN_NOTICE "Can't allocate major number %d for GPIO Devices.\n",
		       GPIO_CHAR_MAJOR);
		return -EAGAIN;
	}

#ifdef CONFIG_DEVFS_FS
	devfs_mk_dir("gpio");
#else
	gpio_class = class_create(THIS_MODULE, "gpio");
	if (IS_ERR(gpio_class)) {
		printk(KERN_ERR "Error creating gpio class.\n");
		unregister_chrdev(GPIO_CHAR_MAJOR, "gpio");
		return PTR_ERR(gpio_class);
	}
#endif

	for (i = 0; i < ARRAYSIZE(gpio_file); i++) {
#ifdef CONFIG_DEVFS_FS
		devfs_mk_cdev(MKDEV(GPIO_CHAR_MAJOR, i),
			S_IFCHR | S_IRUGO | S_IWUGO, "gpio/%s", gpio_file[i].name);
#else
		class_device_create(gpio_class, NULL, MKDEV(GPIO_CHAR_MAJOR, i),
			NULL, gpio_file[i].name);
#endif
	}

	return 0;
}

static void __exit
gpio_exit(void)
{
	int i;

	for (i = 0; i < ARRAYSIZE(gpio_file); i++)
#ifdef CONFIG_DEVFS_FS
		devfs_remove("gpio/%s", gpio_file[i].name);
#else
	{
		if (gpio_file[i].handle != NULL)
			class_device_destroy(gpio_class, MKDEV(GPIO_CHAR_MAJOR, i));
		gpio_file[i].handle = NULL;
	}
	class_destroy(gpio_class);
#endif
	unregister_chrdev(GPIO_CHAR_MAJOR, "gpio");
	si_detach(gpio_sih);
}

module_init(gpio_init);
module_exit(gpio_exit);
