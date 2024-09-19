/*
 * main.h
 *
 *  Created on: Aug 7, 2024
 *      Author: julil
 */

#ifndef MAIN_H_
#define MAIN_H_

//----------------------Librerias---------------------------------
//Sistema
#include <stm32f411xe.h>
#include <stdio.h>
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
#include "A_Star.h"
#include "MPUAccel.h"
#include "PositionRobot.h"

//Definicion de structuras
typedef struct{
	char send_cmd[12];             //informacion del comando
	int firtparameter;
	int secondparameter;
}command_t;

typedef enum{
	sMenuOperation = 0,
	sExecution,
	sAStar,
}state_t;


//------------------Referencias de elementos de FreeRTOS---------------
//Handler relacionados con las tareas
extern TaskHandle_t xHandleTask_Menu;
extern TaskHandle_t xHandleTask_Print;
extern TaskHandle_t xHandleTask_Commands;
extern TaskHandle_t xHandleTask_Line;
extern TaskHandle_t xHandleTask_Turn_itself;
extern TaskHandle_t xHandleTask_Square;
extern TaskHandle_t xHandleTask_Execute_Astar;
extern TaskHandle_t xHandleTask_Stop;
extern TaskHandle_t xHandleTask_Measure;
extern TaskHandle_t xHandleTask_Line_PID;
extern TaskHandle_t xHandleTask_Stop_Execute;
//Cabeceras de las funciones de las Tareas
void vTask_Menu(void * pvParameters);
void vTask_Print(void * pvParameters);
void vTask_Commands(void * pvParameters);
void vTask_Line(void * pvParameters);
void vTask_Stop(void * pvParameters);
void vTask_Measure(void * pvParameters);
void vTask_Line_PID(void * pvParameters);
void vTask_Turn(void *pvParameters);
void vTask_Square(void *pvParameters);
void vTask_Execute_AStar(void * pvParameters);
void vTask_Stop_Execute(void * pvParameters);

//Cabecera de la funcion del Software Timer
void led_state_callback(TimerHandle_t xTimer);

//Handler relacionado con las colas
extern QueueHandle_t xQueue_Print;
extern QueueHandle_t xQueue_StructCommand;
extern QueueHandle_t xQueue_InputData;
extern QueueHandle_t xMailbox_Mode;

//-------------------------Referencia de Perifericos---------------------------
extern USART_Handler_t handler_USART_CommTerm;               //Handler referente con el periferico USART
extern GPIO_Handler_t handler_GPIO_BlinkyPin;                //Handler referente con el elemento GPIO_BlinkyPin
extern BasicTimer_Handler_t handler_TIMER_Motor;
extern BasicTimer_Handler_t handler_TIMER_Sampling;
//----------Motor---------------
extern Motor_Handler_t handler_Motor_R;
extern Motor_Handler_t handler_Motor_L;
//----------MPU6050-------------
extern MPUAccel_Handler_t handler_MPUAccel_MPU6050;
//---------------------------Referencia de Variables------------------------
//------Varaible del MPU-------
extern int16_t gyro_offset;
//-----------Periodo-------------
extern uint8_t time_accion;
extern uint8_t period_sampling;
extern uint16_t timeAction_TIMER_Sampling;
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
extern float cm_L;                                      //Factor de conversion rueda Izquierda [mm/cuentas]
extern float cm_R;
//Definicion de un elemento de la structura state_t
extern state_t next_state;


#endif /* MAIN_H_ */