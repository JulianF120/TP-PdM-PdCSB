/*
 * alarm.c
 *
 *  Created on: 5 oct. 2026
 *      Author: julia
 */


#include "general_def.h"
#include "Button_Manager.h"
#include "stm32f4xx_hal_rtc.h"
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

void initAlarmFSM()
{
	statusAlarm = false;
}

void alarmFSM_update( )
{
	switch(actualState){
	case IDLE:
		break;
	case CONF_CLK_H:
		break;
	case CONF_CLK_M:
		break;
	case CONF_ALARM_H:
		break;
	case CONF_ALARM_M:
		break;
	case UPDATE:
		break;
	case ALARM_RING:
		break;
	default:
		break;
	}
}

static void idelState()
{
	if ( buttonIsPress( BTN_START ) && timePressed( BTN_START ) >= secondsDelay ) {
		actualState = CONF_CLK_H;
	}
	else if ( buttonIsPress( BTN_UP ) && timePressed( BTN_UP ) >= secondsDelay ) {
		actualState = CONF_ALARM_H;
	}
	//Falta definir como manejar la hora. Para manejra el alarm ringing
	else if ( buttonIsPress( BTN_RIGHT) && buttonIsPress( BTN_LEFT ) &&
		timePressed( BTN_RIGHT ) >= secondsDelay && timePressed( BTN_LEFT ) >= secondsDelay ) {
		statusAlarm = !statusAlarm;
	}
}
