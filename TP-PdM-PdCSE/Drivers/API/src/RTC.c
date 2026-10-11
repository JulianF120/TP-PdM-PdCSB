/**
 * @file RTC.c
 * @brief DS1307 real-time clock driver (I2C) and time helpers.
 * @author Julian Ferreira
 * @date 06/10/2026
 */

#include "general_def.h"
#include "RTC_port.h"
#include "ports.h"
#include "RTC.h"
#include "ErrorManager.h"

#define DS1307_I2C_ADDR_7BIT	0x68	/**< @brief 7-bit I2C address of the DS1307. */
#define DS1307_ADDR				( DS1307_I2C_ADDR_7BIT << 1 )	/**< @brief I2C address shifted, as the HAL expects it. */

/** @name DS1307 registers
 *  @{
 */
#define DS1307_REG_SECONDS		0x00	/**< @brief Seconds and clock halt (CH) bit. */
#define DS1307_REG_MINUTES		0x01	/**< @brief Minutes. */
#define DS1307_REG_CONTROL		0x07	/**< @brief Square wave output control. */
/** @} */

/** @name DS1307 register values and masks
 *  @{
 */
#define DS1307_SECONDS_RESET	0x00	/**< @brief Seconds = 0 and CH = 0 (oscillator running). */
#define DS1307_CTRL_SQW_1HZ		0x10	/**< @brief SQWE = 1 and RS = 00: 1 Hz square wave. */
#define DS1307_MINUTES_MASK		0x7F	/**< @brief Valid bits of the minutes register. */
#define DS1307_HOURS_24H_MASK	0x3F	/**< @brief Valid bits of the hours register in 24-hour mode. */
/** @} */

#define DS1307_TIME_WRITE_LEN	3		/**< @brief Bytes written by RTC_SetTime(): seconds, minutes and hours. */
#define DS1307_TIME_READ_LEN	2		/**< @brief Bytes read by RTC_ReadTime(): minutes and hours. */

#define RTC_I2C_CLOCK_HZ		100000	/**< @brief I2C clock frequency, in Hz. */
#define RTC_I2C_OWN_ADDRESS		0		/**< @brief Own I2C address (unused: the microcontroller is the master). */
#define RTC_I2C_TIMEOUT_MS		1000	/**< @brief Timeout of each I2C transfer, in ms. */

#define BCD_NIBBLE_SHIFT		4		/**< @brief Shift of the tens digit in a BCD byte. */
#define BCD_LOW_NIBBLE_MASK		0x0F	/**< @brief Mask of the units digit in a BCD byte. */
#define DECIMAL_BASE			10		/**< @brief Base used in the BCD conversions. */

/** @brief Position of each register in the buffer written by RTC_SetTime(). */
enum { TIME_IDX_SEC, TIME_IDX_MIN, TIME_IDX_HOUR };
/** @brief Position of each register in the buffer read by RTC_ReadTime(), starting at DS1307_REG_MINUTES. */
enum { READ_IDX_MIN, READ_IDX_HOUR };


static void RTC_ConfigureDevice( void );

static I2C_HandleTypeDef hi2c1;	/**< @brief I2C handle used to talk to the DS1307. */

/* Documented in RTC.h */
void RTC_Init( void )
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_I2C1_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = RTC_SQW_PIN;
	GPIO_InitStruct.Mode = RTC_SQW_MODE;
	GPIO_InitStruct.Pull = RTC_SQW_PULL;
	HAL_GPIO_Init(RTC_SQW_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = I2C_RTC_SDA_PIN;
	GPIO_InitStruct.Mode = I2C_RTC_SDA_MODE;
	GPIO_InitStruct.Pull = I2C_RTC_SDA_PULL;
	GPIO_InitStruct.Speed = I2C_RTC_SDA_SPEED;
	GPIO_InitStruct.Alternate = I2C_RTC_SDA_AF;
	HAL_GPIO_Init(I2C_RTC_SDA_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = I2C_RTC_SCL_PIN;
	GPIO_InitStruct.Mode = I2C_RTC_SCL_MODE;
	GPIO_InitStruct.Pull = I2C_RTC_SCL_PULL;
	GPIO_InitStruct.Speed = I2C_RTC_SCL_SPEED;
	GPIO_InitStruct.Alternate = I2C_RTC_SCL_AF;
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

	RTC_ConfigureDevice();
}

/**
 * @brief Starts the DS1307 oscillator and enables the 1 Hz SQW output.
 *
 * @note Writing the seconds register also resets the seconds to 0.
 */
static void RTC_ConfigureDevice( void )
{
	uint8_t reg_sec = DS1307_SECONDS_RESET;
	uint8_t reg_ctrl = DS1307_CTRL_SQW_1HZ;

	// 1. Activar oscilador
	RTC_send_data(&hi2c1, DS1307_ADDR, DS1307_REG_SECONDS, I2C_MEMADD_SIZE_8BIT, &reg_sec, sizeof(reg_sec), RTC_I2C_TIMEOUT_MS);

	// 2. Habilitar onda cuadrada a 1 Hz en el pin SQW
	RTC_send_data(&hi2c1, DS1307_ADDR, DS1307_REG_CONTROL, I2C_MEMADD_SIZE_8BIT, &reg_ctrl, sizeof(reg_ctrl), RTC_I2C_TIMEOUT_MS);
}

/**
 * @brief Converts a decimal value (0 to 99) to BCD.
 *
 * @param[in] val Decimal value.
 * @return Value in BCD.
 */
static uint8_t decToBcd( uint8_t val )
{
	return ( ( val / DECIMAL_BASE ) << BCD_NIBBLE_SHIFT ) | ( val % DECIMAL_BASE );
}

/**
 * @brief Converts a BCD value to decimal.
 *
 * @param[in] val Value in BCD.
 * @return Decimal value.
 */
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

void subtractHour( Clock_time *clock )
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

void subtractMin( Clock_time *clock )
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
	return (actual.Hours == expected.Hours) && (actual.Minutes == expected.Minutes);
}
