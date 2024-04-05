/*
 * PLLDriver.c
 *
 *  Created on: 20/05/2023
 *      Author: julian
 */

#include <PLLDriver.h>

uint8_t auxValue = 0;

void configPLL(uint8_t clockSpeed)
{
	//Guardamos en una variable auxiliar la velocidad del reloj
	auxValue = clockSpeed;

	//---------------------------------0) Preescaler bus APB1------------------------------------------
	//Registro: CFGR

	/*El limite de la fuente de reloj para el bus APB1 es 50 Mhz, por tanto si la velocidad de reloj
	 * especificada es mayor a dicho se activa un preescaler de 4 para dicho bus
	*/
	if(clockSpeed<50)
	{
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;
	}
	else
	{
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
	}

	//---------------------------------1) Calibracion HSI------------------------------------------
	//Registro: CR

	//adjustHSI();

	//-------------2) Seleccion del HSI como la fuente de reloj para el PLL---------------
	//Registro: PLLCFGR
	RCC->PLLCFGR &= ~(0b1<<RCC_PLLCFGR_PLLSRC_Pos);

	//---------------------------2) Configuracion del PLL----------------------------------
	//Registro: PLLCFGR

	//----a) Modificacion del factor divisor M---------
	     //Señal de entrada al VCO entre 1 MHz y 2 MHz
	//De acuerdo al Manual de usuario se especifica un valor de 8 para que la señal de entrada al VCO sea de 2 Mhz; pero funciona bien con 1Mhz
	//Para el caso de 100Mhz, eston valores cambian para tener la presicion que se requiere
	RCC->PLLCFGR |= (18<<RCC_PLLCFGR_PLLM_Pos);  //16

	//----b) Modificacion del factor multiplicador N-----
         //Señal de salida al VCO entre 100 MHz y 438 MHz
		 //El valor de N sera entre de 50 y 100   ------> M=8
		 //El valor de N sera entre de 100 y 400  ------> M=16
	//De acuerdo al Manual de usuario y los calculos realizados el valor de N sera entre 100 y 400, para un valor de P de 4.
	//Para el caso de 100Mhz, eston valores cambian para tener la presicion que se requiere
	RCC->PLLCFGR |= ((clockSpeed*4-14)<<RCC_PLLCFGR_PLLN_Pos); //-14

	//----c) Modificacion del factor divisor P-----
	     //Señal de salida del PLL entre 25 MHz y 100 MHz
	RCC->PLLCFGR |= (0b01<<RCC_PLLCFGR_PLLP_Pos);   //El valor establecido es 4

	//---------------------------3) Activacion PLL----------------------------------
	//Registro: CR

	RCC->CR |= RCC_CR_PLLON;  //Escribimos un valor alto en el bit PLLON para su habilitacion

	//Espera de la activacion del PLL
	while(!(RCC->CR & RCC_CR_PLLRDY))
	{
		__NOP();
	}

	//---------------------------4) Valor de Latencia----------------------------------
	//Registro: ACR

	//Se define el valor de la latencia de acuerdo a la velocidad de reloj establecida
	if (90<clockSpeed && clockSpeed<=100)
	{
		FLASH->ACR |= FLASH_ACR_LATENCY_3WS;
	}
	else if (64<clockSpeed && clockSpeed<=90)
	{
		FLASH->ACR |= FLASH_ACR_LATENCY_2WS;
	}
	else if (30<clockSpeed && clockSpeed<=64)
	{
		FLASH->ACR |= FLASH_ACR_LATENCY_1WS;
	}
	else
	{
		FLASH->ACR |= FLASH_ACR_LATENCY_0WS;
	}

	//------------------5) Seleccion del PLL como la fuente de reloj del sistema----------------------------
	//Registro: CFGR

	RCC->CFGR |= (0b10<<RCC_CFGR_SW_Pos);

}

//Funcion para ajustar el HSI
void adjustHSI(void)
{
	//0b01100;
	uint8_t hsiTrimValue = 0b01100; //Variable de 5 bit usado para la calibracion deñ HSI

	RCC->CR &= ~(0b11111<<RCC_CR_HSITRIM_Pos);                //Limpiamos el campo
	RCC->CR |= hsiTrimValue<<RCC_CR_HSITRIM_Pos;            //Cargamos la calibracion
}

//Funcion que retorna la velocidad de reloj entregado por el PLL o por el HSI en MHz
uint8_t getConfigPLL(void)
{
	uint8_t clockSpeed = 0;
	//Verifica si el PLL esta activo
	if((RCC->CFGR & 0b11) == 0b10)
	{
		clockSpeed = auxValue;
	}
	else
	{
		clockSpeed = CLOCK_SPEED_16MHZ;
	}
	return clockSpeed;
}

//Funcion que retorna la velocidad de reloj entregada al bus APB1
uint8_t getClockAPB1(void)
{
	uint8_t clock = getConfigPLL();  	     //Obtenemos la velocidad de reloj del sistema
	uint8_t clockAPB1 = 0;                   //Variable que guarda la velocidad de reloj entrante al bus APB1

	//verificamos si el preescaler de 4 estara activado deacuerdo a la frecuencia max del bus
	if(clock<50)
	{
		clockAPB1 = clock;
	}
	else
	{
		clockAPB1 = clock/4;
	}
	return clockAPB1;
}

//Funcion que selecciona la señal de reloj saliente del pin MCO1
void configMCO2(uint8_t value)
{
	RCC->CFGR &= ~(0b11<<RCC_CFGR_MCO2_Pos);
	RCC->CFGR |= (value<<RCC_CFGR_MCO2_Pos);
}

//Funcion que selecciona la señal de reloj saliente del pin MCO1
void configMCO2PRE(uint8_t value)
{
	//Selecionamos el prescaler de acuerdo al valor definido
	switch (value){
		case 1:
		{
			RCC->CFGR &= ~(0b111<<RCC_CFGR_MCO2PRE_Pos);
			RCC->CFGR |= (0b000<<RCC_CFGR_MCO2PRE_Pos);
			break;
		}
		case 2:
		{
			RCC->CFGR &= ~(0b111<<RCC_CFGR_MCO2PRE_Pos);
			RCC->CFGR |= (0b100<<RCC_CFGR_MCO2PRE_Pos);
			break;
		}
		case 3:
		{
			RCC->CFGR &= ~(0b111<<RCC_CFGR_MCO2PRE_Pos);
			RCC->CFGR |= (0b101<<RCC_CFGR_MCO2PRE_Pos);
			break;
		}
		case 4:
		{
			RCC->CFGR &= ~(0b111<<RCC_CFGR_MCO2PRE_Pos);
			RCC->CFGR |= (0b110<<RCC_CFGR_MCO2PRE_Pos);
			break;
		}
		case 5:
		{
			RCC->CFGR &= ~(0b111<<RCC_CFGR_MCO2PRE_Pos);
			RCC->CFGR |= (0b111<<RCC_CFGR_MCO2PRE_Pos);
			break;
		}
		default:
		{
			__NOP();
			break;
		}
	}

}
