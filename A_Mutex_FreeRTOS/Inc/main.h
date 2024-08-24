/*
 * main.h
 *
 *  Created on: Aug 7, 2024
 *      Author: julil
 */

#ifndef MAIN_H_
#define MAIN_H_

//----------------------Librerias---------------------------------
//CMSIS
#include <stm32f411xe.h>
//Sistema
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
//FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
//PeripheralDrivers
#include <GPIOxDriver.h>
#include <PLLDriver.h>
#include <USARTxDriver.h>
#include <ExtiDriver.h>

//------------------Referencias de elementos de FreeRTOS---------------
//Handler relacionados con las tareas
extern TaskHandle_t xHandleTask_Print1;
extern TaskHandle_t xHandleTask_Print2;
extern TaskHandle_t xHandleTask_Print3;

//Cabeceras de las funciones de las Tareas
void vTask_Print(void * pvParameters);

//Cabecera de la funcion del Software Timer
void led_state_callback(TimerHandle_t xTimer);

//Handler relacionado con los semaphores Mutex
extern SemaphoreHandle_t xSemaphoreMutex_Print;

//----------------Referencia de Perifericos------------------
//Handler referente con el periferico USART
extern USART_Handler_t handler_USART_CommTerm;
//Handler referente con el elemento GPIO_BlinkyPin
extern GPIO_Handler_t handler_GPIO_BlinkyPin;

//------------------Referencia de Variables------------------
extern volatile uint8_t printOneTime;
extern volatile uint16_t randomNumber;

//-----------------Definicion de funciones Adicionales---------
void prv_PrintString(const portCHAR *pcString);
uint16_t getRandomNumber(void);

#endif /* MAIN_H_ */
