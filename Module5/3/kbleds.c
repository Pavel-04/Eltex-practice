/*
 * kbleds.c − Blink keyboard leds until the module is unloaded.(modified for > 4.15)
 */
#include <linux/module.h>
#include <linux/configfs.h>
#include <linux/init.h>
#include <linux/tty.h>          /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>           /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h>       /* For vc_cons */
#include <linux/vt_kern.h>
#include <linux/timer.h>

static struct kobject *example_kobject;
MODULE_DESCRIPTION("Example module illustrating the use of Keyboard LEDs.");
MODULE_LICENSE("GPL");
struct timer_list my_timer;
struct tty_driver *my_driver;
static int _kbledstatus = 0;
static int test = 8;// cod
static int led_pos = 0;
#define BLINK_DELAY   HZ/5
#define ALL_LEDS_ON   0x07
#define RESTORE_LEDS  0xFF


static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf)
{
        return sprintf(buf, "%d\n", test);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
                      const char *buf, size_t count)
{
        sscanf(buf, "%du", &test);
        if(test==0 || test>8)
			return -EINVAL;
        return count;
}


static struct kobj_attribute foo_attribute =__ATTR(test, 0660, foo_show,
                                                   foo_store);
                                                   
static void my_timer_func(struct timer_list *ptr)
{		
		if (test==8){
			static int sequence[] = {2, 6, 7, 1, 5, 7,2};
			static int num_states = 7;
			
			(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,
                            sequence[led_pos]);
             led_pos = (led_pos + 1) % num_states;
		}
		else {
		
        
			int *pstatus = &_kbledstatus;
			if (*pstatus == test)
                *pstatus = RESTORE_LEDS;
			else
                *pstatus = test;
        
			(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,
                            *pstatus);
		}
        
        my_timer.expires = jiffies + BLINK_DELAY;
        add_timer(&my_timer);
}
static int __init kbleds_init(void)
{
        int i;
        int error = 0;
        printk(KERN_INFO "kbleds: loading\n");
        printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
        for (i = 0; i < MAX_NR_CONSOLES; i++) {
                if (!vc_cons[i].d)
                        break;
                printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i,
                       MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
                       (unsigned long)vc_cons[i].d->port.tty);
        }
        printk(KERN_INFO "kbleds: finished scanning consoles\n");
        example_kobject = kobject_create_and_add("kbleds",
                                                 kernel_kobj);
        if(!example_kobject)
                return -ENOMEM;

        error = sysfs_create_file(example_kobject, &foo_attribute.attr);
        if (error) {
                pr_debug("failed to create the foo file in /sys/kernel/kbleds \n");
                kobject_put(example_kobject);
        }
        
        my_driver = vc_cons[fg_console].d->port.tty->driver;
 
        
        timer_setup(&my_timer, my_timer_func, 0);
        my_timer.expires = jiffies + BLINK_DELAY;
        add_timer(&my_timer);
        return error;
}
static void __exit kbleds_cleanup(void)
{
        printk(KERN_INFO "kbleds: unloading...\n");
        del_timer(&my_timer);
        (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,
                            RESTORE_LEDS);
         kobject_put(example_kobject);
}
module_init(kbleds_init);
module_exit(kbleds_cleanup);
