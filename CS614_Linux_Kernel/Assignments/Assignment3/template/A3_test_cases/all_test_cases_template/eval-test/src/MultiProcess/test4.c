#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include<sys/wait.h>
#include <gen.h>


void do_encryption(int number_of_chunk, int isDMA, int isInterrupt, int chunk_size)
{

  int file_desc = 0, file_size = 0;
  void *buffer =  map_file(&file_desc, &file_size);

  DEV_HANDLE cdev;
  KEY_COMP a=30, b=17;

  cdev = create_device_handle(isDMA, isInterrupt, a, b);
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

  for(int i =0; i<50; i++)
  {
    int childid = fork();
    if(childid == 0)
    {
      int index = i % 4;

      if(index == 0)
      {
           do_encryption(500, UNSET, UNSET, 1);

      } else if (index == 1)
      {
           do_encryption(20, UNSET, SET, 1555);

      } else if (index == 2)
      {
           do_encryption(10, SET, UNSET,(1020*1024));

      } else if (index == 3)
      {
           do_encryption(15, SET, SET, 4096);

      }
      exit(0);
    }
  }
  for(int i =0; i<50; i++)
  {
    wait(NULL);
  }
  printf("Testcase passed\n");
  return 0;
}
