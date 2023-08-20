#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include<sys/wait.h>
#include<pthread.h>
#include <gen.h>

void do_encryption(int number_of_chunk)
{

  int file_desc = 0, file_size = 0;
  void *buffer =  map_file(&file_desc, &file_size);

  DEV_HANDLE cdev;
  KEY_COMP a=30, b=17;

  cdev = create_device_handle(SET, SET, a, b);

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


int main()
{
  pthread_t thread1[2];
  for(int i =0; i< 2; i++)
  {
    int childProcess = fork();
    if(childProcess == 0)
    {
      if(i % 2 == 0)
      {
        int chunk_numbers = 50;
        do_encryption(chunk_numbers);
        exit(0);
      }
    }
    if(pthread_create(&thread1[i], NULL, worker, NULL))
    {
        printf("Cannot Create the threads \n");
        exit(0);
    }
  }
  for(int i =0; i<2; i++)
  {
    wait(NULL);
    pthread_join(thread1[i], NULL);
  }
  printf("Testcase passed\n");
  return 0;
}
