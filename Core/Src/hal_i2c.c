#include "hal_i2c.h"

#define I2C_DEV_NUM_MAX		2

#define HAL_I2C_ADDR		0x07
#define HAL_I2C_BIT_WIDE	16
#define HAL_I2C_MSB_W		0xA0
#define HAL_I2C_MSB_R		0xA1
#define HAL_I2C_ADDR_ALINE	64

#define HAL_I2C_DEV1_SDA_GPIO	GPIOB
#define HAL_I2C_DEV1_SDA_PIN	GPIO_PIN_8

#define HAL_I2C_DEV1_SCL_GPIO	GPIOB
#define HAL_I2C_DEV1_SCL_PIN	GPIO_PIN_9

#define HAL_I2C_DEV2_SDA_GPIO	GPIOB
#define HAL_I2C_DEV2_SDA_PIN	GPIO_PIN_7

#define HAL_I2C_DEV2_SCL_GPIO	GPIOB
#define HAL_I2C_DEV2_SCL_PIN	GPIO_PIN_6

#define HAL_I2C_HIGH		1
#define HAL_I2C_LOW		0

#define HAL_I2C_DELAY_TIME_US	3
#define HAL_I2C_DELAY_TIME_MS   6

#define UINT16_H(x)		(x >> 8)
#define UINT16_L(x)		(x & 0xff)

struct hal_i2c_def i2c_dev = {
	.dev_addr = HAL_I2C_ADDR,
	.bit_wide = HAL_I2C_BIT_WIDE,
	.msb_write = HAL_I2C_MSB_W,
	.msb_read = HAL_I2C_MSB_R,
	.delay_us = HAL_I2C_DELAY_TIME_US,
	.delay_ms = HAL_I2C_DELAY_TIME_MS,
	.aline = HAL_I2C_ADDR_ALINE,
	.state = I2C_ACK,
	.ack = 0,
	.sda = {
		.gpio = HAL_I2C_DEV1_SDA_GPIO,
		.pin = HAL_I2C_DEV1_SDA_PIN,
	},
	.scl = {
		.gpio = HAL_I2C_DEV1_SCL_GPIO,
		.pin = HAL_I2C_DEV1_SCL_PIN,
	},
};

static inline void i2c_delay_us(uint16_t time)
{
	while (time--) {
		asm("nop");
	}
}

static inline void i2c_delay_ms(uint16_t time)
{
	for (int i=0; i<time; i++) {
		for (int j=0; j<8000; j++) {
			asm("nop");asm("nop");
			asm("nop");asm("nop");
		}
	}
}

static inline void i2c_dev_sel(struct hal_i2c_def *dev, uint8_t dev_num)
{
	assert_param(dev_num < I2C_DEV_NUM_MAX);
	switch (dev_num) {
	case 0:
		dev->sda.gpio = HAL_I2C_DEV1_SDA_GPIO;
		dev->sda.pin = HAL_I2C_DEV1_SDA_PIN;
		dev->scl.gpio = HAL_I2C_DEV1_SCL_GPIO;
		dev->scl.pin = HAL_I2C_DEV1_SCL_PIN;
		break;
	case 1:
		dev->sda.gpio = HAL_I2C_DEV2_SDA_GPIO;
		dev->sda.pin = HAL_I2C_DEV2_SDA_PIN;
		dev->scl.gpio = HAL_I2C_DEV2_SCL_GPIO;
		dev->scl.pin = HAL_I2C_DEV2_SCL_PIN;
		break;
	default:
		break;
	}
}

static inline void i2c_addr_sel(struct hal_i2c_def *dev, uint8_t addr)
{
	assert_param(addr < 8);
	dev->dev_addr = addr;
}

static inline void i2c_bit_wide_sel(struct hal_i2c_def *dev, uint8_t bit_wide)
{
	dev->bit_wide = bit_wide;
}

static inline void i2c_gpio_dir_in(GPIO_TypeDef *gpiox, uint16_t pin)
{
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        HAL_GPIO_WritePin(gpiox, pin, GPIO_PIN_SET);
        GPIO_InitStruct.Pin = pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(gpiox, &GPIO_InitStruct);
}

static inline void i2c_gpio_dir_out(GPIO_TypeDef *gpiox, uint16_t pin)
{
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        GPIO_InitStruct.Pin = pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(gpiox, &GPIO_InitStruct);
        HAL_GPIO_WritePin(gpiox, pin, GPIO_PIN_SET);
}

static inline void i2c_start(struct hal_i2c_def *dev)
{
	HAL_GPIO_WritePin(dev->sda.gpio, dev->sda.pin, HAL_I2C_HIGH);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_HIGH);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->sda.gpio, dev->sda.pin, HAL_I2C_LOW);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_LOW);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
}

static inline void i2c_stop(struct hal_i2c_def *dev)
{
	HAL_GPIO_WritePin(dev->sda.gpio, dev->sda.pin, HAL_I2C_LOW);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_HIGH);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->sda.gpio, dev->sda.pin, HAL_I2C_HIGH);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
}

static inline void i2c_ack(struct hal_i2c_def *dev)
{
	HAL_GPIO_WritePin(dev->sda.gpio, dev->sda.pin, HAL_I2C_LOW);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_HIGH);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_LOW);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->sda.gpio, dev->sda.pin, HAL_I2C_HIGH);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
}

static inline void i2c_no_ack(struct hal_i2c_def *dev)
{
	HAL_GPIO_WritePin(dev->sda.gpio, dev->sda.pin, HAL_I2C_HIGH);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_HIGH);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_LOW);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_HIGH);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
}

static inline void i2c_wait_ack(struct hal_i2c_def *dev)
{
	HAL_GPIO_WritePin(dev->sda.gpio, dev->sda.pin, HAL_I2C_HIGH);
	i2c_gpio_dir_in(dev->sda.gpio, dev->sda.pin);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_HIGH);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	dev->ack = HAL_GPIO_ReadPin(dev->sda.gpio, dev->sda.pin);
	i2c_gpio_dir_out(dev->sda.gpio, dev->sda.pin);
	i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_LOW);
}

static inline void i2c_set_byte(struct hal_i2c_def *dev, uint8_t byte)
{
	uint8_t i = 8, data, bit;

	data = byte;
	while (i--) {
		bit = !!(data & 0x80);
		HAL_GPIO_WritePin(dev->sda.gpio, dev->sda.pin, bit);
		i2c_delay_us(HAL_I2C_DELAY_TIME_US);
		HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_HIGH);
		i2c_delay_us(HAL_I2C_DELAY_TIME_US);
		HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_LOW);
		i2c_delay_us(HAL_I2C_DELAY_TIME_US);
		data<<=1;
	}
}

static inline uint8_t i2c_get_byte(struct hal_i2c_def *dev, uint8_t ack)
{
	uint8_t data = 0, i = 8;

	i2c_gpio_dir_in(dev->sda.gpio, dev->sda.pin);
	while (i--) {
		data <<= 1;
		HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_HIGH);
		i2c_delay_us(HAL_I2C_DELAY_TIME_US);
		data += HAL_GPIO_ReadPin(dev->sda.gpio, dev->sda.pin);
		HAL_GPIO_WritePin(dev->scl.gpio, dev->scl.pin, HAL_I2C_LOW);
		i2c_delay_us(HAL_I2C_DELAY_TIME_US);
	}

	i2c_gpio_dir_out(dev->sda.gpio, dev->sda.pin);

	if (ack) {
		i2c_ack(dev);
	} else {
		i2c_no_ack(dev);
	}
	return data;
}

void hal_i2c_init(void)
{

        GPIO_InitTypeDef GPIO_InitStruct = {0};

        GPIO_InitStruct.Pin = i2c_dev.sda.pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(i2c_dev.sda.gpio, &GPIO_InitStruct);
        HAL_GPIO_WritePin(i2c_dev.sda.gpio, i2c_dev.sda.pin, GPIO_PIN_SET);

        GPIO_InitStruct.Pin = i2c_dev.scl.pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(i2c_dev.scl.gpio, &GPIO_InitStruct);
        HAL_GPIO_WritePin(i2c_dev.scl.gpio, i2c_dev.scl.pin, GPIO_PIN_SET);
}

static inline void hal_i2c_write_addr(struct hal_i2c_def *dev, uint16_t addr)
{
	i2c_start(dev);
	i2c_set_byte(dev, dev->msb_write);
	i2c_wait_ack(dev);
	if (dev->bit_wide == 16) {
		i2c_set_byte(dev, UINT16_H(addr));
		i2c_wait_ack(dev);
	}
	i2c_set_byte(dev, UINT16_L(addr));
	i2c_wait_ack(dev);
}

static inline void hal_i2c_write_page(struct hal_i2c_def *dev, uint8_t *src, uint16_t len)
{
	uint16_t i;
	uint8_t *tmp;

	i = len;
	tmp = src;
	while (i--) {
		i2c_set_byte(dev, *tmp);
		i2c_wait_ack(dev);
		tmp++;
	}
	i2c_delay_ms(dev->delay_ms);
}

uint8_t hal_i2c_read_byte(struct hal_i2c_def *dev, uint16_t addr)
{
	uint8_t data;

	hal_i2c_write_addr(dev, addr);
	i2c_stop(dev);

	i2c_start(dev);
	i2c_set_byte(dev, dev->msb_read);
	i2c_wait_ack(dev);

	data = i2c_get_byte(dev, I2C_NO_ACK);
	i2c_stop(dev);

	return data;
}

void hal_i2c_read_inc(struct hal_i2c_def *dev, uint8_t *buf, uint16_t addr, uint32_t len)
{
	uint8_t idx;

	hal_i2c_write_addr(dev, addr);
	i2c_stop(dev);

	i2c_start(dev);
	i2c_set_byte(dev, dev->msb_read);
	i2c_wait_ack(dev);

	idx = len;
	while (idx--) {
		*buf =	i2c_get_byte(dev, I2C_ACK);
		buf++;
	}
	*buf = i2c_get_byte(dev, I2C_NO_ACK);
	i2c_stop(dev);
}

void hal_i2c_write_byte(struct hal_i2c_def *dev, uint16_t addr, uint8_t data)
{
	hal_i2c_write_addr(dev, addr);

	i2c_set_byte(dev, data);
	i2c_wait_ack(dev);
	i2c_stop(dev);
	i2c_delay_ms(dev->delay_ms);
}

void hal_i2c_wriet_inc(struct hal_i2c_def *dev, uint16_t addr, uint8_t *src, uint32_t len)
{
	uint16_t head, body, tail, i;


	hal_i2c_write_addr(dev, addr);

	/* address aline */
	head = dev->aline - (addr % dev->aline);

	hal_i2c_write_page(dev, src, head);
	src += head;

	addr = addr + head;
	body = addr % dev->aline;

	i = body;
	while (i--) {
		hal_i2c_write_page(dev, src, head);
		src += dev->aline;
	}
	tail = addr % dev->aline;
	hal_i2c_write_page(dev, src, tail);
}

uint8_t hal_i2c_dev_scan(struct hal_i2c_def *dev)
{
	return 0;
}

void hal_i2c_dev_sel(struct hal_i2c_def *dev, uint8_t dev_num)
{

}
