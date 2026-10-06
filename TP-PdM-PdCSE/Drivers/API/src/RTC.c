/*
 * RTC.c
 *
 *  Created on: 6 oct. 2026
 *      Author: julia
 */

#include "general_def.h"

typedef struct {
	uint8_t	Hours;
	uint8_t Minutes;
} Clock_time;

static I2C_HandleTypeDef hi2c1;


void initRTC()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_I2C1_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = I2C_RTC_SQW_PIN;
	GPIO_InitStruct.Mode = I2C_RTC_SQW_MODE;
	GPIO_InitStruct.Pull = I2C_RTC_SQW_PULL;
	HAL_GPIO_Init(I2C_RTC_SQW_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = I2C_RTC_SDA_PIN;
	GPIO_InitStruct.Mode = I2C_RTC_SDA_MODE;
	GPIO_InitStruct.Pull = I2C_RTC_SDA_PULL;
	GPIO_InitStruct.Speed = I2C_RTC_SCL_SPEED;
	GPIO_InitStruct.Alternate = I2C_RTC_SCL_AF;
	HAL_GPIO_Init(I2C_RTC_SDA_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = I2C_RTC_SCL_PIN;
	GPIO_InitStruct.Mode = I2C_RTC_SCL_MODE;
	GPIO_InitStruct.Pull = I2C_RTC_SCL_PULL;
	GPIO_InitStruct.Speed = I2C_RTC_SDA_SPEED;
	GPIO_InitStruct.Alternate = I2C_RTC_SDA_AF;
	HAL_GPIO_Init(I2C_RTC_SCL_PORT, &GPIO_InitStruct);

	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	HAL_I2C_Init(&hi2c1);
}
