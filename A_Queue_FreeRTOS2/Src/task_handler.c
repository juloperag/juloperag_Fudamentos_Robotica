/*
 * task_handler.c
 *
 *  Created on: Aug 7, 2024
 *      Author: julil
 */


#include "main.h"

//Prototipos
void process_command(command_t *cmd);
int extract_command(command_t *cmd);

//Mesajes para imprimir por la consola
const char *msg_invalid = "\n/// Ivalid option /// \n";
const char *msg_option_0 = "\n-----Selected option - 0 -----\n";
const char *msg_option_1 = "\n-----Selected option - 1 ------\n";
const char *msg_option_2 = "\n-----Selected option - 2 ------\n";
const char *msg_option_n = "\n-----Option out of range ------\n";

//----------------------Funciones referentes a cada tarea---------------------
//------Tarea Menu-------
void vTask_Menu(void * pvParameters)
{
	//Definimos variables
	uint32_t cmd_addr;
	command_t *cmd;
	int option;

	//Mensaje inicial del menu
	const char* msg_menu = "=======================\n"
			               "|         Menu        |\n"
						   "=======================\n"
						   "LED effect    ---> 0\n"
			               "Date and Time ---> 1\n"
			               "Exit          ---> 2\n"
			               "Enter your choice here : ";
	while(1)
	{
		//Se envia a imprimir el mensaje a la consola
		xQueueSend(xQueue_Print, &msg_menu, portMAX_DELAY);
		//Se espera por el comando a ejecutar
		xTaskNotifyWait(0,0,&cmd_addr, portMAX_DELAY);
		cmd = (command_t* ) cmd_addr;

		//Se verificamos si se tiene un solo caracter
		if(cmd->len == 1)
		{
			//Se transforma un ASCII a un numero 1...
			option = cmd->payload[0] - 48;

			switch(option){
			case 0:
				//Se envia la opcion especificada
				xQueueSend(xQueue_Print, &msg_option_0, portMAX_DELAY);
				//Notificacion del cambio de estado
				next_state = sMainMenu;
				xTaskNotify(xHandleTask_Menu, 0, eNoAction);

				break;
			case 1:
				//Se envia la opcion especificada
				xQueueSend(xQueue_Print, &msg_option_1, portMAX_DELAY);
				//Notificacion del cambio de estado
				next_state = sMainMenu;
				xTaskNotify(xHandleTask_Menu, 0, eNoAction);

				break;
			case 2:
				//Se envia la opcion especificada
				xQueueSend(xQueue_Print, &msg_option_2, portMAX_DELAY);
				//Notificacion del cambio de estado
				next_state = sMainMenu;
				xTaskNotify(xHandleTask_Menu, 0, eNoAction);

				break;
			default:
				//Se envia la opcion especificada
				xQueueSend(xQueue_Print, &msg_option_n, portMAX_DELAY);
				continue; //Se envia la instruccion al sistema de nuevo a esperar
			}
		}
		else
		{
			//Se envia la opcion especificada
			xQueueSend(xQueue_Print, &msg_invalid, portMAX_DELAY);
			//Notificacion del cambio de estado
			next_state = sMainMenu;
			xTaskNotify(xHandleTask_Menu, 0, eNoAction);
		}

		//La tarea se queda en un estado de espera por un tiempo indefinido
		xTaskNotifyWait(0,0,NULL, portMAX_DELAY);
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

//-------Tarea de los comando-------
void vTask_Commands(void * pvParameters)
{
	BaseType_t notify_status = {0};
	command_t cmd = {0};

	while(1)
	{
		//Se espera por la notificacion de la interrupcion
		notify_status = xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Se procesa el comando recibido
			process_command(&cmd);
		}
	}
}

//Funcion que ayuda en el procesamiento del comando
void process_command(command_t *cmd)
{
	extract_command(cmd);

	switch(next_state)
	{
	case sMainMenu:
		//Notificamos a la tarea respectiva
		xTaskNotify(xHandleTask_Menu, (uint32_t) cmd, eSetValueWithoutOverwrite);
		break;

	case sLedEffect:
		//xTaskNotify(xHandleTask_Leds, (uint32_t) cmd, eSetValueWithoutOverwrite);
		break;

	case sRtcMenu:
		//xTaskNotify(xHandleTask_Rtc, (uint32_t) cmd, eSetValueWithoutOverwrite);
		break;
	default:
		__NOP();
		break;
	}
}
//Funcion para obtener el comando
int extract_command(command_t *cmd)
{
	//Definicion de variables
	uint8_t item;
	uint8_t counter_j = 0;
	BaseType_t status;
	//Se verifica si hay un nuevo mensaje
	status = uxQueueMessagesWaiting(xQueue_InputData);
	if(status == 0)
	{
		return -1;
	}
	do{
		//Recibimos un elemento y lo montamos en item
		status = xQueueReceive(xQueue_InputData, &item, 0);
		if(status ==pdTRUE){
			//Vamos llenando el arreglo del comando
			cmd->payload[counter_j++] = item;
		}
	}while(item != '#');

	//Agregamos el elemento nulo y ademas definimos el largo del mensaje
	cmd->payload[counter_j - 1] = '\0';
	cmd->len = counter_j -1;

	return 0;
}

