/*
 * OLED.h
 *
 *  Created on: 4 oct. 2026
 *      Author: Julian Ferreira
 */

#ifndef API_INC_OLED_H_
#define API_INC_OLED_H_

#include "OLED_Draws.h"
#include "RTC.h"

typedef enum { BLINK_NONE, BLINK_HOURS, BLINK_MINUTES } BlinkField_t;

void OLED_Init( void );
void OLED_Clear( void );
void OLED_Draw( uint8_t start_col, uint8_t start_page, OLED_Bitmap_t bitmap );
void OLED_DrawTime( Clock_time clock, bool_t alarmOn, BlinkField_t hidden );
#endif /* API_INC_OLED_H_ */
