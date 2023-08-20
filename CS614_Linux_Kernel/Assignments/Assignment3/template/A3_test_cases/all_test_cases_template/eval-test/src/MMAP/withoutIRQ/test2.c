#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <gen.h>


int main()
{

  int file_desc = 0, file_size = 0;
  void *buffer =  map_file(&file_desc, &file_size);

  DEV_HANDLE cdev;
  KEY_COMP a=30, b=17;
  unsigned int size = (1020*1024);

  cdev = create_device_handle(UNSET, UNSET, a, b);
  char *actual_buff = map_card(cdev, size);
  if(actual_buff == NULL)
  {
      printf("Testcase failed\n");
      exit(0);
  }
  strncpy(actual_buff, buffer, size);

  char *test_buff = create_encrpyted_buffer(actual_buff, a, b, size);
  encrypt(cdev, actual_buff, size, 1);

  if(!isValid(actual_buff, test_buff, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }


  decrypt(cdev, actual_buff, size, 1);

  if(!isValid(buffer, actual_buff, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  unmap_card(cdev, actual_buff);

  printf("Testcase passed\n");

  close_handle(cdev);

  return 0;
}
