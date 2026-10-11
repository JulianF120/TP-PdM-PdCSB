/**
 * @file OLED_port.h
 * @brief Hardware abstraction layer of the OLED driver (SPI transfers).
 * @author Julian Ferreira
 * @date 04/10/2026
 */

#ifndef API_INC_OLED_PORT_H_
#define API_INC_OLED_PORT_H_

#include "general_def.h"

/**
 * @brief Sends a buffer through SPI.
 *
 * @param[in] hspi    SPI handle.
 * @param[in] pData   Data to send.
 * @param[in] Size    Number of bytes to send.
 * @param[in] Timeout Timeout in milliseconds.
 * @return true if the transfer succeeded, false otherwise.
 */
bool_t OLED_send_data(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif /* API_INC_OLED_PORT_H_ */
