/*
 * MotorDriver.c
 *
 *  Created on: Mar 21, 2024
 *      Author: julil
 */

#include <MotorDriver.h>


//Funcion para actualizar el dutty del pwm
void updateDuttyMotor(Motor_Handler_t *ptrMotorHandler,  float newValue)
{
		//Guardar valor del dutty
		ptrMotorHandler->configMotor.dutty = newValue;
		//Actualizamos el valor del dutty
		updateDuttyCyclePercentageFloat(ptrMotorHandler->phandlerPWM, ptrMotorHandler->configMotor.dutty);
}


void updateFrequencyMotor(Motor_Handler_t *ptrMotorHandler, BasicTimer_Handler_t *ptrBTimerHandler, uint16_t newValue)
{
		//Guardar valor del dutty
		*(ptrMotorHandler->configMotor.frecuency) = newValue;
		//Actualizamos el valor del frecuencia y dutty
		updateFrequencyTimer(ptrBTimerHandler, *(ptrMotorHandler->configMotor.frecuency));
		updateDuttyCyclePercentageFloat(ptrMotorHandler->phandlerPWM, ptrMotorHandler->configMotor.dutty);
}

void updateDirMotor(Motor_Handler_t *ptrMotorHandler)
{
		//Cambiar el valor de la direccion
		ptrMotorHandler->configMotor.dir = (~ptrMotorHandler->configMotor.dir)&(0x01);
		//Cambiamos la direccion del motor
		GPIO_writePin(ptrMotorHandler->phandlerGPIOIN, ptrMotorHandler->configMotor.dir&SET);
		statusPolarityPWM(ptrMotorHandler->phandlerPWM, ptrMotorHandler->configMotor.dir&SET);
}

