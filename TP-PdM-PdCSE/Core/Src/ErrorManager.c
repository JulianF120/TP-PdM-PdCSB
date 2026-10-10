/*
 * ErrorManager.c
 *
 *  Created on: 9 oct. 2026
 *      Author: Julian Ferreira
 */

#include "general_def.h"
#include "ErrorManager.h"


void Error_Handler(void) {
	HAL_NVIC_SystemReset();
}
