/*
 * RTC.h
 *
 *  Created on: 6 oct. 2026
 *      Author: julia
 */

#ifndef API_INC_RTC_H_
#define API_INC_RTC_H_

#include "general_def.h"

typedef struct {
	uint8_t	Hours;
	uint8_t Minutes;
} Clock_time;

void initRTC();
void initClock( Clock_time *clock );
void addHour( Clock_time *clock );
void substractHour( Clock_time *clock );
void addMin( Clock_time *clock );
void substractMin( Clock_time *clock );
void RTC_SetTime( Clock_time clock);
void RTC_ReadTime( Clock_time *clock );

#endif /* API_INC_RTC_H_ */
