/**
 * CS614: Linux Kernel Programming
 * TC:Allocate 4 pages, last page physical allocation
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
#define SIZE 16384
#define NUM_PAGES 4

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

int main()
{
   char *ptr;
   struct input ip;
   struct address mapping[NUM_PAGES];
   unsigned long new_addr;

   int fd = open("/dev/cs614",O_RDWR);
   if(fd < 0){
       perror("open");
       exit(-1);
   }

   ptr = (char*)mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
   
   if(ptr == MAP_FAILED){
        perror("mmap");
        exit(-1);
   }
   ptr[13312] = 'A';

   ip.addr = (unsigned long)ptr+8192;
   ip.length = 2;
   ip.buff = mapping;
   
   printf("going to compact VMA ptr [%lx], length:%u, mapping:%lx\n",ip.addr,ip.length,(unsigned long)ip.buff);

   if(ioctl(fd, IOCTL_COMPACT_VMA, &ip) < 0){
       printf("Testcase failed\n");
       exit(-1);
   }
   struct address * temp;
   unsigned page_index = (&ptr[13312]-ptr)>>12;
   unsigned page_offset = ((unsigned long)&ptr[13312])&0xfff;
   printf("index into mapping:%u,offset:%u\n",page_index,page_offset);
   temp = (struct address*)ip.buff+page_index;
   new_addr = (temp->new)<<12+page_offset;
   printf("old address:%lx, new address:%lx\n",(unsigned long)&ptr[13312],new_addr);
   if(temp->new != ((unsigned long)ptr+8192)>>12){
       printf("Testcase Failed\n");
       return -1;
   }
   close(fd);
   return 0;
}
