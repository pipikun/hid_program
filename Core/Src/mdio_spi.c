#include "mdio_spi.h"

struct spi_mdio_status status;

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

static void spi_mdio_enable(struct spi_mdio_status *status)
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

static void spi_mdio_disable(struct spi_mdio_status *status)
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

#define TIMEOUT_10_MS   10
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
        HAL_SPI_TransmitReceive(status.hspi, &buf[DATA_START], &rec_buf[DATA_START], len, TIMEOUT_10_MS);
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
 * breaf: This is fast mode, don't care about 'MISO'.
 */
void spi_mdio_write(uint8_t *buf)
{
        uint8_t len = buf[1];

        /* enable buf chip */
        spi_mdio_enable(&status);
        /*transmit and receive data */
        HAL_SPI_Transmit(status.hspi, &buf[DATA_START], len, TIMEOUT_10_MS);
        /* compatibility mcp2210 */
        buf[1] = 0;
        buf[2] = len;
        /* disable buf chip */
        spi_mdio_disable(&status);
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
        HAL_SPI_TransmitReceive(status.hspi, &buf[DATA_START], &rec_buf[DATA_START], len, TIMEOUT_10_MS);
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
