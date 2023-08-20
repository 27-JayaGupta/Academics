/**
 * CS614: Linux Kernel Programming
 * TC: Destination hole is not big enough
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

#define IOCTL_MVE_VMA_TO _IOR(MAJOR_NUM, 0, char*)
#define IOCTL_MVE_VMA _IOWR(MAJOR_NUM, 1, char*)
#define IOCTL_PROMOTE_VMA _IOR(MAJOR_NUM, 2, char*)
#define IOCTL_COMPACT_VMA _IOWR(MAJOR_NUM, 3, char*)

#define MAP_SIZE 1073741824

struct address{
    unsigned long vma_addr;
    unsigned long to_addr;
};
struct address addr_details;


int main()
{
   char *ptr, *ptr1;

   int fd = open("/dev/cs614",O_RDWR);
   if(fd < 0){
       perror("open");
       return -1;
   }
   ptr = (char*)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0); 
   ptr1 = (char*)mmap(NULL, 1048576, PROT_READ,MAP_ANONYMOUS|MAP_PRIVATE,-1, 0); 
   if(ptr == MAP_FAILED || ptr1 == MAP_FAILED){
        perror("mmap");
        return -1;
   }
   addr_details.vma_addr = (unsigned long)ptr;
   addr_details.to_addr = ((unsigned long)ptr1+4096)&~(0xfff); 
   for(int i=0; i<MAP_SIZE; i+=4096){
       ptr[i] = 'A';
   }
   munmap((void *)ptr1, 1048576); //making a hole at dest address
   printf("going to move ptr [%lx] to [%lx]\n",addr_details.vma_addr,addr_details.to_addr);
   if(ioctl(fd, IOCTL_MVE_VMA_TO, &addr_details) < 0){
       printf("Testcase Passed\n");
       return 0;
   }
   close(fd);
   printf("Testcase Failed\n");
   return -1;
}
