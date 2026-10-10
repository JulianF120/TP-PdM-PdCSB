/*
 * RTC.h
 *
 *  Created on: 6 oct. 2026
 *      Author: Julian Ferreira
 */

#ifndef API_INC_RTC_H_
#define API_INC_RTC_H_
#define MIN_HOUR	0
#define MAX_HOUR	23
#define MAX_MIN		59
#define MIN_MIN		0
#include "general_def.h"

typedef struct {
	uint8_t	Hours;
	uint8_t Minutes;
} Clock_time;

void RTC_Init( void );
void initClock( Clock_time *clock );
void addHour( Clock_time *clock );
void subtractHour( Clock_time *clock );
void addMin( Clock_time *clock );
void subtractMin( Clock_time *clock );
void RTC_SetTime( Clock_time clock);
void RTC_ReadTime( Clock_time *clock );
bool_t sameTime( Clock_time actual, Clock_time expected);

#endif /* API_INC_RTC_H_ */
