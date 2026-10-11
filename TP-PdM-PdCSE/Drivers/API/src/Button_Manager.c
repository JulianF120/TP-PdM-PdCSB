/**
 * @file Button_Manager.c
 * @brief Button manager: debounce FSM and short / long press detection.
 * @author Julian Ferreira
 * @date 03/10/2026
 */

#include "general_def.h"
#include "ports.h"
#include "Button_Manager.h"

static void setShortPress( ButtonId_t btnId  );
static void setLongPress( ButtonId_t btnId  );

/**
 * @brief Possible states of the debounce FSM.
 */
typedef enum{
	BUTTON_UP,       /**< Button is released (idle). */
	BUTTON_FALLING,  /**< Falling edge detected, waiting for debounce confirmation. */
	BUTTON_DOWN,     /**< Button is confirmed pressed. */
	BUTTON_RISING,  /**< Rising edge detected, waiting for debounce confirmation. */
} debounceState_t;

/**
 * @brief State and configuration of one button.
 */
typedef struct{
	debounceState_t currentState;	/**< Current state of the debounce FSM. */
	tick_t pressStart;				/**< Tick when the press was confirmed. */
	uint32_t pin;					/**< GPIO pin of the button. */
	GPIO_TypeDef *port;				/**< GPIO port of the button. */
	delay_t bounceDelay;			/**< Debounce delay. */
	bool_t shortPressFlag;			/**< Pending short press event. */
	bool_t longPressFlag;			/**< Pending long press event. */
	bool_t longFired;				/**< The long press was already reported for the current press. */
}buttonDebounce;

/** @brief Buttons, indexed by ButtonId_t. */
static buttonDebounce buttons[] =
{
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_START_PORT, .pin = BUTTON_START_PIN},
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_UP_PORT, .pin = BUTTON_UP_PIN},
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_DOWN_PORT, .pin = BUTTON_DOWN_PIN},
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_LEFT_PORT, .pin = BUTTON_LEFT_PIN},
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_RIGHT_PORT, .pin = BUTTON_RIGHT_PIN},
};

#define DEBOUNCE_TIME_MS	40		/**< @brief Debounce time in milliseconds. */
#define LONG_PRESS_TIME_MS	2000	/**< @brief Long press time in milliseconds. */

/* Documented in Button_Manager.h */
void Buttons_Init( void ){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = BUTTON_START_PIN;
	GPIO_InitStruct.Mode = BUTTON_START_MODE;
	GPIO_InitStruct.Pull = BUTTON_START_PULL;
	HAL_GPIO_Init( BUTTON_START_PORT, &GPIO_InitStruct );

	GPIO_InitStruct.Pin = BUTTON_UP_PIN;
	GPIO_InitStruct.Mode = BUTTON_UP_MODE;
	GPIO_InitStruct.Pull = BUTTON_UP_PULL;
	HAL_GPIO_Init( BUTTON_UP_PORT, &GPIO_InitStruct );

	GPIO_InitStruct.Pin = BUTTON_DOWN_PIN;
	GPIO_InitStruct.Mode = BUTTON_DOWN_MODE;
	GPIO_InitStruct.Pull = BUTTON_DOWN_PULL;
	HAL_GPIO_Init( BUTTON_DOWN_PORT, &GPIO_InitStruct );

	GPIO_InitStruct.Pin = BUTTON_LEFT_PIN;
	GPIO_InitStruct.Mode = BUTTON_LEFT_MODE;
	GPIO_InitStruct.Pull = BUTTON_LEFT_PULL;
	HAL_GPIO_Init( BUTTON_LEFT_PORT, &GPIO_InitStruct );

	GPIO_InitStruct.Pin = BUTTON_RIGHT_PIN;
	GPIO_InitStruct.Mode = BUTTON_RIGHT_MODE;
	GPIO_InitStruct.Pull = BUTTON_RIGHT_PULL;
	HAL_GPIO_Init( BUTTON_RIGHT_PORT, &GPIO_InitStruct );

	GPIO_InitStruct.Pin = BUZZER_PIN;
	GPIO_InitStruct.Mode = BUZZER_MODE;
	GPIO_InitStruct.Pull = BUZZER_PULL;
	HAL_GPIO_Init( BUZZER_PORT, &GPIO_InitStruct );
}

/* Documented in Button_Manager.h */
void debounceFSM_init( void )
{
	for (uint8_t btnId = 0; btnId < BTN_COUNT; btnId++) {
		buttons[btnId].currentState = BUTTON_UP;
		delayInit( &buttons[btnId].bounceDelay, DEBOUNCE_TIME_MS );
		buttons[btnId].shortPressFlag = false;
		buttons[btnId].longPressFlag = false;
		buttons[btnId].longFired = false;
	}
}

/**
 * Reads the GPIO pin of every button and transitions through the FSM states:
 * BUTTON_UP -> BUTTON_FALLING -> BUTTON_DOWN -> BUTTON_RISING -> BUTTON_UP.
 * A long press flag is set once the button is held for LONG_PRESS_TIME_MS,
 * and a short press flag is set on release if no long press was detected.
 */
void debounceFSM_update( void )
{
	for (uint8_t btnId = 0; btnId < BTN_COUNT; btnId++) {
		buttonDebounce *button = &buttons[btnId];
		switch (button->currentState)
		{
			case BUTTON_UP:
				if (HAL_GPIO_ReadPin(button->port, button->pin) == BUTTON_PRESSED_LEVEL)
				{
					button->currentState = BUTTON_FALLING;
				}
			break;

			case BUTTON_FALLING:
				if (delayRead(&button->bounceDelay))
				{
					if (HAL_GPIO_ReadPin(button->port, button->pin) == BUTTON_PRESSED_LEVEL)
					{
						button->currentState = BUTTON_DOWN;
						button->pressStart = HAL_GetTick();
						button->longFired = false;
					}
					else
					{
						button->currentState = BUTTON_UP;
					}
				}
			break;

			case BUTTON_DOWN:
				setLongPress( btnId );
				if (HAL_GPIO_ReadPin(button->port, button->pin) != BUTTON_PRESSED_LEVEL)
				{
					button->currentState = BUTTON_RISING;
				}
			break;

			case BUTTON_RISING:
				if (delayRead(&button->bounceDelay))
				{
					if (HAL_GPIO_ReadPin(button->port, button->pin) != BUTTON_PRESSED_LEVEL)
					{
						if ( !button->longFired ) {
							setShortPress( btnId );
						}
						button->pressStart = 0;
						button->currentState = BUTTON_UP;
					}
					else
					{
						button->currentState = BUTTON_DOWN;
					}
				}
			break;
		}
	}
}

/**
 * @brief Latches a short press event.
 *
 * @param[in] btnId Button that was pressed.
 */
static void setShortPress( ButtonId_t btnId  )
{
	buttons[btnId].shortPressFlag = true;
}

bool_t shortPress( ButtonId_t btnId )
{
	if ( btnId >= BTN_COUNT ) {
		return false;
	}

	if (buttons[btnId].shortPressFlag)
	{
		buttons[btnId].shortPressFlag = false;
		return true;
	}
	return buttons[btnId].shortPressFlag;
}

/**
 * @brief Latches a long press event if the button was held long enough.
 *
 * The event is reported only once per press.
 *
 * @param[in] btnId Button that is pressed.
 */
static void setLongPress( ButtonId_t btnId  )
{
	buttonDebounce *b = &buttons[btnId];
	if (!b->longFired && HAL_GetTick() - b->pressStart >= LONG_PRESS_TIME_MS) {
		b->longFired = true;    // una sola vez por pulsación
		b->longPressFlag = true;
	}
}

/* Documented in Button_Manager.h */
bool_t buttonIsHeld( ButtonId_t btnId )
{
	if ( btnId >= BTN_COUNT ) {
		return false;
	}

	return buttons[btnId].longFired && buttons[btnId].currentState != BUTTON_UP;
}

bool_t longPress( ButtonId_t btnId )
{
	if ( btnId >= BTN_COUNT ) {
		return false;
	}

	if (buttons[btnId].longPressFlag)
	{
		buttons[btnId].longPressFlag = false;
		return true;
	}
	return buttons[btnId].longPressFlag;
}

void clearKeys( void )
{
	for (uint8_t i = 0; i < BTN_COUNT; i++) {
		buttons[i].shortPressFlag = false;
		buttons[i].longPressFlag = false;
	}
}

