#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harsha");
MODULE_DESCRIPTION("A simple Linux kernel module to report system uptime");
MODULE_VERSION("1.0");

static int __init uptime_report_init(void) {
    struct timespec64 uptime;
    ktime_get_boottime_ts64(&uptime);
    
    printk(KERN_INFO "uptime_report: Module loaded.\n");
    printk(KERN_INFO "uptime_report: System uptime: %lld seconds.\n", uptime.tv_sec);
    return 0;
}

static void __exit uptime_report_exit(void) {
    printk(KERN_INFO "uptime_report: Module unloaded.\n");
}

module_init(uptime_report_init);
module_exit(uptime_report_exit);
