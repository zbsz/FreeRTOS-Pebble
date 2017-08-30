/* flash.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "platform.h"
#include "flash.h"
#include "fs.h"

extern void hw_flash_init(void);
extern void hw_flash_read_bytes(uint32_t, uint8_t*, size_t);

// TODO
// DMA/async?
// what about apps/watchface resource loading?
// document

/// MUTEX
static SemaphoreHandle_t _flash_mutex;
static StaticSemaphore_t _flash_mutex_buf;
uint32_t _flash_get_app_slot_address(uint16_t slot_id);
extern unsigned int _ram_top;
#define portMPU_REGION_READ_WRITE (0x03UL << MPU_RASR_AP_Pos)

static struct hw_driver_ext_flash_t *_flash_driver;

// No ISR here (yet)
static hw_driver_handler_t _callack_handler = {
    .done_isr = NULL
};

void flash_init()
{
    // initialise device specific flash
    _flash_driver = (hw_driver_ext_flash_t *)driver_register((hw_driver_module_init_t)hw_flash_module_init, &_callack_handler);
    assert(_flash_driver->read_bytes && "Read is invalid");
    
//     MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;
//     MPU->RNR  = 0;
//     MPU->RBAR = 0x20000000;
//     MPU->RASR = _ram_top | portMPU_REGION_READ_WRITE  | MPU_RASR_XN_Msk;
//     SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
//     MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;

    _flash_mutex = xSemaphoreCreateMutexStatic(&_flash_mutex_buf);
    
    fs_init();
}

/*
 * Read a given number of bytes SAFELY from the flash chip
 */
void flash_read_bytes(uint32_t address, uint8_t *buffer, size_t num_bytes)
{
    xSemaphoreTake(_flash_mutex, portMAX_DELAY);

    _flash_driver->read_bytes(address, buffer, num_bytes);
    
    xSemaphoreGive(_flash_mutex);
}

void flash_dump(void)
{
    uint8_t buffer[1025];
//     xSemaphoreTake(_flash_mutex, portMAX_DELAY);
    
    for (int i = 0; i < (16384); i++)
    {
        flash_read_bytes(i * 1024, buffer, 1024);
    
        ss_debug_write(buffer, 1024);
    }
 
 while(1);
//     xSemaphoreGive(_flash_mutex);
}
