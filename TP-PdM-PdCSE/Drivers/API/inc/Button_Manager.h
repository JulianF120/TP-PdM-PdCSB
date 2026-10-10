/*
 * Button_Manager.h
 *
 *  Created on: 3 oct. 2026
 *      Author: Julian Ferreira
 */

#ifndef API_INC_BUTTON_MANAGER_H_
#define API_INC_BUTTON_MANAGER_H_

#include "API_delay.h"

typedef enum{
	BTN_START,
	BTN_UP,
	BTN_DOWN,
	BTN_LEFT,
	BTN_RIGHT,
	BTN_COUNT
}ButtonId_t;

void Buttons_Init( void );
void debounceFSM_init( void );
void debounceFSM_update( void );
bool_t longPress( ButtonId_t btnId );
bool_t shortPress( ButtonId_t btnId );
void clearKeys( void );
bool_t buttonIsHeld( ButtonId_t btnId );

#endif /* API_INC_BUTTON_MANAGER_H_ */
