#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>
#include <gen.h>

int main()
{
  DEV_HANDLE cdev;
  char *msg = "Hello CS730!\n";
  KEY_COMP a=30, b=17;
  uint64_t size = strlen(msg);



  cdev = create_device_handle(UNSET, UNSET, a, b);
  char *actual_buff = map_card(cdev, size);
  if(actual_buff == NULL)
  {
      printf("Testcase failed\n");
      exit(0);
  }
  strncpy(actual_buff, msg, size);



  char *test_buff = create_encrpyted_buffer(msg, a, b, size);

  encrypt(cdev, actual_buff, size, 1);

  if(!isValid(actual_buff, test_buff, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }


  decrypt(cdev, actual_buff, size, 1);

  if(!isValid(msg, actual_buff, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  unmap_card(cdev, actual_buff);
  printf("Testcase passed\n");

  close_handle(cdev);
  return 0;
}
