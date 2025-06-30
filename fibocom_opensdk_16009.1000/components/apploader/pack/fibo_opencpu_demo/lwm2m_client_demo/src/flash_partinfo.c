#include "osi_api.h"
#include "osi_log.h"
#include "hal_config.h"

#include "fibo_opencpu.h"

static fibo_flash_partinfo_t flash_partinfo;

fibo_flash_partinfo_t *fibo_get_flash_partinfo(void)
{
    flash_partinfo.spl_address = CONFIG_SPL_FLASH_ADDRESS;
    flash_partinfo.spl_size = CONFIG_SPL_FLASH_SIZE;
    flash_partinfo.boot_address = CONFIG_BOOT_FLASH_ADDRESS;
    flash_partinfo.boot_size = CONFIG_BOOT_FLASH_SIZE;
    flash_partinfo.flash_address = CONFIG_APP_FLASH_ADDRESS;
    flash_partinfo.flash_size = CONFIG_APP_FLASH_SIZE;
    flash_partinfo.appimg_flash_address = CONFIG_APPIMG_FLASH_ADDRESS;
    flash_partinfo.appimg_flash_size = CONFIG_APPIMG_FLASH_SIZE;
    flash_partinfo.fs_sys_address = CONFIG_FS_SYS_FLASH_ADDRESS;
    flash_partinfo.fs_sys_address_size = CONFIG_FS_SYS_FLASH_SIZE;
    flash_partinfo.fs_modem_address = CONFIG_FS_MODEM_FLASH_ADDRESS;
    flash_partinfo.fs_modem_size = CONFIG_FS_MODEM_FLASH_SIZE;
    flash_partinfo.fs_factory_address = CONFIG_FS_FACTORY_FLASH_ADDRESS;
    flash_partinfo.fs_factory_size = CONFIG_FS_FACTORY_FLASH_SIZE;
    return (fibo_flash_partinfo_t *)&flash_partinfo;
}
