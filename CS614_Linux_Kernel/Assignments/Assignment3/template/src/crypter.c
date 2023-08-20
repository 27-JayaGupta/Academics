#include<crypter.h>
#include<string.h>
#include<sys/mman.h>

#define ONE_MB 1024*1024
#define DEV_PRIV_INFO_SIZE 0xa8
#define CHUNK_SIZE (0xfffff - 0xa8)
void* mmap_addr;
unsigned long mmap_len;

/*Function template to create handle for the CryptoCard device.
On success it returns the device handle as an integer*/
DEV_HANDLE create_handle()
{ 
  char handle[10];
  int handle_int;
  int fd = open("/sys/kernel/cs614_cryptocard/get_handle", O_RDONLY);
  if(fd < 0) {
    printf("Failed to open get_handle file for the process\n");
    return ERROR;
  }

  if(read(fd, handle, 10)<0)
  {
    printf("Failed to read handle from get_handle file\n");
    return ERROR;
  }

  handle_int = atoi(handle);

  printf("handle: %d\n", handle_int);

  if(handle_int <= 0){
    printf("Failed to read handle for the process\n");
    return ERROR;
  }

  close(fd);
  return handle_int;
}

/*Function template to close device handle.
Takes an already opened device handle as an arguments*/
void close_handle(DEV_HANDLE cdev)
{ 
  int handle;
  int fd = open("/sys/kernel/cs614_cryptocard/get_handle", O_WRONLY);
  if(fd < 0) {
    printf("Failed to open get_handle file for the process\n");
    exit(-1);
  }

  if(write(fd, "close", 5)<0){
    printf("Failed to write get_handle file for the process\n");
    exit(-1);
  }
}

/*Function template to encrypt a message using MMIO/DMA/Memory-mapped.
Takes four arguments
  cdev: opened device handle
  addr: data address on which encryption has to be performed
  length: size of data to be encrypt
  isMapped: TRUE if addr is memory-mapped address otherwise FALSE
*/
int encrypt(DEV_HANDLE cdev, ADDR_PTR addr, uint64_t length, uint8_t isMapped)
{ 
  int fd = open("/dev/cs614_crypto_device", O_RDWR);
  
  if(fd < 0) {
    printf("Failed to open chardev for the process\n");
    return ERROR;
  }

  char ubuf[length+2];
  ubuf[0] = 'e';

  if(isMapped){
    ubuf[1] = 'm';  // mapped
    if(write(fd, ubuf, length) < 0){
      printf("[ENCRYPT]Failed to write encryt msg for the process\n");
      return ERROR;
    }
    close(fd);
  
    return 0;
  }

  ubuf[1] = 'u'; // unmapped

  unsigned long num_chunks = length/CHUNK_SIZE;
  unsigned long rem = length%CHUNK_SIZE;

  if(rem) num_chunks += 1;

  for(unsigned long chunk = 0; chunk < num_chunks ; chunk++){
    unsigned long offset = chunk*CHUNK_SIZE;
    unsigned long len_msg = length - offset;
    if(len_msg > CHUNK_SIZE)
      len_msg = CHUNK_SIZE;
    
    memcpy(ubuf+2, addr+offset, len_msg);

    if(write(fd, ubuf, len_msg) < 0){
      printf("[ENCRYPT]Failed to write encryt msg for the process\n");
      return ERROR;
    }

    if(read(fd, addr+offset, len_msg) < 0){
      printf("[ENCRYPT]Failed to read encryt msg for the process\n");
      return ERROR;
    }
  }
  
  close(fd);
  
  return 0;
}

/*Function template to decrypt a message using MMIO/DMA/Memory-mapped.
Takes four arguments
  cdev: opened device handle
  addr: data address on which decryption has to be performed
  length: size of data to be decrypt
  isMapped: TRUE if addr is memory-mapped address otherwise FALSE
*/
int decrypt(DEV_HANDLE cdev, ADDR_PTR addr, uint64_t length, uint8_t isMapped)
{
  int fd = open("/dev/cs614_crypto_device", O_RDWR);
  if(fd < 0) {
    printf("Failed to open decrypt chardev for the process\n");
    return ERROR;
  }

  char ubuf[length+2];
  ubuf[0] = 'd';

  if(isMapped){
    ubuf[1] = 'm'; // mapped
    if(write(fd, ubuf, length)<0){
      printf("[DECRYPT]Failed to write decrypt msg for the process\n");
      return ERROR;
    }

    close(fd);
  
    return 0;
  }

  ubuf[1] = 'u'; // unmapped

  unsigned long num_chunks = length/CHUNK_SIZE;
  unsigned long rem = length%CHUNK_SIZE;

  if(rem) num_chunks += 1;

  for(unsigned long chunk = 0; chunk < num_chunks ; chunk++){
    unsigned long offset = chunk*CHUNK_SIZE;
    unsigned long len_msg = length - offset;
    if(len_msg > CHUNK_SIZE)
      len_msg = CHUNK_SIZE;
    
    memcpy(ubuf+2, addr+offset, len_msg);
    
    if(write(fd, ubuf, len_msg) < 0){
      printf("[ENCRYPT]Failed to write encryt msg for the process\n");
      return ERROR;
    }

    if(read(fd, addr+offset, len_msg) < 0){
      printf("[ENCRYPT]Failed to read encryt msg for the process\n");
      return ERROR;
    }
  }

  close(fd);
  
  return 0;
}

/*Function template to set the key pair.
Takes three arguments
  cdev: opened device handle
  a: value of key component a
  b: value of key component b
Return 0 in case of key is set successfully*/
int set_key(DEV_HANDLE cdev, KEY_COMP a, KEY_COMP b)
{   
    char buf[10];
    int count;

    count = sprintf(buf, "%c%c", a, b);
    if(count < 0 ){
      printf("Sprintf failed\n");
      return ERROR;
    }

    int fd = open("/sys/kernel/cs614_cryptocard/keys", O_WRONLY);
    if(fd < 0) {
      printf("Failed to open sysfs file for the process to write keys\n");
      return ERROR;
    }

    if(write(fd, buf, count) < 0){
      printf("Failed to write keys for the process\n");
      return ERROR;
    }

    close(fd);
    return 0;
}

/*Function template to set configuration of the device to operate.
Takes three arguments
  cdev: opened device handle
  type: type of configuration, i.e. set/unset DMA operation, interrupt
  value: SET/UNSET to enable or disable configuration as described in type
Return 0 in case of key is set successfully*/
int set_config(DEV_HANDLE cdev, config_t type, uint8_t value)
{ 
    int fd = open("/sys/kernel/cs614_cryptocard/config", O_WRONLY);
    if(fd < 0) {
      printf("Failed to open config sysfs file for the process to write configs\n");
      return ERROR;
    }

    char buf[1];
    if(type == DMA && value==SET)
      buf[0] = 'd';
    else if(type == DMA && value == UNSET)
      buf[0] = 'm';
    else if(type == INTERRUPT && value==SET)
      buf[0] = 'i';
    else if(type == INTERRUPT && value == UNSET)
      buf[0] = 'n';
    
    if(write(fd, buf, 1) < 0){
      printf("Failed to write configs for the process\n");
      return ERROR;
    }

    close(fd);
    return 0;
}

/*Function template to map device input/output memory into user space.
Takes three arguments
  cdev: opened device handle
  size: amount of memory-mapped into user-space (not more than 1MB strict check)
Return virtual address of the mapped memory*/
ADDR_PTR map_card(DEV_HANDLE cdev, uint64_t size)
{  
  char buf[22];
  int count;
  void* map_addr;
  if(size > ONE_MB){
    printf("User can map only 1MB region\n");
    return NULL;
  }

  size += DEV_PRIV_INFO_SIZE;
  map_addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
  
  if(map_addr == NULL) {
    printf("MMAP call failed\n");
    return NULL;
  }

  mmap_addr = map_addr;
  mmap_len = size;
  
  int fd = open("/sys/kernel/cs614_cryptocard/map", O_WRONLY);
  if(fd < 0) {
    printf("Failed to open config  map file for the process to map the region\n");
    return NULL;
  }

  count = sprintf(buf, "%ld", (unsigned long)map_addr);

  if(write(fd, buf, count) < 0){
    printf("Failed to mmap  region for the process\n");
    return NULL;
  }

  close(fd);

  map_addr += DEV_PRIV_INFO_SIZE;
  printf("New Address of buffer to map: %lx\n", (unsigned long)map_addr);
  return map_addr;
}

/*Function template to device input/output memory into user space.
Takes three arguments
  cdev: opened device handle
  addr: memory-mapped address to unmap from user-space*/
void unmap_card(DEV_HANDLE cdev, ADDR_PTR addr)
{
  if(mmap_addr == NULL){
    printf("Unknown addr: %lx sent to unmap. Addr doesnt exist\n", (long) addr);
  }

  munmap(addr, mmap_len);
  mmap_addr = NULL;
  mmap_len = 0;
}
