/**
 * @file RTC.h
 * @brief Public interface for the DS1307 real-time clock driver (I2C) and
 *        the time helpers.
 * @author Julian Ferreira
 * @date 06/10/2026
 */

#ifndef API_INC_RTC_H_
#define API_INC_RTC_H_
#define MIN_HOUR	0	/**< @brief Lowest valid hour. */
#define MAX_HOUR	23	/**< @brief Highest valid hour (24-hour format). */
#define MAX_MIN		59	/**< @brief Highest valid minute. */
#define MIN_MIN		0	/**< @brief Lowest valid minute. */
#include "general_def.h"

/**
 * @brief Time of day, in 24-hour format.
 */
typedef struct {
	uint8_t	Hours;		/**< Hours, from MIN_HOUR to MAX_HOUR. */
	uint8_t Minutes;	/**< Minutes, from MIN_MIN to MAX_MIN. */
} Clock_time;

/**
 * @brief Initializes the GPIO pins, the I2C peripheral and the DS1307.
 *
 * Starts the RTC oscillator and enables its 1 Hz square wave output (SQW).
 * Calls Error_Handler() if the I2C peripheral fails to initialize.
 */
void RTC_Init( void );

/**
 * @brief Sets a time to 00:00.
 *
 * @param[out] clock Time to initialize. Must not be NULL.
 */
void initClock( Clock_time *clock );

/**
 * @brief Adds one hour, wrapping from 23 to 0.
 *
 * @param[in,out] clock Time to modify. Must not be NULL.
 */
void addHour( Clock_time *clock );

/**
 * @brief Subtracts one hour, wrapping from 0 to 23.
 *
 * @param[in,out] clock Time to modify. Must not be NULL.
 */
void subtractHour( Clock_time *clock );

/**
 * @brief Adds one minute, wrapping from 59 to 0 (the hour is not changed).
 *
 * @param[in,out] clock Time to modify. Must not be NULL.
 */
void addMin( Clock_time *clock );

/**
 * @brief Subtracts one minute, wrapping from 0 to 59 (the hour is not changed).
 *
 * @param[in,out] clock Time to modify. Must not be NULL.
 */
void subtractMin( Clock_time *clock );

/**
 * @brief Writes a time to the RTC and resets the seconds to 0.
 *
 * Nothing is written if the time is out of range.
 *
 * @param[in] clock Time to write.
 */
void RTC_SetTime( Clock_time clock);

/**
 * @brief Reads the current time from the RTC.
 *
 * @param[in,out] clock Where the time read is stored.
 */
void RTC_ReadTime( Clock_time *clock );

/**
 * @brief Compares two times (hours and minutes).
 *
 * @param[in] actual   First time.
 * @param[in] expected Second time.
 * @return true if both times are equal, false otherwise.
 */
bool_t sameTime( Clock_time actual, Clock_time expected);

#endif /* API_INC_RTC_H_ */
