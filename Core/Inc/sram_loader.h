#ifndef __SRAM_LOADER_H__
#define __SRAM_LOADER_H__

#include "main.h"

enum sram_state {
	LOADER_DONE = 1,
	LOADER_CODE_ERROR,
	LOADER_LEN_ERROR,
	LOADER_FLASH_ERROR,
	LOADER_BIN_ERROR,
};

struct sram_loader_config
{
	enum sram_state state;
	uint16_t len;
	uint16_t pc;
	uint32_t offset;
	uint32_t code;
	uint16_t opt;
	uint32_t block;
};

uint32_t _gene_mdio_22_write(uint8_t phy, uint8_t reg, uint16_t val);
uint32_t _gene_mdio_22_read(uint8_t phy, uint8_t reg);

uint32_t _gene_mdio_45_write(uint8_t phy, uint8_t dev, uint16_t val);
uint32_t _gene_mdio_45_read(uint8_t phy, uint8_t dev, uint8_t reg);
uint32_t _gene_mdio_45_addr(uint8_t phy, uint8_t dev, uint16_t reg);

void gene_mdio_22_transmit_package(uint8_t *src, uint16_t *tag, uint16_t len);
void sram_loader(struct sram_loader_config *cfg);


#endif /*__SRAM_LOADER_H__ */
//vim: ts=8 sw=8 noet autoindent:
