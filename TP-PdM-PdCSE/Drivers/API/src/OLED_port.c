/**
 * @file OLED_port.c
 * @brief Hardware abstraction layer of the OLED driver (SPI transfers).
 * @author Julian Ferreira
 * @date 04/10/2026
 */
#include "general_def.h"
#include "OLED_port.h"

bool_t OLED_send_data(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_OK == HAL_SPI_Transmit(hspi, pData, Size, Timeout);
}


