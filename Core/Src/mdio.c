#include "mdio.h"
#include "gpio.h"

#define HIGH                         GPIO_PIN_SET
#define LOW                          GPIO_PIN_RESET

#define MDIO_SET(X)                  HAL_GPIO_WritePin(MDIO_GPIO, MDIO_PIN, X)
#define MDIO_READ()                  HAL_GPIO_ReadPin(MDIO_GPIO, MDIO_PIN)
#define MDC_SET(X)                   HAL_GPIO_WritePin(MDC_GPIO, MDC_PIN, X)

#define MDIO_DIR_IN()                mdio_dir_input()
#define MDIO_DIR_OUT()               mdio_dir_output()

/* It's unpractical */
#define MDIO_WAIT()                  asm("nop")

/* mdio moudle */
enum mdio_clause_type clause_type;
union mdio_pkg mdio_pkg = {0};

uint8_t _phy_addr = 0;
uint8_t _dev_typ = 0;

static void mdio_preamble(void)
{
        uint8_t i = 32;

        MDIO_SET(HIGH);
        while (i--) {
                MDC_SET(LOW);
                MDIO_WAIT();
                MDC_SET(HIGH);
        }
}

/*
 * @brief: call this function will be sent an mdio packets
 *         don't care aboout clause 22 or clause 45 provisions.
 * @return: None
 */
static void mdio_write_byte(union mdio_pkg *pkg)
{
        uint32_t val, i = 32;

        val = pkg->val;
        while (i--) {
                MDC_SET(LOW);
                MDIO_SET((!!(val&0x80000000)));
                MDIO_WAIT();
                MDC_SET(HIGH);
                val<<=1;
        }
}

/* @brief: call this function will be sent an mdio packets
 *         don't care aboout clause 22 or clause 45 provisions.
 * @return: register value.
 */
static uint16_t mdio_read_byte(union mdio_pkg *pkg)
{
        uint32_t val, i;
        uint16_t ret;

        i = 16;
        val = pkg->val;
        while (i--) {
                MDC_SET(LOW);
                MDIO_SET((!!(val&0x80000000)));
                MDIO_WAIT();
                MDC_SET(HIGH);
                val<<=1;
        }
        /* TA time */
        MDIO_DIR_IN();
        /* Start receive data */
        i = 16;
        ret = 0;
        while (i--) {
                MDC_SET(LOW);
                val += MDIO_READ();
                MDIO_WAIT();
                MDC_SET(HIGH);
                ret<<=1;
        }
        MDIO_DIR_OUT();
        return ret;
}

void mdio_pkg_init(union mdio_pkg *pkg, enum mdio_clause_type type)
{
        switch (type) {
        case MDIO_22:
                pkg->ST = MDIO_22_ST;
                pkg->OP = MDIO_22_OP_WRITE;
                pkg->PHYADR = 0;
                pkg->DEVTYPE = 0;
                pkg->TA = 2;
                pkg->DATA = 0x00;
                break;
        case MDIO_45:
                pkg->ST = MDIO_45_ST;
                pkg->OP = MDIO_45_OP_WRITE;
                pkg->PHYADR = 0;
                pkg->DEVTYPE = 0;
                pkg->TA = 2;
                pkg->DATA = 0x00;
                break;
        }
}

void mdio_write_one_byte(union mdio_pkg *pkg)
{
        mdio_write_byte(pkg);
}

void mdio_read_one_byte(union mdio_pkg *pkg)
{
        mdio_read_byte(pkg);
}

void mdio_transfer(union mdio_pkg *pkg, uint8_t *buf, uint32_t len)
{
        while(len--) {
                pkg->DATA = *buf;
                buf++;
                mdio_write_byte(pkg);
        }
}

void mdio_receive(union mdio_pkg *pkg, uint8_t *buf, uint32_t len)
{
        while(len--) {
                *buf = mdio_read_byte(pkg);
                buf++;
        }
}

void mdio_module_init(void)
{
        clause_type = MDIO_22;
        mdio_goio_init();
        mdio_pkg_init(&mdio_pkg, clause_type);
}

uint16_t mdio_22_read(uint8_t phy, uint8_t reg)
{
        uint16_t data;

        mdio_pkg.ST = 1;
        mdio_pkg.OP = MDIO_22_OP_READ;
        mdio_pkg.PHYADR = phy;
        mdio_pkg.DEVTYPE = reg;
        mdio_preamble();
        data = mdio_read_byte(&mdio_pkg);
        debug_s("phy: 0x%02x, reg: 0x%02x, data: 0x%02x\r\n", phy, reg, data);
        return data;
}

void mdio_22_write(uint8_t phy, uint8_t reg, uint16_t val)
{
        mdio_pkg.ST = 1;
        mdio_pkg.OP = MDIO_22_OP_WRITE;
        mdio_pkg.PHYADR = phy;
        mdio_pkg.DEVTYPE = reg;
        mdio_pkg.DATA = val;
        mdio_preamble();
        mdio_write_byte(&mdio_pkg);
        debug_s("phy: 0x%02x, reg: 0x%02x, data: 0x%02x\r\n", phy, reg, val);
}

uint16_t mdio_45_read(uint8_t phy, uint8_t dev, uint16_t reg)
{
        uint16_t data;

        mdio_pkg.ST = 0;
        mdio_pkg.PHYADR = phy;
        mdio_pkg.DEVTYPE = dev;
        mdio_pkg.DATA = reg;

        /* write register address */
        mdio_pkg.OP = MDIO_45_OP_ADDRESS;
        mdio_preamble();
        mdio_write_byte(&mdio_pkg);

        /* read data */
        mdio_pkg.OP = MDIO_45_OP_READ;
        mdio_preamble();
        data = mdio_read_byte(&mdio_pkg);
        debug_s("phy: 0x%02x, reg: 0x%02x, dev: 0x%02x, data: 0x%02x\r\n", phy, reg, dev, data);
        return data;
}

void mdio_45_write(uint8_t phy, uint8_t dev, uint16_t reg, uint16_t val)
{
        mdio_pkg.ST = 0;
        mdio_pkg.PHYADR = phy;
        mdio_pkg.DEVTYPE = dev;
        mdio_pkg.OP = MDIO_45_OP_ADDRESS;

        /* write register address */
        mdio_pkg.DATA = reg;
        mdio_preamble();
        mdio_write_byte(&mdio_pkg);

        /* write register data */
        mdio_pkg.OP = MDIO_45_OP_WRITE;
        mdio_pkg.DATA = val;
        mdio_preamble();
        mdio_write_byte(&mdio_pkg);
        debug_s("phy: 0x%02x, reg: 0x%02x, dev: 0x%02x, data: 0x%02x\r\n", phy, reg, dev, val);
}

void mdio_format_data(uint8_t *src, uint8_t *tag, uint32_t *len, enum mdio_clause_type type)
{


}
