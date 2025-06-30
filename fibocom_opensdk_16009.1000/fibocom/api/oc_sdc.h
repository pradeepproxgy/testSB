#ifndef COMPONENTS_FIBOCOM_OPENCPU_OC_SDC_H
#define COMPONENTS_FIBOCOM_OPENCPU_OC_SDC_H

#include "fibo_opencpu_comm.h"

bool fibo_format_sdc(int sdc_num);

bool fibo_mount_sdc(int sdc_num, const char *path);

void fibo_umount_sdc(const char *path);

INT8 fibo_show_sdcard_space(unsigned long *total, unsigned long *used, unsigned long *rest);

bool fibo_sdio_device_open(uint32_t sdc_num);

bool fibo_sdio_device_close(uint32_t sdc_num);

bool fibo_sdio_device_read(uint32_t sdc_num, uint32_t block_number, void *buffer, uint32_t size);

bool fibo_sdio_device_write(uint32_t sdc_num, uint32_t block_number, void *buffer, uint32_t size);

#endif // COMPONENTS_FIBOCOM_OPENCPU_OC_SDC_H
