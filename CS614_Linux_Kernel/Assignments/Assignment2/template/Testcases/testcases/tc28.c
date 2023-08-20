/**
 * CS614: Linux Kernel Programming
 * TC:Multiple promote calls
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

#define SIZE 2147483648 //2GB
#define TWOMB 2097152

#define MAJOR_NUM 100

#define IOCTL_MVE_VMA_TO _IOR(MAJOR_NUM, 0, char*)
#define IOCTL_MVE_VMA _IOWR(MAJOR_NUM, 1, char*)
#define IOCTL_PROMOTE_VMA _IOR(MAJOR_NUM, 2, char*)
#define IOCTL_COMPACT_VMA _IOWR(MAJOR_NUM, 3, char*)

static void handler(int sig, siginfo_t *si, void *unused)
{
    printf("Testcase Failed\n");
    exit(-1);
}
struct data{
    unsigned long pmd;
    unsigned long pte;
};

int main()
{
   char *ptr;
   char buf[16];
   pid_t pid = getpid();
   unsigned dummy = 1;
   int fd = open("/dev/cs614",O_RDWR);
   int fd_ptw = open("/dev/ptw",O_RDWR);
   if(fd < 0 || fd_ptw < 0){
       perror("open");
       exit(-1);
   }
   struct sigaction sa;
   sa.sa_flags = SA_SIGINFO;
   sigemptyset(&sa.sa_mask);
   sa.sa_sigaction = handler;
   if (sigaction(SIGSEGV, &sa, NULL) == -1){
        perror("sigaction");
	return -1;
   }
   
   ptr = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
   unsigned num_2MB = ((SIZE>>21)<<21)==SIZE?SIZE>>21:(SIZE>>21)+1;
   struct data * dt = (struct data*)mmap(NULL, num_2MB*sizeof(struct data), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE, 0, 0);
   if(ptr == MAP_FAILED || dt == MAP_FAILED){
        perror("mmap");
        exit(-1);
   }
   printf("before promotion\n");
   struct data * temp_dt = dt;
   for(int j=0; j<SIZE; j+=TWOMB){
       unsigned long addr = (unsigned long)ptr+j;
       write(fd_ptw,&addr,sizeof(unsigned long));
       read(fd_ptw, temp_dt,sizeof(struct data));
       printf("pmd:%lx, pte:%lx\n",temp_dt->pmd,temp_dt->pte);
       temp_dt += 1;
   }
      
   // creates the kernel thread
   if(ioctl(fd, IOCTL_PROMOTE_VMA, &dummy) < 0){
       perror("2MB promote thread creation failed\n");
       exit(-1);
   }
   int temp = 1;
   char buff[4];
   sprintf(buff,"%d",temp);
   int fd_sysfs = open("/sys/kernel/kobject_cs614/promote",O_RDWR);
   if(fd_sysfs < 0){
       perror("open");
       exit(-1);
   }

   for(int k=0; k<4; k++){
       memset((void*)((unsigned long)ptr+(k*536870912)),'0',268435456);
       //indicates the kernel thread to promote 4KB pages to 2MB
       if(write(fd_sysfs, buff, sizeof(int)) < 0){
           perror("sysfs write");
           exit(-1);
       }
       unsigned limit = 0;
       do{
           sleep(10);
           limit += 1;
           //check kernel thread has finished work and set sysfs to 0
           lseek(fd_sysfs,0,SEEK_SET);
           if(read(fd_sysfs, buff, sizeof(int)) < 0){
              perror("sysfs read");
              exit(-1);
	   }
           temp = atoi(buff);
           if(limit >= 12){
	       printf("time limit crossed\n");
               break;
	   }
       }while(temp);
   
       if(limit == 12){
           return -1;
       }
   }
   printf("after promotion\n");
   temp_dt = dt;
   for(int j=0; j<SIZE; j+=TWOMB){
       unsigned long addr = (unsigned long)ptr+j;
       write(fd_ptw,&addr,sizeof(unsigned long));
       read(fd_ptw, temp_dt,sizeof(struct data));
       printf("pmd:%lx, pte:%lx\n",temp_dt->pmd,temp_dt->pte);
       temp_dt += 1;
   }
   close(fd);
   close(fd_ptw);
   close(fd_sysfs);
   //munmap((void *)ptr, SIZE);
   return 0;
}
