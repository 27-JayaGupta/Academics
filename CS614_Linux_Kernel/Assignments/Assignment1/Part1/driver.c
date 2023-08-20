#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/err.h>
 

//values to read
#define PID 		0
#define	STATIC_PRIO 	1
#define	COMM 		2
#define PPID		3
#define NVCSW		4

#define DEVNAME "cs614_device"

static int command_no = 8;
static int major;
atomic_t device_opened;
static struct class *chardev_class;
struct device *chardev_device;

/*
** Function Prototypes
*/
static int      __init cs614_driver_init(void);
static void     __exit cs614_driver_exit(void);

static ssize_t chardev_set(struct kobject* kobj,
                         struct kobj_attribute* attr,
                         const char* buf,
                         size_t count)
{
    int newval;
    int err = kstrtoint(buf, 10, &newval);
    if(err || newval<0 || newval > 7)
        return -EINVAL;
    
    command_no = newval;
    return count;
}

static struct kobj_attribute chardev_attribute = __ATTR(cs614_value, 0660, NULL, chardev_set);

static struct attribute *chardev_driver_attrs[] = {
    &chardev_attribute.attr,
    NULL
};

static struct attribute_group chardev_attr_group = {
    .attrs = chardev_driver_attrs,
    .name = "cs614_sysfs",
};

static int chardev_open(struct inode *inode, struct file* file)
{
    atomic_inc(&device_opened);
    try_module_get(THIS_MODULE);
    printk(KERN_INFO "[CHARDEV] Device opened successfully\n");
    return 0;
}

static int chardev_release(struct inode* inode, struct file* file)
{
    atomic_dec(&device_opened);
    module_put(THIS_MODULE);
    printk(KERN_INFO "[CHARDEV] Device closed successfully\n");
    return 0;
}

static ssize_t chardev_read(struct file* filp,
                            char* ubuf,
                            size_t length,
                            loff_t *offset)
{       
        char kbuf[1024];
        int len;

        if(command_no == PID) {
            len = sprintf(kbuf, "%d", current->pid);
            if(copy_to_user(ubuf, kbuf, len))
                return -EINVAL;
            printk(KERN_INFO "%s\n", kbuf);
            return len;
        }
        else if(command_no == STATIC_PRIO) {
            len = sprintf(kbuf, "%d", current->static_prio);
            if(copy_to_user(ubuf, kbuf, len))
                return -EINVAL;
            return len;
        }
        else if(command_no == COMM){
            if(copy_to_user(ubuf, current->comm, sizeof(current->comm)))
                return -EINVAL;
            return strlen(current->comm);
        }
        else if(command_no == PPID){
            len = sprintf(kbuf, "%d", current->real_parent->pid);
            if(copy_to_user(ubuf, kbuf, len))
                return -EINVAL;
            return len;
        }
        else if(command_no == NVCSW) {
            len = sprintf(kbuf, "%ld", current->nvcsw);
            if(copy_to_user(ubuf, kbuf, len))
                return -EINVAL;
            return len;
        }

        return 0;
}

static struct file_operations fops = {
        .read = chardev_read,
        .open = chardev_open,
        .release = chardev_release,
};

static char* chardev_devnode(struct device* dev, umode_t *mode)
{
    if(mode && dev->devt == MKDEV(major,0))
        *mode = 0666;
    return NULL;
} 
 
/*
** Module Init function
*/
static int __init cs614_driver_init(void)
{       
    // Initialize variable 
    int err;

    // Create a sysfs directory for user-space control
    int ret = sysfs_create_group(kernel_kobj, &chardev_attr_group);
    if(unlikely(ret)) {
        printk(KERN_INFO "chardev module: Error in creating sysfs directory cs614_sysfs\n");
        return 0;
    }

    major = register_chrdev(0, DEVNAME, &fops);
    err = major;
    if (err < 0) {      
            printk(KERN_ALERT "Registering char device failed with %d\n", major);   
            goto error_regdev;
    }

    chardev_class = class_create(THIS_MODULE, DEVNAME);
    err = PTR_ERR(chardev_class);
    if(IS_ERR(chardev_class)){
        printk(KERN_INFO "Class creation failed with major %d\n", major);
        goto error_class;
    }
        
    chardev_class->devnode = chardev_devnode;
    chardev_device = device_create(chardev_class, NULL,
                                  MKDEV(major, 0),
                                  NULL, DEVNAME);

    err = PTR_ERR(chardev_device);
    if (IS_ERR(chardev_device)){
        printk(KERN_INFO "Device creation failed with major %d\n", major);
        goto error_device;
    }
            
    atomic_set(&device_opened, 0);
    pr_info("[CHARDEV] Device Driver Insert...Done!!!\n");
    return 0;

error_device:
    class_destroy(chardev_class);
error_class:
    unregister_chrdev(major, DEVNAME);
error_regdev:
    sysfs_remove_group(kernel_kobj, &chardev_attr_group);
    return err;

}

/*
** Module exit function
*/
static void __exit cs614_driver_exit(void)
{       
        device_destroy(chardev_class, MKDEV(major, 0));
        class_destroy(chardev_class);
        unregister_chrdev(major, DEVNAME);
        sysfs_remove_group(kernel_kobj, &chardev_attr_group);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(cs614_driver_init);
module_exit(cs614_driver_exit);
 
MODULE_LICENSE("GPL");
