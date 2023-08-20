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
#include <linux/hugetlb.h>
#include <asm/pgtable_types.h>
#include <linux/mman.h>
#include<linux/kthread.h>
#include <asm/io.h>
#include <linux/gfp.h>
#include<linux/huge_mm.h>
#include <asm/paravirt.h>
#include <linux/kprobes.h>
#include <asm/pgalloc.h>

#define VA_SPACE_RANGE (1UL<<47)
#define TWO_MB (1UL<<21)
#define DEFAULT_PAGE_SIZE (1UL<<12)
#include "btplus.h"

extern int vm_munmap(unsigned long addr, size_t len);
extern unsigned long vm_mmap(struct file *file, unsigned long addr,
        unsigned long len, unsigned long prot,
        unsigned long flag, unsigned long offset);
// extern int split_vma(struct mm_struct *mm, struct vm_area_struct *vma,
//               unsigned long addr, int new_below);


extern void vunmap(const void *addr);
extern void *vmap(struct page **pages, unsigned int count,
	   unsigned long flags, pgprot_t prot);

extern void (*page_fault_hook)(struct pt_regs *regs, unsigned long error_code, unsigned long address);

static int major;
atomic_t  device_opened;
static struct class *demo_class;
struct device *demo_device;
static struct task_struct *kth_task = NULL;
int probe_registered = 0;
int list_init = 0;
pid_t pid;

struct kobject *cs614_kobject;
unsigned promote = 0;
unsigned kth_enabled = 0;
unsigned long ___start_addr; 
unsigned long ___len;

static ssize_t  sysfs_show(struct kobject *kobj,
                        struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store(struct kobject *kobj,
                        struct kobj_attribute *attr,const char *buf, size_t count);

struct kobj_attribute sysfs_attr; 

struct address{
    unsigned long from_addr;
    unsigned long to_addr;
};

struct input{
    unsigned long addr;
    unsigned length;
    struct address * buff;
};

struct old_pmd{
	unsigned long start_addr;
	pmd_t pmd;
	struct list_head list;
};

struct list_head old_pmd_list;

long __align_2mb(unsigned long start){
	unsigned long rem;
	unsigned long div = TWO_MB;
	unsigned long aligned_addr;

	rem = start%div;
	if(rem!=0)
		aligned_addr = start + div - rem;
	else 
		aligned_addr = start;

	return aligned_addr;
}

pgd_t* __get_pgd(struct mm_struct* mm, unsigned long address) {
	pgd_t *pgd;

	pgd = pgd_offset(mm, address);

	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		return NULL;
	
	return pgd;
}

p4d_t* __get_p4d(struct mm_struct* mm, unsigned long address) {
	pgd_t *pgd;
	p4d_t *p4d;

	pgd = __get_pgd(mm, address);
	if(pgd == NULL)
		goto nul_ret;

	p4d = p4d_offset(pgd, address);
	if (p4d_none(*p4d))
		goto nul_ret;

	if (unlikely(p4d_bad(*p4d)))
			goto nul_ret;
	
	return p4d;

nul_ret:
	return NULL;
}

pud_t* __get_pud(struct mm_struct* mm, unsigned long address) {
	p4d_t *p4d;
	pud_t *pud;

	p4d = __get_p4d(mm, address);
	if(p4d == NULL)
		goto nul_ret;
	
	pud = pud_offset(p4d, address);
	if (pud_none(*pud))
			goto nul_ret;
	if (unlikely(pud_bad(*pud)))
			goto nul_ret;
		
	return pud;

nul_ret:
	return NULL;
}

pmd_t* __get_pmd_with_huge(struct mm_struct* mm, unsigned long address) {
	pud_t *pud;
	pmd_t *pmd;

	pud = __get_pud(mm, address);
	if(pud == NULL)
		goto nul_ret;
	
	pmd = pmd_offset(pud, address);
	if (pmd_none(*pmd))
			goto nul_ret;

	return pmd;

nul_ret:
	return NULL;
}

pmd_t* __get_pmd_wo_huge(struct mm_struct* mm, unsigned long address) {
	pud_t *pud;
	pmd_t *pmd;

	pud = __get_pud(mm, address);
	if(pud == NULL)
		goto nul_ret;
	
	pmd = pmd_offset(pud, address);
	if (pmd_none(*pmd))
			goto nul_ret;
	if (unlikely(pmd_trans_huge(*pmd))){
			printk(KERN_INFO "I am huge\n");
			goto nul_ret;
	}

	return pmd;

nul_ret:
	return NULL;
}

int __check_if_huge_page(struct mm_struct* mm, unsigned long address) {
	pmd_t *pmd;

	pmd = __get_pmd_with_huge(mm, address);

	if(pmd == NULL)
		goto nul_ret;

	if((pmd->pmd & _PAGE_PSE) == _PAGE_PSE) 
		return 1;
	else 
		return 0;
	
nul_ret:
		return -1;
}

static int __kprobes __demote_pages(struct kprobe *p, struct pt_regs *regs)
{	
  	if(current->pid == pid) {

		struct mm_struct* mm;
		struct vm_area_struct* vma;
		long vma_addr;
		char* buff;
		unsigned long vm_start;
		pmd_t* pmd;
		struct page* pmd_page;
		int is_pmd_huge;

		struct old_pmd* iterator;
		struct old_pmd* old_val;

		unsigned long start_addr = (unsigned long) regs->di;
		size_t len = (size_t) regs->si;
		unsigned long end_addr = start_addr + len;

		printk(KERN_INFO "[%d]Vm Unmap is called with start addr:%lx and size %ld\n", current->pid, start_addr, len);

		buff = (char*)kzalloc(TWO_MB*sizeof(char), GFP_KERNEL);
		mm = get_task_mm(current);
		vma = find_vma(mm, start_addr);

		if(vma->vm_start > start_addr) {
			printk("No VMA present corresponding to start_addr: %lx\n", start_addr);
			return -EINVAL;
		}

		if(end_addr > vma->vm_end){
			printk("Address range surpassign multiple vma's not possible for one unmap call. start_addr: %lx, end_addr: %lx, vm_start: %lx, vm_end:%lx\n", start_addr, end_addr, vma->vm_start, vma->vm_end);
			return -EINVAL;
		}

		vm_start = start_addr;

		while(true){

			if(vm_start >= vma->vm_end)
				break;
			
			if(vm_start >= end_addr) 
				break;

			is_pmd_huge = __check_if_huge_page(mm, vm_start);

			if(is_pmd_huge < 0) {
				printk("PMD is neither huge nor normal. Either pmd or above level entries are null\n");
				return -EINVAL;
			}

			if(is_pmd_huge) {
				printk("In PMD huge page handling, Address: %lx\n", vm_start);
				vma_addr = __align_2mb(vm_start);
				if(vma_addr!=vm_start) {
					vm_start = vma_addr - TWO_MB;
				}
				printk("In PMD huge page handling, Address: %lx\n", vm_start);

				pmd = __get_pmd_with_huge(mm, vm_start);
				if(pmd == NULL) {
					printk("PMD or above level is null\n");
					return -EINVAL;
				}

				pmd_page = pmd_page(*pmd);
				__free_pages(pmd_page, 9);

				list_for_each_entry_safe(iterator, old_val, &old_pmd_list, list){
					if(iterator->start_addr == vm_start){
						// printk("got list entry\n");
						set_pmd(pmd, iterator->pmd);
						list_del(&iterator->list);
						kfree(iterator);
						break;
					}
				}

				vm_start += TWO_MB;
			} else {
				vma_addr = __align_2mb(vm_start);
				if(vma_addr==vm_start) {
					vm_start += TWO_MB;
				} else {
					vm_start = vma_addr;
				}
			}	
		}
		
		return 0;
		kfree(buff);
    }

	return 0;
}

static struct kprobe kp = {
    .symbol_name   = "__vm_munmap",
	.pre_handler = __demote_pages,
	.post_handler = NULL,
};


void my_pf_handler(struct pt_regs *regs, unsigned long error_code, unsigned long address)
{
  struct task_struct *tsk = current;
  if((address >= ___start_addr) && (address< (___start_addr + ___len*DEFAULT_PAGE_SIZE+ DEFAULT_PAGE_SIZE))) 
  	printk(KERN_INFO "Page fault pid = %d address #%lx RIP#%lx error code = %lx\n", tsk->pid, address, regs->ip, error_code);
  return;
}

static int device_open(struct inode *inode, struct file *file)
{
        atomic_inc(&device_opened);
		// page_fault_hook = &my_pf_handler;
        try_module_get(THIS_MODULE);
        printk(KERN_INFO "Device opened successfully\n");
        return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
		// page_fault_hook = NULL;
        module_put(THIS_MODULE);
		if(kth_enabled && kth_task){
			printk("Stopping  Kernel Thread\n");
			BUG_ON(!kth_task);
			kthread_stop(kth_task);
			kth_task = NULL;
			kth_enabled = 0;
		}

        printk(KERN_INFO "Device closed successfully\n");

        return 0;
}


static ssize_t device_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t * offset){
    printk("read called\n");
    return 0;
}

static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off){
    
    printk("write called\n");
    return 8;
}

int __move_vma_to(unsigned long vma_addr, unsigned long to_addr){

		int ret = 0;
		struct mm_struct* mm;
		struct vm_area_struct* from_vma, *to_vma;
		unsigned long from_len, new_addr;
		char* buff;
		// unsigned long prot;
		// unsigned long flags;

		mm = get_task_mm(current);
		from_vma = vma_lookup(mm, vma_addr);

		
		if(!from_vma){
			pr_info("No VMA corresponding to address: %lx\n", vma_addr);
			ret = -1;
			goto out;
		}
		from_len = from_vma->vm_end - from_vma->vm_start;

		printk("Vma Start Addr: %lx , from addr: %lx\n", from_vma->vm_start, vma_addr);

		if(to_addr + from_len >= VA_SPACE_RANGE){
			pr_err("vm area out of user virtual space bounds\n");
			ret = -1;
			goto out;
		}
		to_vma = find_vma(mm, to_addr);
		if(to_vma){

			if(to_addr >= to_vma->vm_start){
				pr_info("VMA corresponding to destination address: %lx already exists\n", to_addr);
				ret = -1;
				goto out;
			}
			
			if(to_vma->vm_start <= to_addr+from_len){
				pr_info("Not Enough Hole Present in VM Area to allocate space for existing VM Area\n");
				ret = -1;
				goto out;
			}
		}

		// if(vma->vm_page_prot&)
		new_addr = vm_mmap(NULL, to_addr, from_len, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0);

		if(new_addr != to_addr){
			pr_err("mmap failed to destination address\n");
			ret = -1;
			goto out;
		}

		buff = (char*)kzalloc(from_len, GFP_KERNEL);
	    
	    if(copy_from_user(buff,(char*)from_vma->vm_start, from_len)){
	        pr_err("CopyFromUser Error\n");
			ret = -1;
			goto out;
	    }
	    
		if(copy_to_user((char*)new_addr, buff, from_len)){
			pr_err("CopyToUser Error\n");
			ret = -1;
			goto out;
		}

	    kfree(buff);
		
		vm_munmap(from_vma->vm_start, from_len);
		pr_info("VMA State All set\n");

out:
	return ret;
}

int __move_vma(unsigned long vma_addr, unsigned long* new_dest_addr){

		int ret = 0;
		struct mm_struct* mm;
		struct vm_area_struct* from_vma;
		struct vm_area_struct* vma, *old_vma;
		struct vma_iterator vmi;
		unsigned long from_len, to_addr, new_addr;
		char* buff;

		mm = get_task_mm(current);
		from_vma = vma_lookup(mm, vma_addr);
		
		if(!from_vma){
			pr_info("No VMA corresponding to address: %lx\n", vma_addr);
			ret = -1;
			goto out;
		}

		from_len = from_vma->vm_end - from_vma->vm_start;

		vma_iter_init(&vmi, mm, from_vma->vm_end);
		old_vma = from_vma;

		for_each_vma(vmi, vma){
			// We have found a hole
			if(old_vma->vm_end + from_len < vma->vm_start)
				break;
			
			old_vma = vma;
		}

		// Means we are out of virtual address space range
		if(old_vma->vm_end + from_len >= VA_SPACE_RANGE){
			pr_info("Virtual Address Space Full. No new hole found\n");
			ret = -1;
			goto out;
		}

		to_addr = old_vma->vm_end;
		new_addr = vm_mmap(NULL, to_addr, from_len, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0);

		if(new_addr != to_addr){
			pr_err("mmap failed to destination address\n");
			ret = -1;
			goto out;
		}

		buff = (char*)kzalloc(from_len, GFP_KERNEL);
	    
	    if(copy_from_user(buff,(char*)from_vma->vm_start, from_len)){
	        pr_err("CopyFromUser Error\n");
			ret = -1;
			goto out;
	    }
	    
		if(copy_to_user((char*)new_addr, buff, from_len)){
			pr_err("CopyToUser Error\n");
			ret = -1;
			goto out;
		}

		*new_dest_addr = new_addr;

	    kfree(buff);
		
		vm_munmap(from_vma->vm_start, from_len);

out:
	return ret;

}

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

void __clear_ptes(struct mm_struct* mm, unsigned long addr) {

	pte_t* pte;
	unsigned long count = 0;
	unsigned long nr_pages = (1UL<<9);

	while(count++ < nr_pages){
		pte = __check_if_physically_mapped(mm, addr);
		if(pte == NULL) {
			printk("PTE corresponding to address %lx is not present\n", addr);
			addr += DEFAULT_PAGE_SIZE;
			continue;
		}
		
		pte_clear(mm, addr, pte);
		addr += DEFAULT_PAGE_SIZE;
	}
}

bool __check_fully_allocated(struct mm_struct* mm, unsigned long addr) {
	pte_t* pte;
	unsigned long count = 0;
	unsigned long nr_pages = (1UL<<9);

	while(count++ < nr_pages){
		pte = __check_if_physically_mapped(mm, addr);
		if(pte == NULL) {
			printk("PTE corresponding to address %lx is not present\n", addr);
			return false;
		}
		addr += DEFAULT_PAGE_SIZE;
	}

	return true;
}

static int __promote_compacted_vma(struct mm_struct* mm, struct vm_area_struct* vma, char* buff, unsigned long start_addr, unsigned long end_addr) {
	int count;
	unsigned long vm_start;
	struct page* hpage;
	unsigned long maddr;
	pmd_t* pmd;
	pmd_t new_pmd;
	unsigned long pfn_num;
	struct old_pmd* old_pmd_entry;

	vm_start = start_addr;
	while(true){

		vm_start = __align_2mb(vm_start);

		if(vm_start >= vma->vm_end)
			break;
		
		if(vm_start > end_addr) 
			break;

		printk("VMA Start: %lx, VMA End: %lx, Aligned Addr: %lx\n", vma->vm_start, vma->vm_end, vm_start);

		if(vm_start < 0){
			printk("[NOT POSSIBLE] VMA addr: %ld\n", vm_start); 
			return -EINVAL;
		}
	
		if(vm_start + TWO_MB > end_addr)
			break;
	
		if( !__check_fully_allocated(mm, vm_start)) {
			printk("2MB aligned area not fully allocated \n");
			vm_start = vm_start + TWO_MB;
			continue;
		}

		count = access_process_vm(current, vm_start, (void*) buff, TWO_MB, FOLL_REMOTE);
		printk("[0]%c [1]%c\n" , buff[0], buff[1]);
		printk("[4096]%c [4097]%c\n", buff[4096], buff[4097]);

		pmd = __get_pmd_wo_huge(mm, vm_start);

		if(pmd == NULL) {
			printk("PMD entry null either missing pmd entry or already a hugepage\n");
			vm_start = vm_start + TWO_MB;
			continue;
		}

		// CLEAR PTE (2MB region)
		// __clear_ptes(mm, vma_addr);

		old_pmd_entry = (struct old_pmd*) kzalloc (sizeof(struct old_pmd), GFP_KERNEL);
		old_pmd_entry->start_addr = vm_start;
		old_pmd_entry->pmd = *pmd;

		list_add(&old_pmd_entry->list, &old_pmd_list);

		hpage = alloc_pages(GFP_KERNEL, 9);

		if(hpage == NULL) {
			pr_err("Huge Page Physical Allocation failed\n");
			return -EINVAL;
		}
		maddr = (unsigned long)page_address(hpage);
		pfn_num = __page_to_pfn(hpage);

		printk("Hpage Addr: %lx, PFN Num: %lx\n", maddr, pfn_num);
		memcpy((void*)maddr, buff, TWO_MB);

		new_pmd = mk_pmd(hpage, vma->vm_page_prot);
		if(vma->vm_flags & VM_WRITE)
				new_pmd = pmd_mkwrite(pmd_mkdirty(new_pmd));
		set_pmd_at(mm, vm_start, pmd, new_pmd);
		pmd->pmd |= _PAGE_PSE;

		printk("%lx to %lx has been promoted to HuegPage\n", vm_start, vm_start + TWO_MB);
		vm_start = vm_start + TWO_MB;
	}

	return 0;
}


static int __promote_page_in_one_vma(struct mm_struct* mm, struct vm_area_struct* vma, char* buff, struct task_struct* proc){

	long vma_addr;
	int count;
	unsigned long vm_start;
	struct page* hpage;
	unsigned long maddr;
	pmd_t* pmd;
	pmd_t new_pmd;
	unsigned long pfn_num;
	struct old_pmd* old_pmd_entry;

	vm_start = vma->vm_start;

	while(true){

		if(vm_start >= vma->vm_end)
			break;

		vma_addr = __align_2mb(vm_start);
		printk("VMA Start: %lx, VMA End: %lx, Aligned Addr: %lx\n", vma->vm_start, vma->vm_end, vma_addr);

		if(vma_addr < 0){
			printk("[NOT POSSIBLE] VMA addr: %ld\n", vma_addr); 
			return -EINVAL;
		}
	
		if(vma_addr + TWO_MB > vma->vm_end)
			break;
	
		if( !__check_fully_allocated(mm, vma_addr)) {
			printk("2MB aligned area not fully allocated \n");
			vm_start = vma_addr + TWO_MB;
			continue;
		}

		count = access_process_vm(proc, vma_addr, (void*) buff, TWO_MB, FOLL_REMOTE);
		printk("[0]%c [1]%c\n" , buff[0], buff[1]);
		printk("[4096]%c [4097]%c\n", buff[4096], buff[4097]);

		pmd = __get_pmd_wo_huge(mm, vma_addr);

		if(pmd == NULL) {
			printk("PMD entry null either missing pmd entry or already a hugepage\n");
			vm_start = vma_addr + TWO_MB;
			continue;
		}

		// CLEAR PTE (2MB region)
		// __clear_ptes(mm, vma_addr);

		old_pmd_entry = (struct old_pmd*) kzalloc (sizeof(struct old_pmd), GFP_KERNEL);
		old_pmd_entry->start_addr = vma_addr;
		old_pmd_entry->pmd = *pmd;

		list_add(&old_pmd_entry->list, &old_pmd_list);

		hpage = alloc_pages(GFP_KERNEL, 9);

		printk(KERN_INFO "1\n");

		if(hpage == NULL) {
			pr_err("Huge Page Physical Allocation failed\n");
			return -EINVAL;
		}
		printk(KERN_INFO "1\n");
		maddr = (unsigned long)page_address(hpage);
		pfn_num = __page_to_pfn(hpage);

		printk("Hpage Addr: %lx, PFN Num: %lx\n", maddr, pfn_num);
		memcpy((void*)maddr, buff, TWO_MB);
		printk(KERN_INFO "1\n");


		new_pmd = mk_pmd(hpage, vma->vm_page_prot);


		if(vma->vm_flags & VM_WRITE)
				new_pmd = pmd_mkwrite(pmd_mkdirty(new_pmd));
		set_pmd_at(mm, vma_addr, pmd, new_pmd);
		pmd->pmd |= _PAGE_PSE;

		printk("%lx to %lx has been promoted to HuegPage\n", vma_addr, vma_addr + TWO_MB);
		vm_start = vma_addr + TWO_MB;
	}

	return 0;
}

static int __promote_pages(void *task_ptr){
	struct mm_struct* mm;
	struct vm_area_struct* vma;
	struct task_struct* proc;
	struct vma_iterator vmi;
	int ret = 0;
	char* buff;
	
	proc = (struct task_struct*) task_ptr;
	printk("Task Name: %s\n", proc->comm);
	printk("Starting Kthread for Hugepage Promotion/Demotion with pid = %d\n", current->pid); 
	buff = (char*)kzalloc(TWO_MB*sizeof(char), GFP_KERNEL);

	while (!kthread_should_stop()){

		if(!promote){
        	schedule_timeout_interruptible(10);
			continue;
		}
		
		/* Promote pages to hugepages(2MB)*/
		printk("Starting Huge Page Promotion\n");
		
		mm = get_task_mm(proc);
		vma = find_vma(mm, 0);
		vma_iter_init(&vmi, mm, vma->vm_start);
		printk("First Start:%lx, End:%lx\n", vma->vm_start, vma->vm_end);

		for_each_vma(vmi, vma){
			if(__promote_page_in_one_vma(mm, vma, buff, proc) < 0) {
				printk("Promotion in VMA, start:%lx, end: %lx failed\n", vma->vm_start, vma->vm_end);
				ret = -1;
				goto out;
			}
		}

		// Promotion Done
		promote = 0;
		goto out;
    }

out:
	kfree(buff);
    printk("Kthread with pid = %d finishing\n", current->pid); 
	kth_enabled = 0;
	kth_task = NULL;
	return ret;	
}

void __create_kernel_thread(void){
		
		if((!kth_enabled) && !kth_task){
			kth_task = kthread_run(__promote_pages, (void*)current, "hugepage_promote_kth");
			BUG_ON(IS_ERR(kth_task));
			kth_enabled = 1;
		}
}

int __check_page_aligned(unsigned long start_addr){

	if(start_addr % DEFAULT_PAGE_SIZE != 0)
		return -1;
	
	return 0;
}

int __swap_pages_content(unsigned long from_addr, unsigned long to_addr){
	int ret = 0;
	char* buff = NULL;

	buff = (char*) kzalloc(DEFAULT_PAGE_SIZE, GFP_KERNEL);
	if(copy_from_user(buff,(char*)from_addr, DEFAULT_PAGE_SIZE)){
		pr_err("CopyFromUser Error\n");
		ret = -1;
		goto out;
	}

	if(copy_to_user((char*)to_addr, buff, DEFAULT_PAGE_SIZE)){
		pr_err("CopyToUser Error\n");
		ret = -1;
		goto out;
	}

out:
	kfree(buff);
	return ret;
}

int __compact_vma(unsigned long start_addr, unsigned long nr_pages, struct address* buff){

		int ret = 0;
		struct mm_struct* mm;
		struct vm_area_struct* vma;
		unsigned long length = nr_pages * DEFAULT_PAGE_SIZE;
		unsigned long* vacant_page_addr = NULL;
		bool* is_vacant_page = NULL;
		pte_t **pte_buff = NULL;
		pte_t *pte;
		char* buff_promote;

		unsigned mapped_pages_count = 0, vacant_pages_count=0;
		int i, j;
		unsigned long addr;

		mm = get_task_mm(current);
		vma = vma_lookup(mm, start_addr);
		
		vacant_page_addr = (unsigned long *) kzalloc(nr_pages*sizeof(unsigned long), GFP_KERNEL);
		is_vacant_page = (bool*) kzalloc (nr_pages*sizeof(bool), GFP_KERNEL);
		pte_buff = (pte_t**) kzalloc (nr_pages * sizeof(pte_t*), GFP_KERNEL);

		if(!vma){
			pr_err("No VMA corresponding to address: %lx\n", start_addr);
			ret = -1;
			goto out;
		}

		if(__check_page_aligned(start_addr) < 0){
			pr_err("Start Address not page aligned: Start Addr: %lx\n", start_addr);
			ret = -1;
			goto out;
		}

		if(start_addr + length > vma->vm_end){
			pr_err("Address Range out of vma scope\n");
			ret = -1;
			goto out;
		}

		for(i=0; i<nr_pages; i++){
			addr = start_addr+(unsigned long)(i * DEFAULT_PAGE_SIZE);
			pte = __check_if_physically_mapped(current->mm, addr);
			pte_buff[i] = pte;

			if(pte == NULL){
				vacant_page_addr[vacant_pages_count] = addr;
				vacant_pages_count += 1;
				is_vacant_page[i] = true;
				continue;
			}
			
			mapped_pages_count += 1;
			is_vacant_page[i] = false;
		}

		printk("Mapped pages count:%d\n", mapped_pages_count);

		for(i=0, j=0; i<nr_pages; i++){
			addr = start_addr + (unsigned long)(i * DEFAULT_PAGE_SIZE);

			if(is_vacant_page[i] == false) {
				if(i<mapped_pages_count)
					continue;
				else {
					if(__swap_pages_content(addr, vacant_page_addr[j]) < 0){
						printk("Error in Swapping page contents, From Addr: %lx, To Addr: %lx\n", addr, vacant_page_addr[j]);
						ret = -1;
						goto out;
					}

					
					if(!pte_buff[i]){
						printk("Pte Buffer NULL for present page\n");
						ret = -1;
						goto out;
					}

					pte_clear(mm, addr, pte_buff[i]);

					buff[i].to_addr = vacant_page_addr[j];
					buff[(vacant_page_addr[j]-start_addr)>>12].to_addr = addr;
					j += 1;
				}
			}
		}

		buff_promote = (char*)kzalloc(TWO_MB*sizeof(char), GFP_KERNEL);
		if(__promote_compacted_vma(mm, vma, buff_promote, start_addr, start_addr+length)<0){
			printk("[COMPACT VMA] Promotion to Huge page failed\n");
			ret = -1;
			goto promote_out;
		}

promote_out:
	kfree(buff_promote);

out:	
		kfree(pte_buff);
		kfree(vacant_page_addr);
		kfree(is_vacant_page);
		return ret;

}

void __init_mapping_buffer(struct address* buff, unsigned long nr_pages, unsigned long start_addr){
	int i;
	unsigned long addr;

	for(i=0;i<nr_pages;i++){
		addr = start_addr + (unsigned long)(i * DEFAULT_PAGE_SIZE);
		buff[i].from_addr = addr;
		buff[i].to_addr = addr;
	}

}

int __register_kprobe(void) {
	// Registering probe for unmap call

	int ret;
	if(!probe_registered) {
		ret = register_kprobe(&kp);
		if (ret < 0) {
				printk(KERN_INFO "register_kprobe failed, returned %d\n", ret);
				return ret;
		}
		probe_registered = 1;
		printk(KERN_INFO "Planted kprobe at %lx\n", (unsigned long)kp.addr);
	}
	return 0;
}

long device_ioctl(struct file *file,	
		 unsigned int ioctl_num,
		 unsigned long ioctl_param)
{
	//unsigned long addr = 1234;
	int ret = 0; // on failure return -1
	struct address * buff = NULL; 
	struct address * mappings = NULL;
	unsigned long vma_addr = 0;
	unsigned long to_addr = 0;
	unsigned long length = 0;
	struct input* ip;

	pid = current->pid;

	/*
	 * Switch according to the ioctl called
	 */
	switch (ioctl_num) {
	case IOCTL_MVE_VMA_TO:

	    buff = (struct address*)vmalloc(sizeof(struct address)) ;
	    printk("move VMA at a given address");

	    if(copy_from_user(buff,(char*)ioctl_param,sizeof(struct address))){
	        pr_err("MVE_VMA address write error\n");
			ret = -1;
			return ret;
	    }

	    vma_addr = buff->from_addr;
	    to_addr = buff->to_addr;
	    printk("address from :%lx, to:%lx \n",vma_addr,to_addr);
	    vfree(buff);

		return __move_vma_to(vma_addr, to_addr);

	case IOCTL_MVE_VMA:
	    buff = (struct address*)vmalloc(sizeof(struct address)) ;
	    printk("move VMA to available hole address");
	    if(copy_from_user(buff,(char*)ioctl_param,sizeof(struct address))){
	        pr_err("MVE_VMA address write error\n");
			ret = -1;
			return -1;
	    }
	    vma_addr = buff->from_addr;
	    printk("VMA address :%lx \n",vma_addr);
		ret = __move_vma(vma_addr, &buff->to_addr);
		if(copy_to_user((char*)ioctl_param,buff,sizeof(struct address))){
	        pr_err("MVE_VMA address write error\n");
			ret = -1;
			return ret;
	    }
	    vfree(buff);

		return ret;
	case IOCTL_PROMOTE_VMA:
		printk("promote 4KB pages to 2MB\n");
		INIT_LIST_HEAD(&old_pmd_list);
		list_init = 1;
		ret = __register_kprobe();
		if(ret < 0) 
			return ret;
		__create_kernel_thread();
		return ret;

	case IOCTL_COMPACT_VMA:

	    printk("compact VMA\n");
		INIT_LIST_HEAD(&old_pmd_list);
		list_init = 1;
		ret = __register_kprobe();
		if(ret < 0) 
			return ret;

	    ip = (struct input*)vmalloc(sizeof(struct input)) ;
	    if(copy_from_user(ip,(char*)ioctl_param,sizeof(struct input))){
                pr_err("MVE_MERG_VMA address write error\n");
				ret = -1;
                return ret;
        }

	    vma_addr = ip->addr;
	    length = ip->length;
	    buff = ip->buff;
		___start_addr = ip->addr;
		___len = ip->length;
	
	    printk("vma address:%lx, length:%ld, buff:%lx\n",vma_addr,length,(unsigned long)buff);
		mappings = (struct address*) kzalloc (length * sizeof(struct address), GFP_KERNEL);
		__init_mapping_buffer(mappings, length, vma_addr);

		if(__compact_vma(vma_addr, length, mappings) < 0){
			pr_err("ERROR IN COMPACTING VMA. Exiting ....\n");
			kfree(mappings);
			vfree(ip);
			ret = -1;
			return ret;
		}

	    //populate old to new address mapping in user buffer.
	    //number of entries in this buffer is equal to the number of 
	    //virtual pages in vma address range
	    //index of moved addr in mapping table is , index = (addr-vma_address)>>12
	    
	    if(copy_to_user((struct address *)buff, (struct address *)mappings, length * sizeof(struct address))){
	        pr_err("COMPACT VMA read error\n");
			kfree(mappings);
			vfree(ip);
			ret = -1;
			return ret;
	    } 
	    vfree(ip);
		kfree(mappings);
        return ret;
	}
	return ret;
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

//Implement required logic
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf)
{	
    pr_info("sysfs read\n");
	pr_info("Promote value in sysfs_show: %d\n", promote);
	return sprintf(buf, "%d\n", promote);
    return 0;
}

//Implement required logic
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,
                     const char *buf, size_t count)
{
	int newval;
	int err;
	
	printk("sysfs write\n");
	err = kstrtoint(buf, 10, &newval);
	printk("%d, count:%ld, buf[0]:%c\n", newval, count, buf[0]);
	if (err || newval != 1)
			return -EINVAL;
	
	promote = newval;
    return count;
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
        
	cs614_kobject = kobject_create_and_add("kobject_cs614", kernel_kobj);
        
	if(!cs614_kobject)
            return -ENOMEM;
	
	sysfs_attr.attr.name = "promote";
	sysfs_attr.attr.mode = 0666;
	sysfs_attr.show = sysfs_show;
	sysfs_attr.store = sysfs_store;

	err = sysfs_create_file(cs614_kobject, &(sysfs_attr.attr));
	if (err){
	    pr_info("sysfs exists:");
	    goto r_sysfs;
	}
	return 0;
r_sysfs:
	kobject_put(cs614_kobject);
        sysfs_remove_file(kernel_kobj, &sysfs_attr.attr);
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
	kobject_put(cs614_kobject);
	sysfs_remove_file(kernel_kobj, &sysfs_attr.attr);
	if(kth_enabled && kth_task){
		printk("Stopping  Kernel Thread\n");
		BUG_ON(!kth_task);
		kthread_stop(kth_task);
		kth_task = NULL;
		kth_enabled = 0;
	}

	if(probe_registered) {
		unregister_kprobe(&kp);
		probe_registered = 0;
	}

	if(list_init) {
		struct old_pmd* iterator;
		struct old_pmd* del_itr;
		list_for_each_entry_safe(iterator, del_itr, &old_pmd_list, list) {
			list_del(&iterator->list);
			kfree(iterator);
		}
	}

	printk(KERN_INFO "Goodbye kernel\n");
}

MODULE_AUTHOR("cs614");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("assignment2");
