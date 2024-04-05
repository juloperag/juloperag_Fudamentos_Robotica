/*
 * ExtiDriver.c
 *
 *  Created on: 23/03/2023
 *      Author: julian
 */

#include <ExtiDriver.h>
#include <GPIOxDriver.h>

//Configuracion EXTI
void extInt_Config(EXTI_Config_t *extiConfig) // *extiConfig = &handlerEXTI
{
	//----------------------1) Configuramos el PINx como entrada ------------------------
	//Definimos la configuracion EXTI para el pin selecionado
	GPIO_PIN_Config(extiConfig->pGPIOHandler, GPIO_MODE_IN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF0);

	//Cargamos la configuracion del PIN especifico
	GPIO_Config(extiConfig->pGPIOHandler);

	//-----------2) Configuracion de las lineas de conexion del EXTI-----------------------
	//Registro: APB1ENR  En el pefirefico RCC
	//Registro: EXTICR 	 En el periferico SYSCFG

	//-------a) Activamos la señal de reloj al SYSCFG-------------
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	//-------b)Asignamos el canal del EXTI que corresponde al pin a usa-------------
	//Deacuerdo al PIN_X y GPIO_X, seleccionamos la conexion del EXTIx
	switch(extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber)
	{

	//----------------Configuracion EXTI0-----------------
	case 0:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_0
		en el EXTICR*/
		SYSCFG->EXTICR[0] &= ~(0xF<<SYSCFG_EXTICR1_EXTI0_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI0_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI0_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI0_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI0_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI0_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI0_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI1-----------------
	case 1:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_1
		en el EXTICR*/
		SYSCFG->EXTICR[0] &= ~(0xF<<SYSCFG_EXTICR1_EXTI1_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI1_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI1_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI1_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI1_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI1_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI1_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI2-----------------
	case 2:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_2
		en el EXTICR*/
		SYSCFG->EXTICR[0] &= ~(0xF<<SYSCFG_EXTICR1_EXTI2_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI2_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI2_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI2_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI2_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI2_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI2_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI3-----------------
	case 3:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_3
		en el EXTICR*/
		SYSCFG->EXTICR[0] &= ~(0xF<<SYSCFG_EXTICR1_EXTI3_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI3_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI3_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI3_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI3_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI3_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI3_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI4-----------------
	case 4:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_4
		en el EXTICR*/
		SYSCFG->EXTICR[1] &= ~(0xF<<SYSCFG_EXTICR2_EXTI4_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI4_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI4_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI4_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI4_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI4_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI4_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI5-----------------
	case 5:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_5
		en el EXTICR*/
		SYSCFG->EXTICR[1] &= ~(0xF<<SYSCFG_EXTICR2_EXTI5_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI5_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI5_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI5_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI5_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI5_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI5_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI6-----------------
	case 6:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_6
		en el EXTICR*/
		SYSCFG->EXTICR[1] &= ~(0xF<<SYSCFG_EXTICR2_EXTI6_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI6_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI6_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI6_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI6_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI6_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI6_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI7-----------------
	case 7:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_7
		en el EXTICR*/
		SYSCFG->EXTICR[1] &= ~(0xF<<SYSCFG_EXTICR2_EXTI7_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI7_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI7_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI7_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI7_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI7_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI7_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI8-----------------
	case 8:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_8
		en el EXTICR*/
		SYSCFG->EXTICR[2] &= ~(0xF<<SYSCFG_EXTICR3_EXTI8_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI9------------------
	case 9:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_9
		en el EXTICR*/
		SYSCFG->EXTICR[2] &= ~(0xF<<SYSCFG_EXTICR3_EXTI9_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI9_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI9_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI9_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI9_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI9_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI9_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI10-----------------
	case 10:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_10
		en el EXTICR*/
		SYSCFG->EXTICR[2] &= ~(0xF<<SYSCFG_EXTICR3_EXTI10_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI10_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI10_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI10_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI10_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI10_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI10_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI11-----------------
	case 11:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_11
		en el EXTICR*/
		SYSCFG->EXTICR[2] &= ~(0xF<<SYSCFG_EXTICR3_EXTI11_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI11_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI11_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI11_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI11_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI11_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI11_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI12-----------------
	case 12:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_12
		en el EXTICR*/
		SYSCFG->EXTICR[3] &= ~(0xF<<SYSCFG_EXTICR4_EXTI12_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI12_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI12_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI12_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI12_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI12_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI12_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI13-----------------
	case 13:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_13
		en el EXTICR*/
		SYSCFG->EXTICR[3] &= ~(0xF<<SYSCFG_EXTICR4_EXTI13_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI13_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI13_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI13_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI13_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI13_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI13_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI14-----------------
	case 14:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_14
		en el EXTICR*/
		SYSCFG->EXTICR[3] &= ~(0xF<<SYSCFG_EXTICR4_EXTI14_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI14_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI14_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI14_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI14_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI14_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI14_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	//----------------Configuracion EXTI15-----------------
	case 15:
	{
		/*Limpiamos la posicion correspondiente a la configuracion del PIN_15
		en el EXTICR*/
		SYSCFG->EXTICR[3] &= ~(0xF<<SYSCFG_EXTICR4_EXTI15_Pos);

		//Seleccionamos el valor a cargar deacuerdo al puerto establecido
		if(extiConfig->pGPIOHandler->pGPIOx == GPIOA)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI15_PA);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOB)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI15_PB);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOC)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI15_PC);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOD)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI15_PD);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOE)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI15_PE);
		}
		else if(extiConfig->pGPIOHandler->pGPIOx == GPIOH)
		{
			SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI15_PH);
		}
		else
		{
			__NOP();
		}
		break;
	}

	default: {
		__NOP();
		break;
	}
	} //FIN del switch-case

	//-----------------------3)Configuracion de EXTIx--------------------------
	//Registro: EXTI_RTSR		 Registros donde los primeros 16 bit corresponden a los
	//Registro: EXTI_FTSR        16 EXTIx
	//Registro: EXTI_IMR

	//-------a)Selecionamos el tipo de flanco---------
	if(extiConfig->edgeType == EXTERNAL_INTERRUPP_RISING_EDGE)
	{
		EXTI->FTSR &= ~(0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
		EXTI->RTSR &= ~(0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
		EXTI->RTSR |= (0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
	}
	else if (extiConfig->edgeType == EXTERNAL_INTERRUPP_FALLING_EDGE)
	{
		EXTI->FTSR &= ~(0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
		EXTI->RTSR &= ~(0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
		EXTI->FTSR |= (0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	}
	else
	{
		EXTI->FTSR &= ~(0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
		EXTI->RTSR &= ~(0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
		EXTI->FTSR |= (0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
		EXTI->RTSR |= (0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
	}
	//-------b)Activamos la interrupcion del EXTIx---------
	EXTI->IMR &= ~(0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);
	EXTI->IMR |= (0b1<<extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//------------4)Activamos el canal del sistema NVIC para indicar la interrupcion--------------------------

	//4.a Desabilitamos las interrupciones globales
	__disable_irq();
	//4.b Activamos el canal del NVIC para la interrupcion del EXTIx seleccionado
	switch(extiConfig->pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber)
	{

	case 0:
	{
		NVIC_EnableIRQ(EXTI0_IRQn);
		break;
	}

	case 1:
	{
		NVIC_EnableIRQ(EXTI1_IRQn);
		break;
	}

	case 2:
	{
		NVIC_EnableIRQ(EXTI2_IRQn);
		break;
	}

	case 3:
	{
		NVIC_EnableIRQ(EXTI3_IRQn);
		break;
	}

	case 4:
	{
		NVIC_EnableIRQ(EXTI4_IRQn);
		break;
	}

	case 5:
	{
		NVIC_EnableIRQ(EXTI9_5_IRQn);
		break;
	}

	case 6:
	{
		NVIC_EnableIRQ(EXTI9_5_IRQn);
		break;
	}

	case 7:
	{
		NVIC_EnableIRQ(EXTI9_5_IRQn);
		break;
	}

	case 8:
	{
		NVIC_EnableIRQ(EXTI9_5_IRQn);
		break;
	}

	case 9:
	{
		NVIC_EnableIRQ(EXTI9_5_IRQn);
		break;
	}

	case 10:
	{
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		break;
	}

	case 11:
	{
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		break;
	}

	case 12:
	{
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		break;
	}

	case 13:
	{
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		break;
	}

	case 14:
	{
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		break;
	}

	case 15:
	{
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		break;
	}

	default: {
		break;
	}
	}
	//4.c Activamos las interrupciones globales
	__enable_irq();
}



//Definimos las funciones para cuando se genera una interrupcion del EXTIx, 0-15
__attribute__ ((weak)) void callback_extInt0(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt1(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt2(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt3(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt4(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt5(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt6(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt7(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt8(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt9(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt10(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt11(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt12(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt13(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt14(void){
	__NOP();
}

__attribute__ ((weak)) void callback_extInt15(void){
	__NOP();
}


/* Cuando se produce una interrupcion en el NVIC debido a uno de los EXTI, apuntara a una de
 * estas funciones en el vector de interrupciones respectivamente
 */
//Registro: PR    Registro donde los primeros 16 bit corresponden a los 16 EXTIx

//--------------a) EXTI0, EXTI1, EXTI2, EXTI3, EXTI4 --------------
void EXTI0_IRQHandler(void)
{
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI0
	if (EXTI->PR & EXTI_PR_PR0)
	{
		EXTI->PR |= (EXTI_PR_PR0); //Limpiamos la bandera
		callback_extInt0();			//Ejecutamos la ISR
	}
	else
	{
		__NOP();
	}
}

void EXTI1_IRQHandler(void)
{
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI1
	if (EXTI->PR & EXTI_PR_PR1)
	{
		EXTI->PR |= (EXTI_PR_PR1); //Limpiamos la bandera
		callback_extInt1();			//Ejecutamos la ISR
	}
	else
	{
		__NOP();
	}
}

void EXTI2_IRQHandler(void)
{
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI2
	if (EXTI->PR & EXTI_PR_PR2)
	{
		EXTI->PR |= (EXTI_PR_PR2); //Limpiamos la bandera
		callback_extInt2();			//Ejecutamos la ISR
	}
	else
	{
		__NOP();
	}
}

void EXTI3_IRQHandler(void)
{
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI3
	if (EXTI->PR & EXTI_PR_PR3)
	{
		EXTI->PR |= (EXTI_PR_PR3); //Limpiamos la bandera
		callback_extInt3();			//Ejecutamos la ISR
	}
	else
	{
		__NOP();
	}
}

void EXTI4_IRQHandler(void)
{
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI4
	if (EXTI->PR & EXTI_PR_PR4)
	{
		EXTI->PR |= (EXTI_PR_PR4); //Limpiamos la bandera
		callback_extInt4();			//Ejecutamos la ISR
	}
	else
	{
		__NOP();
	}
}

//--------------b) EXTI9-5--------------
void EXTI9_5_IRQHandler(void)
{
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI5
	if (EXTI->PR & EXTI_PR_PR5)
	{
		EXTI->PR |= (EXTI_PR_PR5); //Limpiamos la bandera
		callback_extInt5();			//Ejecutamos la ISR
	}
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI6
	else if(EXTI->PR & EXTI_PR_PR6)
	{
		EXTI->PR |= (EXTI_PR_PR6); //Limpiamos la bandera
		callback_extInt6();			//Ejecutamos la ISR
	}
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI7
	else if(EXTI->PR & EXTI_PR_PR7)
	{
		EXTI->PR |= (EXTI_PR_PR7); //Limpiamos la bandera
		callback_extInt7();			//Ejecutamos la ISR
	}
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI8
	else if(EXTI->PR & EXTI_PR_PR8)
	{
		EXTI->PR |= (EXTI_PR_PR8); //Limpiamos la bandera
		callback_extInt8();			//Ejecutamos la ISR
	}
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI9
	else if(EXTI->PR & EXTI_PR_PR9)
	{
		EXTI->PR |= (EXTI_PR_PR9); //Limpiamos la bandera
		callback_extInt9();			//Ejecutamos la ISR
	}
	else
	{
		__NOP();
	}
}

//--------------b) EXTI15-10--------------
void EXTI15_10_IRQHandler(void)
{
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI10
	if (EXTI->PR & EXTI_PR_PR10)
	{
		EXTI->PR |= (EXTI_PR_PR10); //Limpiamos la bandera
		callback_extInt10();			//Ejecutamos la ISR
	}
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI11
	else if(EXTI->PR & EXTI_PR_PR11)
	{
		EXTI->PR |= (EXTI_PR_PR11); //Limpiamos la bandera
		callback_extInt11();			//Ejecutamos la ISR
	}
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI12
	else if(EXTI->PR & EXTI_PR_PR12)
	{
		EXTI->PR |= (EXTI_PR_PR12); //Limpiamos la bandera
		callback_extInt12();			//Ejecutamos la ISR
	}
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI13
	else if(EXTI->PR & EXTI_PR_PR13)
	{
		EXTI->PR |= (EXTI_PR_PR13); //Limpiamos la bandera
		callback_extInt13();			//Ejecutamos la ISR
	}
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI14
	else if(EXTI->PR & EXTI_PR_PR14)
	{
		EXTI->PR |= (EXTI_PR_PR14); //Limpiamos la bandera
		callback_extInt14();			//Ejecutamos la ISR
	}
	//Evaluamos si la interrupcion que se lanza corresponde al EXTI15
	else if(EXTI->PR & EXTI_PR_PR15)
	{
		EXTI->PR |= (EXTI_PR_PR15); //Limpiamos la bandera
		callback_extInt15();			//Ejecutamos la ISR
	}
	else
	{
		__NOP();
	}
}

