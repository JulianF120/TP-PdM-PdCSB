/**
 * @file OLED.h
 * @brief Public interface for the SSD1306 128x64 OLED display driver (SPI).
 * @author Julian Ferreira
 * @date 04/10/2026
 */

#ifndef API_INC_OLED_H_
#define API_INC_OLED_H_

#include "OLED_Draws.h"
#include "RTC.h"

/**
 * @brief Time field hidden by OLED_DrawTime(), used to make it blink.
 */
typedef enum {
	BLINK_NONE,		/**< Draw the full time. */
	BLINK_HOURS,	/**< Hide the hours. */
	BLINK_MINUTES	/**< Hide the minutes. */
} BlinkField_t;

/**
 * @brief Initializes the GPIO pins, the SPI peripheral and the display.
 *
 * Performs a hardware reset and sends the SSD1306 initialization sequence.
 * Calls Error_Handler() if the SPI peripheral or the display fail to
 * initialize.
 */
void OLED_Init( void );

/**
 * @brief Clears the whole display.
 */
void OLED_Clear( void );

/**
 * @brief Draws a bitmap on the display.
 *
 * The bitmap is not drawn if it does not fit inside the display.
 *
 * @param[in] start_col  First column (0 to 127).
 * @param[in] start_page First page (0 to 7). Each page is 8 pixels high.
 * @param[in] bitmap     Bitmap to draw.
 */
void OLED_Draw( uint8_t start_col, uint8_t start_page, OLED_Bitmap_t bitmap );

/**
 * @brief Draws a time as HH:MM and the alarm icon.
 *
 * Nothing is drawn if the time is out of range.
 *
 * @param[in] clock   Time to draw.
 * @param[in] alarmOn true to draw the alarm icon, false to erase it.
 * @param[in] hidden  Field to leave blank, used to make it blink.
 */
void OLED_DrawTime( Clock_time clock, bool_t alarmOn, BlinkField_t hidden );

#endif /* API_INC_OLED_H_ */
