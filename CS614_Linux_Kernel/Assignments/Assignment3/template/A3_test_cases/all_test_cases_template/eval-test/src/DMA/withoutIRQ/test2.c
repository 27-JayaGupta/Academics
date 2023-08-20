#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>
#include <gen.h>

// Simple Testcase with multiple device hanlder with multiple level of decryption
int main()
{
  DEV_HANDLE cdev1, cdev2;
  char *msg = "Hello CS730!\n";
  char op_text[16];
  uint64_t size = strlen(msg);
  strcpy(op_text, msg);

  // Creating the check_buffer1 with expected encrypted value
  char *encrypt_check_buff1 = create_encrpyted_buffer(msg, 30, 17, size);

  // Creating the check_buffer2 with expected encrypted value
  char *encrypt_check_buff2 = create_encrpyted_buffer(msg, 45, 60, size);

  // Device handle 1
  cdev1 = create_device_handle(SET, UNSET, 30, 17);

  close_handle(cdev1);

  // Device handle 2
  cdev2 = create_device_handle(SET, UNSET, 45, 60);

  // Encrypting with dev handle 2
  encrypt(cdev2, op_text, size, 0);

  if(!isValid(encrypt_check_buff2, op_text, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  // Decryption with dev handle 2
  decrypt(cdev2, op_text, size, 0);

  // Should be equal to the check_buff 1
  if(!isValid(msg, op_text, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }
  close_handle(cdev2);

  // Device handle 1
  cdev1 = create_device_handle(SET, UNSET, 30, 17);
  
  // Encrypting with dev handle 1
  encrypt(cdev1, op_text, size, 0);

  if(!isValid(encrypt_check_buff1, op_text, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  // Decryption with dev handle 1
  decrypt(cdev1, op_text, size, 0);

  // Should be equal to the original text
  if(!isValid(msg, op_text, size))
  {
    printf("Testcase failed\n");
    exit(0);
  }

  printf("Testcase passed\n");

  close_handle(cdev1);
  return 0;
}
