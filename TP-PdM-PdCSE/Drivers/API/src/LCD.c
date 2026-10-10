/*
 * LCD.c
 *
 *  Created on: 4 oct. 2026
 *      Author: julia
 */


#include "general_def.h"
#include "ports.h"
#include "LCD_Draws.h"
#include "LCD_port.h"
#include "RTC.h"
#include "LCD.h"
#include "ErrorManager.h"

#define LCD_WIDTH	128		// columnas
#define LCD_PAGES	8		// 64 px de alto / 8 px por página

static const uint8_t Hour_Dec = 8;
static const uint8_t Hour_Unit = 24;
static const uint8_t Doble_Dot = 40;
static const uint8_t Min_Dec = 48;
static const uint8_t Min_Unit = 64;
static const uint8_t Alarm_Icon = 104;

static void LCD_Init( void );

static SPI_HandleTypeDef hspi1;


void LCDInit( void )
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_SPI1_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = SPI_LCD_RST_PIN;
	GPIO_InitStruct.Mode = SPI_LCD_RST_MODE;
	GPIO_InitStruct.Pull = SPI_LCD_RST_PULL;
	HAL_GPIO_Init(SPI_LCD_RST_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_LCD_DC_PIN;
	GPIO_InitStruct.Mode = SPI_LCD_DC_MODE;
	GPIO_InitStruct.Pull = SPI_LCD_DC_PULL;
	HAL_GPIO_Init(SPI_LCD_DC_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_LCD_CS_PIN;
	GPIO_InitStruct.Mode = SPI_LCD_CS_MODE;
	GPIO_InitStruct.Pull = SPI_LCD_CS_PULL;
	GPIO_InitStruct.Speed = SPI_LCD_CS_SPEED;
	HAL_GPIO_Init(SPI_LCD_CS_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_LCD_CLK_PIN;
	GPIO_InitStruct.Mode = SPI_LCD_CLK_MODE;
	GPIO_InitStruct.Pull = SPI_LCD_CLK_PULL;
	GPIO_InitStruct.Speed = SPI_LCD_CLK_SPEED;
	GPIO_InitStruct.Alternate = SPI_LCD_CLK_AF;
	HAL_GPIO_Init(SPI_LCD_CLK_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_LCD_MOSI_PIN;
	GPIO_InitStruct.Mode = SPI_LCD_MOSI_MODE;
	GPIO_InitStruct.Pull = SPI_LCD_MOSI_PULL;
	GPIO_InitStruct.Speed = SPI_LCD_MOSI_SPEED;
	GPIO_InitStruct.Alternate = SPI_LCD_MOSI_AF;
	HAL_GPIO_Init(SPI_LCD_MOSI_PORT, &GPIO_InitStruct);

	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32; // ~2.6 MHz (seguro para cables Dupont)
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;

	if ( HAL_SPI_Init(&hspi1) != HAL_OK ) {
		Error_Handler();
	}

	LCD_Init();
}

static bool_t LCD_Write_Command(uint8_t cmd) {
	bool_t ret;

    HAL_GPIO_WritePin(SPI_LCD_DC_PORT, SPI_LCD_DC_PIN, GPIO_PIN_RESET); // DC = 0 (Comando)
    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_RESET); // CS = 0 (Activo)

    ret = LCD_send_data(&hspi1, &cmd, 1, 1000 );

    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_SET);   // CS = 1 (Inactivo)

    return ret;
}

static bool_t LCD_Write_Data(uint8_t *data, uint16_t size) {
    bool_t ret;

	HAL_GPIO_WritePin(SPI_LCD_DC_PORT, SPI_LCD_DC_PIN, GPIO_PIN_SET);   // DC = 1 (Dato)
    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_RESET); // CS = 0 (Activo)

    ret = LCD_send_data(&hspi1, data, size, 1000 );

    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_SET);   // CS = 1 (Inactivo)

    return ret;
}

static void LCD_Init( void )
{
	bool_t notError = true;

	HAL_Delay(100); // Esperar a que se estabilice VCC al encender
	// Secuencia de Reset físico por hardware
	HAL_GPIO_WritePin(SPI_LCD_RST_PORT, SPI_LCD_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(SPI_LCD_RST_PORT, SPI_LCD_RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(SPI_LCD_RST_PORT, SPI_LCD_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(20);

	notError &= LCD_Write_Command(0xAE); // Display OFF
	notError &= LCD_Write_Command(0xD5); // Set Display Clock Divide Ratio
	notError &= LCD_Write_Command(0x80);
	notError &= LCD_Write_Command(0xA8); // Set Multiplex Ratio
	notError &= LCD_Write_Command(0x3F);
	notError &= LCD_Write_Command(0xD3); // Set Display Offset
	notError &= LCD_Write_Command(0x00);
	notError &= LCD_Write_Command(0x40); // Set Display Start Line
	notError &= LCD_Write_Command(0x8D); // Charge Pump Setting
	notError &= LCD_Write_Command(0x14); // Enable Charge Pump (Fundamental para encender el panel)
	notError &= LCD_Write_Command(0x20); // Memory Addressing Mode
	notError &= LCD_Write_Command(0x00); // Horizontal Addressing Mode
	notError &= LCD_Write_Command(0xA1); // Set Segment Re-map
	notError &= LCD_Write_Command(0xC8); // Set COM Output Scan Direction
	notError &= LCD_Write_Command(0xDA); // Set COM Pins Hardware Configuration
	notError &= LCD_Write_Command(0x12);
	notError &= LCD_Write_Command(0x81); // Set Contrast Control
	notError &= LCD_Write_Command(0xCF);
	notError &= LCD_Write_Command(0xD9); // Set Pre-charge Period
	notError &= LCD_Write_Command(0xF1);
	notError &= LCD_Write_Command(0xDB); // Set VCOMH Deselect Level
	notError &= LCD_Write_Command(0x40);
	notError &= LCD_Write_Command(0xA4); // Entire Display ON (Resume to RAM content)
	notError &= LCD_Write_Command(0xA6); // Set Normal Display
	notError &= LCD_Write_Command(0xAF); // Display ON

	if ( !notError ) {
		Error_Handler();
	}
}

void LCD_Clear( void ) {
    LCD_Write_Command(0x21); // Columnas 0 a 127
    LCD_Write_Command(0x00);
    LCD_Write_Command(0x7F);

    LCD_Write_Command(0x22); // Páginas 0 a 7
    LCD_Write_Command(0x00);
    LCD_Write_Command(0x07);

    uint8_t zero_byte = 0x00;
    for (uint16_t i = 0; i < 1024; i++) {
        LCD_Write_Data(&zero_byte, 1);
    }
}

void LED_Draw( uint8_t start_col, uint8_t start_page, OLED_Bitmap_t bitmap )
{
	if ( bitmap.data == NULL) {
		return;
	}

    if ( start_col + bitmap.width > LCD_WIDTH ) {
    	return;
    }

    if ( start_page + bitmap.pages > LCD_PAGES ) {
    	return;
    }

	// Ventana de columnas: start_col a start_col + width - 1
    LCD_Write_Command(0x21);
    LCD_Write_Command(start_col);
    LCD_Write_Command(start_col + bitmap.width - 1);

    // Ventana de páginas: start_page a start_page + pages - 1
    LCD_Write_Command(0x22);
    LCD_Write_Command(start_page);
    LCD_Write_Command(start_page + bitmap.pages - 1);

    // Con direccionamiento horizontal, los bytes llenan la ventana página por página
    LCD_Write_Data((uint8_t*)bitmap.data, (uint16_t)bitmap.width * bitmap.pages);
}

static void LED_ClearArea( uint8_t start_col, uint8_t start_page, uint8_t width, uint8_t pages )
{
	if ( width == 0 || pages == 0 ) {
		return;
	}

	if ( start_col + width > LCD_WIDTH ) {
		return;
	}

	if ( start_page + pages > LCD_PAGES ) {
		return;
	}

	uint8_t zero = 0x00;
	LCD_Write_Command(0x21);
	LCD_Write_Command(start_col);
	LCD_Write_Command(start_col + width - 1);
	LCD_Write_Command(0x22);
	LCD_Write_Command(start_page);
	LCD_Write_Command(start_page + pages - 1);
	for (uint16_t i = 0; i < (uint16_t)width * pages; i++) {
		LCD_Write_Data(&zero, 1);
	}
}

void LED_DrawTime( Clock_time clock, bool_t alarmOn, BlinkField_t hidden )
{
	if (clock.Hours > MAX_HOUR) {
		return;
	}

	if (clock.Minutes > MAX_MIN) {
			return;
	}

	OLED_Bitmap_t num = getNumberDraw( 0 );

	if ( hidden == BLINK_HOURS ) {
		LED_ClearArea( Hour_Dec, 2, num.width, num.pages );
		LED_ClearArea( Hour_Unit, 2, num.width, num.pages );
	} else {
		LED_Draw( Hour_Dec, 2, getNumberDraw( clock.Hours / 10 ) );
		LED_Draw( Hour_Unit, 2, getNumberDraw( clock.Hours % 10 ) );
	}

	LED_Draw( Doble_Dot, 2, get_bm_num_doble_dot() );

	if ( hidden == BLINK_MINUTES ) {
		LED_ClearArea( Min_Dec, 2, num.width, num.pages );
		LED_ClearArea( Min_Unit, 2, num.width, num.pages );
	} else {
		LED_Draw( Min_Dec, 2, getNumberDraw( clock.Minutes / 10 ) );
		LED_Draw( Min_Unit, 2, getNumberDraw( clock.Minutes % 10 ) );
	}

	OLED_Bitmap_t icon = get_bm_alrm2();
	if ( alarmOn ) {
		LED_Draw( Alarm_Icon, 2, icon );
	} else {
		LED_ClearArea( Alarm_Icon, 2, icon.width, icon.pages );
	}
}


