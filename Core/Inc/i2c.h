#ifndef __I2C_SW__H
#define	__I2C_SW__H

#include "main.h"

/* Before the API interface block using EEPROM need to initialize the first. */
void i2c_module_init(void);

/* (bl24c256) A page is 64 bytes, so need to prepare 64 bytes cache space */
void eeprom_read_page(uint16_t addr, uint8_t *data);
void eeprom_write_page(uint16_t addr, uint8_t *data);

uint8_t eeprom_read_uint8(uint16_t addr);
uint16_t eeprom_read_uint16(uint16_t addr);
uint32_t eeprom_read_uint32(uint32_t addr);
void eeprom_write_one(uint16_t addr, uint8_t data);
uint8_t i2c_read_byte(uint8_t addr);
void i2c_write_byte(uint8_t addr, uint8_t data);
void i2c_write_data(uint16_t addr, uint8_t len, uint8_t *data);
void i2c_set_dev(uint8_t dev);

#endif
