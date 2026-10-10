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
#include "ErrorManager.h"

#define DS1307_I2C_ADDR_7BIT	0x68
#define DS1307_ADDR				( DS1307_I2C_ADDR_7BIT << 1 )	// la HAL espera la dirección desplazada

/* Registros del DS1307 */
#define DS1307_REG_SECONDS		0x00
#define DS1307_REG_MINUTES		0x01
#define DS1307_REG_CONTROL		0x07

/* Valores y máscaras */
#define DS1307_SECONDS_RESET	0x00	// segundos = 0, CH = 0 (oscilador en marcha)
#define DS1307_CTRL_SQW_1HZ		0x10	// SQWE = 1, RS = 00 -> 1 Hz
#define DS1307_MINUTES_MASK		0x7F
#define DS1307_HOURS_24H_MASK	0x3F

#define DS1307_TIME_WRITE_LEN	3		// segundos, minutos, horas
#define DS1307_TIME_READ_LEN	2		// minutos, horas

#define RTC_I2C_CLOCK_HZ		100000
#define RTC_I2C_OWN_ADDRESS		0		// el micro es maestro, no usa dirección propia
#define RTC_I2C_TIMEOUT_MS		1000

#define BCD_NIBBLE_SHIFT		4
#define BCD_LOW_NIBBLE_MASK		0x0F
#define DECIMAL_BASE			10

enum { TIME_IDX_SEC, TIME_IDX_MIN, TIME_IDX_HOUR };	// orden de los registros al escribir
enum { READ_IDX_MIN, READ_IDX_HOUR };				// orden al leer desde DS1307_REG_MINUTES


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
	hi2c1.Init.ClockSpeed = RTC_I2C_CLOCK_HZ;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = RTC_I2C_OWN_ADDRESS;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	if ( HAL_OK != HAL_I2C_Init(&hi2c1)) {
		Error_Handler();
	}

	RTC_Init();
}

static void RTC_Init( void )
{
    uint8_t reg_sec = DS1307_SECONDS_RESET;
    uint8_t reg_ctrl = DS1307_CTRL_SQW_1HZ;

    // 1. Activar oscilador
    RTC_send_data(&hi2c1, DS1307_ADDR, DS1307_REG_SECONDS, I2C_MEMADD_SIZE_8BIT, &reg_sec, sizeof(reg_sec), RTC_I2C_TIMEOUT_MS);

    // 2. Habilitar onda cuadrada a 1 Hz en el pin SQW
    RTC_send_data(&hi2c1, DS1307_ADDR, DS1307_REG_CONTROL, I2C_MEMADD_SIZE_8BIT, &reg_ctrl, sizeof(reg_ctrl), RTC_I2C_TIMEOUT_MS);
}

static uint8_t decToBcd( uint8_t val )
{
	return ( ( val / DECIMAL_BASE ) << BCD_NIBBLE_SHIFT ) | ( val % DECIMAL_BASE );
}

static uint8_t bcdToDec( uint8_t val )
{
	return ( ( val >> BCD_NIBBLE_SHIFT ) * DECIMAL_BASE ) + ( val & BCD_LOW_NIBBLE_MASK );
}

void initClock( Clock_time *clock )
{
	if ( clock == NULL ) {
		Error_Handler();
	}

	clock->Hours = MIN_HOUR;
	clock->Minutes = MIN_MIN;
}

void addHour( Clock_time *clock )
{
	if ( clock == NULL ) {
		Error_Handler();
	}

	if ( clock->Hours < MAX_HOUR ) {
		clock->Hours += 1;
	}
	else {
		clock->Hours = MIN_HOUR;
	}
}

void substractHour( Clock_time *clock )
{
	if ( clock == NULL ) {
		Error_Handler();
	}

	if ( clock->Hours > MIN_HOUR ) {
			clock->Hours -= 1;
	}
	else {
		clock->Hours = MAX_HOUR;
	}
}

void addMin( Clock_time *clock )
{
	if ( clock == NULL ) {
		Error_Handler();
	}

	if ( clock->Minutes < MAX_MIN ) {
		clock->Minutes += 1;
	}
	else {
		clock->Minutes = MIN_MIN;
	}
}

void substractMin( Clock_time *clock )
{
	if ( clock == NULL ) {
		Error_Handler();
	}

	if ( clock->Minutes > MIN_MIN ) {
			clock->Minutes -= 1;
	}
	else {
		clock->Minutes = MAX_MIN;
	}
}

void RTC_SetTime( Clock_time clock)
{

	if (clock.Hours > MAX_HOUR) {
		return;
	}

	if (clock.Minutes > MAX_MIN) {
			return;
	}

	uint8_t data[DS1307_TIME_WRITE_LEN];
	data[TIME_IDX_SEC] = DS1307_SECONDS_RESET;
	data[TIME_IDX_MIN] = decToBcd(clock.Minutes) & DS1307_MINUTES_MASK;
	data[TIME_IDX_HOUR] = decToBcd(clock.Hours) & DS1307_HOURS_24H_MASK;
	RTC_send_data( &hi2c1, DS1307_ADDR, DS1307_REG_SECONDS, I2C_MEMADD_SIZE_8BIT, data, DS1307_TIME_WRITE_LEN, RTC_I2C_TIMEOUT_MS);
}

void RTC_ReadTime( Clock_time *clock )
{
	if ( clock == NULL ) {
		return;
	}

	if (clock->Hours > MAX_HOUR) {
		return;
	}

	if (clock->Minutes > MAX_MIN) {
			return;
	}

	uint8_t data[DS1307_TIME_READ_LEN];
	RTC_read_data(&hi2c1, DS1307_ADDR, DS1307_REG_MINUTES, I2C_MEMADD_SIZE_8BIT, data, DS1307_TIME_READ_LEN, RTC_I2C_TIMEOUT_MS);

	clock->Minutes = bcdToDec(data[READ_IDX_MIN] & DS1307_MINUTES_MASK);
	clock->Hours = bcdToDec(data[READ_IDX_HOUR] & DS1307_HOURS_24H_MASK);

}

bool_t sameTime( Clock_time actual, Clock_time expected)
{
	return (actual.Hours == expected.Hours) & (actual.Minutes == expected.Minutes);
}
