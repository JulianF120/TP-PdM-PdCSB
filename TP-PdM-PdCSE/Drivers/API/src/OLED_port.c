/*
 * OLED_port.c
 *
 *  Created on: 4 oct. 2026
 *      Author: Julian Ferreira
 */
#include "general_def.h"
#include "OLED_port.h"

bool_t OLED_send_data(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_OK == HAL_SPI_Transmit(hspi, pData, Size, Timeout);
}


