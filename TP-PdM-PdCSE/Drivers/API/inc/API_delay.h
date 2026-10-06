/**
 * @file API_delay.h
 * @brief Public interface for the non-blocking delay module.
 * @author Julian Ferreira
 * @date 10/09/2026
 */

#ifndef API_INC_API_DELAY_H_
#define API_INC_API_DELAY_H_

#include "general_def.h"

typedef uint32_t tick_t;   /**< @brief Type for tick counts in milliseconds. */

/**
 * @brief Structure representing a non-blocking delay.
 */
typedef struct{
   tick_t startTime;   /**< @brief Tick value when the delay was started. */
   tick_t duration;    /**< @brief Duration of the delay in milliseconds. */
   bool_t running;     /**< @brief Flag indicating whether the delay is currently running. */
} delay_t;

/**
 * @brief Initializes a delay structure with the given duration.
 *
 * Sets the delay to a non-running state and stores the specified duration.
 *
 * @param[in,out] delay    Pointer to the delay structure to initialize.
 * @param[in]     duration Delay duration in milliseconds. Must be > 0.
 */
void delayInit( delay_t * delay, tick_t duration );

/**
 * @brief Checks whether the configured delay has elapsed.
 *
 * On the first call, starts the delay by recording the current tick.
 * On subsequent calls, checks if the elapsed time exceeds the configured
 * duration. When the delay expires, it stops automatically and returns true.
 *
 * @param[in,out] delay Pointer to the delay structure.
 * @return true if the delay has elapsed, false otherwise.
 */
bool_t delayRead( delay_t * delay );

/**
 * @brief Updates the duration of an existing delay.
 *
 * @param[in,out] delay    Pointer to the delay structure.
 * @param[in]     duration New duration in milliseconds. Must be > 0.
 */
void delayWrite( delay_t * delay, tick_t duration );

/**
 * @brief Checks whether the delay is currently running.
 *
 * @param[in] delay Pointer to the delay structure.
 * @return true if the delay is running, false otherwise.
 */
bool_t delayIsRunning( delay_t * delay );

#endif /* API_INC_API_DELAY_H_ */
