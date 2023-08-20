/**
 * CS614: Linux Kernel Programming
 * TC: Move multiple VMA to multiple destination addresses
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
   char *ptr[4], *ptr1[4];
   unsigned long new_addr[4];
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
   for(int i=0; i<4; i++){
       ptr[i] = (char*)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0); 
       ptr1[i] = (char*)mmap((void*)((unsigned long)ptr+MAP_SIZE), 1048576, PROT_READ,MAP_ANONYMOUS|MAP_PRIVATE,-1, 0); 
       if(ptr[i] == MAP_FAILED || ptr1[i] == MAP_FAILED){
           perror("mmap");
           return -1;
       }
       addr_details.vma_addr = (unsigned long)ptr[i];
       for(int j=0; j<MAP_SIZE; j+=4096){
           ptr[i][j] = 'A';
       }
       munmap((void *)ptr1[i], 1048576); //making a hole
       printf("going to move ptr [%lx]\n",addr_details.vma_addr);
       if(ioctl(fd, IOCTL_MVE_VMA, &addr_details) < 0){
          printf("Testcase Failed\n");
          return -1;
       }
       unsigned long new_addr = addr_details.to_addr;
       printf("new address:%lx\n",new_addr);
       char * temp = (char*)new_addr;
       for(int j=0; j<MAP_SIZE; j+=4096){
           printf("%c\n",temp[j]);
       }
   }
   close(fd);
   return 0;
}
