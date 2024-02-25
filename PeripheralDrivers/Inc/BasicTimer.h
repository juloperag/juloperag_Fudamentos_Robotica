/*
 * BasicTimer.h
 *
 *  Created on: 18/03/2023
 *      Author: julian
 */

#ifndef INC_BASICTIMER_H_
#define INC_BASICTIMER_H_

#include <stm32f4xx.h>

//Definimos Macros

#define BTIMER_MODE_UP			0
#define BTIMER_MODE_DOWN		1

#define BTIMER_PCNT_1us			1
#define BTIMER_PCNT_10us		10
#define	BTIMER_PCNT_100us		100
#define	BTIMER_PCNT_1ms			1000

#define INTERRUPTION_DISABLE   0
#define INTERRUPTION_ENABLE    1

//Estructura que contiene la configuracion minima para el manejo del Timerx
typedef struct
{
	uint8_t 	TIMx_mode;				//Up or dowm
	uint16_t	TIMx_periodcnt;			//valor en us del periodo del Clock Counter (min: 1, max:4096)
	uint32_t 	TIMX_period;			//Valor en ms(Depende) del periodo del timer o valor del autoreloar
	uint8_t		TIMx_interruptEnable;	//Activa o desactiva el modo interrupcion del timer
}BasicTimer_Config_t;

//Handler para el Timer
typedef struct
{
	BasicTimer_Config_t	TIMx_Config;
	TIM_TypeDef			*ptrTIMx;
}BasicTimer_Handler_t;

//Definimos las cabeceras de las funciones para el uso del timer
void BasicTimer_Config(BasicTimer_Handler_t *ptrBTimerHandler);
void BasicTimer2_Callback(void);
void BasicTimer3_Callback(void);
void BasicTimer4_Callback(void);
void BasicTimer5_Callback(void);
void startTimer(BasicTimer_Handler_t *ptrBTimerHandler);
void statusiInterruptionTimer(BasicTimer_Handler_t *ptrBTimerHandler, uint8_t status);
void updateFrequencyTimer(BasicTimer_Handler_t *ptrBTimerHandler, uint16_t newPer);

#endif /* INC_BASICTIMER_H_ */
