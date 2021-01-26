#include "mdio_spi.h"

struct spi_mdio_status status={.dev=0, .hspi=&hspi1};

static void spi_mdio_dev_sel(struct spi_mdio_status *status, uint8_t dev)
{
        switch (dev) {
        case 0:
                status->dev = 0;
                status->hspi = &hspi1;
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
                break;
        case 1:
                status->dev = 1;
                status->hspi = &hspi2;
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
                HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
                break;
        default:
                break;
        }
}

void spi_mdio_enable(struct spi_mdio_status *status)
{
        switch (status->dev) {
        case PHY1:
                HAL_GPIO_WritePin(PHY1_BUF_OE_GPIO_Port, PHY1_BUF_OE_Pin, GPIO_PIN_RESET);
                break;
        case PHY2:
                HAL_GPIO_WritePin(PHY2_BUF_OE_GPIO_Port, PHY2_BUF_OE_Pin, GPIO_PIN_RESET);
                break;
        }
}

void spi_mdio_disable(struct spi_mdio_status *status)
{
        switch (status->dev) {
        case PHY1:
                HAL_GPIO_WritePin(PHY1_BUF_OE_GPIO_Port, PHY1_BUF_OE_Pin, GPIO_PIN_SET);
                break;
        case PHY2:
                HAL_GPIO_WritePin(PHY2_BUF_OE_GPIO_Port, PHY2_BUF_OE_Pin, GPIO_PIN_SET);
                break;
        }
}

void spi_mdio_config(uint8_t *buf)
{
        status.clause = buf[3];
        spi_mdio_dev_sel(&status, buf[4]);
        /* config success. */
        buf[2] = 0xaa;
}

#define TIMEOUT_1_MS    10
#define DATA_START      3
void spi_mdio_send(uint8_t *buf)
{
        uint8_t len = buf[1];
        uint8_t rec_buf[64];

        /* clear buf */
        for (int i=0; i<64; i++) {
                rec_buf[i] = 0;
        }
        /* enable buf chip */
        spi_mdio_enable(&status);
        /*transmit and receive data */
        HAL_SPI_TransmitReceive(status.hspi, &buf[DATA_START], &rec_buf[DATA_START], len, TIMEOUT_1_MS);
        /* return data */
        for (int i=DATA_START; i<64; i++) {
                buf[i] = rec_buf[i];
        }
        /* compatibility mcp2210 */
        buf[1] = 0;
        buf[2] = len;
        /* disable buf chip */
        spi_mdio_disable(&status);
}

/*
 * brief: This is fast mode, don't care about 'MISO'.
 */
void spi_mdio_write(uint8_t *buf, uint8_t len)
{
        /* enable buf chip */
        spi_mdio_enable(&status);

        /*transmit data */
        HAL_SPI_Transmit(status.hspi, buf, len, TIMEOUT_1_MS);

        /* disable buf chip */
        spi_mdio_disable(&status);
}

/*
 * brief: This is fast mode.
 */
void spi_mdio_get(uint8_t *src, uint8_t *tag, uint8_t len)
{
	/* enable buf chip */
        spi_mdio_enable(&status);

        /*transmit data */
        HAL_SPI_TransmitReceive(status.hspi, src, tag, len, TIMEOUT_1_MS);

        /* disable buf chip */
        spi_mdio_disable(&status);
}

/*
 * brief: This is fast mode, don't care about 'MISO'.
 */
void spi_mdio_write_fs(uint8_t *buf, uint8_t len)
{
        uint8_t rpl[128];
        uint8_t i, idx;
        uint8_t size;

        /* reset */
        for (i=0; i<128; i++) {
                rpl[i] = 0xff;
        }

        idx = 4;
        /* replance */
        for (i=0; i<len; i+=4) {
                rpl[idx]   = buf[i];
                rpl[idx+1] = buf[i+1];
                rpl[idx+2] = buf[i+2];
                rpl[idx+3] = buf[i+3];
                idx += 8;
        }
        /* double length */
        size = len * 2;
        spi_mdio_write(rpl, size);
        buf[2] = 0x55;
}

void spi_mdio_read(uint8_t *buf)
{
        uint8_t len = buf[1];
        uint8_t rec_buf[64];

        /* clear buf */
        for (int i=0; i<64; i++) {
                rec_buf[i] = 0;
        }
        /* enable buf chip */
        spi_mdio_enable(&status);
        /*transmit and receive data */
        HAL_SPI_TransmitReceive(status.hspi, &buf[DATA_START], &rec_buf[DATA_START], len, TIMEOUT_1_MS);
        /* return data */
        for (int i=DATA_START; i<64; i++) {
                buf[i] = rec_buf[i];
        }
        /* compatibility mcp2210 */
        buf[1] = 0;
        buf[2] = len;
        /* disable buf chip */
        spi_mdio_disable(&status);
}
