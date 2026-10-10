/*
 * LCD_port.h
 *
 *  Created on: 4 oct. 2026
 *      Author: julia
 */

#ifndef API_INC_LCD_PORT_H_
#define API_INC_LCD_PORT_H_

bool_t LCD_send_data(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif /* API_INC_LCD_PORT_H_ */
