/**
 * CS614: Linux Kernel Programming
 * TC: Destination Address is not available
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

#define MAP_SIZE 16384

struct address{
    unsigned long vma_addr;
    unsigned long to_addr;
};
struct address addr_details;
int main()
{
   char *ptr, *ptr1;
   unsigned long new_addr = 0;

   int fd = open("/dev/cs614",O_RDWR);
   if(fd < 0){
       perror("open");
       return -1;
   }

   ptr = (char*)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0); 
   if(ptr == MAP_FAILED){
        perror("mmap");
        return -1;
   }
   addr_details.vma_addr = (unsigned long)ptr;

   for(int i=0; i<MAP_SIZE; i+=4096){
       ptr[i] = 'A';
   }
   printf("going to move ptr [%lx]\n",addr_details.vma_addr);
   if(ioctl(fd, IOCTL_MVE_VMA, &addr_details) < 0){
       printf("Testcase Failed\n");
       exit(-1);
   }

   new_addr = addr_details.to_addr;
   char * temp = (char*)new_addr;
   for(int i=0; i<MAP_SIZE; i+=4096){
       printf("%c\n",temp[i]);
   } 
   close(fd);
   return 0;
}
