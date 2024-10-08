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
extern TaskHandle_t xHandleTask_Counting;
extern TaskHandle_t xHandleTask_Print;
//Cabeceras de las funciones de las Tareas
void vTask_Test_counting(void * pvParameters);
void vTask_Print(void * pvParameters);

//Cabecera de la funcion del Software Timer
void led_state_callback(TimerHandle_t xTimer);

//Handler relacionado con las colas
extern QueueHandle_t xQueue_Print;

//Handler relacionado con los semaphores
extern SemaphoreHandle_t xBinarySemaphore_Working;
extern SemaphoreHandle_t xCountingSemaphore_Working;

//----------------Referencia de Perifericos------------------
//Handler referente con el periferico USART
extern USART_Handler_t handler_USART_CommTerm;
//Handler referente con el elemento GPIO_BlinkyPin
extern GPIO_Handler_t handler_GPIO_BlinkyPin;

//------------------Referencia de Variables------------------
extern volatile uint8_t printOneTime;
extern volatile uint16_t randomNumber;

#endif /* MAIN_H_ */
