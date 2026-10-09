/*
 * RTC.c
 *
 *  Created on: 6 oct. 2026
 *      Author: julia
 */

#include "general_def.h"
#include "RTC_port.h"
#include "ports.h"
#include "RTC.h"

#define DS1307_ADDR ( 0x68 << 1 )
#define MIN_HOUR	0
#define MAX_HOUR	23
#define MAX_MIN		59
#define MIN_MIN		0

static void RTC_Init( void );

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
	RTC_Init();
}

static void RTC_Init( void )
{
    uint8_t reg_sec = 0x00;     // CH = 0 (arranca oscilador), segundos = 0
    uint8_t reg_ctrl = 0x10;    // SQWE = 1, Frecuencia = 1 Hz (registro 0x07)

    // 1. Activar oscilador (Registro 0x00)
    RTC_send_data(&hi2c1, DS1307_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &reg_sec, 1, HAL_MAX_DELAY);

    // 2. Habilitar onda cuadrada a 1 Hz en el pin SQW (Registro 0x07)
    RTC_send_data(&hi2c1, DS1307_ADDR, 0x07, I2C_MEMADD_SIZE_8BIT, &reg_ctrl, 1, HAL_MAX_DELAY);
}

static uint8_t decToBcd( uint8_t val )
{
	return ( ( val / 10 ) << 4 ) | ( val % 10 );
}

static uint8_t bcdToDec( uint8_t val )
{
	return ( ( val >> 4 ) * 10 ) + ( val & 0x0f );
}

void initClock( Clock_time *clock )
{
	clock->Hours = MIN_HOUR;
	clock->Minutes = MIN_MIN;
}

void addHour( Clock_time *clock )
{
	if ( clock->Hours < MAX_HOUR ) {
		clock->Hours += 1;
	}
	else {
		clock->Hours = MIN_HOUR;
	}
}

void substractHour( Clock_time *clock )
{
	if ( clock->Hours > MIN_HOUR ) {
			clock->Hours -= 1;
	}
	else {
		clock->Hours = MAX_HOUR;
	}
}

void addMin( Clock_time *clock )
{
	if ( clock->Minutes < MAX_MIN ) {
		clock->Minutes += 1;
	}
	else {
		clock->Minutes = MIN_MIN;
	}
}

void substractMin( Clock_time *clock )
{
	if ( clock->Minutes > MIN_MIN ) {
			clock->Minutes -= 1;
	}
	else {
		clock->Minutes = MAX_MIN;
	}
}

void RTC_SetTime( Clock_time clock)
{
	uint8_t data[3];
	data[0] =  0x7F;
	data[1] = decToBcd(clock.Minutes) & 0x7F;
	data[2] = decToBcd(clock.Hours) & 0x3F;
	RTC_send_data( &hi2c1, DS1307_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, data, 3, HAL_MAX_DELAY);
}

//Revisar ver de hacer checkeo
void RTC_ReadTime( Clock_time *clock )
{
	uint8_t data[3];
	RTC_read_data(&hi2c1, DS1307_ADDR, 0x01, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);

	clock->Minutes = bcdToDec(data[0] & 0x7F);
	clock->Hours = bcdToDec(data[1] & 0x3F);

}

bool_t sameTime( Clock_time actual, Clock_time expected)
{
	return actual.Hours == expected.Hours & actual.Minutes == expected.Minutes;
}
