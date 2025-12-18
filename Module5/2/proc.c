#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
 
static int len,temp;
static char *msg;
static const int N = 12;
static ssize_t read_proc(struct file *filp, char *buf, size_t count, loff_t *offp );
static ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp);
static void create_new_proc_entry(void);
static int proc_init (void);
static void proc_cleanup(void);

static ssize_t read_proc(struct file *filp, char *buf, size_t count, loff_t *offp ) {
    static int rec;
    if(count > temp) {
        count = temp;
    }
    temp = temp - count;
    
    rec = copy_to_user(buf, msg, count);
    if (rec>0){
        count-=rec;
        temp+=rec;
    }
    if(count == 0)
        temp = len;
    return count;
}
 
static ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp) {
    static int rec;
    if (count>N){
        return -ENOMEM;
    }
    rec = copy_from_user(msg, buf, count);
    if(rec>0)count-=rec;

    len = count;
    temp = len;
    return count;
}
 
static const struct proc_ops proc_fops = {
    proc_read: read_proc,
    proc_write: write_proc,
};
 
static void create_new_proc_entry(void) { //use of void for no arguments is compulsory now
    proc_create("hello", 0, NULL, &proc_fops);
    msg = kmalloc(N * sizeof(char), GFP_KERNEL);
}

static int proc_init (void) {
    create_new_proc_entry();
    return 0;
}
 
static void proc_cleanup(void) {
    remove_proc_entry("hello", NULL);
    kfree(msg);
}
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pavel");
module_init(proc_init);
module_exit(proc_cleanup);