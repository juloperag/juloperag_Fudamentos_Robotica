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
#include "stdio.h"
#include <stdint.h>
#include <string.h>
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

//Definicon de structuras
typedef struct{
	uint8_t payload[10];        //informacion recibida por el serial
	uint32_t len;               //lenght -> tamaño de los datos recibidos
}command_t;

typedef enum{
	sMainMenu = 0,
	sLedEffect,
	sRtcMenu,
}state_t;


//Handler relacionados con las tareas
extern TaskHandle_t xHandleTask_Menu;
extern TaskHandle_t xHandleTask_Print;
extern TaskHandle_t xHandleTask_Commands;
//Cabeceras de las funciones de las Tareas
void vTask_Menu(void * pvParameters);
void vTask_Print(void * pvParameters);
void vTask_Commands(void * pvParameters);

//Cabecera de la funcion del Software Timer
void led_state_callback(TimerHandle_t xTimer);

//Handler relacionado con las colas
extern QueueHandle_t xQueue_Print;
extern QueueHandle_t xQueue_InputData;

//Handler del semaforo
extern SemaphoreHandle_t xBinarySemaphore;

//Handler referente con el periferico USART
extern USART_Handler_t handler_USART_CommTerm;

//Definicion de un elemento de la structura state_t
extern state_t next_state;


#endif /* MAIN_H_ */
