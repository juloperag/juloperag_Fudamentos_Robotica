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
#include "timers.h"
//----Perifericos
#include <GPIOxDriver.h>
#include <BasicTimer.h>
#include <ExtiDriver.h>
#include <USARTxDriver.h>
#include <PwmDriver.h>
#include <PLLDriver.h>
//------CMSIS------
#include <arm_math.h>
#include <math.h>
//-----Proyecto-------
#include <MotorDriver.h>
#include "MPUAccel.h"
#include "PositionRobot.h"



typedef enum{
	sMainMenu = 0,
	sGo,
	sRtcMenu,
}state_t;


//------------------Referencias de elementos de FreeRTOS---------------
//Handler relacionados con las tareas
extern TaskHandle_t xHandleTask_Menu;
extern TaskHandle_t xHandleTask_Print;
extern TaskHandle_t xHandleTask_Commands;
extern TaskHandle_t xHandleTask_Go;
extern TaskHandle_t xHandleTask_Stop;
//Cabeceras de las funciones de las Tareas
void vTask_Menu(void * pvParameters);
void vTask_Print(void * pvParameters);
void vTask_Commands(void * pvParameters);
void vTask_Go(void * pvParameters);
void vTask_Stop(void * pvParameters);

//Cabecera de la funcion del Software Timer
void led_state_callback(TimerHandle_t xTimer);

//Handler relacionado con las colas
extern QueueHandle_t xQueue_Print;
extern QueueHandle_t xQueue_InputData;

//-------------------------Referencia de Perifericos---------------------------
extern USART_Handler_t handler_USART_CommTerm;               //Handler referente con el periferico USART
extern GPIO_Handler_t handler_GPIO_BlinkyPin;                //Handler referente con el elemento GPIO_BlinkyPin
extern BasicTimer_Handler_t handler_TIMER_Motor;
extern BasicTimer_Handler_t handler_TIMER_Sampling;
//-----Motor---------------
extern Motor_Handler_t handler_Motor_R;
extern Motor_Handler_t handler_Motor_L;
//---------------------------Referencia de Variables------------------------
//-----------Periodo-------------
extern uint8_t time_accion;
//-----Variables de los modos y operacion---
extern float velSetPoint;
extern uint8_t duttySetPoint;
extern uint16_t frequency_PWM_Motor;
//-----Variables PID-----------
extern Parameters_PID_t parameter_PID_distace;
//--------Variables del timerSampling---------
extern uint16_t time_accumulated;
extern uint16_t counting_action ;
//--------Odometria----------------
extern Parameters_Position_t parameter_Posicion_Robot; 	//Estructura que almacena la posicion del robot
//Definicion de un elemento de la structura state_t
extern state_t next_state;


#endif /* MAIN_H_ */
