/**
 * @file Button_Manager.h
 * @brief Public interface for the button manager (debounce, short and long press).
 *
 * Each button is debounced by its own FSM. A press produces exactly one
 * event: a long press if the button is held for the long press time, or a
 * short press when it is released before that. Events are latched until they
 * are consumed with shortPress() / longPress() or discarded with clearKeys().
 *
 * @author Julian Ferreira
 * @date 03/10/2026
 */

#ifndef API_INC_BUTTON_MANAGER_H_
#define API_INC_BUTTON_MANAGER_H_

#include "API_delay.h"

/**
 * @brief Identifiers of the available buttons.
 */
typedef enum{
	BTN_START,	/**< Start / confirm button. */
	BTN_UP,		/**< Up / increment button. */
	BTN_DOWN,	/**< Down / decrement button. */
	BTN_LEFT,	/**< Left button. */
	BTN_RIGHT,	/**< Right button. */
	BTN_COUNT	/**< Number of buttons (not a valid button). */
}ButtonId_t;

/**
 * @brief Configures the GPIO pins of the buttons and the buzzer.
 *
 * Enables the GPIO port clocks and sets every button pin as an input.
 */
void Buttons_Init( void );

/**
 * @brief Initializes the debounce FSM of every button.
 *
 * Sets all buttons to the released state, configures their debounce delay
 * and clears any pending short or long press event.
 */
void debounceFSM_init( void );

/**
 * @brief Updates the debounce FSM of every button.
 *
 * Must be called periodically from the main loop. It reads each button,
 * confirms presses and releases after the debounce time and latches the
 * short and long press events.
 */
void debounceFSM_update( void );

/**
 * @brief Reads and consumes the long press event of a button.
 *
 * A long press is reported once per press, as soon as the button has been
 * held for the long press time.
 *
 * @param[in] btnId Button to query.
 * @return true if a long press was pending (the event is cleared),
 *         false otherwise or if @p btnId is not valid.
 */
bool_t longPress( ButtonId_t btnId );

/**
 * @brief Reads and consumes the short press event of a button.
 *
 * A short press is reported when the button is released before the long
 * press time.
 *
 * @param[in] btnId Button to query.
 * @return true if a short press was pending (the event is cleared),
 *         false otherwise or if @p btnId is not valid.
 */
bool_t shortPress( ButtonId_t btnId );

/**
 * @brief Discards every pending short and long press event.
 *
 * Useful when changing state, so that presses made in a previous state are
 * not processed in the new one.
 */
void clearKeys( void );

/**
 * @brief Checks whether a button is being held past the long press time.
 *
 * Unlike longPress(), this function does not consume any event, so it can be
 * used to detect button combinations.
 *
 * @param[in] btnId Button to query.
 * @return true while the button remains pressed after the long press time,
 *         false otherwise or if @p btnId is not valid.
 */
bool_t buttonIsHeld( ButtonId_t btnId );

#endif /* API_INC_BUTTON_MANAGER_H_ */
