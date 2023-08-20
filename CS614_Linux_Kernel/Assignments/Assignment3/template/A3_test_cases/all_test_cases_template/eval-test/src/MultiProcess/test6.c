#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include<sys/wait.h>
#include<signal.h>
#include <pthread.h>
#include <gen.h>


void do_encryption(int number_of_chunk)
{

  int file_desc = 0, file_size = 0;
  void *buffer =  map_file(&file_desc, &file_size);

  DEV_HANDLE cdev;
  KEY_COMP a=30, b=17;

  cdev = create_device_handle(UNSET, UNSET, a, b);

  unsigned int chunk_size = 1;
  // unsigned int number_of_chunk = 10;

  for(unsigned int chunk = 0; chunk < number_of_chunk ; chunk++)
  {
      unsigned int offset = (chunk*chunk_size);
      void *buffer_address = buffer + offset;
      char * encrypt_buff = create_encrpyted_buffer(buffer_address, a, b, chunk_size);

      encrypt(cdev, buffer_address, chunk_size, 0);
      if(!isValid(encrypt_buff, buffer_address, chunk_size))
      {
        printf("Testcase failed\n");
        exit(0);
      }

      decrypt(cdev, buffer_address, chunk_size, 0);

      decryptMessage(encrypt_buff, chunk_size, a, b);

      if(!isValid(encrypt_buff, buffer_address, chunk_size))
      {
        printf("Testcase failed\n");
        exit(0);
      }
  }

  close_handle(cdev);
  munmap(buffer, file_size);
  close(file_desc);

}

void* worker(void* value)
{
    //printf("Thread param is %d \n",( * ( int * )value));
    do_encryption(100);
}

int main(void){
    pthread_t thread1[2];
    if(pthread_create(&thread1[0], NULL, worker, NULL))
    {
        printf("Cannot Create the threads \n");
        exit(0);
    }

    if(pthread_create(&thread1[1], NULL, worker, NULL))
    {
        printf("Cannot Create the threads \n");
        exit(0);
    }

    pthread_join(thread1[0], NULL);

    pthread_join(thread1[1], NULL);

    printf("Testcase Passed\n");
    exit(0);
}
