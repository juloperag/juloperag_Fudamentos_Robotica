/*
 * task_handler.c
 *
 *  Created on: Aug 7, 2024
 *      Author: julil
 */


#include "main.h"

//------------------Definiciones generales----------------------------------
//--------------------------Macros--------------------------
#define SGMFALSE 0                              //Macros para verificar el exito de la separacion de las caracteristicas del grid map
#define SGMTRUE 1
#define PATHFALSE 0                             //Macros para verificar si se tiene con path listo
#define PATHTRUE 1
#define distanceBetweenWheels 10600             //Distacia entre ruedas     10430
#define ENABLE_PATH_ASTAR_BIT 0b1               //Bit que indica que hay un camino habilitado
#define EXECUTE_PATH_ASTAR_BIT 0b10             //Bit para ejecutar AStar
#define ENABLE_OPERATION_BIT 0b1                //Bit para habilitar el uso de tarea ejecucion operaciones
#define EXECUTE_OPERATION_BIT 0b10              //Bit para indicar una ejecucion de operacion
//-------------------Cabeceras de funciones-----------------
//----Cabeceras de funciones para los motores----------
void config_motor(int firth, float second, float third);
void status_motor(uint8_t status);
void PID_simple(Parameters_PID_t *ptrPIDHandler, float timer, float setpoint, float measure);
void correction(Motor_Handler_t *ptrMotorHandler);
void init_coordinates(void);
//----Cabeceras de funciones para el control de los modos------
void straight_line(uint8_t dutty);
void turn_itself(int16_t turn_grad);
void config_mode(uint8_t status, float dutty_L, float dutty_R);
//----Cabeceras de las operaciones------
void set_operation_square(Parameters_Operation_t *prtList, double dis_side, double direction_square);
void set_operation_AStar(Parameters_Operation_t *prtList, file_cell_t *file_cell, Parameters_Position_t *ptrParameterPosition, Parameters_Path_t *ptrParameterPath);
void set_operation_in_queue(Parameters_Operation_t list[30]);
//----Cabeceras de funciones de los comandos--------
void process_stringsend(char stringsend[500]);
int extract_stringsend(char stringsend[500]);
//---Cabeceras de A-Star----------
uint8_t Separate_parameters(item_A_Star_t* ptritem, char *parameter_string);	                 //Funcion para separar los parametros presentes en el string
void send_path(file_cell_t *file_cell, Cell_map_t array_string[20][20], uint8_t row, uint8_t colum); //Funcion para imprimir la ruta encontrada

//------------------Variables---------------------
//--------AStar-------------
item_A_Star_t xitemAStar = {0};
file_cell_t *file_path;
//---------Puntero-----------
Motor_Handler_t *handler_Motor_Execute = {0};     //Handler que se refiere a uno de los motores
//---------mensajes----------
const char *msg_com_invalid = "Comando invalido  \n";
const char *msg_stop = "Accion de parada \n";
const char *msg_finish_Operation = "Operaciones Finalizadas \n";
const char *msg_fail = "Uno o varios parametros son incorrectos, volver a intentar \n";
//-------Odometria-------------
Parameters_Path_t parameter_Path_Robot = {0};           //Estructura que almacena los parametros del camino a recorrer
float cos_cal  = 0;                                     //Variables que almacenan el calculo del seno y coseno
float sin_cal  = 0;
double ang_for_Displament_ICR = 0;                //Variables para correccion del angulo en el giro
double ang_complementary = 0;
Parameters_Position_t parameter_Posicion_Robot; 	//Estructura que almacena la posicion del robot
MPUTimerSample_t sample_Gyro = {0};                     //Estructura para almacenar los datos del muestreo
float sum_ang  = 0;                               //Angulo acumulado
float prom_ang = 0;                               //Angulo promedio
//-----Variables de los modos y operacion---
float vel_Setpoint_R = 0;                         //Diferentes setpoit
float vel_Setpoint_L = 0;




//-----------------------Inicio definicion funciones de las Task---------------------------

//------------Tareas de para recepcion y envio de informacion------------------
//-------Tarea de Imprimir-------
void vTask_Print(void * pvParameters)
{
	//Variable para guardad mensaje a enviar
	char *msg;

	while(1)
	{
		//Se espera por el puntero del mensaje
		xQueueReceive(xQueue_Print, &msg, portMAX_DELAY);
		//Enviamos por puerto serial dicho mensaje
		writeMsg(&handler_USART_CommTerm, msg);
	}
}
//-------Tarea de los comando-------
void vTask_Commands(void * pvParameters)
{
	BaseType_t notify_status = {0};
	char stringsend[500];

	while(1)
	{
		//Se espera por la notificacion de la interrupcion
		notify_status = xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Se procesa el comando recibido
			process_stringsend(stringsend);
		}
	}
}
//------Tarea Menu-------
void vTask_Menu(void * pvParameters)
{
	//Variables para la recepcion
	command_t xReceivedStructure;
	BaseType_t notify_status;
	//Mensaje inicial del menu
	const char *msg_menu = "=======================\n"
			               "|     Menu Operation   |\n"
						   "=======================\n"
						   "line # --- #:dis[mm] \n"
						   "turn # # --- #:ang[°] #:dir 0->L 1->R \n"
						   "square # # --- #:lad[mm], #:dir 0->L 1->R \n"
						   "applyastar \n"
						   "exepathastar \n"
						   "init \n";
	//Se envia el mensaje del menu de opciones
	xQueueSend(xQueue_Print, &msg_menu, portMAX_DELAY);

	while(1)
	{
		//Se envia el mensaje del menu de opciones
		if(next_state == sMenuOperation){ timer_delay(&handler_TIMER_Delay, &countingTimer, 1000); xQueueSend(xQueue_Print, &msg_menu, portMAX_DELAY); }
		//Se espera por la recepcion de un comando
		xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		//Se verificamos recibe el comando
		notify_status = xQueueReceive(xQueue_StructCommand, &xReceivedStructure, 0 );
		//Se verificamos el tipo de comando
		if(notify_status == pdTRUE)
		{
			if(strcmp(xReceivedStructure.send_cmd, "line") == 0)
			{
				//Notificamos a la tarea respectiva
				xTaskNotify(xHandleTask_Line, (uint32_t) xReceivedStructure.firtparameter, eSetValueWithoutOverwrite);
			}
			else if(strcmp(xReceivedStructure.send_cmd, "turn") == 0)
			{
				//Notificamos a la tarea respectiva
				xTaskNotify(xHandleTask_Turn, (uint32_t) &xReceivedStructure, eSetValueWithoutOverwrite);
			}
			else if(strcmp(xReceivedStructure.send_cmd, "square") == 0)
			{
				//Notificamos a la tarea respectiva
				xTaskNotify(xHandleTask_Square, (uint32_t) &xReceivedStructure, eSetValueWithoutOverwrite);
			}
			else if(strcmp(xReceivedStructure.send_cmd, "applyastar") == 0)
			{
				//Cambio de state
				next_state = sAStar;
			}
			else if(strcmp(xReceivedStructure.send_cmd, "exepathastar") == 0)
			{
				//Notificamos a la tarea respectiva
				xTaskNotify(xHandleTask_Execute_Astar, 0, eNoAction);
			}
			else if(strcmp(xReceivedStructure.send_cmd, "init") == 0)
			{
				//Restablecimiento de coordenadas
				init_coordinates();
				//Cambio de state
				next_state = sMenuOperation;
			}
			else
			{
				//Cambio de state
				next_state = sMenuOperation;
				//Se envia la opcion especificada
				xQueueSend(xQueue_Print, &msg_com_invalid, portMAX_DELAY);
				//Entregamos el procesador a la Tarea Print
				taskYIELD();
			}
		}
	}

}

//-----------------------Tareas de Operaciones--------------------
//------------Tarea grupo de eventos---------------
void vTask_Execute_Operation(void *pvParameters)
{
	//Definicion de variables
	Parameters_Operation_t option_operation;
	const EventBits_t xBitsToWaitFor = (ENABLE_OPERATION_BIT | EXECUTE_OPERATION_BIT);
	BaseType_t status;
	//Ciclo de la tarea
	while(1)
	{
		//Se espera por lis bit eventos para generar una condicion de desbloqueo
		xEventGroupWaitBits(xEventGroup_Execute_Operation, xBitsToWaitFor, pdFALSE, pdTRUE, portMAX_DELAY);
		//Impiamos el bit correspondiente a la ejecucion
		xEventGroupClearBits(xEventGroup_Execute_Operation, EXECUTE_OPERATION_BIT);
		//Recibimos las opciones de las operaciones
		status = xQueueReceive(xQueue_Operation, &option_operation, 0);
		//Si se presenta una operacion se ejecuta
		if(status ==pdTRUE)
		{
			//Aseguramos que seguimos en status de ejecucion
			next_state = sExecution;
			//Delay para espera la finalizacion del modo
			timer_delay(&handler_TIMER_Delay, &countingTimer, 500);
			//Deacuerdo a la operacion se configura
			switch(option_operation.operacion)
			{
				case LINE:{
					//Definicion de parametros
					change_coordinates_position(&parameter_Path_Robot, option_operation.x_destination, option_operation.y_destination,
							parameter_Path_Robot.goal_Position_x, parameter_Path_Robot.goal_Position_y);
					//Configuracion inicial linea recta
					straight_line(duttySetPoint);
					break;
				}
				case TURN:{
					//Configuracion inicial del giro
					turn_itself(option_operation.grad_Rotative);
					break;
				}
				default:{ break; }
			}
		}
		else
		{
			//Impiamos el bit correspondiente a la habilitacion
			xEventGroupClearBits(xEventGroup_Execute_Operation, ENABLE_OPERATION_BIT);
			//cambio de status
			next_state = sMenuOperation;
			//Se envia la opcion especificada
			xQueueSend(xQueue_Print, &msg_finish_Operation, portMAX_DELAY);
			//Entregamos el procesador a la Tarea Print
			taskYIELD();
			//Notificamos Tarea de menu para imprimir mensaje de operaciones
			xTaskNotify(xHandleTask_Menu, 0, eNoAction);
		}
	}
}
//------------Tarea linea---------------
void vTask_Line(void * pvParameters)
{
	//Definicion de variable de notificacion
	BaseType_t notify_status = {0};
	uint32_t parameter;

	while(1)
	{
		//Se espera por la notificacion
		notify_status = xTaskNotifyWait(0,0,&parameter, portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Definicion de parametros
			change_position(&parameter_Path_Robot, parameter, parameter_Path_Robot.goal_Position_x, parameter_Path_Robot.goal_Position_y);
			//Configuracion inicial linea recta
			straight_line(duttySetPoint);
			//Cambio de state
			next_state = sExecution;
		}
	}
}
//------Tarea de Turn------------
void vTask_Turn(void *pvParameters)
{
	//Definicion de variable
	BaseType_t notify_status = {0};
	command_t *xReceivedStructure;
	int16_t degrees;
	uint32_t data;
	//Ciclo de la tarea
	while(1)
	{
		//Se espera por la notificacion
		notify_status = xTaskNotifyWait(0,0,&data,portMAX_DELAY);
		xReceivedStructure = (command_t*)  data;
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Cambio de signo si e giro es hacia la derecha
			if(xReceivedStructure->secondparameter==1){ degrees = -1*xReceivedStructure->firtparameter;}
			else{degrees = xReceivedStructure->firtparameter;}
			//Configuracion inicial del giro
			turn_itself(degrees);
			//Cambio de state
			next_state = sExecution;
		}
	}
}
//--------Tarea de Square------------
void vTask_Square(void *pvParameters)
{
	//Definicion de variable
	BaseType_t notify_status = {0};
	command_t *xReceivedStructure;
	Parameters_Operation_t list_operation[30];
	uint32_t data;
	//Ciclo de la tarea
	while(1)
	{
		//Se espera por la notificacion
		notify_status = xTaskNotifyWait(0,0,&data,portMAX_DELAY);
		xReceivedStructure = (command_t*)  data;
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Restablecimiento de coordenadas
			init_coordinates();
			//Configuracion de operaciones
			set_operation_square(list_operation, xReceivedStructure->firtparameter, xReceivedStructure->secondparameter);
			//Definir operaciones en la cola de operaciones
			set_operation_in_queue(list_operation);
			//Cambio de state
			next_state = sExecution;
		}
	}
}
//------------Tarea execute Astar--------------
void vTask_Execute_AStar(void * pvParameters)
{
	//Definicion de variable de notificacion
	BaseType_t notify_status = {0};
	uint8_t status = PATHFALSE;
	const char *msg_Fail_Execute_Path = "El path generado por AStar no se a establecido \n";
	Parameters_Operation_t list_operation[30];
	//Se envia al buzon un valor falso
	xQueueOverwrite(xMailbox_Path, &status);
	//Ciclo de la tarea
	while(1)
	{
		//Se espera por la notificacion
		notify_status = xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Se verifica se verifica el estado del path
			xQueuePeek(xMailbox_Path, &status, 0);
			if(status == PATHTRUE)
			{
				//Restablecimiento de coordenadas
				init_coordinates();
				//Configuracion de operaciones
				set_operation_AStar(list_operation, file_path, &parameter_Posicion_Robot, &parameter_Path_Robot);
				//Definir operaciones en la cola de operaciones
				set_operation_in_queue(list_operation);
				//Cambio de state
				next_state = sExecution;
			}
			else
			{
				  //Se envia la opcion especificada
				  xQueueSend(xQueue_Print, &msg_Fail_Execute_Path, portMAX_DELAY);
				  //Entregamos el procesador a la Tarea Print
				  taskYIELD();
				  //cambio de status
				  next_state = sMenuOperation;
			}

		}
	}
}


//--------------Tareas de aplicacion de AStar---------------
//------Tarea de separacion del grip map------------
void vTask_Separate_GripMap(void *pvParameters)
{
	//Definicion de variable
	BaseType_t notify_status = {0};
	char *xReceivedString;
	char parameterStringGM[500];
	uint32_t data;
	uint8_t status_separate;
	//Ciclo de la tarea
	while(1)
	{
		//Se espera por la notificacion
		notify_status = xTaskNotifyWait(0,0,&data,portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Se realiza un casting
			xReceivedString = (char*)  data;
			//Se realiza una copia del string
		    memcpy(parameterStringGM, xReceivedString, 500 * sizeof(char));
			//-------------Separar el String en los parametros del grid map-------------
		    status_separate = Separate_parameters(&xitemAStar, parameterStringGM);
		    //Se verifica el estado de la separacion
		    if(status_separate)
		    {
				//Notificamos la tarea respectiva
				xTaskNotify(xHandleTask_Apply_Astar, 0, eNoAction);
		    }
		    else
		    {
				//cambio de status
				next_state = sMenuOperation;
				//Se envia la opcion especificada
				xQueueSend(xQueue_Print, &msg_fail, portMAX_DELAY);
				//Entregamos el procesador a la Tarea Print
				taskYIELD();
		    }
		}
	}
}
//------Tarea de Aplicacion de A Star------------
void vTask_Apply_Astar(void * pvParameters)
{
	//Definicion de variable
	BaseType_t notify_status = {0};
	file_cell_t file_Open[50];
	uint8_t index_file = 0;
	uint8_t status = PATHFALSE;
	const char *msg_Finish_AStar = "Calculo de A Star Finalizado \n";
	//Ciclo de la tarea
	while(1)
	{
		//Se espera por la notificacion
		notify_status = xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
		  //----------------------------------Inicio A Star----------------------------------
		  //---------Creacion de la malla con cada una de sus celdas-----------
		  build_grid_map(xitemAStar.grid_map, xitemAStar.grid_map_row,
				  xitemAStar.grid_map_colum, xitemAStar.cell_separation);
		  //-------Calculo de la heuristica de la celda de acuerdo a la posicion objetivo-------
		  heuristic_cell_map(xitemAStar.grid_map, xitemAStar.grid_map_row,
				  xitemAStar.grid_map_colum, xitemAStar.goal_x, xitemAStar.goal_y);
		  //------------------Aplicacion del algoritmo A star------------------
		  index_file = aplicattion_A_Star(file_Open, xitemAStar.grid_map, xitemAStar.grid_map_row, xitemAStar.grid_map_colum,
				  xitemAStar.start_x, xitemAStar.start_y, xitemAStar.goal_x, xitemAStar.goal_y);
		  file_path = &file_Open[index_file];
		  //-----------------Impresion de la ruta encontrada--------------------
		  send_path(file_path, xitemAStar.grid_map, xitemAStar.grid_map_row, xitemAStar.grid_map_colum);
		  //----------------------------------Fin A Star----------------------------------
		  //Envio como disponible el path MailBox
		  status = PATHTRUE;
		  xQueueOverwrite(xMailbox_Path, &status);
		  //Se envia la opcion especificada
		  xQueueSend(xQueue_Print, &msg_Finish_AStar, portMAX_DELAY);
		  //Entregamos el procesador a la Tarea Print
		  taskYIELD();
		  //cambio de status
		  next_state = sMenuOperation;
		}
	}
}




//--------------Tareas de parada de operacion---------------
//------------Tarea de Stop---------------
void vTask_Stop(void * pvParameters)
{
	//Variables para la recepcion
	command_t xReceivedStructure;
	BaseType_t notify_status;
	const EventBits_t xBitsToWaitFor = (ENABLE_OPERATION_BIT | EXECUTE_OPERATION_BIT);
	uint8_t mode = 0;
	//Ciclo de la tarea
	while(1)
	{
		//Se espera por la recepcion de un comando
		xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		//Se verificamos recibe el comando
		notify_status = xQueueReceive(xQueue_StructCommand, &xReceivedStructure, 0 );
		//Se verificamos el tipo de comando
		if(notify_status == pdTRUE)
		{
			if(strcmp(xReceivedStructure.send_cmd, "stop") == 0)
			{
				//verificamos el modo
				xQueuePeek(xMailbox_Mode, &mode, 0);
				//Se lee el grupo de eventos para limpiar posibles eventos si fuera necesario
			    xEventGroupClearBits(xEventGroup_Execute_Operation, xBitsToWaitFor);
				//Paramos los motores
				status_motor(RESET);
				//Si se esta en el modo 2 se cambia la direccion de giro
				if(mode==2){ updateDirMotor(handler_Motor_Execute); }
				//Se envia la opcion especificada
				xQueueSend(xQueue_Print, &msg_stop, portMAX_DELAY);
				//cambio de status
				next_state = sMenuOperation;
				//Notificamos Tarea de menu para imprimir mensaje de operaciones
				xTaskNotify(xHandleTask_Menu, 0, eNoAction);
			}
			else
			{
				//cambio de status
				next_state = sExecution;
			}
		}
	}
}
//------------Tarea de parada en ejecucccion---------------
void vTask_Stop_Execute(void * pvParameters)
{
	//Definicion de variable
	BaseType_t notify_status = {0};
	uint8_t mode = 0;
	uint32_t ulNotificationValue;
	const EventBits_t xBitsToWaitFor = (ENABLE_OPERATION_BIT | EXECUTE_OPERATION_BIT);
	EventBits_t xReturnBists;

	//Ciclo de la tarea
	while(1)
	{
		//Se espera por la notificacion
		notify_status = xTaskNotifyWait(0,0,&ulNotificationValue,portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//verificamos el modo
			xQueuePeek(xMailbox_Mode, &mode, 0);
			//Verificamos las condiciones de parada
			if(mode==1)
			{
				if(distance_traveled(&parameter_Path_Robot, parameter_Posicion_Robot.xg_position, parameter_Posicion_Robot.yg_position)>parameter_Path_Robot.line_Distance){
					//desactivamos los motores
					status_motor(RESET);
					//Guardamos la posicion final
					parameter_Posicion_Robot.xg_position_inicial = parameter_Posicion_Robot.xg_position;
					parameter_Posicion_Robot.yg_position_inicial = parameter_Posicion_Robot.yg_position;
					//Vericamos si el bit de las operaciones esta activo
					xReturnBists = xEventGroupWaitBits(xEventGroup_Execute_Operation, xBitsToWaitFor, pdFALSE, pdTRUE, 0);
					//Deacuerdo al valor retornado se ejecuta una accion
					if(xReturnBists & ENABLE_OPERATION_BIT) { xEventGroupSetBits(xEventGroup_Execute_Operation, EXECUTE_OPERATION_BIT);}
					else
					{
						//cambio de status
						next_state = sMenuOperation;
						//Notificamos Tarea de menu para imprimir mensaje de operaciones
						xTaskNotify(xHandleTask_Menu, 0, eNoAction);
					}
				}
			}
			else if(mode==2)
			{
				if((fabs(parameter_Posicion_Robot.grad_relativo)*correct_ang_Gyro_Drift) > (fabs(parameter_Path_Robot.rotative_Grad_Relative))){
					//Paramos los motores
					status_motor(RESET);
					updateDirMotor(handler_Motor_Execute);
					//Correccion angulo;
					parameter_Posicion_Robot.grad_relativo = parameter_Posicion_Robot.grad_relativo*correct_ang_Gyro_Drift;
					//Vericamos si el bit de las operaciones esta activo
					xReturnBists = xEventGroupWaitBits(xEventGroup_Execute_Operation, xBitsToWaitFor, pdFALSE, pdTRUE, 0);
					//Deacuerdo al valor retornado se ejecuta una accion
					if(xReturnBists & ENABLE_OPERATION_BIT) { xEventGroupSetBits(xEventGroup_Execute_Operation, EXECUTE_OPERATION_BIT);}
					else
					{
						//cambio de status
						next_state = sMenuOperation;
						//Notificamos Tarea de menu para imprimir mensaje de operaciones
						xTaskNotify(xHandleTask_Menu, 0, eNoAction);
					}
				}
			}
			else{ __NOP(); }
		}
	}
}


//-----------Tareas correspondientes durante la ejecucion de operaciones--------
//------------Tarea de mediciones---------------
void vTask_Measure(void * pvParameters)
{
	//Definicion de variable
	uint8_t mode = 0;
	const TickType_t xDelaySamplingms = pdMS_TO_TICKS(period_sampling);
	//Suspendemos la Tarea
	vTaskSuspend(xHandleTask_Measure);
	//Ciclo de la tarea
	while(1)
	{
		//Se bloque la tarea por un tiempo definido
		vTaskDelay(xDelaySamplingms);
		//Leemos el angulo
		parameter_Posicion_Robot.grad_relativo = getAngle(&handler_MPUAccel_MPU6050, period_sampling, parameter_Posicion_Robot.grad_relativo, READ_GYRO_Z, gyro_offset);
		//De acuerdo al modo se ejecuta una accion
		xQueuePeek(xMailbox_Mode, &mode, 0);
		if(mode == 1 )
		{
			//Acumulamos los angulos
			sum_ang += parameter_Posicion_Robot.grad_relativo;
			//----------------Accion a realizar con un tiempo especifico--------------------
			if(counting_action>=timeAction_TIMER_Sampling)
			{
				//Calculamos el angulo promedio y la establecemis como el angulo relativo
				prom_ang = sum_ang/counting_action;
				parameter_Posicion_Robot.phi_relativo = (prom_ang*M_PI)/180;          //[rad]
				parameter_Posicion_Robot.phi_relativo = atan2(sin(parameter_Posicion_Robot.phi_relativo),cos(parameter_Posicion_Robot.phi_relativo));
				//Calculamos la velocidad
				handler_Motor_L.parametersMotor.distance = (cm_L*handler_Motor_L.parametersMotor.count);                   //[mm]
				handler_Motor_R.parametersMotor.distance = (cm_R*handler_Motor_R.parametersMotor.count);				   //[mm]
				handler_Motor_L.parametersMotor.velocity = handler_Motor_L.parametersMotor.distance/time_accion;      //[m/s]
				handler_Motor_R.parametersMotor.velocity = handler_Motor_R.parametersMotor.distance/time_accion;      //[m/s]
				//Reiniciamos el numero de conteos
				handler_Motor_R.parametersMotor.count = handler_Motor_L.parametersMotor.count = 0;
				//Reiniciamos variable
				sum_ang = counting_action = 0;
				//Notificamos a la tarea respectiva
				xTaskNotify(xHandleTask_Line_PID, 0, eNoAction);
			}
			else{ counting_action++;}
		}
		else if(mode == 2)
		{
			//Notificamos a la tarea respectiva
			xTaskNotify(xHandleTask_Stop_Execute, 0, eNoAction);
		}
		else{ vTaskSuspend(xHandleTask_Measure); }
	}
}
//------------Tarea Line PID---------------
void vTask_Line_PID(void * pvParameters)
{
	//Definicion de variable
	BaseType_t notify_status = {0};
	float sampling_timer = ((float) time_accion/1000);
	float distance_c = 0;
	char bufferMsg[40] = {0};
	char *prtbuffer = bufferMsg;
	float distance_recta = 0;

	while(1)
	{
		//Se espera por la notificacion
		notify_status = xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Calculo odometria
			distance_c = (handler_Motor_R.parametersMotor.distance+handler_Motor_L.parametersMotor.distance)/2;  	//[mm]
			parameter_Posicion_Robot.xr_position += distance_c*(cos(parameter_Posicion_Robot.phi_relativo));        //[mm]
			parameter_Posicion_Robot.yr_position += distance_c*(sin(parameter_Posicion_Robot.phi_relativo));       //[mm]
			//Paso de c.relativa a c.globales
			parameter_Posicion_Robot.xg_position = parameter_Posicion_Robot.xg_position_inicial + parameter_Posicion_Robot.xr_position*cos_cal - parameter_Posicion_Robot.yr_position*sin_cal;
			parameter_Posicion_Robot.yg_position = parameter_Posicion_Robot.yg_position_inicial + parameter_Posicion_Robot.xr_position*sin_cal + parameter_Posicion_Robot.yr_position*cos_cal;
			//Convertimos el valor y imprimemos
			sprintf(bufferMsg,"&%#.4f\t%#.4f\n", parameter_Posicion_Robot.xg_position , parameter_Posicion_Robot.yg_position);
			xQueueSend(xQueue_Print, &prtbuffer, portMAX_DELAY);
			//Control PID para la distancia
			distance_recta = (distance_to_straight_line(&parameter_Path_Robot, parameter_Posicion_Robot.xg_position, parameter_Posicion_Robot.yg_position))/1000;
			PID_simple(&parameter_PID_distace, sampling_timer, 0,  distance_recta);
			//Aplicacndo correcion
			vel_Setpoint_L = velSetPoint - parameter_PID_distace.u;
			vel_Setpoint_R = velSetPoint + parameter_PID_distace.u;
			//Aplicacion del PID
			PID_simple(&handler_Motor_L.parametersMotor.parametersPID, sampling_timer, vel_Setpoint_L,  handler_Motor_L.parametersMotor.velocity);
			PID_simple(&handler_Motor_R.parametersMotor.parametersPID, sampling_timer, vel_Setpoint_R,  handler_Motor_R.parametersMotor.velocity);
			//Cambiamos valores
			handler_Motor_L.configMotor.new_dutty += handler_Motor_L.parametersMotor.parametersPID.u;
			handler_Motor_R.configMotor.new_dutty += handler_Motor_R.parametersMotor.parametersPID.u;
			//Correccion del dutty
			correction(&handler_Motor_L);
			correction(&handler_Motor_R);
			//Notificamos a la tarea respectiva
			xTaskNotify(xHandleTask_Stop_Execute, 0, eNoAction);
		}
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



//-----------------------Inicio definicion funciones para recepcion de mensaje---------------------------
//Funcion que ayuda en el procesamiento del comando
void process_stringsend(char stringsend[500])
{
	//Definicion de structura del comando
	command_t structcmd = {0};
	//Extraemos el string enviado
	extract_stringsend(stringsend);
	//De acuerdo al state se procesa el mensaje
	if(next_state==sMenuOperation || next_state==sExecution)
	{
		//Funcion que lee la cadena de caracteres y la divide en los elementos definidos
		sscanf(stringsend, "%s %u %u", structcmd.send_cmd, &structcmd.firtparameter, &structcmd.secondparameter);
		//Envio de struct a la cola
		xQueueSend(xQueue_StructCommand, &structcmd, 0);
		//De al state se ejecuta una respectiva tarea
		switch(next_state)
		{
		case sMenuOperation:
			//Notificamos a la tarea respectiva
			xTaskNotify(xHandleTask_Menu, 0, eNoAction);
			break;
		case sExecution:
			//Notificamos a la tarea respectiva
			xTaskNotify(xHandleTask_Stop, 0, eNoAction);
			break;
		default:
			__NOP();
			break;
		}
		//cambio de status
		next_state = sNullReception;
	}
	else if(next_state==sAStar)
	{

		//cambio de status
		next_state = sNullReception;
		//Notificamos a la tarea respectiva
		xTaskNotify(xHandleTask_Separate_GridMap, (uint32_t) stringsend, eSetValueWithoutOverwrite);
	}
	else{ __NOP(); }
}
//Funcion para obtener el comando
int extract_stringsend(char cmd[500])
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
			cmd[counter_j++] = item;
		}
	}while(item != '@');

	//Agregamos el elemento nulo y ademas definimos el largo del mensaje
	cmd[counter_j - 1] = '\0';

	return 0;
}
//-----------------------Fin definicion funciones para recepcion de mensajes---------------------------


//-------------Inicio de la definicion de las funciones para la contruccion de la lista de operaciones ----------------------------------
//Operaciones del Cuadrado
void set_operation_square(Parameters_Operation_t *prtList, double dis_side, double direction_square)
{
	//Definicion de variables
	Parameter_build_t parameter_build = {0};
	int8_t value_side = 0;
	//Definicion de las coordenadas del cuadrado
	if(direction_square == 0){value_side = 1;}
	else{ value_side = -1;}
	double coordination_position_square[2][5] = {{0, dis_side, dis_side, 0, 0},{0, 0, value_side*dis_side,value_side*dis_side,0}};
	//Definimos la posicion inicial del cuadrado
	parameter_build.initline_x = coordination_position_square[0][0]; parameter_build.initline_y = coordination_position_square[0][0];
	parameter_build.grad_vector_init = 0; parameter_build.number_operation = 0;
	parameter_build.delta_before[0] = dis_side; parameter_build.delta_before[1] = 0;
	//Construccion de las operaciones
	for(uint8_t i=1; i<5; i++)
	{
		//Contruimos la operacion
		build_Operation(prtList, &parameter_build, coordination_position_square[0][i], coordination_position_square[1][i]);
		//Aumentamos el recorrido en la lista
		parameter_build.routelist++;
	}
	//Agregamos indicador de la operacion final
	prtList[parameter_build.number_operation+1].operacion = NULL_OPERATION;
}

//Operaciones del AStar
void set_operation_AStar(Parameters_Operation_t *prtList, file_cell_t *file_cell, Parameters_Position_t *ptrParameterPosition, Parameters_Path_t *ptrParameterPath)
{
	//Definicion de variables
	Parameter_build_t parameter_build = {0};
	//Definimos la posicion inicial del cuadrado
	//-----------------NOTA: RECORDAR QUE EL OPPY ESTA INICIALMENTE ORIENTADO 90 GRADOS CON RESPECTO AL EJE X---------------
	ptrParameterPath->rotative_Grad = ptrParameterPosition->grad_grobal = 90;
	parameter_build.initline_x = ptrParameterPosition->xg_position_inicial = ptrParameterPath->goal_Position_x = (file_cell->ptrCell_parent[0]->coor_x)*10;
	parameter_build.initline_y = ptrParameterPosition->yg_position_inicial = ptrParameterPath->goal_Position_y = (file_cell->ptrCell_parent[0]->coor_y)*10;
	parameter_build.grad_vector_init = 0; parameter_build.number_operation = parameter_build.routelist = 0;
	parameter_build.delta_before[0] = 0; parameter_build.delta_before[1] = 10;
	//Construccion de las operaciones
	for(uint8_t i=1; i<100; i++)
	{
		//Comprobamos si la celda no es un elemento nulo
		if(file_cell->ptrCell_parent[i] != NULL)
		{
			//Construimos la operacion
			build_Operation(prtList, &parameter_build, (file_cell->ptrCell_parent[i]->coor_x)*10, (file_cell->ptrCell_parent[i]->coor_y)*10);
			//Aumentamos el valor en el recorrido
			parameter_build.routelist++;
		}
		else{break;}
	}
	//Se Agrega la operacion final para llegar al goal
	//Construimos la operacion
	build_Operation(prtList, &parameter_build, (file_cell->ptrCell_file->coor_x)*10, (file_cell->ptrCell_file->coor_y)*10);
	//Agregamos indicador de la operacion final
	prtList[parameter_build.number_operation+1].operacion = NULL_OPERATION;
}
//----------------Fin de la definicion de las funciones para la contruccion de la lista de operaciones ----------------------------------




//------------------------------Inicio de la definicion de funciones del modo----------------------------------------
//-----Establecer Operaciones-----
void set_operation_in_queue(Parameters_Operation_t list[30])
{
	//Definimos structura para recibir los elementos y asi vaciar la cola
	Parameters_Operation_t xReceiveClear;
	//Definicion bits
	const EventBits_t xBitsSet = (ENABLE_OPERATION_BIT | EXECUTE_OPERATION_BIT);
	//Limpiamos la cola de operaciones
	while(xQueueReceive(xQueue_Operation, &xReceiveClear, 0) == pdPASS){__NOP();}
	//Recorremos la lista de operaciones
	for(uint8_t i=0; i<30; i++)
	{
		//Verificamos que la operacion en la lista no sea nula
		if(list[i].operacion != NULL_OPERATION)
		{
			//Agregamos configuraciones de operaciones a la cola
			xQueueSendToBack(xQueue_Operation, &list[i], 0);
		}
		else{ break;}
	}
	//Establecemos Event Flag para ejecutar la funcion encargada de las operaciones
	xEventGroupSetBits(xEventGroup_Execute_Operation, xBitsSet);
}
//------linea recta------
void straight_line(uint8_t dutty)   //a = [mm]
{
	//---------Calculo parametros de la linea recta---------
	calculation_parameter_distance(&parameter_Path_Robot);
	//---------Configuracion coordenadas medidas---------
	//Coordenadas Globales
	parameter_Posicion_Robot.grad_grobal += parameter_Posicion_Robot.grad_relativo;
	//Reiniciaos Coordenadas relativas
	parameter_Posicion_Robot.xr_position = parameter_Posicion_Robot.yr_position = 0;
	parameter_Posicion_Robot.phi_relativo = 0;
	parameter_Posicion_Robot.grad_relativo = 0;
	ang_for_Displament_ICR = 0;
	//---------Configuracion coordenadas teoricas---------
	parameter_Path_Robot.rotative_Grad_Relative = 0;
	//Calculos extra
	cos_cal = cos(((parameter_Posicion_Robot.grad_grobal*M_PI)/180));
	sin_cal = sin(((parameter_Posicion_Robot.grad_grobal*M_PI)/180));
	//-----------------PID-------------------------
	//Establecemos valores iniciales
	handler_Motor_L.parametersMotor.parametersPID.e = handler_Motor_L.parametersMotor.parametersPID.e_prev = 0;
	handler_Motor_L.parametersMotor.parametersPID.u =  handler_Motor_L.parametersMotor.parametersPID.e_intel = 0;
	handler_Motor_R.parametersMotor.parametersPID.e = handler_Motor_R.parametersMotor.parametersPID.e_prev = 0;
	handler_Motor_R.parametersMotor.parametersPID.u =  handler_Motor_R.parametersMotor.parametersPID.e_intel = 0;
	parameter_PID_distace.e = parameter_PID_distace.e_prev = parameter_PID_distace.u =  parameter_PID_distace.e_intel = 0;
	//-------------Configruacion Modo--------------
	//Definimos el dutty inicial
	handler_Motor_L.configMotor.new_dutty = duttySetPoint;
	handler_Motor_R.configMotor.new_dutty = duttySetPoint;
	//Reiniciamos variables
	counting_action = 0;
	//Cargamos la configuracion del modo e iniciamos el modo
	config_mode(1, dutty, dutty);
}
//---------Giro sobre si mismo---------
void turn_itself(int16_t turn_grad)     //a = [grados], b = direccion giro
{
	//-------------Configruacion Modo--------------
	//Definicion del angulo de giro
	parameter_Path_Robot.rotative_Grad += parameter_Path_Robot.rotative_Grad_Relative += turn_grad;
	//Cambiamso la direccion del motor
	if(turn_grad<0)
	{
		//Seleccionamos el motor derecho
		handler_Motor_Execute = &handler_Motor_R;
		//Actualizamos la direccion del motor
		updateDirMotor(handler_Motor_Execute);
	}
	else
	{
		//Seleccionamos el motor izquierdo
		handler_Motor_Execute = &handler_Motor_L;
		//Actualizamos la direccion del motor
		updateDirMotor(handler_Motor_Execute);
	}

	//Cargamos la configuracion del modo e iniciamos el modo
	config_mode(2,24,25);
}
//Configuracion del modo
void config_mode(uint8_t status, float dutty_L, float dutty_R)
{
	//Especificar el modo de operacion por medio de una Mailbox
	xQueueOverwrite(xMailbox_Mode, &status);
	//Cargamos la configuracion
	config_motor(dutty_L, dutty_R, frequency_PWM_Motor); //Tipo de Estudio, por dutty L, por dutty R, fre pwm [hz]
	//Iniciamos los motores
	status_motor(SET);
}
//Reinicio de coordenadas
void init_coordinates(void)
{
	//Reinicio de varibable
	ang_for_Displament_ICR = 0;
	ang_complementary = 0;
	//Reinicio de parametros de la structura de la posicion del robot
	parameter_Posicion_Robot.grad_grobal = 0; parameter_Posicion_Robot.grad_relativo = 0; parameter_Posicion_Robot.phi_relativo = 0;
	parameter_Posicion_Robot.xg_position = 0; parameter_Posicion_Robot.xg_position_inicial = 0; parameter_Posicion_Robot.xr_position = 0;
	parameter_Posicion_Robot.yg_position = 0; parameter_Posicion_Robot.yg_position_inicial = 0; parameter_Posicion_Robot.yr_position = 0;
	//Reinicio de parametros de la structura de path
	parameter_Path_Robot.goal_Position_x = 0; parameter_Path_Robot.goal_Position_y = 0;
	parameter_Path_Robot.line_Distance = 0;
	parameter_Path_Robot.rotative_Grad = 0; parameter_Path_Robot.rotative_Grad_Relative = 0;
	parameter_Path_Robot.start_position_x = 0; parameter_Path_Robot.start_position_y = 0;
}

//-----------------------------Fin de la definicio de funciones del modo--------------------------------------------


//----------------------Iinicio definicion de las funciones de la Operacion Motor---------------------------------
void status_motor(uint8_t status)
{
	//Variable de modo
	uint8_t mode = 0;
	//Deacuerdo al estado
	if(status == SET)
	{
		//Activamos el motor
		statusInOutPWM(handler_Motor_L.phandlerPWM, CHANNEL_ENABLE);
		statusInOutPWM(handler_Motor_R.phandlerPWM, CHANNEL_ENABLE);
		GPIO_writePin(handler_Motor_L.phandlerGPIOIN, (handler_Motor_L.configMotor.dir)&SET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOIN, (handler_Motor_R.configMotor.dir)&SET);
		GPIO_writePin(handler_Motor_L.phandlerGPIOEN, RESET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOEN, RESET);
		//Reanudamos la Tarea
		vTaskResume(xHandleTask_Measure);
	}
	else
	{
		//Desactivamos el motor
		statusInOutPWM(handler_Motor_L.phandlerPWM, CHANNEL_DISABLE);
		statusInOutPWM(handler_Motor_R.phandlerPWM, CHANNEL_DISABLE);
		GPIO_writePin(handler_Motor_L.phandlerGPIOIN, (handler_Motor_L.configMotor.dir)&RESET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOIN, (handler_Motor_R.configMotor.dir)&RESET);
		GPIO_writePin(handler_Motor_L.phandlerGPIOEN, SET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOEN, SET);
		//Especificar el modo de operacion por medio de una Mailbox
		xQueueOverwrite(xMailbox_Mode, &mode);
	}
}
//Funcion para al configuracion de los motores
void config_motor(int firth, float second, float third)  //Tipo de Estudio, por dutty L, por dutty R, fre pwm [hz]
{
	//Establecer valores
	handler_Motor_R.parametersMotor.count = 0;
	handler_Motor_L.parametersMotor.count = 0;
	//Actualizamos el valor del dutty y frecuencia
	float value_period = 100000/third;
	updateFrequencyTimer(&handler_TIMER_Motor, value_period);
	updateDuttyMotor(&handler_Motor_R, second);
	updateDuttyMotor(&handler_Motor_L, firth);
}
//----------------------Fin definicion de las funciones de la Operacion Motor---------------------------------




//----------------------------Inicio de la definicion de las funciones para el PID-----------------------------------------
void PID_simple(Parameters_PID_t *ptrPIDHandler, float timer, float setpoint, float measure)
{
	//Calculo del error
	ptrPIDHandler->e = setpoint-measure;
    // Controle PID
	float pro =  ptrPIDHandler->kp*ptrPIDHandler->e;
	ptrPIDHandler->e_intel +=  ptrPIDHandler->e*timer;
	float deriv =  ptrPIDHandler->kd*(ptrPIDHandler->e - ptrPIDHandler->e_prev)/timer;
	ptrPIDHandler->u =  pro + ptrPIDHandler->ki*ptrPIDHandler->e_intel + deriv;        //Ley del controlador PID discreto
     //Retorno a los valores reales
	ptrPIDHandler->e_prev = ptrPIDHandler->e;
}

void correction(Motor_Handler_t *ptrMotorHandler)
{
	//Definimos variables auxiliares
	float port_dutty = 0;
	//Guardamos valor
	port_dutty = ptrMotorHandler->configMotor.new_dutty;
    //Saturo el porcentaje de dutty en un tope maximo y minimo
    if (port_dutty >= 60) { port_dutty = 60; }
    else if(port_dutty <= duttySetPoint-5) { port_dutty = duttySetPoint-5; }
    else{ __NOP();}

    //Actualizamoe el valor del dutty
    updateDuttyMotor(ptrMotorHandler, port_dutty);
}
//----------------------------Fin de la definicion de las funciones para el PID-----------------------------------------




//Funcion para separar los diferentes parametros del string
uint8_t Separate_parameters(item_A_Star_t* ptritem, char *parameter_string)
{
  //Definicion de variables
  char buffercharSeparate[10];
  uint8_t index_charSeparate = 0;
  uint8_t flag_Separate = 0;
  uint8_t status_parameter = 0;
  uint8_t index_init_Grid_map = 0;
  uint8_t findStart = 0;
  uint8_t findGoal = 0;

  //---------------Definicion de los parametros iniciales---------------
  for (uint16_t i = 0; parameter_string[i] != '\0'; i++)
  {
    //Verificacion del estado
    if(status_parameter<3)
    {
      //Busqueda de la separacion dentro del string
      if (parameter_string[i] == ':')
      {
        buffercharSeparate[index_charSeparate] = '\0';
        index_charSeparate = 0;
        flag_Separate = 1;
      }
      else
      {
        buffercharSeparate[index_charSeparate] = parameter_string[i];
        index_charSeparate++;
      }
      //Si la bandera se levanta se asigna el valor correspondiente al parametro
      if(flag_Separate == 1)
      {
        switch(status_parameter)
        {
          case 0:
          {
        	//Convertimos valor y lo almacenamos
        	ptritem->grid_map_row = atoi(buffercharSeparate);
            //Verificacion parametros mal enviados
            if(ptritem->grid_map_row>20){ return SGMFALSE; }
            break;
          }
          case 1:
          {
          	//Convertimos valor y lo almacenamos
        	ptritem->grid_map_colum = atoi(buffercharSeparate);
            //Verificacion parametros mal enviados
            if(ptritem->grid_map_colum>20){ return SGMFALSE; }
            break;
          }
          case 2:
          {
            //Convertimos valor y lo almacenamos
        	ptritem->cell_separation = atof(buffercharSeparate);
            //Verificacion parametros mal enviados
            if(ptritem->cell_separation>100){ return SGMFALSE; }
            break;
          }
        }
        //Aumentamos y reiniciamos bandera
        status_parameter++;
        flag_Separate = 0;
      }
    }
    else
    {
      index_init_Grid_map = i;
      break;
    }
    //Verificacion parametros mal enviados
    if(index_charSeparate>5){ return SGMFALSE; }
  }
  //---------------transformacion del string grid map en un array---------------
  //Variables para los indices
  uint8_t index_row = 0;
  uint8_t index_col = 0;

  //Separacion de cada caracter
  for (uint16_t i = index_init_Grid_map; parameter_string[i] != '\0'; i++)
  {
    if (parameter_string[i] == ';')
    {
      //Se aumenta el indice de la fila y se reinicia el indice de la columna
      index_row++;
      index_col = 0;
      //Verificacion parametros mal enviados
      if(index_row>ptritem->grid_map_row){ return SGMFALSE; }
    }
    else
    {
      //Se guarda el caracter
      ptritem->grid_map[index_row][index_col].feature = parameter_string[i];
      //se aumenta el indice de la columna
      index_col++;
      //Verificacion parametros mal enviados
      if(index_col>ptritem->grid_map_colum){ return SGMFALSE; }
    }
  }
  //---------------Definicion del start y del goal---------------
  //Recorremo el array creado para encontrar la posicion de los objetivos
  for(int i = 0; i < ptritem->grid_map_row; i++)
  {
    for(int j = 0; j < ptritem->grid_map_colum; j++)
    {
      if(ptritem->grid_map[i][j].feature == 'S')
      {
        //Definimos su posicion
    	ptritem->start_x = j*ptritem->cell_separation;
    	ptritem->start_y = i*ptritem->cell_separation;
    	//Aumentamos valor
    	findStart++;
      }
      else if (ptritem->grid_map[i][j].feature == 'G')
      {
		  //Definimos su posicion
		  ptritem->goal_x= j*ptritem->cell_separation;
		  ptritem->goal_y= i*ptritem->cell_separation;
		  //Aumentamos valor
		  findGoal++;
      }
    }
  }
  /*Si el codigo llego hasta aqui falta un ultima verificacion, con lo cual
  se envia un TRUE o FALSe*/
  if(findStart==1 && findGoal==1){ return SGMTRUE;}
  else{ return SGMFALSE; }
}


//Funcion para imprimir la ruta encontrada
void send_path(file_cell_t *file_cell, Cell_map_t array_string[20][20], uint8_t row, uint8_t colum)
{
  //Variables
  uint8_t index = 0;
  char buffermsg[25] = {0};
  char *ptrmsg = buffermsg;

  //Cambiamos los caracteres de la malla de strings por caracteres que indican la ruta establecida con A Star
  while(1)
  {
	if(file_cell->ptrCell_parent[index] != NULL)
	{
	   array_string[file_cell->ptrCell_parent[index]->index_row][file_cell->ptrCell_parent[index]->index_col].feature = '+';
	  index++;
	}
	else
	{
	  break;
	}
  }
  //Indica de nuevo el inicio del recorrido
   array_string[file_cell->ptrCell_parent[0]->index_row][file_cell->ptrCell_parent[0]->index_col].feature = 'S';

  //Envio de caracter para indicar que se trata del grid map
  buffermsg[0] = '$'; buffermsg[1] = '\0';
  //Se envia la opcion especificada
  xQueueSend(xQueue_Print, &ptrmsg, portMAX_DELAY);
  //Entregamos el procesador a la Tarea Print
  taskYIELD();
  //Imprimir la malla modificada
  for(int i=0;i<row;i++)
  {
	for(int j=0;j<colum;j++)
	{
	  //Agregamos las caracteristicas por fila a un buffer
	  buffermsg[j] = array_string[i][j].feature;
	  index = j;
	}
	//Agragamos el valor nullo al final del string
	buffermsg[index+1] = ';';
	buffermsg[index+2] = '\0';
	//Se envia la opcion especificada
	xQueueSend(xQueue_Print, &ptrmsg, portMAX_DELAY);
	//Entregamos el procesador a la Tarea Print
	taskYIELD();
  }

}


//------------------------------Fin de la definicion de funciones del A-STAR----------------------------------------

