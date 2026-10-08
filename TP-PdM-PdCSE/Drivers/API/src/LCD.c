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

#define SSD1306_CMD_FADE_BLINK  0x23
#define BLINK_INTERVAL 1

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

	HAL_SPI_Init(&hspi1);

	LCD_Init();
}

static void LCD_Write_Command(uint8_t cmd) {
    HAL_GPIO_WritePin(SPI_LCD_DC_PORT, SPI_LCD_DC_PIN, GPIO_PIN_RESET); // DC = 0 (Comando)
    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_RESET); // CS = 0 (Activo)

    LCD_send_data(&hspi1, &cmd, 1, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_SET);   // CS = 1 (Inactivo)
}

static void LCD_Write_Data(uint8_t *data, uint16_t size) {
    HAL_GPIO_WritePin(SPI_LCD_DC_PORT, SPI_LCD_DC_PIN, GPIO_PIN_SET);   // DC = 1 (Dato)
    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_RESET); // CS = 0 (Activo)

    LCD_send_data(&hspi1, data, size, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_SET);   // CS = 1 (Inactivo)
}

static void LCD_Init( void )
{
	HAL_Delay(100); // Esperar a que se estabilice VCC al encender
	// Secuencia de Reset físico por hardware
	HAL_GPIO_WritePin(SPI_LCD_RST_PORT, SPI_LCD_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(SPI_LCD_RST_PORT, SPI_LCD_RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(SPI_LCD_RST_PORT, SPI_LCD_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(20);

	LCD_Write_Command(0xAE); // Display OFF
	LCD_Write_Command(0xD5); // Set Display Clock Divide Ratio
	LCD_Write_Command(0x80);
	LCD_Write_Command(0xA8); // Set Multiplex Ratio
	LCD_Write_Command(0x3F);
	LCD_Write_Command(0xD3); // Set Display Offset
	LCD_Write_Command(0x00);
	LCD_Write_Command(0x40); // Set Display Start Line
	LCD_Write_Command(0x8D); // Charge Pump Setting
	LCD_Write_Command(0x14); // Enable Charge Pump (Fundamental para encender el panel)
	LCD_Write_Command(0x20); // Memory Addressing Mode
	LCD_Write_Command(0x00); // Horizontal Addressing Mode
	LCD_Write_Command(0xA1); // Set Segment Re-map
	LCD_Write_Command(0xC8); // Set COM Output Scan Direction
	LCD_Write_Command(0xDA); // Set COM Pins Hardware Configuration
	LCD_Write_Command(0x12);
	LCD_Write_Command(0x81); // Set Contrast Control
	LCD_Write_Command(0xCF);
	LCD_Write_Command(0xD9); // Set Pre-charge Period
	LCD_Write_Command(0xF1);
	LCD_Write_Command(0xDB); // Set VCOMH Deselect Level
	LCD_Write_Command(0x40);
	LCD_Write_Command(0xA4); // Entire Display ON (Resume to RAM content)
	LCD_Write_Command(0xA6); // Set Normal Display
	LCD_Write_Command(0xAF); // Display ON
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

void LED_Draw( uint8_t start_col, uint8_t start_page, OLED_Bitmap_t bitmap ) {
    // Ventana acotada a 8 columnas de ancho (start_col a start_col + 7)
    LCD_Write_Command(0x21);
    LCD_Write_Command(start_col);
    LCD_Write_Command(start_col + bitmap.width);

    // Ventana acotada a 2 páginas de alto (start_page a start_page + 1)
    LCD_Write_Command(0x22);
    LCD_Write_Command(start_page);
    LCD_Write_Command(start_page + bitmap.pages);

    // Al enviar los 16 bytes, los primeros 8 pintan arriba y los otros 8 abajo automáticamente
    LCD_Write_Data((uint8_t*)bitmap.data, (uint16_t)bitmap.width * bitmap.pages);
}

void LED_DrawTime( Clock_time clcok, bool_t alarmOn )
{
	uint8_t decHour = clcok.Hours / 10;
	uint8_t unitHour = clcok.Hours % 10;
	uint8_t decMin = clcok.Minutes / 10;
	uint8_t unitMin = clcok.Minutes % 10;

	uint8_t start_page = 0;
	LCD_Clear();
	LED_Draw( Hour_Dec, start_page, getNumberDraw( decHour ) );
	start_page += 3;
	LED_Draw( Hour_Unit, start_page, getNumberDraw( unitHour ) );
	start_page += 3;
	LED_Draw( Doble_Dot, start_page, get_bm_num_doble_dot() );
	start_page += 3;
	LED_Draw( Min_Dec, start_page, getNumberDraw( decMin ) );
	start_page += 3;
	LED_Draw( Min_Unit, start_page, getNumberDraw( unitMin ) );
	if ( alarmOn ) {
		LED_Draw( Alarm_Icon, start_page, get_bm_alrm2() );
	}

}

void LED_BlinkEnable()
{
	LCD_Write_Command( SSD1306_CMD_FADE_BLINK );
	LCD_Write_Command(0x30 | (BLINK_INTERVAL & 0x0F));
}

void LED_BlinkDisable()
{
	LCD_Write_Command( SSD1306_CMD_FADE_BLINK );
	LCD_Write_Command( 0x00 );
}

