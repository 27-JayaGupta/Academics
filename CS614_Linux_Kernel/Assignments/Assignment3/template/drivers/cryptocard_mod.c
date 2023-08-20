#include<linux/module.h>
#include<linux/kernel.h>
#include <linux/io.h>
#include<linux/slab.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include <asm/pgtable.h>
#include <asm/io.h>

#include "cryptocard_mod.h"

int handle_count = 1;
char crypto_driver_name[] = "cryptocard";
struct crypto_hw *crypto_hw_details;
struct process_config global_config;
struct proc_keys_list_entry* active_proc_on_device;
int sanity_check = 0;

static int major;
atomic_t device_opened;
static struct class *chardev_class;
struct device *chardev_device;

static LIST_HEAD(proc_keys_list);
static DEFINE_MUTEX(keys_list_mutex);

static DEFINE_MUTEX(encrypt_decrypt_mutex);

static const struct pci_device_id crypto_pci_tbl[] = {
	CRYPTO_DEVICE(0x1234, 0xdeba),

	/* required last entry */
	{0,}
};

unsigned long count = 0;
static int crypto_init_module(void);
static void crypto_exit_module(void);
static int crypto_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void crypto_remove(struct pci_dev *pdev);

static struct pci_driver crypto_driver = {
	.name     = crypto_driver_name,
	.id_table = crypto_pci_tbl,
	.probe    = crypto_probe,
	.remove   = crypto_remove,
	// .shutdown = crypto_shutdown,
	// .err_handler = &crypto_err_handler
};

MODULE_DEVICE_TABLE(pci, crypto_pci_tbl);

// ############################ utility functions #######################################
pte_t* __check_if_physically_mapped(struct mm_struct* mm, unsigned long address){
	    
		pgd_t *pgd;
        p4d_t *p4d;
        pud_t *pud;
        pmd_t *pmd;
        pte_t *ptep;
        // struct mm_struct *mm = current->mm;

        pgd = pgd_offset(mm, address);
        if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
                goto nul_ret;
        
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

        pmd = pmd_offset(pud, address);
        if (pmd_none(*pmd))
                goto nul_ret;
        if (unlikely(pmd_trans_huge(*pmd))){
                printk(KERN_INFO "I am huge\n");
                goto nul_ret;
        }

        ptep = pte_offset_map(pmd, address);

        if(!ptep){
			printk(KERN_INFO "pte_p is null\n\n");
			goto nul_ret;
        }
		
		if(ptep->pte == 0)
			return NULL;
        
		return ptep;

nul_ret:
		return NULL;
}

unsigned long __get_pfn(struct mm_struct* mm, unsigned long addr){

	pte_t* pte;

	pte = __check_if_physically_mapped(mm, addr);
	if(pte == NULL){
		printk("No mapping present in kernel for the given address\n");
		return 0;
	}
	return pte_pfn(*pte);
}

bool crypto_check_device_live(void){

	unsigned int live_check = 0x00010001, live_check_comp;
	unsigned int num;
	writel(live_check, crypto_hw_details->hw_addr+4);
	live_check_comp = readl(crypto_hw_details->hw_addr+4);

	// printk("Live Check: %d, Comp: %d\n", live_check, live_check_comp);
	num = live_check ^ live_check_comp;

	if((num+1) == 0)
		return true;
	else 
		return false;
}

void set_device_keys(char key_a, char key_b){
	writel((key_a<<8)|(key_b), crypto_hw_details->hw_addr + KEY_OFFSET);
}

struct proc_keys_list_entry* find_process_in_list(void){
	int flag = 0;
	struct proc_keys_list_entry* iterator;

	mutex_lock(&keys_list_mutex);
	list_for_each_entry(iterator, &proc_keys_list, list) {
		if (iterator->pid == current->pid) {
				flag = 1;
				break;
		}
	}
	mutex_unlock(&keys_list_mutex);
	if(flag)
		return iterator;
	else
		return NULL;
}

int get_mmio_configs(char utility, bool intr){

	int config_flag = 0;

	if(utility == 'e')
		config_flag |= MMIO_ENCRYPTION;
	else 
		config_flag |= MMIO_DECRYPTION;
	
	if(intr)
		config_flag |= MMIO_INTERRUPT;
	else
		config_flag |= MMIO_NO_INTERRUPT;

	return config_flag;
}

int get_dma_configs(char utility, bool intr){

	int config_flag = 0;

	if(utility == 'e')
		config_flag |= DMA_ENCRYPTION;
	else 
		config_flag |= DMA_DECRYPTION;
	
	if(intr)
		config_flag |= DMA_INTERRUPT;
	else
		config_flag |= DMA_NO_INTERRUPT;

	config_flag |= START_DMA;

	return config_flag;
}

// ############################ encryption/decryption functions #######################################

// Length is the len of msg to be encrypted or decrypted
void perform_mmio_encrypt_decrypt(struct file* filp,
									const char* ubuf,
									size_t length,
									loff_t *offset,
									struct proc_keys_list_entry* iterator)
{	
	int status;

	if(count %100 == 0) printk("Doing MMIO for process with pid: %d for %ld time\n", iterator->pid, count);
	writel(length, crypto_hw_details->hw_addr + MMIO_MSG_LEN_OFFSET);

	if(iterator->config.interrupt_allowed)
		atomic_set(&iterator->is_intr_handled, 0);
	
	writel(get_mmio_configs(ubuf[0], iterator->config.interrupt_allowed), crypto_hw_details->hw_addr + MMIO_STATUS_OFFSET);

	// For unmapped case, copy the buffer to device
	if(ubuf[1] == 'u'){
		memcpy(crypto_hw_details->hw_addr+MMIO_BUFFER_OFFSET, ubuf+2, length);
	}

	writeq(MMIO_BUFFER_OFFSET, crypto_hw_details->hw_addr+MMIO_ADDR_OFFSET);

	if(!(iterator->config.interrupt_allowed)){
		// Since it is a non-interruptible mmio keep polling
		while(true){
			status = readl(crypto_hw_details->hw_addr + MMIO_STATUS_OFFSET);
			if(status%2 == 0)
				break;
		}
	}
	else {
		while(atomic_read(&iterator->is_intr_handled) == 0);
	}

	// For unmapped case, copy the decrypted/encrypted to the kernel buffer
	// Read call after it will copy this data to user space.
	if(ubuf[1] == 'u'){
		// For unmapped processes
		if(iterator->result == NULL || iterator->result_len != length){
			if(iterator->result) 
				kfree(iterator->result);
			iterator->result = (char*) kzalloc(length * sizeof(char), GFP_KERNEL);
			iterator->result_len = length;
		}

		memcpy(iterator->result, crypto_hw_details->hw_addr+MMIO_BUFFER_OFFSET, length);
	}

	// for mapped processees release the lock as you will not come back again to read the message
	active_proc_on_device = NULL;
	mutex_unlock(&encrypt_decrypt_mutex);

	return;
}

void perform_dma_encrypt_decrypt(struct file* filp,
                            const char* ubuf,
                            size_t length,
                            loff_t *offset,
                            struct proc_keys_list_entry* iterator)
{
	long status;

	if(count %100 == 0) printk("Doing DMA for process with pid: %d for %ld time\n", iterator->pid, count);
	writeq(length, crypto_hw_details->hw_addr + DMA_LEN_OFFSET);

	memcpy(crypto_hw_details->dma_virt_addr, ubuf+2, length);

	if(iterator->config.interrupt_allowed)
		atomic_set(&iterator->is_intr_handled, 0);
	
	writeq(get_dma_configs(ubuf[0], iterator->config.interrupt_allowed), crypto_hw_details->hw_addr + DMA_COMMAND_REG_OFFSET);
	
	if(!(iterator->config.interrupt_allowed)){
		// Since it is a non-interruptible dma keep polling	
		while(true){
			status = readq(crypto_hw_details->hw_addr + DMA_COMMAND_REG_OFFSET);
			if(status%2 == 0)
				break;
		}
	} else {
		while(atomic_read(&iterator->is_intr_handled) == 0);
	}

	if(iterator->result == NULL || iterator->result_len != length){
		if(iterator->result) 
			kfree(iterator->result);
		iterator->result = (char*) kzalloc(length * sizeof(char), GFP_KERNEL);
		iterator->result_len = length;
	}

	memcpy(iterator->result, crypto_hw_details->dma_virt_addr, length);

	active_proc_on_device = NULL;
	mutex_unlock(&encrypt_decrypt_mutex);
	return;
}

// ############################ SYSFS UTILITY functions #######################################
static ssize_t key_set(struct kobject* kobj,
                         struct kobj_attribute* attr,
                         const char* buf,
                         size_t count)
{		
	// Create entry for the process if not there in the list
	struct proc_keys_list_entry* iterator;

	iterator = find_process_in_list();

	if(!iterator){
		printk("%s: Device not registered. Call get_handle function to register the device first\n", __func__);
		return -EINVAL;	
	}

	iterator->keya = buf[0];
	iterator->keyb = buf[1];
	iterator->is_key_set = 1;
	return count;
}

static ssize_t  handle_status(struct kobject *kobj,
                        	struct kobj_attribute *attr,
							char *buf)
{	
	struct proc_keys_list_entry* iterator;
	struct proc_keys_list_entry* entry;
	
	iterator = find_process_in_list();

	if(iterator){
		entry = iterator;
	} else {
		mutex_lock(&keys_list_mutex);
		entry = (struct proc_keys_list_entry*) kzalloc (sizeof(struct proc_keys_list_entry), GFP_KERNEL);
		entry->pid = current->pid;
		entry->handle = handle_count++;
		entry->result = NULL;
		entry->result_len = 0;
		entry->is_config_set = 0;
		entry->is_key_set = 0;
		list_add(&entry->list, &proc_keys_list);
		mutex_unlock(&keys_list_mutex);
	}
	
	return sprintf(buf, "%d", entry->handle);
}

static ssize_t handle_set(struct kobject* kobj,
                         struct kobj_attribute* attr,
                         const char* buf,
                         size_t count)
{
	struct proc_keys_list_entry* iterator;
    struct proc_keys_list_entry* del_itr;

    mutex_lock(&keys_list_mutex);
    list_for_each_entry_safe(iterator, del_itr, &proc_keys_list, list) {
		if(iterator->pid == current->pid){
			list_del(&iterator->list);
			kfree(iterator);
			break;
		}
    }
    mutex_unlock(&keys_list_mutex);
	return count;
}

static ssize_t config_set(struct kobject* kobj,
                         struct kobj_attribute* attr,
                         const char* buf,
                         size_t count)
{	
	struct proc_keys_list_entry* iterator;

	iterator = find_process_in_list();

	if(!iterator){
		printk("%s: Device not registered. Call get_handle function to register the device first\n", __func__);
		return -EINVAL;	
	}

	iterator->is_config_set = true;

	if(buf[0] == 'd'){
		global_config.type = DMA;
		iterator->config.type = DMA;
	}
	else if(buf[0] == 'm'){
		global_config.type = MMIO;
		iterator->config.type = MMIO;
	}
	else if(buf[0] == 'i'){
		global_config.interrupt_allowed = true;
		iterator->config.interrupt_allowed = true;
	}
	else if(buf[0] == 'n'){
		global_config.interrupt_allowed = false;
		iterator->config.interrupt_allowed = false;
	}
	
	return count;
}

static ssize_t map_set(struct kobject* kobj,
                         struct kobj_attribute* attr,
                         const char* buf,
                         size_t count)
{

	struct mm_struct* mm;
	struct vm_area_struct* vma;
	unsigned long vma_addr;
	unsigned long size;
	unsigned long pfn;

	// 10 here is base of number --> decimal or hexa or octal
	if(kstrtoul(buf, 10, &vma_addr) < 0 ){
		printk("Unable to read address from buffer\n");
		return -EINVAL;
	}

	mm = get_task_mm(current);
	vma = find_vma(mm, vma_addr);

	if(vma->vm_start > vma_addr) {
		printk("No VMA present corresponding to start_addr: %lx\n", vma_addr);
		return -EINVAL;
	}

	size = vma->vm_end - vma->vm_start;

	pfn = __get_pfn(mm, (unsigned long) crypto_hw_details->hw_addr);

	if(io_remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot) < 0){
		printk("Page Mapping Failed to userspace\n");
		return -EINVAL;
	}

	return count;
}

static struct kobj_attribute handle_attribute = __ATTR(get_handle, 0660, handle_status, handle_set);
static struct kobj_attribute key_attribute = __ATTR(keys, 0220, NULL, key_set);
static struct kobj_attribute config_attribute = __ATTR(config, 0220, NULL, config_set);
static struct kobj_attribute map_attribute = __ATTR(map, 0660, NULL, map_set);

static struct attribute *chardev_driver_attrs[] = {
    &key_attribute.attr,
	&handle_attribute.attr,
	&config_attribute.attr,
	&map_attribute.attr,
    NULL
};

static struct attribute_group chardev_attr_group = {
    .attrs = chardev_driver_attrs,
    .name = "cs614_cryptocard",
};

// ############################ PCI DEVICE OPERATIONS ##################################

static irqreturn_t crypto_intr(int irq, void *data){
	int intr_raiser;

	if(data != (void*) crypto_intr){
		printk("Not our interrupt\n");
		return IRQ_NONE; // not our interrupt
	}

	// If device not live consider interrupt handled
	if(!crypto_check_device_live())
		return IRQ_HANDLED;
	
	// // Acknowledge the interrupt
	intr_raiser = readl(crypto_hw_details->hw_addr+ISR_OFFSET);
	
	if(intr_raiser != INTR_MMIO && intr_raiser != INTR_DMA){
		printk("Unknown intr raiser\n");
		return IRQ_NONE; // not our intr generator
	}

	// // Interrupt acknowledged
	writel(intr_raiser, crypto_hw_details->hw_addr + INTR_ACK_REG_OFFSET);

	if(active_proc_on_device == NULL){
		printk("No active process on device. Dunno for which process interrupt was generated.\n");
		return IRQ_HANDLED;
	}

	atomic_set(&active_proc_on_device->is_intr_handled, 1);
	return IRQ_HANDLED;
}

static int crypto_request_irq(struct pci_dev *pdev)
{
	irq_handler_t handler = crypto_intr;
	int irq_flags = IRQF_SHARED;
	int err;

	// printk("Handling irq number: %d", pdev->irq);

	err = request_irq(pdev->irq, handler, irq_flags, crypto_driver_name,
			  (void*) crypto_intr);

	if (err) {
		pr_err("Unable to allocate interrupt Error: %d\n", err);
	}

	return err;
}

static void crypto_free_irq(struct pci_dev *pdev)
{
	free_irq(pdev->irq, (void*) crypto_intr);
}

static int crypto_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    int bars, err;
	unsigned long int id;
	void* dma_addr;

	printk("Probe method Called. Matching Device found for the driver\n");
	printk("Device Name: %s\n", pci_name(pdev));

	global_config.type = MMIO;
	global_config.interrupt_allowed = false;

	crypto_hw_details = (struct crypto_hw*)kzalloc(sizeof(struct crypto_hw), GFP_KERNEL);

    bars = pci_select_bars(pdev, IORESOURCE_MEM);
	err = pci_enable_device_mem(pdev);
	
    if(err)
        return err;
    
    err = pci_request_selected_regions(pdev, bars, crypto_driver_name);
	if (err)
		goto err_pci_reg;

    crypto_hw_details->pdev = pdev;
    crypto_hw_details->bars = bars;
    crypto_hw_details->need_ioport = 0;

    crypto_hw_details->hw_addr = pci_ioremap_bar(pdev, BAR_0);

    if (!crypto_hw_details->hw_addr)
		goto err_ioremap;

	crypto_hw_details->have_64_dma = false;
	crypto_hw_details->have_32_dma = false;
	if (!dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64))) {
		crypto_hw_details->have_64_dma = true;
	} else {
		err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
		if (err) {
			pr_err("No usable DMA config, aborting\n");
			goto err_dma;
		}
		crypto_hw_details->have_32_dma = true;
	}

	dma_addr = dma_alloc_coherent(&pdev->dev, DMA_BUFFER_SIZE, &crypto_hw_details->dma_handle, GFP_DMA);

	if(dma_addr == NULL){
		printk("DMA Memory allocation failed\n");
		goto err_dma;
	}

	crypto_hw_details->dma_virt_addr = (char*) dma_addr;

	// Write dma addr to the device
	writeq(crypto_hw_details->dma_handle, crypto_hw_details->hw_addr+DMA_ADDR_OFFSET);

	// Regsiter interrupt handler
	err = crypto_request_irq(pdev);
	if (err)
		goto err_req_irq;

	printk("MEMIO Address: BAR_0: %s", crypto_hw_details->hw_addr);
	printk("PCI_Name: %s\n", pci_name(pdev));
	id = readl(crypto_hw_details->hw_addr);
	printk("Identification code: %lx", id);
	if(!crypto_check_device_live()){
		printk("Device is not live\n");
		return -EINVAL;
	}

	return 0;
	
err_req_irq:
	dma_free_coherent(&pdev->dev, DMA_BUFFER_SIZE, (void*)crypto_hw_details->dma_virt_addr, crypto_hw_details->dma_handle);
err_dma:
	iounmap(crypto_hw_details->hw_addr);
err_ioremap:
	pci_release_selected_regions(pdev, bars);
err_pci_reg:
	return err;    
}

static void crypto_remove(struct pci_dev *pdev){
	crypto_free_irq(pdev);
	dma_free_coherent(&pdev->dev, DMA_BUFFER_SIZE, (void*)crypto_hw_details->dma_virt_addr, crypto_hw_details->dma_handle);
	iounmap(crypto_hw_details->hw_addr);
	pci_release_selected_regions(pdev, crypto_hw_details->bars);
	pci_disable_device(pdev);
	kfree(crypto_hw_details);	
}

// ############################ Chardev Operations ##################################

static int chardev_open(struct inode *inode, struct file* file)
{
    atomic_inc(&device_opened);
    try_module_get(THIS_MODULE);
    return 0;
}

static int chardev_release(struct inode* inode, struct file* file)
{   
    atomic_dec(&device_opened);
    module_put(THIS_MODULE);
    return 0;
}


// Make sure that first write is called and then read is called
// Otherwise there will be locking issues, cant unlock encrypt_decrypt_mutex if not hold in write.
// It is the duty of library to ensure this

static ssize_t chardev_read(struct file* filp,
                            char* ubuf,
                            size_t length,
                            loff_t *offset)
{
	struct proc_keys_list_entry* iterator;
	int count = 0;

	iterator = find_process_in_list();

	if(!iterator){
		printk("%s: Device not registered. Call get_handle function to register the device first\n", __func__);
		mutex_unlock(&encrypt_decrypt_mutex);
		return -EINVAL;	
	}

	if(ubuf){
		count = iterator->result_len;
		if(copy_to_user(ubuf, iterator->result, count)){
			printk("[encrypt-decrypt] copy to user failed\n");
			mutex_unlock(&encrypt_decrypt_mutex);
			return -EINVAL;
		}
		// kfree(iterator->result);
		// iterator->result = NULL;
		// iterator->result_len = 0;
	}

	return count;
}

static ssize_t chardev_write(struct file* filp,
                            const char* ubuf,
                            size_t length,
                            loff_t *offset)
{
	struct proc_keys_list_entry* iterator;
	
	mutex_lock(&encrypt_decrypt_mutex);

	count ++;
	iterator = find_process_in_list();

	if(!iterator){
		printk("%s: Device not registered. Call get_handle function to register the device first\n", __func__);
		mutex_unlock(&encrypt_decrypt_mutex);
		return -EINVAL;	
	}

	active_proc_on_device = iterator;

	if(iterator->is_key_set){ 
		set_device_keys(iterator->keya, iterator->keyb);
	} // else program will use the most recent configuration

	if(!(iterator->is_config_set)){
		iterator->config.type = global_config.type;
		iterator->config.interrupt_allowed = global_config.interrupt_allowed;
	}

	if(iterator->config.type == MMIO)
		perform_mmio_encrypt_decrypt(filp, ubuf, length, offset, active_proc_on_device);
	else{
		if(ubuf[1] == 'm'){
			printk("Direct Mapping is used only for MMIO\n");
			mutex_unlock(&encrypt_decrypt_mutex);
			return -EINVAL;
		}

		perform_dma_encrypt_decrypt(filp, ubuf, length, offset, active_proc_on_device);
	}

	return length;
}

static struct file_operations fops = {
	.read = chardev_read,
	.write = chardev_write,
	.open = chardev_open,
	.release = chardev_release,
};

static char* chardev_devnode(struct device* dev, umode_t *mode)
{
    if(mode && dev->devt == MKDEV(major,0))
        *mode = 0666;
    return NULL;
} 

// ############## Module Initialisation ##############
static int __init crypto_init_module(void)
{
	int ret;
	int err;

	ret = sysfs_create_group(kernel_kobj, &chardev_attr_group);
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

	ret = pci_register_driver(&crypto_driver);
	if(ret) {
		printk("PCI Register device Failed\n");
		goto error_pci;
	}
	pr_info("Device Driver Initialised...Done!!!\n");
	return 0;

error_pci:
	device_destroy(chardev_class, MKDEV(major, 0));
error_device:
    class_destroy(chardev_class);
error_class:
    unregister_chrdev(major, DEVNAME);
error_regdev:
    sysfs_remove_group(kernel_kobj, &chardev_attr_group);
    return err;
}

module_init(crypto_init_module);

static void __exit crypto_exit_module(void)
{   
	pci_unregister_driver(&crypto_driver);
	device_destroy(chardev_class, MKDEV(major, 0));
    class_destroy(chardev_class);
    unregister_chrdev(major, DEVNAME);
	sysfs_remove_group(kernel_kobj, &chardev_attr_group);
	
	printk(KERN_INFO "Goodbye kernel\n");
}
module_exit(crypto_exit_module);

MODULE_AUTHOR("cs614");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("assignment3");