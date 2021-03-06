#include "hid_cmd.h"
#include "i2c.h"
#include "mdio.h"
#include "spi.h"
#include "mdio_spi.h"
#include "flash.h"
#include "sram_loader.h"

struct i2c_param i2c_cfg;
struct i2c_download i2c_dwn;
struct mdio_param mdio_cfg;
struct mdio_download mdio_dwn;
struct sram_loader_config sram_cfg;

uint8_t fw_buf[FIRMWARE_SIZE] = {0};

void hid_init(void)
{
	// auto reload dev_sel
	spi_dev_sel(USER_CFG->dev_id, USER_CFG->clause);
	sram_cfg.clause = USER_CFG->clause;
	mdio_cfg.clause = USER_CFG->clause;

	// auto load firmware to sram.
	if (USER_CFG->autoload == 1) {
		sram_loader(&sram_cfg);
	}
}

static void hid_i2c_firmware_download(void)
{
	uint16_t page, less, addr, idx = 0;

	page = (uint16_t)(i2c_dwn.size/DEV_PAGE_SIZE);
	addr = i2c_dwn.first_addr;

	/* hand */
	less = DEV_PAGE_SIZE - (addr % DEV_PAGE_SIZE);
	if (less != DEV_PAGE_SIZE) {
		i2c_write_data(addr, less, &fw_buf[idx]);
		idx = less;
		addr += less;
	}

	/* body */
	for (int i=0; i< page; i++) {
		eeprom_write_page(addr, &fw_buf[idx]);
		idx += DEV_PAGE_SIZE;
		addr += DEV_PAGE_SIZE;
	}

	/* tial */
	if (idx < i2c_dwn.size) {
		less = i2c_dwn.size - idx;
		i2c_write_data(addr, less, &fw_buf[idx]);
	}
}

static void hid_i2c_config(uint8_t *buf)
{
	uint16_t addr = 0;

	i2c_cfg.dev = buf[3];
	i2c_cfg.addr_width = buf[4];
	addr = buf[5];
	addr<<=8;
	addr+=buf[6];
	i2c_cfg.addr = addr;
	i2c_set_dev(i2c_cfg.dev);
}

static void hid_i2c_write(uint8_t *buf)
{
	uint8_t len;

	len = buf[3];
	/* just support 64 bytes align */
	i2c_write_data(i2c_cfg.addr, len, &buf[4]);
}

static void hid_i2c_read(uint8_t *buf)
{
	uint8_t len;
	uint16_t addr;
	uint8_t *dat;

	len = buf[3];
	addr = i2c_cfg.addr;

	dat = &buf[4];
	for (int i=0; i<len; i++) {
		*dat = eeprom_read_uint8(addr + i);
		dat++;
	}
}

static void hid_i2c_download_cfg(uint8_t *buf)
{
	uint16_t size;

	size = buf[3];
	size<<=8;
	size+= buf[4];

	i2c_dwn.size = size;
	i2c_dwn.idx = 0;
	i2c_dwn.first_addr = i2c_cfg.addr;
}

static uint8_t hid_i2c_download(uint8_t *buf)
{
	uint8_t len;

	len = buf[3];
	for (int i=4; i<(len+4); i++) {
		fw_buf[i2c_dwn.idx] = buf[i];
		i2c_dwn.idx++;
	}

	/* download */
	if (i2c_dwn.idx >= i2c_dwn.size) {
		hid_i2c_firmware_download();
		buf[0] = CMD_I2C_DOWNLOAD + 1;
		buf[2] = 0xff;
		return 0;
	}
	return 2;
}

static void hid_mdio_download_cfg(uint8_t *buf)
{
	uint32_t addr, size;

	addr = buf[3];
	addr <<= 8;
	addr += buf[4];

	size = buf[5];
	size <<=24;
	size += buf[6];
	size <<=16;
	size += buf[7];
	size <<=8;
	size += buf[8];

	mdio_dwn.size = size;
	mdio_dwn.addr = addr;
	mdio_dwn.type = buf[9];
	mdio_dwn.idx = 0;

	/* enable buf chip */
	spi_mdio_enable(&status);

	buf[2] = 0xcc;
}

static void hid_flash_download(uint8_t *buf, uint8_t len)
{
	uint16_t idx = mdio_dwn.idx;

	for (int i=0; i<len; i++) {
		fw_buf[idx+i] = buf[i];
	}
}

static void hid_flash_firmware_save(uint8_t *buf)
{
	uint32_t len, page, less;

	len = mdio_dwn.size;
	page = (int)(len/FLASH_PAGE_SIZE);
	less = (int)(len%FLASH_PAGE_SIZE);
	if (less > 0) page++;
	for (int i=0; i<page; i++) {
		firmware_save(i, &fw_buf[i*FLASH_PAGE_SIZE]);
	}

	/* load to sram */
	sram_loader(&sram_cfg);
}

static uint8_t hid_mdio_download(uint8_t *buf)
{
	uint8_t len;

	len = buf[3];

	/* sram download  */
	switch (mdio_dwn.type) {
	case SRAM_FW:
		spi_mdio_write_fs(&buf[4], len);
		break;
	case FLASH_FW:
		hid_flash_download(&buf[4], len);
		break;
	default:
		break;
	}
	mdio_dwn.idx += len;
	if (mdio_dwn.idx >= mdio_dwn.size) {
		buf[0] = CMD_MDIO_DOWNLOAD + 1;
		buf[1] = 0;
		buf[2] = 0xAA;
		if (mdio_dwn.type == FLASH_FW) {
			hid_flash_firmware_save(buf);
		} else {
			spi_mdio_disable(&status);
		}
		return 0;
	}
	return 2;
}

static void hid_mdio_cfg(uint8_t *buf)
{
	uint8_t clause, dev_sel;

	clause = buf[3];
	dev_sel = buf[4];

	spi_dev_sel(dev_sel, clause);
	sram_cfg.clause = clause;
	buf[2] = 0xaa;
}

static void hid_mdio_read(uint8_t *buf)
{
	spi_mdio_read(buf);
}

static void hid_mdio_write(uint8_t *buf)
{
	spi_mdio_send(buf);
}

static void hid_mcu_config(uint8_t *buf)
{
	uint8_t cmd, val;

	struct hid_user_config cfg = {
		.autoload = USER_CFG->autoload,
		.type = USER_CFG->type,
		.phy_id = USER_CFG->phy_id,
		.dev_id = USER_CFG->dev_id,
		.clause = USER_CFG->clause
	};

	cmd = buf[3];
	val = buf[4];

	switch (cmd) {
	case 1:
		cfg.autoload = val;
		break;
	case 2:
		cfg.dev_id = val;
		break;
	case 3:
		cfg.phy_id = val;
		break;
	case 4:
		cfg.type = val;
		break;
	case 5:
		cfg.clause = val;
		break;
	default:
		buf[1] = 0x01;
		break;
	}

	user_config_update(&cfg);
}

static void hid_mcu_dump(uint8_t *buf)
{
	uint32_t code;

	/* dump load info */
	buf[3] = sram_cfg.state;

	code = sram_cfg.code;
	for (uint32_t i=0; i<4; i++) {
		buf[4+i] = code & 0xff;
		code >>=8;
	}

	buf[8] = sram_cfg.len>>8;
	buf[9] = sram_cfg.len&0xff;

	buf[10] = sram_cfg.opt>>8;
	buf[11] = sram_cfg.opt&0xff;

	buf[12] = USER_CFG->autoload;
	buf[13] = USER_CFG->type;
	buf[14] = USER_CFG->phy_id;
	buf[15] = USER_CFG->dev_id;
	buf[16] = USER_CFG->clause;
	buf[17] = sram_cfg.clause;
}

uint8_t hid_cmd_entry(uint8_t *buf)
{
	uint8_t cmd = buf[0];
	uint8_t ret = 0;

	switch (cmd) {
	case CMD_I2C_CONFIG:
		hid_i2c_config(buf);
		break;
	case CMD_I2C_WRITE:
		hid_i2c_write(buf);
		break;
	case CMD_I2C_READ:
		hid_i2c_read(buf);
		break;
	case CMD_I2C_DOWNLOAD_CFG:
		hid_i2c_download_cfg(buf);
		break;
	case CMD_I2C_DOWNLOAD:
		ret = hid_i2c_download(buf);
		break;
	case CMD_MDIO_CFG:
		hid_mdio_cfg(buf);
		break;
	case CMD_MDIO_WRITE:
		hid_mdio_write(buf);
		break;
	case CMD_MDIO_READ:
		hid_mdio_read(buf);
		break;
	case CMD_MDIO_DOWNLOAD_CFG:
		hid_mdio_download_cfg(buf);
		break;
	case CMD_MDIO_DOWNLOAD:
		ret = hid_mdio_download(buf);
		break;
	case CMD_MCU_CONFIG:
		hid_mcu_config(buf);
		break;
	case CMD_MCU_DUMP:
		hid_mcu_dump(buf);
		break;
	default:
		break;
	}
	return ret;
}
