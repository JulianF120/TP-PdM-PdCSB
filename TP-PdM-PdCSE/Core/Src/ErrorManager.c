#include "general_def.h"


void Error_Handler(void) {
    HAL_NVIC_SystemReset();
}
