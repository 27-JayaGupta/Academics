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


void do_encryption(int number_of_chunk, int chunk_size)
{

  int file_desc = 0, file_size = 0;
  void *buffer =  map_file(&file_desc, &file_size);

  DEV_HANDLE cdev;
  KEY_COMP a=30, b=17;

  cdev = create_device_handle(SET, SET, a, b);
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
    int chunk_size = 0;
    int count = 0;
    int param = *( int *) value;
    if(param % 4 == 0)
    {
        chunk_size = 4096;
        count = 50;
    } else if (param % 4 == 1)
    {
          chunk_size = 1020*1024;
          count = 10;
    } else if (param % 4 == 2)
    {
        chunk_size = 1;
        count = 100;
      /* code */
    } else
    {
      /* code */
      chunk_size = 1555;
      count = 10;
    }
    do_encryption(count, chunk_size);
}

int main(void){
    pthread_t thread1[12];

    for(int i =0; i< 12; i++)
    {
      int param = i;
      if(pthread_create(&thread1[0], NULL, worker, &param))
      {
          printf("Cannot Create the threads \n");
          exit(0);
      }
    }
    for(int i =0; i< 12; i++)
    {
       pthread_join(thread1[i], NULL);
    }
    printf("Testcase Passed\n");
    exit(0);
}
