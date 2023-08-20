#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/file.h>
#include<linux/fs.h>
#include<linux/path.h>
#include<linux/slab.h>
#include<linux/dcache.h>
#include<linux/sched.h>
#include<linux/uaccess.h>
#include<linux/fs_struct.h>
#include <asm/tlbflush.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include <linux/kthread.h> 
#include <linux/delay.h>
#include <linux/vmalloc.h>

#include "btplus.h"

static int major;
atomic_t  device_opened;
static struct class *demo_class;
struct device *demo_device;

static unsigned long gptr;
static pte_t *gpte;

struct data{
    unsigned long pmd;
    unsigned long pte;
};


struct address{
    unsigned long from_addr;
    unsigned long to_addr;
};

struct input{
    unsigned long addr;
    unsigned length;
    struct address * buff;
};

static int device_open(struct inode *inode, struct file *file)
{
        atomic_inc(&device_opened);
        try_module_get(THIS_MODULE);
        printk(KERN_INFO "Device opened successfully\n");
        return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);
        printk(KERN_INFO "Device closed successfully\n");

        return 0;
}
static pte_t* get_pte(unsigned long address, struct data* dt)
{
        pgd_t *pgd;
        p4d_t *p4d;
        pud_t *pud;
        pmd_t *pmd;
        pte_t *ptep;
        struct mm_struct *mm = current->mm;
        
        pgd = pgd_offset(mm, address);
        if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
                goto nul_ret;
        printk(KERN_INFO "pgd(va) [%lx] pgd (pa) [%lx] *pgd [%lx]\n", (unsigned long)pgd, __pa(pgd), pgd->pgd); 
        p4d = p4d_offset(pgd, address);
        if (p4d_none(*p4d))
                goto nul_ret;
        if (unlikely(p4d_bad(*p4d)))
                goto nul_ret;
        pud = pud_offset(p4d, address);
        if (pud_none(*pud))
                goto nul_ret;
        if (unlikely(pud_bad(*pud)))
                goto nul_ret;
        printk(KERN_INFO "pud(va) [%lx] pud (pa) [%lx] *pud [%lx]\n", (unsigned long)pud, __pa(pud), pud->pud); 

        pmd = pmd_offset(pud, address);
        if (pmd_none(*pmd))
                goto nul_ret;
        if (unlikely(pmd_trans_huge(*pmd))){
                printk(KERN_INFO "I am huge\n");
                goto nul_ret;
        }
        printk(KERN_INFO "pmd(va) [%lx] pmd (pa) [%lx] *pmd [%lx]\n", (unsigned long)pmd, __pa(pmd), pmd->pmd);
	dt->pmd = pmd->pmd;
        ptep = pte_offset_map(pmd, address);
        if(!ptep){
                printk(KERN_INFO "pte_p is null\n\n");
                goto nul_ret;
        }
        printk(KERN_INFO "pte(va) [%lx] pte (pa) [%lx] *pte [%lx]\n", (unsigned long)ptep, __pa(ptep), ptep->pte);
	dt->pte = ptep->pte;
        return ptep;

        nul_ret:
               printk(KERN_INFO "Address could not be translated\n");
               return NULL;

}


static ssize_t device_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t * offset){
    struct data dt;
    //struct data *temp = (struct data*)buffer;
    printk("read called\n");
    gpte = get_pte(gptr,&dt);
    stac();
    memcpy(buffer,&dt,sizeof(struct data));
    //sprintf((char*)&(temp->pmd),"%lu",dt.pmd);
    //sprintf((char*)&(temp->pte),"%lu",dt.pte);
    clac();
    printk("pmd:%lx,pte:%lx\n",dt.pmd,dt.pte);
    return 0;
}

static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off){
        unsigned long addr = 0;
	stac();
            addr = *((unsigned long *)buff);
    clac();

    //sprintf(&addr,"%lu",*(unsigned long*)buff);
    /*char kbuf[8];
    if(copy_from_user(kbuf,buff,len)){
        printk("copy from user failed\n");
    }
    kbuf[7] = '\0';
    if(kstrtol(kbuf,0,&addr)<0){
        printk("kstrtoul failed\n");
    }*/
    //sscanf(buff,"%lu",&addr);
    printk(KERN_INFO "In write %lx\n", addr);
    gptr = addr;
    return sizeof(unsigned long);
}


long device_ioctl(struct file *file,	
		 unsigned int ioctl_num,
		 unsigned long ioctl_param)
{
	printk("ioctl called\n");
	return 0;
}


static struct file_operations fops = {
        .read = device_read,
        .write = device_write,
	.unlocked_ioctl = device_ioctl,
        .open = device_open,
        .release = device_release,
};

static char *demo_devnode(struct device *dev, umode_t *mode)
{
        if (mode && dev->devt == MKDEV(major, 0))
                *mode = 0666;
        return NULL;
}



int init_module(void)
{
        int err;
	printk(KERN_INFO "Hello kernel\n");
        major = register_chrdev(0, DEVNAME, &fops);
        err = major;
        if (err < 0) {      
             printk(KERN_ALERT "Registering char device failed with %d\n", major);   
             goto error_regdev;
        }                 
        
        demo_class = class_create(THIS_MODULE, DEVNAME);
        err = PTR_ERR(demo_class);
        if (IS_ERR(demo_class))
                goto error_class;

        demo_class->devnode = demo_devnode;

        demo_device = device_create(demo_class, NULL,
                                        MKDEV(major, 0),
                                        NULL, DEVNAME);
        err = PTR_ERR(demo_device);
        if (IS_ERR(demo_device))
                goto error_device;
 
        printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);                                                              
        atomic_set(&device_opened, 0);
        
	return 0;

error_device:
         class_destroy(demo_class);
error_class:
        unregister_chrdev(major, DEVNAME);
error_regdev:
        return  err;
}

void cleanup_module(void)
{
        device_destroy(demo_class, MKDEV(major, 0));
        class_destroy(demo_class);
        unregister_chrdev(major, DEVNAME);
	printk(KERN_INFO "Goodbye kernel\n");
}

MODULE_AUTHOR("cs614");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("assignment2");
