/*
 * LCD_port.c
 *
 *  Created on: 4 oct. 2026
 *      Author: julia
 */
#include "general_def.h"

void LCD_send_data(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	HAL_SPI_Transmit(hspi, pData, Size, Timeout);
}


