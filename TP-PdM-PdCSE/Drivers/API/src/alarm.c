/*
 * alarm.c
 *
 *  Created on: 5 oct. 2026
 *      Author: julia
 */


#include "general_def.h"
#include "Button_Manager.h"
#include "RTC.h"
#include "ports.h"
#include "LCD.h"

#define BLINK_PERIOD_MS 500
#define ALARM_RING_PERIOD_MS 200

static void idelState();
static void confClockH_State();
static void confClockM_State();
static void confAlarmH_State();
static void confAlarmM_State();
static void updateState();
static void alarmRingState();
static void BuzzerManager();

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

static alarmStates actualState;
static bool_t statusAlarm;
static Clock_time clock;
static Clock_time alarm;
static GPIO_PinState sqw_prev;
static bool_t ringOff;
static delay_t blinkDelay;
static bool_t blinkVisible;
static bool_t redraw;
static bool_t comboHandled;
static delay_t ringDelay;

void initAlarmFSM()
{
	statusAlarm = false;
	actualState = IDLE;
	sqw_prev = GPIO_PIN_RESET;
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

void alarmFSM_update( )
{
	switch(actualState){
	case IDLE:
		idelState();
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

static void idelState()
{
	GPIO_PinState sqw_curr = HAL_GPIO_ReadPin( I2C_RTC_SQW_PORT, I2C_RTC_SQW_PIN );
	if ( ringOff && !sameTime( clock, alarm ) ) {
		ringOff = !ringOff;
	}
	if ( longPress( BTN_START ) ) {
		changeState( CONF_CLK_H );
	}
	else if ( longPress( BTN_UP ) ) {
		changeState( CONF_ALARM_H );
	}
	else if ( buttonIsHeld( BTN_RIGHT ) && buttonIsHeld( BTN_LEFT ) && !comboHandled ) {
		statusAlarm = !statusAlarm;
		LED_DrawTime( clock, statusAlarm, BLINK_NONE );
		comboHandled = true;
	}
	else if ( statusAlarm && sameTime(clock, alarm) && !ringOff )
	{
		changeState( ALARM_RING );
	}
	else if (sqw_prev == GPIO_PIN_RESET && sqw_curr == GPIO_PIN_SET)
	{
		changeState( UPDATE );
	}
	if ( !buttonIsHeld( BTN_RIGHT ) || !buttonIsHeld( BTN_LEFT ) ) {
			comboHandled = false;
	}
	sqw_prev = sqw_curr;
}

static void confClockH_State()
{
	drawConf( clock, BLINK_HOURS );
	if ( shortPress( BTN_LEFT ) || shortPress( BTN_RIGHT ) ) {
		changeState( CONF_CLK_M );
	}
	else if (shortPress( BTN_START )) {
		RTC_SetTime( clock );
		LED_DrawTime( clock, statusAlarm, BLINK_NONE );
		changeState( IDLE );
	}
	else if ( shortPress( BTN_UP ) ) {
		addHour( &clock );
		blinkRestart();
	}
	else if ( shortPress( BTN_DOWN ) ) {
		substractHour( &clock );
		blinkRestart();
	}
}

static void confClockM_State()
{
	drawConf( clock, BLINK_MINUTES );
	if ( shortPress( BTN_LEFT ) || shortPress( BTN_RIGHT ) ) {
		changeState( CONF_CLK_H );
	}
	else if ( shortPress( BTN_START )) {
		RTC_SetTime( clock );
		LED_DrawTime( clock, statusAlarm, BLINK_NONE );
		changeState( IDLE );
	}
	else if ( shortPress( BTN_UP ) ) {
		addMin( &clock );
		blinkRestart();
	}
	else if ( shortPress( BTN_DOWN ) ) {
		substractMin( &clock );
		blinkRestart();
	}
}

static void confAlarmH_State()
{
	drawConf( alarm, BLINK_HOURS );
	if ( shortPress( BTN_LEFT ) || shortPress( BTN_RIGHT ) ) {
		changeState( CONF_ALARM_M );
	}
	else if (shortPress( BTN_START )) {
		changeState( IDLE );
		LED_DrawTime( clock, statusAlarm, BLINK_NONE );
	}
	else if ( shortPress( BTN_UP ) ) {
		addHour( &alarm );
		blinkRestart();
	}
	else if ( shortPress( BTN_DOWN ) ) {
		substractHour( &alarm );
		blinkRestart();
	}
}

static void confAlarmM_State()
{
	drawConf( alarm, BLINK_MINUTES );
	if ( shortPress( BTN_LEFT ) || shortPress( BTN_RIGHT ) ) {
		changeState( CONF_ALARM_H );
	}
	else if ( shortPress( BTN_START )) {
		changeState( IDLE );
		LED_DrawTime( clock, statusAlarm, BLINK_NONE );
	}
	else if ( shortPress( BTN_UP ) ) {
		addMin( &alarm );
		blinkRestart();
	}
	else if ( shortPress( BTN_DOWN ) ) {
		substractMin( &alarm );
		blinkRestart();
	}
}

static void updateState()
{
	RTC_ReadTime( &clock );
	LED_DrawTime( clock, statusAlarm, BLINK_NONE );
	actualState = IDLE;
}

static void alarmRingState()
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
	actualState = next;
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
		LED_DrawTime( t, statusAlarm, blinkVisible ? BLINK_NONE : field );
		redraw = false;
	}
}

static void BuzzerManager()
{
	if ( delayRead( &ringDelay ) ) {
		HAL_GPIO_TogglePin( BUZZER_PORT, BUZZER_PIN );
	}
}
