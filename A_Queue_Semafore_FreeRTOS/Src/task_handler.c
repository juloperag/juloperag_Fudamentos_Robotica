/*
 * task_handler.c
 *
 *  Created on: Aug 7, 2024
 *      Author: julil
 */


#include "main.h"


//Mesajes para imprimir por la consola
const char *msg_working = "\n - - Working - - \n";
//Variable para guardar mensaje
char auxMsg[64] = {0};

//-----------------------Inicio definicion funciones de las Task---------------------------

//--------Tarea de Counting---------------------
void vTask_Test_counting( void * pvParameters )
{

	const TickType_t xMaxExpectedBlockTime = pdMS_TO_TICKS( 1000 );

    while(1)
    {
    	/* En este if() el sistema espera que se tenga un elemento disponible en el semaforo
    	 * cuando esta disponible actual -> imprime un mensaje que dice "working"*/
    	if (xSemaphoreTake(xCountingSemaphore_Working, xMaxExpectedBlockTime) == pdPASS) {

    		/* Este bloque es para imprimir solo 1 vez el valor del numero aleatorio*/
    		if(printOneTime == 1){
    			sprintf(auxMsg, "counter i = %d\n", randomNumber);
    			char *dummy = auxMsg;
    			xQueueSend(xQueue_Print, &dummy, portMAX_DELAY);
    			printOneTime = 0;
    		}
    		/* Imprime las peticiones de trabajo que genera el numero aleatorio en la interupcion*/
			xQueueSend(xQueue_Print, &msg_working, portMAX_DELAY);
    	}
    }
}



//-------Tarea de Imprimir-------
void vTask_Print(void * pvParameters)
{
	//Variable para guardad mensaje a enviar
	uint32_t *msg;

	while(1)
	{
		//Se espera por el puntero del mensaje
		xQueueReceive(xQueue_Print, &msg, portMAX_DELAY);
		//Enviamos por puerto serial dicho mensaje
		writeMsg(&handler_USART_CommTerm, (char *) msg);
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


//-----------------------Fin definicion funciones Adicionales---------------------------

