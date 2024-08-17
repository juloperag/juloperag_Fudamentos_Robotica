/*
 * ExtiDriver.h
 *
 *  Created on: 23/03/2023
 *      Author: julian
 */

#ifndef EXTIDRIVER_H_
#define EXTIDRIVER_H_


#include <stm32f4xx.h>
#include <GPIOxDriver.h>

#define EXTERNAL_INTERRUPP_FALLING_EDGE 0
#define EXTERNAL_INTERRUPP_RISING_EDGE 1
#define EXTERNAL_INTERRUPP_RISING_FALLING_EDGE 2

//Macros que definen la prioridad del EXTI
enum
{
	e_EXTI_PRIOPITY_6 = 6,
	e_EXTI_PRIOPITY_7,
	e_EXTI_PRIOPITY_8,
	e_EXTI_PRIOPITY_9,
	e_EXTI_PRIOPITY_10,
	e_EXTI_PRIOPITY_11,
	e_EXTI_PRIOPITY_12,
	e_EXTI_PRIOPITY_13,
	e_EXTI_PRIOPITY_14,
	e_EXTI_PRIOPITY_15
};

/*Definimos una estructura que contine los registros del periferico EXTI y
 * la configuracion de la interrupcion como flaco de subida o bajada
 */

typedef struct
{
	GPIO_Handler_t *pGPIOHandler;	// Canal ADC que será utilizado para la conversión ADC
	uint8_t			edgeType;		// Se selecciona si se desea un tipo de flanco subiendo o bajando
}EXTI_Config_t;

//Definimos las cabeceras de las funciones a utilizar

void extInt_Config(EXTI_Config_t *extiConfig);
void exti_Config_Int_Priority(EXTI_Config_t *extiConfig, uint8_t newPriority);
void callback_extInt0(void);
void callback_extInt1(void);
void callback_extInt2(void);
void callback_extInt3(void);
void callback_extInt4(void);
void callback_extInt5(void);
void callback_extInt6(void);
void callback_extInt7(void);
void callback_extInt8(void);
void callback_extInt9(void);
void callback_extInt10(void);
void callback_extInt11(void);
void callback_extInt12(void);
void callback_extInt13(void);
void callback_extInt14(void);
void callback_extInt15(void);

#endif /* EXTIDRIVER_H_ */
