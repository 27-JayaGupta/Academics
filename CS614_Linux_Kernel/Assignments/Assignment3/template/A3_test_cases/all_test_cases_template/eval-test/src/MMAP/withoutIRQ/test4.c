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
  char *actual_mmap_buff = map_card(cdev, size);
  if(actual_mmap_buff == NULL)
  {
      printf("Testcase failed\n");
      exit(0);
  }
  strncpy(actual_mmap_buff, buffer, size);

  char *test_mmap_buff = create_encrpyted_buffer(buffer, a, b, size);
  encrypt(cdev, actual_mmap_buff, size, 1);


  int without_mmap_size = 1024;

  char * without_mmap_buf = (char *)malloc(without_mmap_size);

  strncpy(without_mmap_buf, (buffer+15), without_mmap_size);

  char *test_buff = create_encrpyted_buffer((buffer+15), a, b, without_mmap_size);
  encrypt(cdev, without_mmap_buf, size, 1);

  if(!isValid(without_mmap_buf, test_buff, without_mmap_size))
  {
    printf("Testcase failed\n");
    exit(0);
  }


  if(!isValid(test_mmap_buff, actual_mmap_buff, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  unmap_card(cdev, actual_mmap_buff);

  printf("Testcase passed\n");

  close_handle(cdev);

  return 0;
}
