#include "general_def.h"
#include "ErrorManager.h"


void Error_Handler(void) {
    HAL_NVIC_SystemReset();
}
