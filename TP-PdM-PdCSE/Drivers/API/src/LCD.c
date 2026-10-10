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
#define LCD_BUFFER_SIZE	( LCD_WIDTH * LCD_PAGES )
#define LCD_BLANK_BYTE	0x00

#define LCD_SPI_TIMEOUT_MS		1000
#define LCD_POWER_ON_DELAY_MS	100		// Esperar a que se estabilice VCC al encender
#define LCD_RESET_PRE_DELAY_MS	10
#define LCD_RESET_PULSE_MS		20
#define LCD_RESET_RECOVERY_MS	20

/* Comandos del SSD1306 */
#define SSD1306_CMD_DISPLAY_OFF			0xAE
#define SSD1306_CMD_SET_CLOCK_DIV		0xD5
#define SSD1306_CMD_SET_MULTIPLEX		0xA8
#define SSD1306_CMD_SET_DISPLAY_OFFSET	0xD3
#define SSD1306_CMD_SET_START_LINE		0x40	// línea de inicio 0
#define SSD1306_CMD_CHARGE_PUMP			0x8D
#define SSD1306_CMD_MEMORY_MODE			0x20
#define SSD1306_CMD_SEG_REMAP			0xA1
#define SSD1306_CMD_COM_SCAN_DEC		0xC8
#define SSD1306_CMD_SET_COM_PINS		0xDA
#define SSD1306_CMD_SET_CONTRAST		0x81
#define SSD1306_CMD_SET_PRECHARGE		0xD9
#define SSD1306_CMD_SET_VCOMH			0xDB
#define SSD1306_CMD_DISPLAY_RESUME_RAM	0xA4
#define SSD1306_CMD_NORMAL_DISPLAY		0xA6
#define SSD1306_CMD_DISPLAY_ON			0xAF
#define SSD1306_CMD_SET_COLUMN_ADDR		0x21
#define SSD1306_CMD_SET_PAGE_ADDR		0x22

/* Argumentos de los comandos */
#define SSD1306_CLOCK_DIV_DEFAULT		0x80
#define SSD1306_MULTIPLEX_64			0x3F	// 64 filas - 1
#define SSD1306_DISPLAY_OFFSET_NONE		0x00
#define SSD1306_CHARGE_PUMP_ENABLE		0x14
#define SSD1306_MEMORY_MODE_HORIZONTAL	0x00
#define SSD1306_COM_PINS_ALT			0x12
#define SSD1306_CONTRAST_LEVEL			0xCF
#define SSD1306_PRECHARGE_PERIOD		0xF1
#define SSD1306_VCOMH_LEVEL				0x40

#define SPI_CRC_POLYNOMIAL_DEFAULT	7		// no se usa: CRC deshabilitado

#define TIME_START_PAGE		2
#define DECIMAL_BASE		10

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
	hspi1.Init.CRCPolynomial = SPI_CRC_POLYNOMIAL_DEFAULT;

	if ( HAL_SPI_Init(&hspi1) != HAL_OK ) {
		Error_Handler();
	}

	LCD_Init();
}

static bool_t LCD_Write_Command(uint8_t cmd) {
	bool_t ret;

    HAL_GPIO_WritePin(SPI_LCD_DC_PORT, SPI_LCD_DC_PIN, GPIO_PIN_RESET); // DC = 0 (Comando)
    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_RESET); // CS = 0 (Activo)

    ret = LCD_send_data(&hspi1, &cmd, sizeof(cmd), LCD_SPI_TIMEOUT_MS );

    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_SET);   // CS = 1 (Inactivo)

    return ret;
}

static bool_t LCD_Write_Data(uint8_t *data, uint16_t size) {
    bool_t ret;

	HAL_GPIO_WritePin(SPI_LCD_DC_PORT, SPI_LCD_DC_PIN, GPIO_PIN_SET);   // DC = 1 (Dato)
    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_RESET); // CS = 0 (Activo)

    ret = LCD_send_data(&hspi1, data, size, LCD_SPI_TIMEOUT_MS );

    HAL_GPIO_WritePin(SPI_LCD_CS_PORT, SPI_LCD_CS_PIN, GPIO_PIN_SET);   // CS = 1 (Inactivo)

    return ret;
}

static void LCD_Init( void )
{
	bool_t notError = true;

	HAL_Delay(LCD_POWER_ON_DELAY_MS);
	// Secuencia de Reset físico por hardware
	HAL_GPIO_WritePin(SPI_LCD_RST_PORT, SPI_LCD_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(LCD_RESET_PRE_DELAY_MS);
	HAL_GPIO_WritePin(SPI_LCD_RST_PORT, SPI_LCD_RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(LCD_RESET_PULSE_MS);
	HAL_GPIO_WritePin(SPI_LCD_RST_PORT, SPI_LCD_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(LCD_RESET_RECOVERY_MS);

	notError &= LCD_Write_Command(SSD1306_CMD_DISPLAY_OFF);
	notError &= LCD_Write_Command(SSD1306_CMD_SET_CLOCK_DIV);
	notError &= LCD_Write_Command(SSD1306_CLOCK_DIV_DEFAULT);
	notError &= LCD_Write_Command(SSD1306_CMD_SET_MULTIPLEX);
	notError &= LCD_Write_Command(SSD1306_MULTIPLEX_64);
	notError &= LCD_Write_Command(SSD1306_CMD_SET_DISPLAY_OFFSET);
	notError &= LCD_Write_Command(SSD1306_DISPLAY_OFFSET_NONE);
	notError &= LCD_Write_Command(SSD1306_CMD_SET_START_LINE);
	notError &= LCD_Write_Command(SSD1306_CMD_CHARGE_PUMP);
	notError &= LCD_Write_Command(SSD1306_CHARGE_PUMP_ENABLE); // Fundamental para encender el panel
	notError &= LCD_Write_Command(SSD1306_CMD_MEMORY_MODE);
	notError &= LCD_Write_Command(SSD1306_MEMORY_MODE_HORIZONTAL);
	notError &= LCD_Write_Command(SSD1306_CMD_SEG_REMAP);
	notError &= LCD_Write_Command(SSD1306_CMD_COM_SCAN_DEC);
	notError &= LCD_Write_Command(SSD1306_CMD_SET_COM_PINS);
	notError &= LCD_Write_Command(SSD1306_COM_PINS_ALT);
	notError &= LCD_Write_Command(SSD1306_CMD_SET_CONTRAST);
	notError &= LCD_Write_Command(SSD1306_CONTRAST_LEVEL);
	notError &= LCD_Write_Command(SSD1306_CMD_SET_PRECHARGE);
	notError &= LCD_Write_Command(SSD1306_PRECHARGE_PERIOD);
	notError &= LCD_Write_Command(SSD1306_CMD_SET_VCOMH);
	notError &= LCD_Write_Command(SSD1306_VCOMH_LEVEL);
	notError &= LCD_Write_Command(SSD1306_CMD_DISPLAY_RESUME_RAM);
	notError &= LCD_Write_Command(SSD1306_CMD_NORMAL_DISPLAY);
	notError &= LCD_Write_Command(SSD1306_CMD_DISPLAY_ON);

	if ( !notError ) {
		Error_Handler();
	}
}

void LCD_Clear( void ) {
    LCD_Write_Command(SSD1306_CMD_SET_COLUMN_ADDR);
    LCD_Write_Command(0);
    LCD_Write_Command(LCD_WIDTH - 1);

    LCD_Write_Command(SSD1306_CMD_SET_PAGE_ADDR);
    LCD_Write_Command(0);
    LCD_Write_Command(LCD_PAGES - 1);

    uint8_t zero_byte = LCD_BLANK_BYTE;
    for (uint16_t i = 0; i < LCD_BUFFER_SIZE; i++) {
        LCD_Write_Data(&zero_byte, sizeof(zero_byte));
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
    LCD_Write_Command(SSD1306_CMD_SET_COLUMN_ADDR);
    LCD_Write_Command(start_col);
    LCD_Write_Command(start_col + bitmap.width - 1);

    // Ventana de páginas: start_page a start_page + pages - 1
    LCD_Write_Command(SSD1306_CMD_SET_PAGE_ADDR);
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

	uint8_t zero = LCD_BLANK_BYTE;
	LCD_Write_Command(SSD1306_CMD_SET_COLUMN_ADDR);
	LCD_Write_Command(start_col);
	LCD_Write_Command(start_col + width - 1);
	LCD_Write_Command(SSD1306_CMD_SET_PAGE_ADDR);
	LCD_Write_Command(start_page);
	LCD_Write_Command(start_page + pages - 1);
	for (uint16_t i = 0; i < (uint16_t)width * pages; i++) {
		LCD_Write_Data(&zero, sizeof(zero));
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

	if ( hidden == BLINK_HOURS ) {
		LED_ClearArea( Hour_Dec, TIME_START_PAGE, DIGIT_WIDTH, DIGIT_PAGES );
		LED_ClearArea( Hour_Unit, TIME_START_PAGE, DIGIT_WIDTH, DIGIT_PAGES );
	} else {
		LED_Draw( Hour_Dec, TIME_START_PAGE, getNumberDraw( clock.Hours / DECIMAL_BASE ) );
		LED_Draw( Hour_Unit, TIME_START_PAGE, getNumberDraw( clock.Hours % DECIMAL_BASE ) );
	}

	LED_Draw( Doble_Dot, TIME_START_PAGE, get_bm_num_doble_dot() );

	if ( hidden == BLINK_MINUTES ) {
		LED_ClearArea( Min_Dec, TIME_START_PAGE, DIGIT_WIDTH, DIGIT_PAGES );
		LED_ClearArea( Min_Unit, TIME_START_PAGE, DIGIT_WIDTH, DIGIT_PAGES );
	} else {
		LED_Draw( Min_Dec, TIME_START_PAGE, getNumberDraw( clock.Minutes / DECIMAL_BASE ) );
		LED_Draw( Min_Unit, TIME_START_PAGE, getNumberDraw( clock.Minutes % DECIMAL_BASE ) );
	}

	if ( alarmOn ) {
		LED_Draw( Alarm_Icon, TIME_START_PAGE, get_bm_alrm2() );
	} else {
		LED_ClearArea( Alarm_Icon, TIME_START_PAGE, ICON_WIDTH, ICON_PAGES );
	}
}


