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
  //Creating the check_buffer with expected encrpyted value
  char *encrypt_check_buff = create_encrpyted_buffer(msg, a, b, size);

  // Creating device handler and setting the configurations and key
  cdev = create_device_handle(SET, SET, a, b);


  // Device Encrypt
  encrypt(cdev, op_text, size, 0);

  // Should be equal to check_buffer
  if(!isValid(encrypt_check_buff, op_text, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }


  // Decrypt
  decrypt(cdev, op_text, size, 0);

   // Should be equal to the original buffer
  if(!isValid(msg, op_text, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  printf("Testcase passed\n");

  close_handle(cdev);
  return 0;
}
