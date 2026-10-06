/**
 * @file API_delay.c
 * @brief Implementation of the non-blocking delay module.
 * @details Provides non-blocking delay functionality based on the HAL tick
 *          counter (HAL_GetTick). Delays do not block execution; instead,
 *          they are polled via delayRead() to check for expiration.
 * @author Julian Ferreira
 * @date 10/09/2026
 */

#include "API_delay.h"

/**
 * @brief Initializes a delay structure with the given duration.
 *
 * Sets the running flag to false and stores the specified duration.
 * If the pointer is NULL or the duration is zero, the function returns
 * without modifying anything.
 *
 * @param[in,out] delay    Pointer to the delay structure to initialize.
 * @param[in]     duration Delay duration in milliseconds. Must be > 0.
 */
void delayInit( delay_t * delay, tick_t duration ){

	if (delay == NULL || duration == 0) {
		return;
	}

	delay->running = false;
	delay->duration = duration;
}

/**
 * @brief Checks whether the configured delay has elapsed.
 *
 * On the first call (delay not running), records the current tick as the
 * start time and sets the delay to running. On subsequent calls, compares
 * the elapsed time against the configured duration. When the delay expires,
 * it automatically resets to a non-running state and returns true.
 *
 * @param[in,out] delay Pointer to the delay structure.
 * @return true if the delay has elapsed, false otherwise or if delay is NULL.
 */
bool_t delayRead( delay_t * delay ){

	if (delay == NULL) {
	        return false;
	}

	tick_t time = HAL_GetTick();

	if (!delay->running){
		delay->running = true;
		delay->startTime = time;
	}
	else {
		if (HAL_GetTick() - delay->startTime > delay->duration){
			delay->running = false;
			return true;
		}
	}
	return false;
}

/**
 * @brief Updates the duration of an existing delay.
 *
 * If the pointer is NULL or the duration is zero, the function returns
 * without modifying anything.
 *
 * @param[in,out] delay    Pointer to the delay structure.
 * @param[in]     duration New duration in milliseconds. Must be > 0.
 */
void delayWrite( delay_t * delay, tick_t duration ){

	if (delay == NULL || duration == 0) {
	        return;
	}

	delay->duration = duration;
}

/**
 * @brief Checks whether the delay is currently running.
 *
 * @param[in] delay Pointer to the delay structure.
 * @return true if the delay is running, false otherwise.
 */
bool_t delayIsRunning(delay_t * delay){
	return delay->running;
}
