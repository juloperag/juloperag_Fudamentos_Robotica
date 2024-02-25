/**
 ******************************************************************************
 * @file           : GIOPxDriver.c
 * @author         : juloperag
 * @brief          : Driver GPIOx
 ******************************************************************************
 */

/* Se desarrolla el programa para que sea lo mas generico posible
 * para identificar el puerto GPIO y el pin seleccionado
 */

#include <GPIOxDriver.h>

//Configuracion de los registros para un pin
void GPIO_PIN_Config(GPIO_Handler_t *configuracion, uint8_t mode, uint8_t otyper, uint8_t ospeedr,uint8_t pupdr,uint8_t af)
{
	configuracion->GPIO_PinConfig.GPIO_PinModer       = mode; 		// = GPIO_MODE_x->IN-0, OUT-2, ALTFN-3, ANALOG-4
	configuracion->GPIO_PinConfig.GPIO_PinOTPype	  = otyper;		// = GPIO_OTYPER_x-> PUSHPULL-0, OPENDRAIN-1
	configuracion->GPIO_PinConfig.GPIO_PinSpeed	      = ospeedr;	// = GPIO_OSPEEDR_x-> LOW-0, MEDIUM-1, FAST-2, HIGH-3
	configuracion->GPIO_PinConfig.GPIO_PinPUPdControl = pupdr; 		// = GPIO_PUPDR_x -> NOTHING-0, PULLUP-1, PULLDOWN-2, RESERVED-3
	configuracion->GPIO_PinConfig.GPIO_PinAltFunMode  = af;			// = AFx, 0-15
}

//Activacion de la señal de reloj de un elemento en especifico

void GPIO_Config (GPIO_Handler_t *pGPIOHandler)
{
	//variables para hacer todo paso a paso
	uint32_t auxConfig = 0;
	uint32_t auxPosition = 0;

	//----------------------1) Activamos el periferico-----------------------------

	//Verificamos para GIOPA
	if (pGPIOHandler->pGPIOx == GPIOA)
	{
		//Escribimos 1 (SET) en la posicion correspondiente al GPIOA
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN);  //(SET << RCC_AHB1ENR_GPIOA_EN);
		/*Modificamos el registro AHB1ENR(32 bit)presente en periferico RCC, vease el 6.3.9 RCC_AHB1ENR---AHB1
		peripheral clock enable register*/
	}

	//Verificamos para GIOPB
	else if (pGPIOHandler->pGPIOx == GPIOB)
	{
		//Escribimos 1 (SET) en la posicion correspondiente al GPIOB
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN);  //(SET << RCC_AHB1ENR_GPIOB_EN);
		/*Modificamos el registro AHB1ENR(32 bit)presente en periferico RCC, vease el 6.3.9 RCC_AHB1ENR---AHB1
		peripheral clock enable register*/
	}

	//Verificamos para GIOPC
	else if (pGPIOHandler->pGPIOx == GPIOC)
	{
		//Escribimos 1 (SET) en la posicion correspondiente al GPIOC
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN);  //(SET << RCC_AHB1ENR_GPIOC_EN);
		/*Modificamos el registro AHB1ENR(32 bit)presente en periferico RCC, vease el 6.3.9 RCC_AHB1ENR---AHB1
		peripheral clock enable register*/
	}

	//Verificamos para GIOPD
	else if (pGPIOHandler->pGPIOx == GPIOD)
	{
		//Escribimos 1 (SET) en la posicion correspondiente al GPIOD
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN);  //(SET << RCC_AHB1ENR_GPIOD_EN);
		/*Modificamos el registro AHB1ENR(32 bit)presente en periferico RCC, vease el 6.3.9 RCC_AHB1ENR---AHB1
		peripheral clock enable register*/
	}

	//Verificamos para GIOPE
	else if (pGPIOHandler->pGPIOx == GPIOE)
	{
		//Escribimos 1 (SET) en la posicion correspondiente al GPIOE
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOEEN);  //(SET << RCC_AHB1ENR_GPIOE_EN);
		/*Modificamos el registro AHB1ENR(32 bit)presente en periferico RCC, vease el 6.3.9 RCC_AHB1ENR---AHB1
		peripheral clock enable register*/
	}

	//Verificamos para GIOPH
	else if (pGPIOHandler->pGPIOx == GPIOH)
	{
		//Escribimos 1 (SET) en la posicion correspondiente al GPIOH
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOHEN);  //(SET << RCC_AHB1ENR_GPIOH_EN);
		/*Modificamos el registro AHB1ENR(32 bit)presente en periferico RCC, vease el 6.3.9 RCC_AHB1ENR---AHB1
		peripheral clock enable register*/
	}

	//----------------2) Configurando el registro GPIOx_MODER----------------------
	//SE ENTIENDE

	//Leemos el PinNumber para mover una cantidad de veces  el pinModer
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinModer << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Cargamos auxConfig en el registro MODER
	pGPIOHandler->pGPIOx->MODER &= ~(0b11 << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->MODER |= auxConfig;

	//----------------3) Configurando el registro GPIOx_OTYPER----------------------
	//SE ENTIENDE

	//Leemos el PinNumber para mover una cantidad de veces el pinOTPype
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinOTPype << pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Cargamos  auxConfig en el registro OTYPER
	pGPIOHandler->pGPIOx->OTYPER &= ~(SET << pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->OTYPER |= auxConfig;

	//----------------4) Configurando el registro GPIOx_OSPEEDR----------------------
	//SE ENTIENDE

	//Leemos el PinNumber para mover una cantidad de veces el pinSpeed
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinSpeed << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Cargamos  auxConfig en el registro OSPEEDR
	pGPIOHandler->pGPIOx->OSPEEDR &= ~(0b11 << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->OSPEEDR |= auxConfig;

	//----------------5) Configurando el registro GPIOx_PUPDR----------------------
	//SE ENTIENDE

	//Leemos el PinNumber para mover una cantidad de veces el pinSpeed
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinPUPdControl << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Cargamos  auxConfig en el registro PUPDR
	pGPIOHandler->pGPIOx->PUPDR &= ~(0b11 << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
	pGPIOHandler->pGPIOx->PUPDR|= auxConfig;

	//---------------- MODER: Configurando funciones alternativas----------------------

	//si el Modo de configuracion escogido es GPIO_MODE_ALTFN, entonces activamos el AF
	if(pGPIOHandler->GPIO_PinConfig.GPIO_PinModer == GPIO_MODE_ALTFN)
	{
		//Para los pines 0 a 8 escogemos el AFRL
		if (pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber < 8)
		{
			auxPosition = 4*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber;

			//Cargamos auxPosition en el registro AFRL
			pGPIOHandler->pGPIOx->AFR[0] &= ~(0b1111 << auxPosition);
			pGPIOHandler->pGPIOx->AFR[0] |= (pGPIOHandler->GPIO_PinConfig.GPIO_PinAltFunMode << auxPosition);
		}

		//Para los pines 8 a 15 escogemos el AFRH
		else
		{
			auxPosition = 4*(pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber-8);

			//Cargamos auxPosition en el registro AFRL
			pGPIOHandler->pGPIOx->AFR[1] &= ~(0b1111 << auxPosition);
			pGPIOHandler->pGPIOx->AFR[1] |= (pGPIOHandler->GPIO_PinConfig.GPIO_PinAltFunMode << auxPosition);
		}
	}
}

//---------------- MODER: Configurando Output: Registro BSRR----------------------

void GPIO_writePin (GPIO_Handler_t *pPinHandler, uint8_t newState)
{
	//Limpiamos la posicion que deseamos
	//pPinHandler->pGPIOx->ODR &= ~(SET << pPinHandler->GPIO_PinConfig.GPIO_PinNumber);
	if (newState == SET)
	{
		//Trabajamos con la parte baja del registro
		pPinHandler->pGPIOx->BSRR |= (SET<<pPinHandler->GPIO_PinConfig.GPIO_PinNumber);
	}
	else
	{
		//Trabajamos con la parte alta del registro
		pPinHandler->pGPIOx->BSRR |= (SET<<(pPinHandler->GPIO_PinConfig.GPIO_PinNumber+16));
	}
}

//---------------- MODER: Lectura input----------------------

uint32_t GPIO_RedPin (GPIO_Handler_t *pPinHandler)
{
	//Creamos una variable auxiliar
	uint32_t pinValue = 0;

	/*Cargamos el valor del registro IDR y lo desplazamos a la posicion
	 * correspondiente
	 */
	pinValue = (pPinHandler->pGPIOx->IDR >> pPinHandler->GPIO_PinConfig.GPIO_PinNumber)&(0b1);

	return pinValue;
}


//---------------- Toogle ---------------------
void GPIOxTooglePin(GPIO_Handler_t *pPinHandler)
{
	pPinHandler->pGPIOx->ODR  ^= (SET << pPinHandler->GPIO_PinConfig.GPIO_PinNumber);
}


