#include "hid_cmd.h"
#include "i2c.h"
#include "mdio.h"
#include "spi.h"

struct i2c_param i2c_cfg;
struct i2c_download i2c_dwn;
struct mdio_param mdio_cfg;

uint8_t fw_buf[FIRMWARE_SIZE] = {0};

static void hid_firmware_download(void)
{
        uint16_t page, less, addr, idx = 0;

        page = (uint16_t)(i2c_dwn.size/PAGE_SIZE);
        addr = i2c_dwn.first_addr;

        /* hand */
        less = PAGE_SIZE - (addr % PAGE_SIZE);
        if (less != PAGE_SIZE) {
                i2c_write_data(addr, less, &fw_buf[idx]);
                idx = less;
                addr += less;
        }

        /* body */
        for (int i=0; i< page; i++) {
                eeprom_write_page(addr, &fw_buf[idx]);
                idx += PAGE_SIZE;
                addr += PAGE_SIZE;
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
                hid_firmware_download();
                buf[0] = CMD_I2C_DOWNLOAD + 1;
                buf[2] = 0xff;
                return 0;
        }
        return 2;
}

static void hid_mdio_cfg(uint8_t *buf)
{
        mdio_cfg.clause = buf[3];
        mdio_cfg.op = buf[4];
}

static void hid_mdio_write(uint8_t *buf)
{
        uint8_t phy = buf[3];
        uint8_t reg = buf[4];
        uint16_t data = 0;
        uint16_t data_45;

        data = buf[5];
        data<<=8;
        data += buf[6];

        switch (mdio_cfg.clause) {
        case 0x22:
                mdio_22_write(phy, reg, data);
                break;
        case 0x45:
                data_45 = buf[7];
                data_45 <<= 8;
                data_45 += buf[8];
                mdio_45_write(phy, reg, data, data_45);
                break;
        default:
                buf[2] = 0x33;
                break;
        }
}

static void hid_mdio_read(uint8_t *buf)
{
        uint8_t phy = buf[3];
        uint8_t reg = buf[4];
        uint16_t data;

        data = buf[5];
        data<<=8;
        data += buf[6];

        switch (mdio_cfg.clause) {
        case 0x22:
                data = mdio_22_read(phy, reg);
                break;
        case 0x45:
                data = mdio_45_read(phy, reg, data);
                break;
        default:
                buf[2] = 0x33;
                break;
        }
        buf[6] = data>>8;
        buf[7] = data & 0xff;
}

void hid_mcp2210_rw(uint8_t *buf)
{
        uint8_t hand;

        hand = buf[8];
        hand = hand & 0x00;
}

void hid_mcp2210_45_rw(uint8_t *buf)
{
        uint8_t len = buf[1];
        uint8_t rec[64]={0};

        for (int j=0; j<64; j++) {
                rec[j] = 0;
        }
        HAL_SPI_TransmitReceive(&hspi1, &buf[4], &rec[4], len, 10);
        for (int i=4; i<64; i++) {
                buf[i] = rec[i];
        }
        buf[1] = 0;
        buf[2] = len;
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
        case CMD_MCP2210_RW:
                hid_mcp2210_45_rw(buf);
                break;
        default:
                break;
        }
        return ret;
}
