#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>
#include<linux/kthread.h>

static int kth_enabled = 0;
static long num_runs = 0;
static struct task_struct *kth_task = NULL;

static int kth_fn(void *unused)
{
    printk("Starting Kthread with pid = %d\n", current->pid); 	
    while (!kthread_should_stop())
    {
	num_runs++;    
        schedule_timeout_interruptible(10);
    }
    printk("Kthread with pid = %d finishing\n", current->pid); 	
    return 0;
}

static ssize_t kth_status(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", kth_enabled);
}

static ssize_t kth_set(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
        int newval;
        int err = kstrtoint(buf, 10, &newval);
        if (err || newval < 0 || newval > 1 )
                return -EINVAL;
        kth_enabled = (newval != 0);
        if(kth_enabled && !kth_task){
	    kth_task = kthread_run(kth_fn, NULL, "cs614kth");
	    BUG_ON(IS_ERR(kth_task));
	}else if(kth_task && !kth_enabled){
		kthread_stop(kth_task);
		kth_task = NULL;
	}	
        return count;
}

static struct kobj_attribute kth_attribute = __ATTR(control, 0644, kth_status, kth_set);

static ssize_t get_numruns(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%ld\n", num_runs);
}

static struct kobj_attribute numruns_attribute = __ATTR(numruns, 0444, get_numruns, NULL);

static struct attribute *kth_attrs[] = {
        &kth_attribute.attr,
        &numruns_attribute.attr,
        NULL,
};
static struct attribute_group kth_attr_group = {
        .attrs = kth_attrs,
        .name = "cs614hook",
};

int init_module(void)
{
        int ret = sysfs_create_group (kernel_kobj, &kth_attr_group);
        if(unlikely(ret))
                printk(KERN_INFO "demo: can't create sysfs\n");
	printk(KERN_INFO "All set to play\n");
	return 0;
}

void cleanup_module(void)
{
        sysfs_remove_group (kernel_kobj, &kth_attr_group);
        if(kth_enabled){
		BUG_ON(!kth_task);
		kthread_stop(kth_task);
	}	
	printk(KERN_INFO "Removed the trap hook handler\n");
}
MODULE_LICENSE("GPL");
