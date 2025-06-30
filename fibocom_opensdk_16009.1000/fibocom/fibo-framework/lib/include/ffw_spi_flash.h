#ifndef LIB_INCLUDE_SPIFLASH_FS
#define LIB_INCLUDE_SPIFLASH_FS

typedef int INT32;
typedef unsigned char UINT8;
typedef unsigned int UINT32;


bool ffw_ext_flash_poweron(uint8_t pinsel, uint8_t val, uint8_t div);

bool ffw_ext_flash_poweron_v2(uint8_t pinsel, uint8_t val, uint8_t div);

INT32 ffw_ext_flash_init(uint8_t Pinsel);

INT32 ffw_ext_flash_info(UINT32 *pulId, UINT32 *pulCapacity);

INT32 ffw_ext_flash_read(UINT32 faddr, UINT8 *data,UINT32 size);

INT32 ffw_ext_flash_write(UINT32 faddr, UINT8 *data,UINT32 size);

INT32 ffw_ext_flash_erase(UINT32 faddr, UINT32 size);

void ffw_ext_flash_mode_set(uint8_t mode);

INT32 ffw_ffsmountExtflash(UINT32 uladdr_start, UINT32 ulsize,
                              char *dir,UINT8 spi_pin_sel,
                              bool format_on_fail,bool force_format);

INT32 ffw_ffs_unmount_ext_flash(const char *dir);

INT32 ffw_ffs_format_ext_flash(const char *dir);

INT32 ffw_ext_flash_format(UINT8 spi_pin_sel);

extern bool fibo_ext_hal_flash_poweron(uint8_t pinsel, uint8_t val, uint8_t div);
extern bool fibo_ext_hal_flash_poweron_v2(uint8_t pinsel, uint8_t val, uint8_t div);
extern INT32 fibo_ext_hal_flash_init(uint8_t Pinsel);
extern INT32 fibo_ext_hal_flash_info(UINT32 *pulId, UINT32 *pulCapacity);
extern INT32 fibo_ext_hal_flash_read(UINT32 faddr, UINT8 *data,UINT32 size);
extern INT32 fibo_ext_hal_flash_write(UINT32 faddr, UINT8 *data,UINT32 size);
extern INT32 fibo_ext_hal_flash_erase(UINT32 faddr, UINT32 size);
extern INT32 fibo_hal_ffsmountExtflash(UINT32 uladdr_start, UINT32 ulsize,
                              char *dir,UINT8 spi_pin_sel,
                              bool format_on_fail,bool force_format);

extern INT32 fibo_hal_unmount_ext_flash(const char *dir);
extern INT32 fibo_hal_format_ext_flash(const char *dir);
extern INT32 fibo_ext_hal_flash_format(UINT8 spi_pin_sel);

extern void ext_flash_mode_set(uint8_t mode);

#endif
