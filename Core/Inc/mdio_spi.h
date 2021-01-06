#ifndef __MDIO_SPI_H__
#define __MDIO_SPI_H__

#include "main.h"
#include "spi.h"

enum phy_dev
{
        PHY1 = 0,
        PHY2,
};

enum clause_sel
{
        CLASUE_45 = 0,
        CLASUE_22 = 1,
};

struct spi_mdio_status {
        enum phy_dev dev;
        enum clause_sel clause;

        SPI_HandleTypeDef *hspi;
};

void spi_mdio_send(uint8_t *buf);
void spi_mdio_config(uint8_t *buf);
void spi_mdio_write(uint8_t *buf, uint8_t len);
void spi_mdio_read(uint8_t *buf);

#endif /*__MDIO_SPI_H__ */
//vim: ts=8 sw=8 noet autoindent:
