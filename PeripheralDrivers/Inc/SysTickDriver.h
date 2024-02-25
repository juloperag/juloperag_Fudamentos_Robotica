/*
 * SysTickDriver.h
 *
 *  Created on: 29/04/2023
 *      Author: julian
 */

#ifndef SYSTICKDRIVER_H_
#define SYSTICKDRIVER_H_

#include <stm32f4xx.h>

//Deficion de macros
#define SYSRICK_LOAD_VALUE_16MHz_1ms    16000  // Numero de ciclos en 1ms
#define SYSRICK_LOAD_VALUE_100MHz_1ms   100000 // Numero de ciclos en 1ms

//Definicion de funciones
void config_SysTick_ms(void);   //uint8_t systemClock
uint64_t getTicksMs(void);
void delay_ms(uint32_t wait_time_ms);

void config_SysTick_us(void);
uint64_t getTicksUs(void);
void delay_us(uint32_t wait_time_us);

#endif /* SYSTICKDRIVER_H_ */
