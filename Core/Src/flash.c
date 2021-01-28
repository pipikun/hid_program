#include "flash.h"

HAL_StatusTypeDef flash_write_page(uint32_t page, uint16_t *data)
{
	FLASH_EraseInitTypeDef erase_conf;

	uint32_t error;
	uint32_t addr;

	addr = FIRMWARE_BASE + page * FLASH_PAGE_SIZE;

	/* Flash erase  */
	HAL_FLASH_Unlock();
	erase_conf.TypeErase = FLASH_TYPEERASE_PAGES;
	erase_conf.PageAddress = addr;
	erase_conf.NbPages = 1;
	HAL_FLASHEx_Erase(&erase_conf, &error);

	/* Flash program */
	for (int i = 0; i<FLASH_PAGE_SIZE; i+=2) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr+i, *data);
		data++;
	}
	HAL_FLASH_Lock();
	return HAL_OK;
}

HAL_StatusTypeDef firmware_save(uint32_t page, uint8_t *data)
{

	FLASH_EraseInitTypeDef erase_conf;

	uint32_t error;
	uint32_t addr;
	uint16_t val;

	addr = FIRMWARE_BASE + page * FLASH_PAGE_SIZE;

	/* Flash erase  */
	HAL_FLASH_Unlock();
	erase_conf.TypeErase = FLASH_TYPEERASE_PAGES;
	erase_conf.PageAddress = addr;
	erase_conf.NbPages = 1;
	HAL_FLASHEx_Erase(&erase_conf, &error);

	/* Flash program */
	for (int i = 0; i<FLASH_PAGE_SIZE; i+=2) {
		val = *data;
		data++;
		val += (*data)<<8;
		data++;
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr+i, val);
	}
	HAL_FLASH_Lock();
	return HAL_OK;
}

void flash_read_page(uint32_t page, uint8_t *data)
{
	uint32_t addr;

	addr = FIRMWARE_BASE + page * FLASH_PAGE_SIZE;
	for (int i=0; i<FLASH_PAGE_SIZE; i++) {
		*data = *(__IO uint8_t *)(addr + i);
		data++;
	}
}

void user_config_update(struct hid_user_config *cfg)
{
	uint8_t flash_cfg[FLASH_PAGE_SIZE];
	uint8_t *data = flash_cfg;
	uint32_t error;
	uint32_t addr;
	uint16_t val;

	FLASH_EraseInitTypeDef erase_conf;

	flash_cfg[0] = cfg->autoload;
	flash_cfg[1] = cfg->dev_id;
	flash_cfg[2] = cfg->phy_id;
	flash_cfg[3] = cfg->type;
	flash_cfg[4] = cfg->clause;

	addr = USER_CFG_BASE;

	/* Flash erase  */
	HAL_FLASH_Unlock();
	erase_conf.TypeErase = FLASH_TYPEERASE_PAGES;
	erase_conf.PageAddress = addr;
	erase_conf.NbPages = 1;
	HAL_FLASHEx_Erase(&erase_conf, &error);

	/* Flash program */
	for (int i = 0; i<FLASH_PAGE_SIZE; i+=2) {
		val = *data;
		data++;
		val += (*data)<<8;
		data++;
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr+i, val);
	}
	HAL_FLASH_Lock();
}

uint8_t flash_read_u8(uint32_t addr)
{
	return  *(__IO uint8_t *)(FIRMWARE_BASE + addr);
}

uint16_t flash_read_u16(uint32_t addr)
{
	return  *(__IO uint16_t *)(FIRMWARE_BASE + addr);
}

uint32_t flash_read_u32(uint32_t addr)
{
	return  *(__IO uint32_t *)(FIRMWARE_BASE + addr);
}

uint64_t flas_read_u64(uint32_t addr)
{
	return  *(__IO uint64_t *)(FIRMWARE_BASE + addr);
}


