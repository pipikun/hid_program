#ifndef __HID_CMD_H__
#define __HID_CMD_H__

#include "main.h"

#define CMD_I2C_CONFIG                  0x20
#define CMD_I2C_WRITE                   0x21
#define CMD_I2C_READ                    0x22
#define CMD_I2C_DOWNLOAD_CFG            0x24
#define CMD_I2C_DOWNLOAD                0x25

#define CMD_MDIO_CFG                    0x30
#define CMD_MDIO_WRITE                  0x31
#define CMD_MDIO_READ                   0x32

#define CMD_MDIO_DOWNLOAD_CFG           0x34
#define CMD_MDIO_DOWNLOAD               0x35

#define CMD_MCU_CONFIG                  0x40
#define CMD_MCU_DUMP                    0x41

#define FIRMWARE_SIZE                   (64 * 1024)

// erom page size
#define DEV_PAGE_SIZE                   (64)

enum mdio_down_type {
        SRAM_FW = 0,
        FLASH_FW,
};

struct i2c_param {
        uint16_t addr;
        uint8_t dev;
        uint8_t addr_width;
        uint8_t page_size;
};

struct i2c_download {
        uint16_t size;
        uint16_t first_addr;
        uint16_t idx;
};

struct mdio_param {
        uint8_t clause;
        uint8_t op;
};

struct mdio_download {
        uint32_t size;
        uint32_t addr;
        uint16_t idx;
        uint8_t type;
};

uint8_t hid_cmd_entry(uint8_t *buf);

#endif /*__HID_CMD_H__ */
//vim: ts=8 sw=8 noet autoindent:
