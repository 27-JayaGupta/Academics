#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <gen.h>


// Creating  and Testing the chunk size of 1.
int main()
{

  int file_desc = 0, file_size = 0;
  void *buffer =  map_file(&file_desc, &file_size);

  DEV_HANDLE cdev;
  KEY_COMP a=30, b=17;

  cdev = create_device_handle(SET, SET, a, b);

  unsigned int chunk_size = 1;
  unsigned int number_of_chunk = 10;

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

  printf("Testcase passed\n");
  close_handle(cdev);
  munmap(buffer, file_size);
  close(file_desc);

  return 0;
}
