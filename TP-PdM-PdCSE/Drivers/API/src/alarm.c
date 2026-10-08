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

static void idelState();
static void confClockH_State();
static void confClockM_State();
static void confAlarmH_State();
static void confAlarmM_State();
static void updateState();
static void alarmRingState();

const uint16_t secondsDelay = 2000; //miliseconds

typedef enum{
	IDLE,
	CONF_CLK_H,
	CONF_CLK_M,
	CONF_ALARM_H,
	CONF_ALARM_M,
	UPDATE,
	ALARM_RING
}alarmStates;

static alarmStates actualState;
static bool_t statusAlarm;
static Clock_time clock;
static Clock_time alarm;
static GPIO_PinState sqw_prev;
static bool_t ringOff;

void initAlarmFSM()
{
	statusAlarm = false;
	actualState = IDLE;
	sqw_prev = GPIO_PIN_RESET;
	ringOff = false;
	initClock( &clock );
	initClock( &alarm );

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
		break;
	}
}

static void idelState()
{
	GPIO_PinState sqw_curr = HAL_GPIO_ReadPin( I2C_RTC_SQW_PORT, I2C_RTC_SQW_PIN );
	if ( ringOff && sameTime( clock, alarm ) ) {
		ringOff = !ringOff;
	}
	if ( buttonIsPress( BTN_START ) && timePressed( BTN_START ) >= secondsDelay ) {
		actualState = CONF_CLK_H;
	}
	else if ( buttonIsPress( BTN_UP ) && timePressed( BTN_UP ) >= secondsDelay ) {
		actualState = CONF_ALARM_H;
	}
	else if ( buttonIsPress( BTN_RIGHT) && buttonIsPress( BTN_LEFT ) &&
		timePressed( BTN_RIGHT ) >= secondsDelay && timePressed( BTN_LEFT ) >= secondsDelay ) {
		statusAlarm = !statusAlarm;
	}
	else if ( statusAlarm && sameTime(clock, alarm) && !ringOff )
	{
		actualState = ALARM_RING;
	}
	else if (sqw_prev == GPIO_PIN_RESET && sqw_curr == GPIO_PIN_SET)
	{
		actualState = UPDATE;
	}
	sqw_prev = sqw_curr;
}

static void confClockH_State()
{
	if ( buttonIsPress( BTN_LEFT ) || buttonIsPress( BTN_RIGHT ) ) {
		actualState = CONF_CLK_M;
	}
	else if (buttonIsPress( BTN_START )) {
		RTC_SetTime( clock );
		actualState = IDLE;
	}
	else if ( buttonIsPress( BTN_UP ) ) {
		addHour( &clock );
		//Enviar update a la pantalla
	}
	else if ( buttonIsPress( BTN_DOWN ) ) {
		substractHour( &clock );
		//Enviar update a la pantalla
	}
}

static void confClockM_State()
{
	if ( buttonIsPress( BTN_LEFT ) || buttonIsPress( BTN_RIGHT ) ) {
		actualState = CONF_CLK_H;
	}
	else if (buttonIsPress( BTN_START )) {
		RTC_SetTime( clock );
		actualState = IDLE;
	}
	else if ( buttonIsPress( BTN_UP ) ) {
		addMin( &clock );
		//Enviar update a la pantalla
	}
	else if ( buttonIsPress( BTN_DOWN ) ) {
		substractMin( &clock );
		//Enviar update a la pantalla
	}
}

static void confAlarmH_State()
{
	if ( buttonIsPress( BTN_LEFT ) || buttonIsPress( BTN_RIGHT ) ) {
		actualState = CONF_ALARM_M;
	}
	else if (buttonIsPress( BTN_START )) {
		actualState = IDLE;
	}
	else if ( buttonIsPress( BTN_UP ) ) {
		addHour( &alarm );
		//Enviar update a la pantalla
	}
	else if ( buttonIsPress( BTN_DOWN ) ) {
		substractHour( &alarm );
		//Enviar update a la pantalla
	}
}

static void confAlarmM_State()
{
	if ( buttonIsPress( BTN_LEFT ) || buttonIsPress( BTN_RIGHT ) ) {
		actualState = CONF_ALARM_H;
	}
	else if (buttonIsPress( BTN_START )) {
		actualState = IDLE;
	}
	else if ( buttonIsPress( BTN_UP ) ) {
		addMin( &alarm );
		//Enviar update a la pantalla
	}
	else if ( buttonIsPress( BTN_DOWN ) ) {
		substractMin( &alarm );
		//Enviar update a la pantalla
	}
}

static void updateState()
{
	RTC_ReadTime( &clock );
	//actualizar pantalla
	actualState = IDLE;
}

static void alarmRingState()
{
	if ( buttonIsPress( BTN_START ) ) {
		actualState = IDLE;
	}
	//Ver como manjear el buzzer
}
