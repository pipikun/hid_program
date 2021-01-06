#include "stdint.h"
#include <stdio.h>
#include <stdlib.h>
#include "i2c.h"
#include "gpio.h"
#include "config.h"

#define i2c_sda_set(x)			HAL_GPIO_WritePin(I2C_SDA_GPIO, I2C_SDA_PIN, x)
#define i2c_sda_dir_in()		i2c_gpio_dir_input()
#define i2c_sda_dir_out()		i2c_gpio_dir_output()
#define i2c_sda_get()			HAL_GPIO_ReadPin(I2C_SDA_GPIO, I2C_SDA_PIN)
#define i2c_scl_set(x)			HAL_GPIO_WritePin(I2C_SCL_GPIO,  I2C_SCL_PIN, x)

#define I2C_HIGH			GPIO_PIN_SET
#define I2C_LOW				GPIO_PIN_RESET
#define I2C_PAGE_SIZE			64

#define I2C_DELAY_TIME			0

#define UINT16_H(X)			(X >> 8)
#define UINT16_L(X)			(X & 0x00ff)

uint8_t I2C_MSB_W = 0xa0;
uint8_t I2C_MSB_R = 0xa1;

void i2c_delay_us(uint16_t time)
{
        time = 3;
        while(time--) {
                asm("nop");
        }
}

void i2c_delay_ms(uint16_t time)
{
        for (int i=0; i<time; i++) {
                for (int j=0; j<8000; j++) {
                        asm("nop"); asm("nop");
                        asm("nop"); asm("nop");
                }
        }
}

void i2c_start(void)
{
        i2c_sda_set(I2C_HIGH);
        i2c_scl_set(I2C_HIGH);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_sda_set(I2C_LOW);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_scl_set(I2C_LOW);
        i2c_delay_us(I2C_DELAY_TIME);
}

void i2c_stop(void)
{
        i2c_sda_set(I2C_LOW);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_scl_set(I2C_HIGH);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_sda_set(I2C_HIGH);
        i2c_delay_us(I2C_DELAY_TIME);
}

void i2c_ack(void)
{
        i2c_sda_set(I2C_LOW);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_scl_set(I2C_HIGH);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_scl_set(I2C_LOW);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_sda_set(I2C_HIGH);
}

void i2c_no_ack(void)
{
        i2c_sda_set(I2C_HIGH);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_scl_set(I2C_HIGH);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_scl_set(I2C_LOW);
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_scl_set(I2C_HIGH);
}

uint8_t i2c_wait_ack(void)
{
        uint8_t ack;

        i2c_sda_set(I2C_HIGH);
        i2c_sda_dir_in();
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_scl_set(I2C_HIGH);
        i2c_delay_us(I2C_DELAY_TIME);
        ack = i2c_sda_get();
        i2c_sda_dir_out();
        i2c_delay_us(I2C_DELAY_TIME);
        i2c_scl_set(I2C_LOW);

        return ack;
}

void i2c_send_byte(uint8_t byte)
{
        int i=8;
        uint8_t data, bit;

        data = byte;
        while (i--) {
                bit = !!(data & 0x80);
                i2c_sda_set(bit);
                i2c_delay_us(I2C_DELAY_TIME);
                i2c_scl_set(I2C_HIGH);
                i2c_delay_us(I2C_DELAY_TIME);
                i2c_scl_set(I2C_LOW);
                i2c_delay_us(I2C_DELAY_TIME);
                data <<= 1;
        }
}

uint8_t i2c_receive_byte(uint8_t ack)
{
        uint8_t data = 0, i=8;

        i2c_sda_dir_in();
        while (i--) {
                data <<=1;
                i2c_scl_set(I2C_HIGH);
                i2c_delay_us(I2C_DELAY_TIME);
                data += i2c_sda_get();
                i2c_scl_set(I2C_LOW);
                i2c_delay_us(I2C_DELAY_TIME);
        }
        i2c_sda_dir_out();
        if (ack == 0)
                i2c_no_ack();
        else
                i2c_ack();

        return data;
}

void i2c_module_init(void)
{
        i2c_gpio_init();
}

void eeprom_read_page(uint16_t addr, uint8_t *data)
{
        uint8_t i;

        i2c_start();
        i2c_send_byte(I2C_MSB_W);
        i2c_wait_ack();
        i2c_send_byte(UINT16_H(addr));
        i2c_wait_ack();
        i2c_send_byte(UINT16_L(addr));
        i2c_wait_ack();
        i2c_stop();

        i2c_start();
        i2c_send_byte(I2C_MSB_R);
        i2c_wait_ack();

        /* read a page */
        i = I2C_PAGE_SIZE - 1;
        while (i--) {
                *data = i2c_receive_byte(1);
                data++;
        }
        /* receive the last byte and noack */
        *data = i2c_receive_byte(0);
        i2c_stop();
}

void eeprom_write_page(uint16_t addr, uint8_t *data)
{
        uint8_t i;

        i2c_start();
        i2c_send_byte(I2C_MSB_W);
        i2c_wait_ack();
        i2c_send_byte(UINT16_H(addr));
        i2c_wait_ack();
        i2c_send_byte(UINT16_L(addr));
        i2c_wait_ack();

        i = I2C_PAGE_SIZE;
        while (i--) {
                i2c_send_byte(*data);
                i2c_wait_ack();
                data++;
        }
        i2c_stop();
        i2c_delay_ms(6);
}

void eeprom_write_one(uint16_t addr, uint8_t data)
{
        i2c_start();
        i2c_send_byte(I2C_MSB_W);
        i2c_wait_ack();
        i2c_send_byte(UINT16_H(addr));
        i2c_wait_ack();
        i2c_send_byte(UINT16_L(addr));
        i2c_wait_ack();
        i2c_send_byte(data);
        i2c_wait_ack();
        i2c_stop();
        i2c_delay_ms(6);
}

uint8_t eeprom_read_uint8(uint16_t addr)
{
        uint8_t data;

        i2c_start();
        i2c_send_byte(I2C_MSB_W);
        i2c_wait_ack();
        i2c_send_byte(UINT16_H(addr));
        i2c_wait_ack();
        i2c_send_byte(UINT16_L(addr));
        i2c_wait_ack();
        i2c_stop();

        i2c_start();
        i2c_send_byte(I2C_MSB_R);
        i2c_wait_ack();

        data = i2c_receive_byte(0);
        i2c_stop();

        return data;
}

uint16_t eeprom_read_uint16(uint16_t addr)
{
        uint16_t data;

        i2c_start();
        i2c_send_byte(I2C_MSB_W);
        i2c_wait_ack();
        i2c_send_byte(UINT16_H(addr));
        i2c_wait_ack();
        i2c_send_byte(UINT16_L(addr));
        i2c_wait_ack();
        i2c_stop();

        i2c_start();
        i2c_send_byte(I2C_MSB_R);
        i2c_wait_ack();

        data = i2c_receive_byte(1);
        data <<= 8;
        data |= i2c_receive_byte(0);
        i2c_stop();

        return data;
}


uint32_t eeprom_read_uint32(uint32_t addr)
{
        uint32_t data;
        uint8_t i;

        i2c_start();
        i2c_send_byte(I2C_MSB_W);
        i2c_wait_ack();
        i2c_send_byte(UINT16_H(addr));
        i2c_wait_ack();
        i2c_send_byte(UINT16_L(addr));
        i2c_wait_ack();
        i2c_stop();

        i2c_start();
        i2c_send_byte(I2C_MSB_R);
        i2c_wait_ack();

        i = 3;
        data = 0;
        while (i--) {
                data += i2c_receive_byte(1);
                data <<= 8;
        }
        data += i2c_receive_byte(0);
        i2c_stop();

        return data;
}

void i2c_write_byte(uint8_t addr, uint8_t data)
{
        i2c_start();
        i2c_send_byte(I2C_MSB_W);
        i2c_wait_ack();
        i2c_send_byte(addr);
        i2c_wait_ack();
        i2c_send_byte(data);
        i2c_wait_ack();
        i2c_stop();
        i2c_delay_ms(6);

}

uint8_t i2c_read_byte(uint8_t addr)
{

        uint8_t data;

        i2c_start();
        i2c_send_byte(I2C_MSB_W);
        i2c_wait_ack();
        i2c_send_byte(addr);
        i2c_wait_ack();
        i2c_stop();

        i2c_start();
        i2c_send_byte(I2C_MSB_R);
        i2c_wait_ack();

        data = i2c_receive_byte(0);
        i2c_stop();

        return data;
}

void i2c_write_data(uint16_t addr, uint8_t len, uint8_t *data)
{
        i2c_start();
        i2c_send_byte(I2C_MSB_W);
        i2c_wait_ack();
        i2c_send_byte(UINT16_H(addr));
        i2c_wait_ack();
        i2c_send_byte(UINT16_L(addr));
        i2c_wait_ack();

        uint8_t i = len;
        while (i--) {
                i2c_send_byte(*data);
                i2c_wait_ack();
                data++;
        }
        i2c_stop();
        i2c_delay_ms(3);
}

void i2c_set_dev(uint8_t dev)
{
        I2C_MSB_W  = 0xa0 + (dev<<1);
        I2C_MSB_R  = 0xa1 + (dev<<1);
}
