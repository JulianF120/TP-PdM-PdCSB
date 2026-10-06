/*
 * LCD.h
 *
 *  Created on: 4 oct. 2026
 *      Author: julia
 */

#ifndef API_INC_LCD_H_
#define API_INC_LCD_H_

#include "LCD_Draws.h"

void LCDInit( void );
void LCD_Clear( void );
void LED_Draw( uint8_t start_col, uint8_t start_page, OLED_Bitmap_t bitmap );

#endif /* API_INC_LCD_H_ */
