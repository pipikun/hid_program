#ifndef __HAL_I2C_H__
#define __HAL_I2C_H__

#include "main.h"

enum hal_i2c_state {
	I2C_ACK = 0,
	I2C_NO_ACK,
	I2C_ERROR,
};

struct hal_i2c_pin {
	GPIO_TypeDef *gpio;
	uint16_t pin;
};

struct hal_i2c_def {
	uint8_t dev_addr;
	uint8_t bit_wide;
	uint8_t msb_write;
	uint8_t msb_read;
	uint8_t ack;
	uint8_t delay_us;
	uint8_t delay_ms;
	uint8_t aline;
	enum hal_i2c_state state;

	struct hal_i2c_pin sda;
	struct hal_i2c_pin scl;
};


#endif /*__HAL_I2C_H__ */
//vim: ts=8 sw=8 noet autoindent:
