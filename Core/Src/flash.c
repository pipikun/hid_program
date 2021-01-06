#include "flash.h"

HAL_StatusTypeDef flash_write_page(uint32_t page_idx, uint16_t *data)
{
        FLASH_EraseInitTypeDef erase_conf;

        uint32_t page_error;
        uint32_t addr;

        addr = FIRMWARE_BASE + page_idx * FLASH_PAGE_SIZE;

        /* Flash erase  */
        HAL_FLASH_Unlock();
        erase_conf.TypeErase = FLASH_TYPEERASE_PAGES;
        erase_conf.PageAddress = addr;
        erase_conf.NbPages = 1;
        HAL_FLASHEx_Erase(&erase_conf, &page_error);

        /* Flash program */
        for (int i = 0; i<FLASH_PAGE_SIZE; i+=2) {
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr+i, *data);
                data++;
        }
        HAL_FLASH_Lock();
        return HAL_OK;
}

void flash_read_page(uint32_t page_idx, uint8_t *data)
{
        uint32_t addr;

        addr = FIRMWARE_BASE + page_idx * FLASH_PAGE_SIZE;
        for (int i=0; i<FLASH_PAGE_SIZE; i++) {
                *data = *(__IO uint8_t *)(addr + i);
                 data++;
        }
}

uint8_t flash_read_byte(uint32_t addr)
{
        return  *(__IO uint8_t *)(FIRMWARE_BASE + addr);
}

