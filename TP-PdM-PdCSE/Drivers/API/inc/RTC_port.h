/**
 * @file RTC_port.h
 * @brief Hardware abstraction layer of the RTC driver (I2C transfers).
 * @author Julian Ferreira
 * @date 06/10/2026
 */

#ifndef API_INC_RTC_PORT_H_
#define API_INC_RTC_PORT_H_

#include "general_def.h"

/**
 * @brief Writes a buffer to a register of an I2C device.
 *
 * @param[in] hi2c       I2C handle.
 * @param[in] DevAddress Device address, already shifted left by one bit.
 * @param[in] MemAddress First register to write.
 * @param[in] MemAddSize Size of the register address (I2C_MEMADD_SIZE_8BIT or I2C_MEMADD_SIZE_16BIT).
 * @param[in] pData      Data to write.
 * @param[in] Size       Number of bytes to write.
 * @param[in] Timeout    Timeout in milliseconds.
 * @return true if the transfer succeeded, false otherwise.
 */
bool_t RTC_send_data( I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout );

/**
 * @brief Reads a buffer from a register of an I2C device.
 *
 * @param[in]  hi2c       I2C handle.
 * @param[in]  DevAddress Device address, already shifted left by one bit.
 * @param[in]  MemAddress First register to read.
 * @param[in]  MemAddSize Size of the register address (I2C_MEMADD_SIZE_8BIT or I2C_MEMADD_SIZE_16BIT).
 * @param[out] pData      Where the data read is stored.
 * @param[in]  Size       Number of bytes to read.
 * @param[in]  Timeout    Timeout in milliseconds.
 * @return true if the transfer succeeded, false otherwise.
 */
bool_t RTC_read_data( I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout );

#endif /* API_INC_RTC_PORT_H_ */
