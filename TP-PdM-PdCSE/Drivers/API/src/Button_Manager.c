/*
 * Button_Manager.c
 *
 *  Created on: 3 oct. 2026
 *      Author: julia
 */

#include "general_def.h"
#include "ports.h"
#include "Button_Manager.h"

/**
 * @brief Possible states of the debounce FSM.
 */
typedef enum{
	BUTTON_UP,       /**< Button is released (idle). */
	BUTTON_FALLING,  /**< Falling edge detected, waiting for debounce confirmation. */
	BUTTON_DOWN,     /**< Button is confirmed pressed. */
	BUTTON_RAISING,  /**< Rising edge detected, waiting for debounce confirmation. */
} debounceState_t;

typedef struct{
	debounceState_t currentState;
	tick_t pressStart;
	uint32_t pin;
	GPIO_TypeDef *port;
	delay_t bounceDealy;
	bool_t wasPressed;
}buttonDebounce;

static buttonDebounce buttons[] =
{
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_START_PORT, .pin = BUTTON_START_PIN},
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_UP_PORT, .pin = BUTTON_UP_PIN},
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_DOWN_PORT, .pin = BUTTON_DOWN_PIN},
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_LEFT_PORT, .pin = BUTTON_LEFT_PIN},
		{.currentState = BUTTON_UP, .pressStart = 0, .port = BUTTON_RIGHT_PORT, .pin = BUTTON_RIGHT_PIN},
};

static uint8_t debounceTime = 40;     /**< @brief Debounce time in milliseconds. */

void initButtons( void ){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();

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
}

/**
 * @brief Initializes the debounce FSM.
 *
 * Configures the debounce delay and sets the initial state to BUTTON_UP.
 */
void debounceFSM_init( void )
{
	for (uint8_t btnId = 0; btnId < BTN_COUNT; btnId++) {
		buttons[btnId].currentState = BUTTON_UP;
		delayInit( &buttons[btnId].bounceDealy, debounceTime );
		buttons[btnId].wasPressed = false;
	}
}

/**
 * @brief Updates the debounce FSM based on the current button state.
 *
 * Reads the B1 button GPIO pin and transitions through the FSM states.
 * When a stable press or release is confirmed after the debounce delay,
 * the corresponding callback (buttonPressed / buttonReleased) is invoked.
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
				if (delayRead(&button->bounceDealy))
				{
					if (HAL_GPIO_ReadPin(button->port, button->pin) == BUTTON_PRESSED_LEVEL)
					{
						button->wasPressed = true;
						button->currentState = BUTTON_DOWN;
						button->pressStart = HAL_GetTick();
					}
					else
					{
						button->currentState = BUTTON_UP;
					}
				}
			break;

			case BUTTON_DOWN:
				if (HAL_GPIO_ReadPin(button->port, button->pin) != BUTTON_PRESSED_LEVEL)
				{
					button->currentState = BUTTON_RAISING;
				}
			break;

			case BUTTON_RAISING:
				if (delayRead(&button->bounceDealy))
				{
					if (HAL_GPIO_ReadPin(button->port, button->pin) != BUTTON_PRESSED_LEVEL)
					{
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

bool_t buttonIsPress( ButtonId_t btnId )
{
	return buttons[btnId].currentState == BUTTON_DOWN;
}



tick_t timePressed(ButtonId_t btnId)
{
    return buttons[btnId].currentState == BUTTON_DOWN ? HAL_GetTick() - buttons[btnId].pressStart : 0;
}

bool_t readKey(ButtonId_t btnId)
{
	if (buttons[btnId].wasPressed)
	{
		buttons[btnId].wasPressed = false;
		return true;
	}
	return buttons[btnId].wasPressed;
}
