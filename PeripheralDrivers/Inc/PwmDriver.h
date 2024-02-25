/*
 * PwmDriver.h
 *
 *  Created on: 3/05/2023
 *      Author: julian
 */

#ifndef PWMDRIVER_H_
#define PWMDRIVER_H_

#include <stm32f4xx.h>

//Definicion de macros

#define PWM_CHANNEL_1   0
#define PWM_CHANNEL_2   1
#define PWM_CHANNEL_3   2
#define PWM_CHANNEL_4   3

#define PWM_DUTTY_0_PERCENT      0
#define PWM_DUTTY_100_PERCENT    100

#define POLARITY_HIGH   0
#define POLARITY_LOW    1

#define CHANNEL_DISABLE   0
#define CHANNEL_ENABLE    1


//Estructura que contiene la configuracion minima para el manejo del PWM
typedef struct
{
	uint8_t     channel;        //Canal PWM relacionado con el TIMER
	uint16_t	duttyCicle;		//Valor en tiempo de la señal en alto
	uint8_t     polarity;       //Define la polaridad de la señal PWM
}PWM_Config_t;

//Definicion del Handler del PWM
typedef struct
{
	PWM_Config_t 	config;   //Configuracion inicial del PWM
	TIM_TypeDef	  *ptrTIMx;   // TIMER al que esta asociado el PWM
}PWM_Handler_t;

//Definicion de las cabeceras de las funciones para el uso del PWM
void pwm_Config(PWM_Handler_t *prtPwmHandler);
//Dutty
void setDuttyCycle(PWM_Handler_t *prtPwmHandler);
void updateDuttyCycle(PWM_Handler_t *prtPwmHandler, uint16_t newDutty);
void updateDuttyCyclePercentage(PWM_Handler_t *prtPwmHandler, uint8_t newDuttyPercentage);
//Enable PWM
void statusInOutPWM(PWM_Handler_t *prtPwmHandler, uint8_t status);
//Polarity
void statusPolarityPWM(PWM_Handler_t *prtPwmHandler, uint8_t status);

#endif /* PWMDRIVER_H_ */
