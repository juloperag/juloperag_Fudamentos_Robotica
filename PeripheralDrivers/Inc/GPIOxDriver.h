/*
 * Hal.h
 *
 *  Created on: Mar 2, 2023
 *      Author: juloperag
 */

/*Describe una estructura que nos permite configurar el PINx, ademas tambien la creacion de una estructura
 *Hlader que contiene la configuracion del usuario y el apuntador al periferico.
 */

#ifndef INC_GIOXDRIVER_H_
#define INC_GIOXDRIVER_H_

#include <stm32f4xx.h>

/*===Valores estandar para las configuraciones===*/

/*8.4.1 GPIOx_MODER(dos bit por PIN)*/
#define GPIO_MODE_IN    	0
#define GPIO_MODE_OUT    	1
#define GPIO_MODE_ALTFN    	2
#define GPIO_MODE_ANALOG   	3

/*8.4.2 GPIOx_OTYPER(un bit por PIN)*/
#define GPIO_OTYPER_PUSHPULL    	0
#define GPIO_OTYPER_OPENDRAIN   	1

/*8.4.3 GPIOx_OSPEEDR(dos bit por PIN)*/
#define GPIO_OSPEEDR_LOW    		0
#define GPIO_OSPEEDR_MEDIUM   		1
#define GPIO_OSPEEDR_FAST    		2
#define GPIO_OSPEEDR_HIGH  			3

/*8.4.4 GPIOx_PUPDR(dos bit por PIN)*/
#define GPIO_PUPDR_NOTHING    		0
#define GPIO_PUPDR_PULLUP  			1
#define GPIO_PUPDR_PULLDOWN    		2
#define GPIO_PUPDR_RESERVED  		3

/*8.4.5 GPIOx_IDR(un bit por PIN) - este es el registro para leer el estado de un PIN*/

/*8.4.6 GPIOx_ODR(un bit por PIN) - este es el registro para escribir el estado de un PIN,
 * Aunque es preferible utilizar el registro BRSS
  */

/* Definicion de los nombres de los pines*/
#define PIN_0					0
#define PIN_1					1
#define PIN_2					2
#define PIN_3					3
#define PIN_4					4
#define PIN_5					5
#define PIN_6					6
#define PIN_7					7
#define PIN_8					8
#define PIN_9					9
#define PIN_10					10
#define PIN_11					11
#define PIN_12					12
#define PIN_13					13
#define PIN_14					14
#define PIN_15					15

/* Definicion de funciones alternas*/
#define AF0				0b0000
#define AF1				0b0001
#define AF2				0b0010
#define AF3				0b0011
#define AF4				0b0100
#define AF5				0b0101
#define AF6				0b0110
#define AF7				0b0111
#define AF8				0b1000
#define AF9				0b1001
#define AF10			0b1010
#define AF11			0b1011
#define AF12			0b1100
#define AF13			0b1101
#define AF14			0b1110
#define AF15			0b1111


//Definimos una estructura de datos que contienen las variables para la configuracion del PIN
typedef struct
{
	uint8_t GPIO_PinNumber;			// Pin con el que se desea trabajar
	uint8_t GPIO_PinModer;			// Modo de la  configuracion
	uint8_t	GPIO_PinOTPype;			// Selecionados PPD o OpenDrain
	uint8_t GPIO_PinSpeed;  		// Velocidad de  puesta del pin(solo salida)
	uint8_t GPIO_PinPUPdControl;	// Seleccionar si se desea una salida Pull-up, Pull-down o libre
	uint8_t	GPIO_PinAltFunMode;		// Selecionados cual es la funcion alternativa que desea seleccionar

}GPIO_PinConfig_t;


typedef struct
{
	/*Renombramos la estructura para usarla con el fin de configurar el puerto
	 */
	GPIO_PinConfig_t GPIO_PinConfig;

	/*Renombramos la estructura para usarla con el fin de definir la direccion al puerto que el pin corresponde
	 */
	GPIO_TypeDef *pGPIOx;

}GPIO_Handler_t;

/* Definicion de las cabeceras de las funciones del GPIOxDriver */

void GPIO_PIN_Config(GPIO_Handler_t *configuracion,uint8_t mode, uint8_t otyper, uint8_t ospeedr,uint8_t pupdr, uint8_t af);
void GPIO_Config (GPIO_Handler_t *pGPIOHandler);
void GPIO_writePin (GPIO_Handler_t *pPinHandler, uint8_t newState);
uint32_t GPIO_RedPin (GPIO_Handler_t *pPinHandler);
void GPIOxTooglePin(GPIO_Handler_t *pPinHandler);

#endif



