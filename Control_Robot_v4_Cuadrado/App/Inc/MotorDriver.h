/*
 * MotorDriver.h
 *
 *  Created on: Mar 21, 2024
 *      Author: julil
 */

#ifndef INC_MOTORDRIVER_H_
#define INC_MOTORDRIVER_H_

#include <GPIOxDriver.h>
#include <PwmDriver.h>
#include <BasicTimer.h>

//Estructura que contiene los parametros del PID
typedef struct
{
	float phi_relativo;                                  //Angulo relativo       [rad]
	float xr_position, yr_position;        				 //Coordenadas relativas [mm]
	float xg_position_inicial, yg_position_inicial;      //Coordenadas gloabales iniciales [mm]
	float xg_position,yg_position;         				 //Coordenadas gloabales [mm]
	float ang_Giro;                                      //Grados que esta girado el robet [grados]

}Parameters_Position_t;


//Estructura que contiene los parametros del PID
typedef struct
{
	float u,u_1;           					//Acción de Control now y back
	float e,e_1,e_2;   						//Valores PID discreto
	float q0,q1,q2; 					 	//Costantes del PID discreto
	float u_intel, e_prev;				    //Valores PID simple
	uint16_t kp, ki, kd;				       //Constantes PID simple

}Parameters_PID_t;

//Estructura que contiene los parametros del motor
typedef struct
{
	Parameters_PID_t parametersPID;         //Parametros del PID
	uint16_t count;                 		//Numero de cuentas del motor
	float velocity;						    //Velodicad actual
	float distance;                         //Distancia recorrida en un intervalo de tiempo

}Parameters_Motor_t;

//Estructura que contiene la configuracion del motor
typedef struct
{
	uint16_t *frecuency;             //frecuencia del timer
	float dutty;                   //dutty de pwm del motor
	uint8_t dir;                    //indica el tipo de giro

}Config_Motor_t;

//Estrutura que contienes la configuracion del motor como los handler para los diferentes perifericos para el uso del robot
typedef struct
{
	Parameters_Motor_t parametersMotor;    //Parametros del motor
	Config_Motor_t configMotor;            //Configuracion del motor
	GPIO_Handler_t *phandlerGPIOIN;        //Handler para el el pin del IN del driver del motor
	GPIO_Handler_t *phandlerGPIOEN;        //Handler para el el pin del EN del driver del motor
	PWM_Handler_t *phandlerPWM;            //Handler del pwm para el driver del motor
}Motor_Handler_t;

//Definimos las cabeceras de las funciones
void updateDuttyMotor(Motor_Handler_t *ptrMotorHandler, float newValue);
void updateFrequencyMotor(Motor_Handler_t *ptrMotorHandler, BasicTimer_Handler_t *ptrBTimerHandler, uint16_t newValue);
void updateDirMotor(Motor_Handler_t *ptrMotorHandler);

#endif /* INC_MOTORDRIVER_H_ */
