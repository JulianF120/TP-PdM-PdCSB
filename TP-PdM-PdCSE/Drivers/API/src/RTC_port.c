/**
 * @file RTC_port.c
 * @brief Hardware abstraction layer of the RTC driver (I2C transfers).
 * @author Julian Ferreira
 * @date 06/10/2026
 */


#include "general_def.h"
#include "RTC_port.h"

bool_t RTC_send_data(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_OK == HAL_I2C_Mem_Write(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout);
}

bool_t RTC_read_data(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_OK == HAL_I2C_Mem_Read(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout);
}
