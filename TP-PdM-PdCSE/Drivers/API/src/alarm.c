/*
 * alarm.c
 *
 *  Created on: 5 oct. 2026
 *      Author: Julian Ferreira
 */


#include "general_def.h"
#include "Button_Manager.h"
#include "RTC.h"
#include "ports.h"
#include "OLED.h"

#define BLINK_PERIOD_MS 500
#define ALARM_RING_PERIOD_MS 200

static void idleState( void );
static void confClockH_State( void );
static void confClockM_State( void );
static void confAlarmH_State( void );
static void confAlarmM_State( void );
static void updateState( void );
static void alarmRingState( void );
static void BuzzerManager( void );

typedef enum{
	IDLE,
	CONF_CLK_H,
	CONF_CLK_M,
	CONF_ALARM_H,
	CONF_ALARM_M,
	UPDATE,
	ALARM_RING
}alarmStates;

static void changeState( alarmStates next );
static void blinkRestart( void );
static void drawConf( Clock_time t, BlinkField_t field );

static alarmStates currentState;
static bool_t alarmEnabled;
static Clock_time clock;
static Clock_time alarm;
static GPIO_PinState sqwPrev;
static bool_t ringOff;
static delay_t blinkDelay;
static bool_t blinkVisible;
static bool_t redraw;
static bool_t comboHandled;
static delay_t ringDelay;

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

static void updateState( void )
{
	RTC_ReadTime( &clock );
	OLED_DrawTime( clock, alarmEnabled, BLINK_NONE );
	currentState = IDLE;
}

static void alarmRingState( void )
{
	if ( shortPress( BTN_START ) ) {
		changeState( IDLE );
		HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, BUZZER_OFF_LEVEL);
		ringOff = true;
	}
	BuzzerManager();
}

static void changeState( alarmStates next )
{
	clearKeys();
	blinkRestart();
	currentState = next;
}

static void blinkRestart( void )   // al entrar a un CONF_* o al cambiar el valor
{
	blinkVisible = true;
	delayInit( &blinkDelay, BLINK_PERIOD_MS );   // reinicia la cuenta
	redraw = true;
}

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

static void BuzzerManager( void )
{
	if ( delayRead( &ringDelay ) ) {
		HAL_GPIO_TogglePin( BUZZER_PORT, BUZZER_PIN );
	}
}
