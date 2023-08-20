#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include<sys/wait.h>
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

int main()
{
  int flag = 0;

  flag = 1;
  int childProcess = fork();

  if(childProcess == 0)
  {
    if(flag == 1)
    {
      int chunk_numbers = 100;
      printf("Child process 1 with %d \n", chunk_numbers);
      do_encryption(chunk_numbers);
      printf("Child process 1 executed\n");
      exit(0);
    }
  }

  flag = 2;
  childProcess = fork();
  if(childProcess == 0)
  {
    if(flag == 2)
    {
      int chunk_numbers = 50;
      printf("Child process 2 with %d \n", chunk_numbers);
      do_encryption(chunk_numbers);
      printf("Child process 2 executed\n");
      exit(0);
    }
  }

  printf("Parent process 2 with %d \n", 50);
  do_encryption(50);

  for(int i =0; i<2; i++)
  {
    wait(NULL);
  }
  printf("Testcase passed\n");
  return 0;
}
