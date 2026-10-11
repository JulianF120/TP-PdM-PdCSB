/**
 * @file ErrorManager.c
 * @brief Error handling of the application.
 * @author Julian Ferreira
 * @date 09/10/2026
 */

#include "general_def.h"
#include "ErrorManager.h"


void Error_Handler(void) {
	HAL_NVIC_SystemReset();
}
