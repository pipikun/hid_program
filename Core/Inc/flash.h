#ifndef __FLASH_H__
#define __FLASH_H__

#include "main.h"

/* STM32 firmware size */
#define STM32_PROGRAM_SIZE      (64 * 1024 )
#define STM32_PROGRAM_PAGE      (STM32_PROGRAM_SIZE / FLASH_PAGE_SIZE)

/* firmware address start */
#define FIRMWARE_BASE           (FLASH_BASE + (STM32_PROGRAM_PAGE + 1) * FLASH_PAGE_SIZE)
#define FIRMWARE_PAGE_MAX       ((64 * 1024) * FLASH_PAGE_SIZE)

/* User flash */
#define USER_FLASH_BASE         (FIRMWARE_BASE + 1)

HAL_StatusTypeDef flash_write_page(uint32_t page, uint16_t *data);
HAL_StatusTypeDef firmware_save(uint32_t page, uint8_t *data);
void flash_read_page(uint32_t page, uint8_t *data);
uint8_t flash_read_u8(uint32_t addr);
uint16_t flash_read_u16(uint32_t addr);
uint32_t flash_read_u32(uint32_t addr);
uint64_t flas_read_u64(uint32_t addr);

#endif /*__FLASH_H__ */
//vim: ts=8 sw=8 noet autoindent:
