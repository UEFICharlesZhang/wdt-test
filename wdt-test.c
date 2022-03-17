// SPDX-License-Identifier: GPL-2.0+
/*
 *	TestDog:	A Test Watchdog Device
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/reboot.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <linux/workqueue.h>

#define TIMER_MARGIN	60		/* Default is 60 seconds */
static unsigned int soft_margin = TIMER_MARGIN;	/* in seconds */
module_param(soft_margin, uint, 0);
MODULE_PARM_DESC(soft_margin,
	"Watchdog soft_margin in seconds. (0 < soft_margin < 65536, default="
					__MODULE_STRING(TIMER_MARGIN) ")");

static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout,
		"Watchdog cannot be stopped once started (default="
				__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

static int soft_noboot;
module_param(soft_noboot, int, 0);
MODULE_PARM_DESC(soft_noboot,
	"Softdog action, set to 1 to ignore reboots, 0 to reboot (default=0)");

static int soft_panic;
module_param(soft_panic, int, 0);
MODULE_PARM_DESC(soft_panic,
	"Softdog action, set to 1 to panic, 0 to reboot (default=0)");

static char *soft_reboot_cmd;
module_param(soft_reboot_cmd, charp, 0000);
MODULE_PARM_DESC(soft_reboot_cmd,
	"Set reboot command. Emergency reboot takes place if unset");

static bool soft_active_on_boot;
module_param(soft_active_on_boot, bool, 0000);
MODULE_PARM_DESC(soft_active_on_boot,
	"Set to true to active Softdog on boot (default=false)");

static int timeleft;
static int softdog_start(struct watchdog_device *w)
{
    printk(KERN_ERR "softdog_start\n");
	timeleft = soft_margin;
	return 0;
}

static int softdog_stop(struct watchdog_device *w)
{
	printk(KERN_ERR "softdog_stop\n");
	return 0;
}

// static int softdog_ping(struct watchdog_device *wdt_dev)
// {
// 	printk(KERN_ERR "softdog_ping\n");
// 	return 0;
// }


static int softdog_set_timeout(struct watchdog_device *wdt_dev,
		unsigned int timeout)
{
	printk(KERN_ERR "softdog_set_timeout:%d\n",timeout);
	soft_margin=timeout;
	return 0;
}

static unsigned int softdog_get_timeleft(struct watchdog_device *wdd)
{
	if (timeleft > 0)
		timeleft--;
	else
		timeleft = 0;

	printk(KERN_ERR "softdog_get_timeleft:%d\n",timeleft);
	return timeleft;
}

static struct watchdog_info softdog_info = {
	.identity = "Test Watchdog",
	.options = WDIOF_SETTIMEOUT | WDIOF_MAGICCLOSE,
};

static const struct watchdog_ops softdog_ops = {
	.owner = THIS_MODULE,
	.start = softdog_start,
	// .ping = softdog_ping,
	.stop = softdog_stop,
	.set_timeout	= softdog_set_timeout,
	.get_timeleft	= softdog_get_timeleft,
};

static struct watchdog_device softdog_dev = {
	.info = &softdog_info,
	.ops = &softdog_ops,
	.min_timeout = 1,
	.max_timeout = 65535,
	.timeout = TIMER_MARGIN,
};

static int __init softdog_init(void)
{
	int ret;

	watchdog_init_timeout(&softdog_dev, soft_margin, NULL);
	watchdog_set_nowayout(&softdog_dev, nowayout);
	watchdog_stop_on_reboot(&softdog_dev);

	if (soft_active_on_boot)
		softdog_start(&softdog_dev);

	ret = watchdog_register_device(&softdog_dev);
	if (ret)
		return ret;

	pr_info("initialized. soft_noboot=%d soft_margin=%d sec soft_panic=%d (nowayout=%d)\n",
		soft_noboot, softdog_dev.timeout, soft_panic, nowayout);
	pr_info("             soft_reboot_cmd=%s soft_active_on_boot=%d\n",
		soft_reboot_cmd ?: "<not set>", soft_active_on_boot);

	return 0;
}
module_init(softdog_init);

static void __exit softdog_exit(void)
{
	watchdog_unregister_device(&softdog_dev);
}
module_exit(softdog_exit);

MODULE_AUTHOR("Charles Zhang");
MODULE_DESCRIPTION("Test Watchdog Device Driver");
MODULE_LICENSE("GPL");

