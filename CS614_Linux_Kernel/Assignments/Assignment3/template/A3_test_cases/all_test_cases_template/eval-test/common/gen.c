
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <crypter.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <gen.h>

void* map_file(int * file_desc, int *file_size)
{
  struct stat file_stats;
  int fd = open("/dev/shm/in_memory.txt", O_RDWR);
  if(fd < 0)
  {
     printf("Error Occured while opening the file \n");
     exit(0);
  }

  fstat(fd, &file_stats);

  void * buff_address;
  buff_address = mmap(NULL, file_stats.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_POPULATE, fd, 0);
  if(buff_address <= 0)
  {
    printf("Error while mapping the file\n");
    exit(0);
  }

  *file_desc = fd;
  *file_size = file_stats.st_size;

  return buff_address;

}

char encryptChar(int a, int b, char p)
{
  char c;
	c = (p + a) % 26;
	c = (c + b) % 26;
	return c;
}

char decryptChar(int a, int b, char c)
{
	char p;
	p = (c - b);
	if ( p < 0)
		p = p + 26;
	p = (p - a) % 26;
	if ( p < 0)
		p = p + 26;
	return (char) p;
}

DEV_HANDLE create_device_handle(uint8_t isDMA, uint8_t isInterrupt, KEY_COMP a, KEY_COMP b)
{

  DEV_HANDLE cdev = create_handle();
  if(cdev == ERROR)
  {
    printf("Testcase failed: Unable to create handle for device\n");
    exit(0);
  }

  if(set_config(cdev, DMA, isDMA) == ERROR){
    printf("Testcase failed: Unable to confiure DMA/MMIO\n");
    exit(0);
  }

  if(set_config(cdev, INTERRUPT, isInterrupt) == ERROR){
    printf("Testcase failed: Unable to configure interrupt\n");
    exit(0);
  }

  if(set_key(cdev, a, b) == ERROR){
    printf("Testcase failed: Unable to set key\n");
    exit(0);
  }
  return cdev;
}

int isValid(const char* buff1, const char* buff2, uint64_t size)
{
  uint64_t index = 0;
  while( index < size && buff1[index] == buff2[index])
    index++;

  return (index == size ? TRUE : FALSE) ;
}

void encryptMessage(char* buf, uint64_t length, KEY_COMP a, KEY_COMP b)
{
  for (uint64_t pos = 0; pos < length; pos++)
  {
    if(buf[pos] >= 65 && buf[pos] <= 90)

      buf[pos] = (char) (encryptChar(a,b, buf[pos] - 'A') + 'A');
    else if(buf[pos] >= 97 && buf[pos] <= 122)
      buf[pos] = (char) (encryptChar(a,b, buf[pos] - 'a') + 'a');
    else
      buf[pos] = buf[pos];
  }
}

void decryptMessage(char* buf, uint64_t length, KEY_COMP a, KEY_COMP b)
{
  for (uint64_t pos = 0; pos < length; pos++)
  {
    if(buf[pos] >= 65 && buf[pos] <= 90)
      buf[pos] = (char) (decryptChar(a,b, buf[pos] - 'A') + 'A');
    else if(buf[pos] >= 97 && buf[pos] <= 122)
      buf[pos] = (char) (decryptChar(a,b, buf[pos] - 'a') + 'a');
    else
      buf[pos] = buf[pos];
  }
}

char *create_encrpyted_buffer(char *msg, KEY_COMP a, KEY_COMP b, uint64_t size)
{
  char *encrypted_buf = (char*)malloc(size);
  strncpy(encrypted_buf, msg, size);
  encryptMessage(encrypted_buf, size, a, b);
  return encrypted_buf;
}
