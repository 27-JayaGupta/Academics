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
  unsigned int size = (340*1024);

  cdev = create_device_handle(UNSET, SET, a, b);

  char *test_buff = create_encrpyted_buffer(buffer, a, b, size);

  char *actual_buff1 = map_card(cdev, size);
  char *actual_buff2 = map_card(cdev, size);
  char *actual_buff3 = map_card(cdev, size);

  int buffer_condition = (actual_buff1 == NULL || actual_buff2 == NULL || actual_buff3 == NULL);

  buffer_condition  = buffer_condition || (!((actual_buff1 != actual_buff2) &&  (actual_buff2 != actual_buff3))) ;

  if(buffer_condition)
  {
      printf("Testcase failed\n");
      exit(0);
  }

  unmap_card(cdev, actual_buff2);

  actual_buff2 = map_card(cdev, size);

  if(actual_buff2 == NULL)
  {
      printf("Testcase failed\n");
      exit(0);
  }


  strncpy(actual_buff2, buffer, size);
  encrypt(cdev, actual_buff2, size, 1);
  if(!isValid(actual_buff2, test_buff, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }
  decrypt(cdev, actual_buff2, size, 1);
  if(!isValid(buffer, actual_buff2, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  unmap_card(cdev, actual_buff1);

  unmap_card(cdev, actual_buff2);

  unmap_card(cdev, actual_buff3);

  printf("Testcase passed\n");

  close_handle(cdev);

  return 0;
}
