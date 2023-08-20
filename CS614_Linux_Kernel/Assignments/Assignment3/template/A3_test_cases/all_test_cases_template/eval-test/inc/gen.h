#include<crypter.h>
#ifndef GEN_H_
#define GEN_H_

DEV_HANDLE create_device_handle(uint8_t isDMA, uint8_t isInterrupt, KEY_COMP a, KEY_COMP b);
int isValid(const char* buff1, const char* buff2, uint64_t size);
void encryptMessage(char* buf, uint64_t length, KEY_COMP a, KEY_COMP b);
void decryptMessage(char* buf, uint64_t length, KEY_COMP a, KEY_COMP b);
char *create_encrpyted_buffer(char *msg, KEY_COMP a, KEY_COMP b, uint64_t size);
void* map_file(int * file_desc, int *file_size);

#endif
