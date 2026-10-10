/*
 * LCD.h
 *
 *  Created on: 4 oct. 2026
 *      Author: julia
 */

#ifndef API_INC_LCD_H_
#define API_INC_LCD_H_

#include "LCD_Draws.h"
#include "RTC.h"

typedef enum { BLINK_NONE, BLINK_HOURS, BLINK_MINUTES } BlinkField_t;

void LCDInit( void );
void LCD_Clear( void );
void LED_Draw( uint8_t start_col, uint8_t start_page, OLED_Bitmap_t bitmap );
void LED_DrawTime( Clock_time clock, bool_t alarmOn, BlinkField_t hidden );
#endif /* API_INC_LCD_H_ */
