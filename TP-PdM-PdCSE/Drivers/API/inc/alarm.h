/**
 * @file alarm.h
 * @brief Public interface for the alarm clock FSM.
 *
 * The FSM shows the time on the OLED display, lets the user set the clock
 * and the alarm with the buttons, and rings the buzzer when the alarm time
 * is reached.
 *
 * Controls:
 * - Hold START: set the clock.
 * - Hold UP: set the alarm.
 * - Hold LEFT + RIGHT: enable / disable the alarm.
 * - While setting: UP / DOWN change the value, LEFT / RIGHT switch between
 *   hours and minutes, START confirms.
 * - While ringing: START turns the alarm off.
 *
 * @author Julian Ferreira
 * @date 05/10/2026
 */

#ifndef API_INC_ALARM_H_
#define API_INC_ALARM_H_

/**
 * @brief Initializes the alarm FSM.
 *
 * Disables the alarm, sets the clock and the alarm to 00:00, and then reads
 * and draws the current time from the RTC. The buttons, the RTC and the
 * display must be initialized before calling this function.
 */
void alarmFSM_init( void );

/**
 * @brief Runs one step of the alarm FSM.
 *
 * Must be called periodically from the main loop, after debounceFSM_update().
 */
void alarmFSM_update( void );

#endif /* API_INC_ALARM_H_ */
