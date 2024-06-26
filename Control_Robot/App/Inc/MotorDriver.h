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

//Estructura que contiene la configuracion del motor
typedef struct
{
	uint16_t *frecuency;             //frecuencia del timer
	uint8_t dutty;                   //dutty de pwm del motor
	uint8_t dir;                     //indica el tipo de giro
	uint16_t count;                  //Numero de cuentas del motor

}Config_Motor_t;

//Estrutura que contienes la configuracion del motor como los handler para los diferentes perifericos para el uso del robot
typedef struct
{
	Config_Motor_t configMotor;            //Configuracion del motor
	GPIO_Handler_t *phandlerGPIOIN;        //Handler para el el pin del IN del driver del motor
	GPIO_Handler_t *phandlerGPIOEN;        //Handler para el el pin del EN del driver del motor
	PWM_Handler_t *phandlerPWM;            //Handler del pwm para el driver del motor
}Motor_Handler_t;

//Definimos las cabeceras de las funciones
void updateDuttyMotor(Motor_Handler_t *ptrMotorHandler, uint8_t newValue);
void updateFrequencyMotor(Motor_Handler_t *ptrMotorHandler, BasicTimer_Handler_t *ptrBTimerHandler, uint16_t newValue);
void updateDirMotor(Motor_Handler_t *ptrMotorHandler);

#endif /* INC_MOTORDRIVER_H_ */
