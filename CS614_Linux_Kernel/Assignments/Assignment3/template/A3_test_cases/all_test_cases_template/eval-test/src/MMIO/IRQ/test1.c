#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>
#include <gen.h>
// Simple testcase to check basic functionality
int main()
{
  DEV_HANDLE cdev;
  char *msg = "Hello CS730!\n";
  char op_text[16];
  KEY_COMP a=30, b=17;
  uint64_t size = strlen(msg);


  strcpy(op_text, msg);
  char *encrypt_check_buff = create_encrpyted_buffer(msg, a, b, size);

  cdev = create_device_handle(UNSET, SET, a, b);

  encrypt(cdev, op_text, size, 0);

  if(!isValid(encrypt_check_buff, op_text, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  decrypt(cdev, op_text, size, 0);

  if(!isValid(msg, op_text, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  printf("Testcase passed\n");

  close_handle(cdev);
  return 0;
}
