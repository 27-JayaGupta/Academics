#ifndef _CRYPT_DRIVER_H
#define _CRYPT_DRIVER_H

#include <linux/dma-mapping.h>

#define DEVNAME "cs614_crypto_device"

#define CRYPTO_DEVICE(vendor_id, device_id) {\
	PCI_DEVICE(vendor_id, device_id)}

#define BAR_0 0
#define BAR_1 1

#define ID_OFFSET 0x00
#define LIVELINESS_OFFSET 0x04

#define KEY_OFFSET 0x08

#define MMIO_MSG_LEN_OFFSET 0x0c
#define MMIO_STATUS_OFFSET 0x20
#define MMIO_ADDR_OFFSET 0x80
#define MMIO_BUFFER_OFFSET 0xa8

#define MMIO_ENCRYPTION 0x00
#define MMIO_DECRYPTION 0x02
#define MMIO_INTERRUPT 0x80
#define MMIO_NO_INTERRUPT 0x00

#define DMA_LEN_OFFSET 0x98
#define DMA_ADDR_OFFSET 0x90
#define DMA_COMMAND_REG_OFFSET 0xa0

#define DMA_ENCRYPTION 0x00
#define DMA_DECRYPTION 0x02
#define DMA_INTERRUPT 0x04
#define DMA_NO_INTERRUPT 0x00
#define START_DMA 0x01

#define ISR_OFFSET 0x24
#define INTR_RAISE_REG_OFFSET 0x60
#define INTR_ACK_REG_OFFSET 0x64

#define INTR_MMIO 0x001
#define INTR_DMA 0x100

enum DEVICE_MODE{
    MMIO,
    DMA
};

#define DMA_BUFFER_SIZE (0xfffff - 0xa8)
struct crypto_hw {
    u8 __iomem *hw_addr;
    struct pci_dev* pdev;
    int bars;
    int need_ioport;
    char* dma_virt_addr;
    dma_addr_t dma_handle;
    bool have_64_dma;
    bool have_32_dma;
};

struct process_config{
    enum DEVICE_MODE type;
    bool interrupt_allowed; 
};

struct proc_keys_list_entry{
    struct list_head list;
    pid_t pid;
    int handle;
    unsigned char keya;
    unsigned char keyb;
    char* result;
    int result_len;
    struct process_config config;
    bool is_config_set;
    bool is_key_set;
    atomic_t is_intr_handled;
};

bool crypto_check_device_live(void);
void set_device_keys(char, char);
struct proc_keys_list_entry* find_process_in_list(void);
void perform_mmio_encrypt_decrypt(struct file* filp,
									const char* ubuf,
									size_t length,
									loff_t *offset,
                                    struct proc_keys_list_entry* iterator);

void perform_dma_encrypt_decrypt(struct file* filp,
                            const char* ubuf,
                            size_t length,
                            loff_t *offset,
                            struct proc_keys_list_entry* iterator);

int get_mmio_configs(char utility, bool intr);
int get_dma_configs(char utility, bool intr);

#endif