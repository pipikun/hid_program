#ifndef __MDIO_H__
#define __MDIO_H__

#include "main.h"

#define MDIO_22_CLAUSE
#define MDIO_45_CLAUSE

#define MDIO_22_ST              1
#define MDIO_22_OP_WRITE        0x01
#define MDIO_22_OP_READ         0x02
#define MDIO_22_TA              0x02

#define MDIO_45_ST              0
#define MDIO_45_OP_ADDRESS      0x00
#define MDIO_45_OP_WRITE        0x01
#define MDIO_45_OP_READ         0x03
#define MDIO_45_OP_INC_ADDR     0x02
#define MDIO_45_TA              0x02

#define MDIO_PREAMBLE		(0xffffffff)

enum mdio_clause_type {
        MDIO_45 = 0,
        MDIO_22,
};

enum mdio_op_type {
        ADDRESS = 0,
        WRITE,
        READ,
        INC_ADDR,
};

enum mdio_dev_type {
        RESERVER = 0,
        PMA_OR_PMD,
        WIS,
        PCS,
        PHY_XS,
        DTE_XS,
};

union mdio_pkg {
        struct {
                uint32_t DATA:16;
                uint32_t TA:2;
                uint32_t DEVTYPE:5;
                uint32_t PHYADR:5;
                uint32_t OP:2;
                uint32_t ST:2;
        };
        uint32_t val;
};

void mdio_transfer(union mdio_pkg *pkg, uint8_t *buf, uint32_t len);
void mdio_receive(union mdio_pkg *pkg, uint8_t *buf, uint32_t len);

void mdio_pkg_init(union mdio_pkg *pkg, enum mdio_clause_type type);
void mdio_write_one_byte(union mdio_pkg *pkg);
void mdio_read_one_byte(union mdio_pkg *pkg);
void spi_mdio_get(uint8_t *src, uint8_t *tag, uint8_t len);

void mdio_module_init(void);

void mdio_45_write(uint8_t phy, uint8_t dev, uint16_t reg, uint16_t val);
uint16_t mdio_45_read(uint8_t phy, uint8_t dev, uint16_t reg);
void mdio_22_write(uint8_t phy, uint8_t reg, uint16_t val);
uint16_t mdio_22_read(uint8_t phy, uint8_t reg);
#endif /*__MDIO_H__ */
//vim: ts=8 sw=8 noet autoindent:
