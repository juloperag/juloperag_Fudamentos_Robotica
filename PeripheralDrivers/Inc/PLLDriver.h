/*
 * PLLDriver.h
 *
 *  Created on: 20/05/2023
 *      Author: julian
 */

#ifndef PLLDRIVER_H_
#define PLLDRIVER_H_

#include <stm32f4xx.h>

//Definicion de macros
#define CLOCK_SPEED_16MHZ  16
#define CLOCK_SPEED_25MHZ  25
#define CLOCK_SPEED_80MHZ  80
#define CLOCK_SPEED_100MHZ  100

//Definicion de cabeceras
void configPLL(uint8_t clockSpeed);

void adjustHSI(void);

uint8_t getConfigPLL(void);
uint8_t getClockAPB1(void);

void configMCO1(uint8_t value);
void configMCO1PRE(uint8_t value);

#endif /* PLLDRIVER_H_ */
