/*
 * RTC_port.h
 *
 *  Created on: 6 oct. 2026
 *      Author: Julian Ferreira
 */

#ifndef API_INC_RTC_PORT_H_
#define API_INC_RTC_PORT_H_

#include "general_def.h"

bool_t RTC_send_data( I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout );
bool_t RTC_read_data( I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout );

#endif /* API_INC_RTC_PORT_H_ */
