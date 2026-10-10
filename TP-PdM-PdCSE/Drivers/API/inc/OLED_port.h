/*
 * OLED_port.h
 *
 *  Created on: 4 oct. 2026
 *      Author: Julian Ferreira
 */

#ifndef API_INC_OLED_PORT_H_
#define API_INC_OLED_PORT_H_

#include "general_def.h"

bool_t OLED_send_data(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif /* API_INC_OLED_PORT_H_ */
