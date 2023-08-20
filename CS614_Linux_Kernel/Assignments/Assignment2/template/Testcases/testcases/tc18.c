/**
 * CS614: Linux Kernel Programming
 * TC: Move VMA to dest and move it back to src location
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

static void handler(int sig, siginfo_t *si, void *unused){

    printf("Testcase Failed\n");
    exit(-1);
}

int main()
{
   char *ptr, *ptr1, *ptr2;

   int fd = open("/dev/cs614",O_RDWR);
   if(fd < 0){
       perror("open");
       return -1;
   }
   struct sigaction sa;
   sa.sa_flags = SA_SIGINFO;
   sigemptyset(&sa.sa_mask);
   sa.sa_sigaction = handler;
   if (sigaction(SIGSEGV, &sa, NULL) == -1){
        perror("sigaction");
	return -1;
   }
   ptr2 = (char*)mmap(NULL, 1073741824, PROT_READ,MAP_ANONYMOUS|MAP_PRIVATE,-1, 0);
   unsigned long addr_src = ((unsigned long)ptr2+4096)&~(0xfff);
   munmap((void *)ptr2, 1073741824); 
   ptr = (char*)mmap((void*)addr_src, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0); 
   ptr1 = (char*)mmap(ptr, MAP_SIZE, PROT_READ,MAP_ANONYMOUS|MAP_PRIVATE,-1, 0); 
   if(ptr == MAP_FAILED || ptr1 == MAP_FAILED || ptr2 == MAP_FAILED){
        perror("mmap");
        return -1;
   }
   addr_details.vma_addr = (unsigned long)ptr; 
   for(int i=0; i<MAP_SIZE; i+=4096){
       ptr[i] = 'A';
   }
   munmap((void *)ptr1, MAP_SIZE); //making a hole
   printf("going to move ptr [%lx]\n",addr_details.vma_addr);
   if(ioctl(fd, IOCTL_MVE_VMA, &addr_details) < 0){
       printf("Testcase Failed\n");
       return -1;
   }
   unsigned long new_addr = addr_details.to_addr;
   printf("new address:%lx\n",new_addr);

   if(munmap((void *)ptr, MAP_SIZE) < 0){
       perror("munmap");
       return -1;
   }
   ptr2 = (char*)mmap(ptr1, MAP_SIZE, PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1, 0); 
   munmap((void *)ptr2, MAP_SIZE); //making a hole

   addr_details.vma_addr = ((unsigned long)new_addr+4096)&~(0xfff);
   printf("going to move ptr [%lx]\n",addr_details.vma_addr);
   if(ioctl(fd, IOCTL_MVE_VMA, &addr_details) < 0){
       printf("Testcase Failed\n");
       return -1;
   }
   new_addr = addr_details.to_addr;
   printf("new address:%lx\n",new_addr);

   //After successful VMA move below access should succeed
   char * temp = (char*)new_addr;

   printf("%c\n",temp[0]);
   close(fd);
   return 0;
}
