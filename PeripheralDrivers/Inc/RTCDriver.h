/*
 * RTCDriver.h
 *
 *  Created on: 4/06/2023
 *      Author: julian
 */

#ifndef RTCDRIVER_H_
#define RTCDRIVER_H_

#include <stm32f4xx.h>

#define LUNES      1
#define MARTES     2
#define MIERCOLES  3
#define JUEVES     4
#define VIERNES    5
#define SABADO     6
#define DOMINGO    7

#define ENERO         1
#define FEBRERO       2
#define MARZO         3
#define ABRIL         4
#define MAYO          5
#define JUNIO         6
#define JULIO         7
#define AGOSTO        8
#define SEPTIEMBRE    9
#define OCTUBRE       10
#define NOVIEMBRE     11
#define DICIEMBRE     12

//Estructura que contiene la configuracion minima para el uso del RTC
typedef struct
{
	uint8_t  year;
	uint8_t  dayWeek;
	uint8_t  month;
	uint8_t  date;
	uint8_t  hour;
	uint8_t  min;
	uint8_t  seg;

}RTC_Handler_t;

//Definicion de las cabeceras de las funciones para el uso del RTC
void configRTC(RTC_Handler_t *rtcConfig);
void timeConfigRTC(RTC_Handler_t *rtcConfig);
void updatetimeConfigRTC(RTC_Handler_t *rtcConfig, uint8_t newhour, uint8_t newmin, uint8_t newseg);
void dateConfigRTC(RTC_Handler_t *rtcConfig);
void updateDateConfigRTC(RTC_Handler_t *rtcConfig, uint8_t newyear, uint8_t newmonth, uint8_t newdate, uint8_t newdayweek);

void initTimeShowRTC(void);
void initDateShowRTC(void);

uint8_t segShowRTC(void);
uint8_t minShowRTC(void);
uint8_t hourShowRTC(void);
uint8_t dayweekShowRTC(void);
uint8_t dateShowRTC(void);
uint8_t monthShowRTC(void);
uint8_t yearShowRTC(void);

#endif /* RTCDRIVER_H_ */
