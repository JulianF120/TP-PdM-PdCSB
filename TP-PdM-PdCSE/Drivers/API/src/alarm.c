/**
 * @file alarm.c
 * @brief Alarm clock FSM.
 * @author Julian Ferreira
 * @date 05/10/2026
 */


#include "general_def.h"
#include "Button_Manager.h"
#include "RTC.h"
#include "ports.h"
#include "OLED.h"

#define BLINK_PERIOD_MS 500			/**< @brief Blink period of the field being set, in ms. */
#define ALARM_RING_PERIOD_MS 200	/**< @brief On / off period of the buzzer while ringing, in ms. */

static void idleState( void );
static void confClockH_State( void );
static void confClockM_State( void );
static void confAlarmH_State( void );
static void confAlarmM_State( void );
static void updateState( void );
static void alarmRingState( void );
static void BuzzerManager( void );

/**
 * @brief States of the alarm FSM.
 */
typedef enum{
	IDLE,			/**< Shows the time and waits for buttons, the alarm or the next second. */
	CONF_CLK_H,		/**< Setting the clock hours. */
	CONF_CLK_M,		/**< Setting the clock minutes. */
	CONF_ALARM_H,	/**< Setting the alarm hours. */
	CONF_ALARM_M,	/**< Setting the alarm minutes. */
	UPDATE,			/**< Reads the time from the RTC and redraws it. */
	ALARM_RING		/**< The alarm is ringing. */
}alarmStates;

static void changeState( alarmStates next );
static void blinkRestart( void );
static void drawConf( Clock_time t, BlinkField_t field );

static alarmStates currentState;	/**< @brief Current state of the FSM. */
static bool_t alarmEnabled;			/**< @brief The alarm is enabled. */
static Clock_time clock;			/**< @brief Current time (or the time being set). */
static Clock_time alarm;			/**< @brief Alarm time. */
static GPIO_PinState sqwPrev;		/**< @brief Previous level of the SQW pin, to detect its rising edge. */
static bool_t ringOff;				/**< @brief The alarm was turned off during the current alarm minute. */
static delay_t blinkDelay;			/**< @brief Blink timer of the field being set. */
static bool_t blinkVisible;			/**< @brief The field being set is currently visible. */
static bool_t redraw;				/**< @brief The display must be redrawn. */
static bool_t comboHandled;			/**< @brief The LEFT + RIGHT combo was already handled for the current press. */
static delay_t ringDelay;			/**< @brief Buzzer on / off timer. */

/* Documented in alarm.h */
void alarmFSM_init( void )
{
	alarmEnabled = false;
	currentState = IDLE;
	sqwPrev = GPIO_PIN_RESET;
	ringOff = false;
	initClock( &clock );
	initClock( &alarm );
	delayInit( &blinkDelay, BLINK_PERIOD_MS );
	delayInit( &ringDelay, ALARM_RING_PERIOD_MS );
	blinkVisible = true;
	redraw = true;
	comboHandled = false;
	updateState();
}

/* Documented in alarm.h */
void alarmFSM_update( void )
{
	switch(currentState){
	case IDLE:
		idleState();
		break;
	case CONF_CLK_H:
		confClockH_State();
		break;
	case CONF_CLK_M:
		confClockM_State();
		break;
	case CONF_ALARM_H:
		confAlarmH_State();
		break;
	case CONF_ALARM_M:
		confAlarmM_State();
		break;
	case UPDATE:
		updateState();
		break;
	case ALARM_RING:
		alarmRingState();
		break;
	default:
		changeState( IDLE );
		break;
	}
}

/**
 * @brief IDLE state.
 *
 * Handles, in order of priority: entering the clock or alarm setup, toggling
 * the alarm, starting the alarm, and reading the time on each rising edge of
 * the RTC SQW signal (once per second).
 */
static void idleState( void )
{
	GPIO_PinState sqwCurr = HAL_GPIO_ReadPin( RTC_SQW_PORT, RTC_SQW_PIN );
	if ( ringOff && !sameTime( clock, alarm ) ) {
		ringOff = false;
	}
	if ( longPress( BTN_START ) ) {
		changeState( CONF_CLK_H );
	}
	else if ( longPress( BTN_UP ) ) {
		changeState( CONF_ALARM_H );
	}
	else if ( buttonIsHeld( BTN_RIGHT ) && buttonIsHeld( BTN_LEFT ) && !comboHandled ) {
		alarmEnabled = !alarmEnabled;
		OLED_DrawTime( clock, alarmEnabled, BLINK_NONE );
		comboHandled = true;
	}
	else if ( alarmEnabled && sameTime(clock, alarm) && !ringOff )
	{
		changeState( ALARM_RING );
	}
	else if (sqwPrev == GPIO_PIN_RESET && sqwCurr == GPIO_PIN_SET)
	{
		changeState( UPDATE );
	}
	if ( !buttonIsHeld( BTN_RIGHT ) || !buttonIsHeld( BTN_LEFT ) ) {
		comboHandled = false;
	}
	sqwPrev = sqwCurr;
}

/**
 * @brief CONF_CLK_H state: sets the clock hours.
 *
 * START writes the time to the RTC and returns to IDLE.
 */
static void confClockH_State( void )
{
	drawConf( clock, BLINK_HOURS );
	if ( shortPress( BTN_LEFT ) || shortPress( BTN_RIGHT ) ) {
		changeState( CONF_CLK_M );
	}
	else if (shortPress( BTN_START )) {
		RTC_SetTime( clock );
		OLED_DrawTime( clock, alarmEnabled, BLINK_NONE );
		changeState( IDLE );
	}
	else if ( shortPress( BTN_UP ) ) {
		addHour( &clock );
		blinkRestart();
	}
	else if ( shortPress( BTN_DOWN ) ) {
		subtractHour( &clock );
		blinkRestart();
	}
}

/**
 * @brief CONF_CLK_M state: sets the clock minutes.
 *
 * START writes the time to the RTC and returns to IDLE.
 */
static void confClockM_State( void )
{
	drawConf( clock, BLINK_MINUTES );
	if ( shortPress( BTN_LEFT ) || shortPress( BTN_RIGHT ) ) {
		changeState( CONF_CLK_H );
	}
	else if ( shortPress( BTN_START )) {
		RTC_SetTime( clock );
		OLED_DrawTime( clock, alarmEnabled, BLINK_NONE );
		changeState( IDLE );
	}
	else if ( shortPress( BTN_UP ) ) {
		addMin( &clock );
		blinkRestart();
	}
	else if ( shortPress( BTN_DOWN ) ) {
		subtractMin( &clock );
		blinkRestart();
	}
}

/**
 * @brief CONF_ALARM_H state: sets the alarm hours.
 *
 * START returns to IDLE.
 */
static void confAlarmH_State( void )
{
	drawConf( alarm, BLINK_HOURS );
	if ( shortPress( BTN_LEFT ) || shortPress( BTN_RIGHT ) ) {
		changeState( CONF_ALARM_M );
	}
	else if (shortPress( BTN_START )) {
		changeState( IDLE );
		OLED_DrawTime( clock, alarmEnabled, BLINK_NONE );
	}
	else if ( shortPress( BTN_UP ) ) {
		addHour( &alarm );
		blinkRestart();
	}
	else if ( shortPress( BTN_DOWN ) ) {
		subtractHour( &alarm );
		blinkRestart();
	}
}

/**
 * @brief CONF_ALARM_M state: sets the alarm minutes.
 *
 * START returns to IDLE.
 */
static void confAlarmM_State( void )
{
	drawConf( alarm, BLINK_MINUTES );
	if ( shortPress( BTN_LEFT ) || shortPress( BTN_RIGHT ) ) {
		changeState( CONF_ALARM_H );
	}
	else if ( shortPress( BTN_START )) {
		changeState( IDLE );
		OLED_DrawTime( clock, alarmEnabled, BLINK_NONE );
	}
	else if ( shortPress( BTN_UP ) ) {
		addMin( &alarm );
		blinkRestart();
	}
	else if ( shortPress( BTN_DOWN ) ) {
		subtractMin( &alarm );
		blinkRestart();
	}
}

/**
 * @brief UPDATE state: reads the time from the RTC, redraws it and returns to IDLE.
 */
static void updateState( void )
{
	RTC_ReadTime( &clock );
	OLED_DrawTime( clock, alarmEnabled, BLINK_NONE );
	currentState = IDLE;
}

/**
 * @brief ALARM_RING state: rings the buzzer until START is pressed.
 */
static void alarmRingState( void )
{
	if ( shortPress( BTN_START ) ) {
		changeState( IDLE );
		HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, BUZZER_OFF_LEVEL);
		ringOff = true;
	}
	BuzzerManager();
}

/**
 * @brief Changes the FSM state.
 *
 * Discards the pending button events and restarts the blink, so the new
 * state starts from a known condition.
 *
 * @param[in] next New state.
 */
static void changeState( alarmStates next )
{
	clearKeys();
	blinkRestart();
	currentState = next;
}

/**
 * @brief Restarts the blink with the field visible and requests a redraw.
 *
 * Called when entering a setup state and when the value changes, so the new
 * value is visible right away.
 */
static void blinkRestart( void )
{
	blinkVisible = true;
	delayInit( &blinkDelay, BLINK_PERIOD_MS );   // reinicia la cuenta
	redraw = true;
}

/**
 * @brief Draws a time while it is being set, making one field blink.
 *
 * The display is only redrawn when the blink toggles or the value changes.
 *
 * @param[in] t     Time to draw.
 * @param[in] field Field that blinks.
 */
static void drawConf( Clock_time t, BlinkField_t field )
{
	if ( delayRead( &blinkDelay ) ) {
		blinkVisible = !blinkVisible;
		redraw = true;
	}
	if ( redraw ) {
		OLED_DrawTime( t, alarmEnabled, blinkVisible ? BLINK_NONE : field );
		redraw = false;
	}
}

/**
 * @brief Toggles the buzzer every ALARM_RING_PERIOD_MS.
 */
static void BuzzerManager( void )
{
	if ( delayRead( &ringDelay ) ) {
		HAL_GPIO_TogglePin( BUZZER_PORT, BUZZER_PIN );
	}
}
