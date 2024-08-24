/*
 * task_handler.c
 *
 *  Created on: Aug 7, 2024
 *      Author: julil
 */


#include "main.h"


//-----------------------Inicio definicion funciones de las Task---------------------------

//-------Tarea de Imprimir-------
void vTask_Print(void * pvParameters)
{
	//Variable pata copiar informacion
	char *pcStringToPrint;
	pcStringToPrint = (char *) pvParameters;

	while(1)
	{
		//Imprimimos el String
		prv_PrintString(pcStringToPrint);
		//Aplicamos a la tarea un estado de bloqueo
		vTaskDelay(getRandomNumber() & 0xFF);
	}
}


//-----------------------Fin definicion funciones de las Task---------------------------




//-----------------------Inicio definicion funciones del Software Timer---------------------------

void led_state_callback(TimerHandle_t xTimer)
{
	//Cambio estado opuesto Led
	GPIOxTooglePin(&handler_GPIO_BlinkyPin);
}

//-----------------------Fin Definicion Funciones del Software Timer---------------------------




//-----------------------Inicio definicion funciones Adicionales---------------------------

//Se imprime el mensaje
void prv_PrintString(const portCHAR *pcString)
{
	char cbuffer[60];
	//Tomamos el token del Mutex
	xSemaphoreTake(xSemaphoreMutex_Print, portMAX_DELAY);
	//Enviamos por puerto serial dicho mensaje
	sprintf(cbuffer, "%s", pcString);
	writeMsg(&handler_USART_CommTerm, (char *) cbuffer);
	//Entregamos el token al Mutex
	xSemaphoreGive(xSemaphoreMutex_Print);
}


//Generamos un numero Ramdom
uint16_t getRandomNumber(void)
{
	srand( xTaskGetTickCount() );
	int randomnumber;
	randomnumber = (rand() % 10) +1;
	return (uint16_t)randomnumber;
}

//-----------------------Fin definicion funciones Adicionales---------------------------




