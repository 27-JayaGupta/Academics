/**
 * CS614: Linux Kernel Programming
 * TC:Allocate big enough VMA, Access 2MB region, compaction should promote
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/fcntl.h>
#include<signal.h>
#include<sys/ioctl.h>
#include<sys/mman.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#define MAJOR_NUM 100
#define SIZE (1<<24)
#define NUM_PAGES 4096
#define TWOMB 2097152

#define IOCTL_MVE_VMA_TO _IOR(MAJOR_NUM, 0, char*)
#define IOCTL_MVE_VMA _IOWR(MAJOR_NUM, 1, char*)
#define IOCTL_PROMOTE_VMA _IOR(MAJOR_NUM, 2, char*)
#define IOCTL_COMPACT_VMA _IOWR(MAJOR_NUM, 3, char*)

struct address{
    unsigned long old;
    unsigned long new;
};

struct input{
    unsigned long addr;
    unsigned length;
    struct address* buff;
};
struct data{
    unsigned long pmd;
    unsigned long pte;
};

int main()
{
   char *ptr;
   struct input ip;
   struct address mapping[NUM_PAGES];
   unsigned long new_addr;

   int fd = open("/dev/cs614",O_RDWR);
   int fd_ptw = open("/dev/ptw",O_RDWR);
   if(fd < 0 || fd_ptw < 0){
       perror("open");
       exit(-1);
   }

   ptr = (char*)mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
   unsigned long page_align = ((unsigned long)ptr+4096)&~(0xfff);
   munmap(ptr,SIZE);
   ptr = (char*)mmap((void*)page_align, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
   unsigned num_2MB = ((SIZE>>21)<<21)==SIZE?SIZE>>21:(SIZE>>21)+1;
   struct data *dt = (struct data*)mmap(NULL, num_2MB*sizeof(struct data), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE, 0, 0);
   if(ptr == MAP_FAILED || dt == MAP_FAILED){
        perror("mmap");
        exit(-1);
   }

   for(int i=0; i<SIZE; i+=8192){
       ptr[i] = 'A'+i%10;
   }

   ip.addr = (unsigned long)ptr;
   ip.length = (SIZE>>12);
   ip.buff = mapping;

   printf("before compaction\n");
   struct data * temp_dt = dt;
   for(int j=0; j<SIZE; j+=TWOMB){
       unsigned long addr = (unsigned long)ptr+j;
       write(fd_ptw,&addr,sizeof(unsigned long));
       read(fd_ptw, temp_dt,sizeof(struct data));
       printf("pmd:%lx, pte:%lx\n",temp_dt->pmd,temp_dt->pte);
       temp_dt += 1;
   }

   printf("going to compact VMA ptr [%lx], length:%u, mapping:%lx\n",ip.addr,ip.length,(unsigned long)ip.buff);

   if(ioctl(fd, IOCTL_COMPACT_VMA, &ip) < 0){
       printf("Testcase failed\n");
       exit(-1);
   }
   
   printf("after compaction\n");
   temp_dt = dt;
   for(int j=0; j<SIZE; j+=TWOMB){
       unsigned long addr = (unsigned long)ptr+j;
       write(fd_ptw,&addr,sizeof(unsigned long));
       read(fd_ptw, temp_dt,sizeof(struct data));
       printf("pmd:%lx, pte:%lx\n",temp_dt->pmd,temp_dt->pte);
       temp_dt += 1;
   }

   close(fd);
   return 0;
}
