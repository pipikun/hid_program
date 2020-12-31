/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB6 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
void i2c_gpio_init(void)
{
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_GPIOC_CLK_ENABLE();

        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(I2C_SDA_GPIO, I2C_SDA_PIN, GPIO_PIN_RESET);
        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(I2C_SCL_GPIO, I2C_SCL_PIN, GPIO_PIN_RESET);

        /*Configure GPIO pin : I2C_SDA */
        GPIO_InitStruct.Pin = I2C_SDA_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(I2C_SDA_GPIO, &GPIO_InitStruct);

        /*Configure GPIO pin : I2C_SCL */
        GPIO_InitStruct.Pin = I2C_SCL_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(I2C_SCL_GPIO, &GPIO_InitStruct);

        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(I2C_SDA_GPIO, I2C_SDA_PIN, GPIO_PIN_SET);
        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(I2C_SCL_GPIO, I2C_SCL_PIN, GPIO_PIN_SET);
}


void i2c_gpio_dir_output(void)
{
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_GPIOC_CLK_ENABLE();

        /* Configure GPIO pin : I2C_SDA_PIN */
        GPIO_InitStruct.Pin = I2C_SDA_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(I2C_SDA_GPIO, &GPIO_InitStruct);
        HAL_GPIO_WritePin(I2C_SDA_GPIO, I2C_SDA_PIN, GPIO_PIN_SET);
}

void i2c_gpio_dir_input(void)
{
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_GPIOC_CLK_ENABLE();

        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(I2C_SDA_GPIO, I2C_SDA_PIN, GPIO_PIN_SET);
        /*Configure GPIO pin : MDIO */
        GPIO_InitStruct.Pin = I2C_SDA_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(I2C_SDA_GPIO, &GPIO_InitStruct);
}

/* mdio mdoule */
void mdio_goio_init(void)
{
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_GPIOC_CLK_ENABLE();

        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(MDIO_GPIO, MDIO_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MDC_GPIO, MDC_PIN, GPIO_PIN_RESET);

        /*Configure GPIO pin : MDIO */
        GPIO_InitStruct.Pin = MDIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(MDIO_GPIO, &GPIO_InitStruct);

        /*Configure GPIO pin : MDC */
        GPIO_InitStruct.Pin = MDC_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(MDC_GPIO, &GPIO_InitStruct);

        /*Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(MDIO_GPIO, MDIO_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MDC_GPIO, MDC_PIN, GPIO_PIN_SET);
}

void mdio_dir_output(void)
{
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_GPIOC_CLK_ENABLE();

        /*Configure GPIO pin : MDIO */
        GPIO_InitStruct.Pin = MDIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(MDIO_GPIO, &GPIO_InitStruct);
}

void mdio_dir_input(void)
{
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_GPIOC_CLK_ENABLE();

        /*Configure GPIO pin : MDIO */
        GPIO_InitStruct.Pin = MDIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(MDIO_GPIO, &GPIO_InitStruct);
}

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
