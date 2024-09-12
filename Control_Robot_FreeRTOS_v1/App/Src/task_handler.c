/*
 * task_handler.c
 *
 *  Created on: Aug 7, 2024
 *      Author: julil
 */


#include "main.h"

//-----Cabeceras de funciones para los motores----------
void config_motor(uint8_t status, int firth, float second, float third);
void status_motor(uint8_t status);
void PID_simple(Parameters_PID_t *ptrPIDHandler, float timer, float setpoint, float measure);
void correction(Motor_Handler_t *ptrMotorHandler);
void init_coordinates(void);
//----Cabeceras de funciones para el control de los modos------
void straight_line(uint8_t dutty);
void config_mode(uint8_t status, float dutty_L, float dutty_R);
//-----Cabeceras de funciones de los comandos--------
void process_command(char cmd[20]);
int extract_command(char cmd[20]);
//------------------Variables---------------------
//---------mensajes----------
const char *msg_com_invalid = "Comando invalido  \n";
const char *msg_stop= "Accion de parada \n";
//-------Odometria-------------
Parameters_Path_t parameter_Path_Robot = {0};           //Estructura que almacena los parametros del camino a recorrer
float cos_cal  = 0;                                     //Variables que almacenan el calculo del seno y coseno
float sin_cal  = 0;
double ang_for_Displament_ICR = 0;                //Variables para correccion del angulo en el giro
double ang_complementary = 0;
//-----Variables de los modos y operacion---
float vel_Setpoint_R = 0;                         //Diferentes setpoit
float vel_Setpoint_L = 0;


//-----------------------Inicio definicion funciones de las Task---------------------------
//------Tarea Menu-------
void vTask_Menu(void * pvParameters)
{
	//Variables para la recepcion
	uint32_t cmd_addr;
	//Variables de la separacin
	char cmd[20]= {0};
	int firtsParameter = 0;
	int secondParameter = 0;

	//Mensaje inicial del menu
	const char* msg_menu = "=======================\n"
			               "|         Menu        |\n"
						   "=======================\n"
						   "go ---> inicia \n";

	while(1)
	{
		if(next_state == sMainMenu){ xQueueSend(xQueue_Print, &msg_menu, portMAX_DELAY); }
		//Se espera por el comando a ejecutar
		xTaskNotifyWait(0,0,&cmd_addr, portMAX_DELAY);
		//Funcion que lee la cadena de caracteres y la divide en los elementos definidos
		sscanf((char* ) cmd_addr, "%s %u %u", cmd, &firtsParameter, &secondParameter);

		//Se verificamos si se tiene un solo caracter
		if(strcmp(cmd, "go") == 0)
		{
			//Notificamos a la tarea respectiva
			//xTaskNotify(xHandleTask_Go, (uint32_t) firtsParameter, eSetValueWithoutOverwrite);
			xTaskNotify(xHandleTask_Go, 0, eNoAction);
			//Cambio de state
			next_state = sGo;
		}
		else
		{
			//Se envia la opcion especificada
			xQueueSend(xQueue_Print, &msg_com_invalid, portMAX_DELAY);
			/*Se envia una notificacion previa con la finalidad de desbloquear
			 la tarea, la cual se bloquea para que se envie el mensaje por USART*/
			xTaskNotify(xHandleTask_Menu, 0, eNoAction);
			xTaskNotifyWait(0,0,NULL, portMAX_DELAY);
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

//-------Tarea de los comando-------
void vTask_Commands(void * pvParameters)
{
	BaseType_t notify_status = {0};
	char cmd[20];

	while(1)
	{
		//Se espera por la notificacion de la interrupcion
		notify_status = xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Se procesa el comando recibido
			process_command(cmd);
		}
	}
}

//------------Tarea Go---------------
void vTask_Go(void * pvParameters)
{
	//Definicion de variable de notificacion
	BaseType_t notify_status = {0};

	while(1)
	{
		//Se espera por la notificacion
		notify_status = xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		//Si es verdadero se recibe una notificacion
		if(notify_status == pdTRUE)
		{
			//Restablecimiento de coordenadas
			init_coordinates();
			//Definicion de parametros
			change_position(&parameter_Path_Robot, 3000);
			//Configuracion inicial linea recta
			straight_line(duttySetPoint);
		}
	}
}

//------------Tarea de Stop---------------
void vTask_Stop(void * pvParameters)
{
	//Variables para la recepcion
	uint32_t cmd_addr;
	//Variables de la separacin
	char cmd[20]= {0};
	int firtsParameter = 0;
	int secondParameter = 0;

	while(1)
	{
		//Se espera por el comando a ejecutar
		xTaskNotifyWait(0,0,&cmd_addr, portMAX_DELAY);
		//Funcion que lee la cadena de caracteres y la divide en los elementos definidos
		sscanf((char* ) cmd_addr, "%s %u %u", cmd, &firtsParameter, &secondParameter);
		//Si es verdadero se recibe una notificacion
		if(strcmp(cmd, "stop") == 0)
		{
			//Paramos los motores
			status_motor(RESET);
			//Restablecimiento de coordenadas
			init_coordinates();
			//cambio de status
			next_state = sMainMenu;
			//Se envia la opcion especificada
			xQueueSend(xQueue_Print, &msg_stop, portMAX_DELAY);
			/*Se envia una notificacion previa con la finalidad de desbloquear
			 la tarea, la cual se bloquea para que se envie el mensaje por USART*/
			xTaskNotify(xHandleTask_Menu, 0, eNoAction);
			xTaskNotifyWait(0,0,NULL, portMAX_DELAY);
		}
	}
}


//------------Tarea Go---------------
void vTask_Line_PID(void * pvParameters)
{
	//Definicion de variable
	BaseType_t notify_status = {0};
	float sampling_timer = ((float) time_accion/1000);
	float distance_c = 0;
	char bufferMsg[64] = {0};
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
			xQueueSend(xQueue_Print, &bufferMsg, portMAX_DELAY);
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



//-----------------------Inicio definicion funciones para la comunicacion---------------------------

//Funcion que ayuda en el procesamiento del comando
void process_command(char cmd[20])
{
	extract_command(cmd);

	switch(next_state)
	{
	case sMainMenu:
		//Notificamos a la tarea respectiva
		xTaskNotify(xHandleTask_Menu, (uint32_t) cmd, eSetValueWithoutOverwrite);
		break;
	case sGo:
		xTaskNotify(xHandleTask_Stop, (uint32_t) cmd, eSetValueWithoutOverwrite);
		break;
	default:
		__NOP();
		break;
	}
}
//Funcion para obtener el comando
int extract_command(char cmd[20])
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
	}while(item != '#');

	//Agregamos el elemento nulo y ademas definimos el largo del mensaje
	cmd[counter_j - 1] = '\0';

	return 0;
}

//-----------------------Fin definicion funciones para la comunicacion---------------------------


//------------------------------Inicio de la definicion de funciones del modo----------------------------------------
//------linea recta------
void straight_line(uint8_t dutty)   //a = [mm]
{
	//---------Calculo parametros de la linea recta---------
	calculation_parameter_distance(&parameter_Path_Robot);
	//---------Configuracion coordenadas medidas---------
	//Coordenadas Globales
	parameter_Posicion_Robot.grad_grobal += ang_complementary;
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
	time_accumulated = counting_action = 0;
	//Cargamos la configuracion del modo e iniciamos el modo
	config_mode(1, dutty, dutty);
}


void config_mode(uint8_t status, float dutty_L, float dutty_R)
{
	//Cargamos la configuracion
	config_motor(status, dutty_L, dutty_R, frequency_PWM_Motor); //Tipo de Estudio, por dutty L, por dutty R, fre pwm [hz]
	//Iniciamos los motores
	status_motor(SET);
}

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
	if(status == 1)
	{
		//Activamos el motor
		statusInOutPWM(handler_Motor_L.phandlerPWM, CHANNEL_ENABLE);
		statusInOutPWM(handler_Motor_R.phandlerPWM, CHANNEL_ENABLE);
		GPIO_writePin(handler_Motor_L.phandlerGPIOIN, (handler_Motor_L.configMotor.dir)&SET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOIN, (handler_Motor_R.configMotor.dir)&SET);
		GPIO_writePin(handler_Motor_L.phandlerGPIOEN, RESET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOEN, RESET);
		//Activamos la interrupcion
		statusiInterruptionTimer(&handler_TIMER_Sampling, INTERRUPTION_ENABLE);
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
		//Desactivamos interrupcion
		statusiInterruptionTimer(&handler_TIMER_Sampling, INTERRUPTION_DISABLE);
	}
}

//Funcion para al configuracion de los motores
void config_motor(uint8_t status, int firth, float second, float third)  //Tipo de Estudio, por dutty L, por dutty R, fre pwm [hz]
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

