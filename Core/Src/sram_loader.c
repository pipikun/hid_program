#include "sram_loader.h"
#include "hid_cmd.h"
#include "main.h"
#include "mdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "mdio_spi.h"
#include "flash.h"

#define PHY_ADR		0
#define DEV_TYPE	0
#define BLOCK_SIZE	9
#define LOADE_SIZE	6
#define EXECUTE_CMD	0x8006
#define MAGIC		0xbeef937c
#define OFFSET		16

struct sram_loader_config cfg;

static inline void  _uint32_to_uint8(uint32_t *src, uint8_t *tag, uint16_t src_len)
{
	uint32_t data;

	for (int i=0; i<src_len; i++) {
		data = *src++;
		for (int j=0; j<4; j++) {
			*tag = (data>>24) & 0xff;
			 tag++;
			data<<=8;
		}
	}
}

static void __sram_loader_delay_us(uint32_t time)
{
	uint32_t i = time * 100;
	while(i--) {
		asm("nop");
	}
}

static inline void _mdio_22_write(uint8_t phy, uint8_t reg, uint16_t val)
{
	uint32_t src[2];
	uint8_t tag[8];

	src[0] = MDIO_PREAMBLE;
	src[1] = _gene_mdio_22_write(phy, reg, val);

	_uint32_to_uint8(src, tag, 2);

	/* spi transmit */
	spi_mdio_write(tag, 8);
}

static inline void _mdio_45_write(uint8_t phy, uint8_t reg, uint16_t val)
{
	uint32_t src[2];
	uint8_t tag[8];

	src[0] = MDIO_PREAMBLE;
	src[1] = _gene_mdio_45_write(phy, reg, val);

	_uint32_to_uint8(src, tag, 2);

	/* spi transmit */
	spi_mdio_write(tag, 8);
}

static inline uint16_t _mdio_22_read(uint8_t phy, uint8_t reg)
{
	uint32_t src[2];
	uint8_t tag[8];
	uint8_t ret[8];

	src[0] = MDIO_PREAMBLE;
	src[1] = _gene_mdio_22_read(phy, reg);

	_uint32_to_uint8(src, tag, 2);

	/* spi transmit */
	spi_mdio_get(tag,ret, 8);

	return (ret[6]<<8)|(ret[7]);
}

static inline void _sram_load_22(uint16_t *buf, uint16_t len)
{
	uint16_t size, data_size;
	uint32_t *src;
	uint8_t *tag;

	size = len * 2;
	data_size = len * 8;

	src = (uint32_t *)malloc(size * sizeof(uint32_t));
	tag = (uint8_t *)malloc(data_size * sizeof(uint8_t));

	if (src == NULL || tag == NULL) {
		Error_Handler();
	}

	for (int i=0,idx=0; i<len-1; i++,idx+=2) {
		src[idx] = MDIO_PREAMBLE;
		src[idx+1] = _gene_mdio_22_write(PHY_ADR, i+1, buf[i]);
	}
	src[size-2] = MDIO_PREAMBLE;
	src[size-1] = _gene_mdio_22_write(PHY_ADR, 0, buf[len-1]);

	/* uint32_t to uint8_t */
	_uint32_to_uint8(src, tag, size);

	/* spi transmit */
	spi_mdio_write(tag, data_size);

	free(src);
	free(tag);
}

static inline void _sram_load_45(uint16_t *buf, uint16_t len)
{
	uint16_t size, data_size;
	uint32_t *src;
	uint8_t *tag;

	size = len * 4;
	data_size = len * 16;

	src = (uint32_t *)malloc(size * sizeof(uint32_t));
	tag = (uint8_t *)malloc(data_size * sizeof(uint8_t));

	if (src == NULL || tag == NULL) {
		Error_Handler();
	}

	for (int i=0,idx=0; i<len-1; i++,idx+=4) {
		src[idx] = MDIO_PREAMBLE;
		src[idx+1] = _gene_mdio_45_addr(PHY_ADR, DEV_TYPE, i+1);
		src[idx+2] = MDIO_PREAMBLE;
		src[idx+3] = _gene_mdio_45_write(PHY_ADR, DEV_TYPE , buf[i]);
	}
	src[size-4] = MDIO_PREAMBLE;
	src[size-3] = _gene_mdio_45_addr(PHY_ADR, DEV_TYPE, 0);
	src[size-2] = MDIO_PREAMBLE;
	src[size-1] = _gene_mdio_45_write(PHY_ADR, DEV_TYPE , buf[len-1]);

	/* uint32_t to uint8_t */
	_uint32_to_uint8(src, tag, size);

	/* spi transmit */
	spi_mdio_write(tag, data_size);

	free(src);
	free(tag);
}

static inline void _get_header(struct sram_loader_config *cfg)
{
	cfg->code = flash_read_u32(0);
	cfg->len = flash_read_u16(4);
	cfg->offset = flash_read_u32(6);
	cfg->pc = flash_read_u16(10);
	cfg->opt = flash_read_u16(12);
}

static inline void __flash_load_start(void)
{
	// set_load_data0 = 0
	_mdio_22_write(0, 31, 0x85);
	_mdio_22_write(0, 1, 0);

	// set_load_data0 = 0xe0c6
	_mdio_22_write(0, 31, 0x85);
	_mdio_22_write(0, 1, 0xe0c6);

	// set_rstn_cpu(0)
	_mdio_22_write(0, 31, 0x80);
	_mdio_22_write(0, 9, 0xfff7);
	__sram_loader_delay_us(1000);

	// set_rstn_cpu(1)
	_mdio_22_write(0, 31, 0x80);
	_mdio_22_write(0, 9, 0xffff);
	__sram_loader_delay_us(1000);

	// set_page()
	_mdio_22_write(0, 31, 0x85);
}

static inline void __flash_load_stop(void)
{
	// set_load_data0 = 0
	_mdio_22_write(0, 31, 0x85);
	_mdio_22_write(0, 1, 0);

	// set_load_data1 = 0
	_mdio_22_write(0, 31, 0x85);
	_mdio_22_write(0, 2, 0);

	// set_load_data_go = 0x4000
	_mdio_22_write(0, 31, 0x85);
	_mdio_22_write(0, 0, 0x4000);
}

static inline void __copy_to_sram(struct sram_loader_config *cfg, uint32_t ram_addr, uint32_t addr)
{
	uint16_t block[BLOCK_SIZE];

	block[0] = ram_addr>>16;
	block[1] = ram_addr&0xffff;

	for (int i=2, idx=0; i<BLOCK_SIZE-1; i++, idx+=2) {
		block[i] = flash_read_u16(idx + addr);
	}
	block[8] = EXECUTE_CMD;

	if (cfg->clause == MDIO_22) {
		_sram_load_22(block, BLOCK_SIZE);
	} else if (cfg->clause == MDIO_45) {
		_sram_load_45(block, BLOCK_SIZE);
	}
}

static void __flash_loader(struct sram_loader_config *cfg)
{
	uint32_t sram_addr, flash_addr, len, block;

	_get_header(cfg);

	if (cfg->code != MAGIC) {
	       cfg->state = LOADER_CODE_ERROR;
	       return;
	}

	if (cfg->len > FIRMWARE_SIZE) {
		cfg->state = LOADER_LEN_ERROR;
		return;
	}

	flash_addr = OFFSET;
	sram_addr =  0;
	block = 0;
	len = (cfg->len-16-4);

	while (true) {
		__copy_to_sram(cfg, sram_addr, flash_addr);
		sram_addr += (LOADE_SIZE * 2);
		flash_addr += (LOADE_SIZE * 2);
		block++;
		if (sram_addr >= len) {
			cfg->state = LOADER_DONE;
			break;
		}
		__sram_loader_delay_us(1); // about 45us
	}

	cfg->block = block;
	if (sram_addr > len) {
		cfg->state = LOADER_BIN_ERROR;
		return;
	}
}

void sram_loader(struct sram_loader_config *cfg)
{
	__flash_load_start();
	__flash_loader(cfg);
	__flash_load_stop();
}

