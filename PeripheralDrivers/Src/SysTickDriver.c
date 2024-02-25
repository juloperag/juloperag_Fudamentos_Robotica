/*
 * SysTickDriver.c
 *
 *  Created on: 29/04/2023
 *      Author: julian
 */

#include <SysTickDriver.h>
#include <PLLDriver.h>

//Variables para el uso del Systick
uint64_t ticks = 0;
uint64_t ticks_start = 0;
uint64_t ticks_counting = 0;

//Funcion para la configurar del Systick
void config_SysTick_ms(void)
{
	//Reiniciamos el valor de la variable que cuenta el tiempo
	ticks = 0;

	/*Cargamos el valor del limite de incrementos que representa 1ms
	 * Depende de la señal de reloj interno del MCU
	 */

	uint8_t clock = getConfigPLL();  	     //Obtenemos la velocidad de reloj del sistema
	SysTick->LOAD = clock*1000;              //Cargamos el valor correspondiente a 1 ms
//	switch(systemClock)
//	{
//	//Caso para el reloj HSI -> 16Mhz
//	case 0:
//	{
//		SysTick->LOAD = SYSRICK_LOAD_VALUE_16MHz_1ms;
//		break;
//	}
//	//Caso para el reloj HSI -> 100Mhz
//	case 1:
//	{
//		SysTick->LOAD = SYSRICK_LOAD_VALUE_100MHz_1ms;
//		break;
//	}
//	//El caso por defecto sera de 16Mhz
//	default:
//	{
//		SysTick->LOAD = SYSRICK_LOAD_VALUE_16MHz_1ms;
//
//		break;
//	}
//	}

	//Limpiamos el valor actual del Systick
	SysTick->VAL = 0;

	//Configuramos el relog interno como el reloj para el timer
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	//Desactivamos las interrupciones globales

	__disable_irq();

	//Matriculamos la interrupcion en el NVIC
	NVIC_EnableIRQ(SysTick_IRQn);
	//Activamos la interrupcion debido al conteo a cero del SysTick
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	//Actimos el timer
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	//Activamos las interrupciones globales
	__enable_irq();

}

//Funcion que retorna el tiempo en ms transcurrido desde que que inicio el SysTick
uint64_t getTicksMs(void)
{
	return ticks;
}

//Funcion que genera un delay de ms
void delay_ms(uint32_t wait_time_ms)
{
	//Guardamos el valor de ticks transcurridos hasta el momento
	ticks_start = getTicksMs();
	//Guardamos tambien el valor de ticks transcurridos hasta el momento en una nueva variable
	ticks_counting = getTicksMs();

	//Realizamos un while que espera que se cumpla el tiempo asignado
	while(ticks_counting<(ticks_start+(uint64_t)wait_time_ms))
	{
		//Actualizamos el valor
		ticks_counting = getTicksMs();
	}
}

/* Cuando se produce una interrupcion en el NVIC debido SysTick, apuntara a esta
 * funcion en el vector de interrupciones
 */

//Funcion para la configurar del Systick
void config_SysTick_us(void)
{
	//Reiniciamos el valor de la variable que cuenta el tiempo
	ticks = 0;

	/*Cargamos el valor del limite de incrementos que representa 1us
	 * Depende de la señal de reloj interno del MCU
	 */

	uint8_t clock = getConfigPLL();  	     //Obtenemos la velocidad de reloj del sistema
	SysTick->LOAD = clock;                  //Cargamos el valor correspondiente a 1 us

	//Limpiamos el valor actual del Systick
	SysTick->VAL = 0;

	//Configuramos el relog interno como el reloj para el timer
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	//Desactivamos las interrupciones globales

	__disable_irq();

	//Matriculamos la interrupcion en el NVIC
	NVIC_EnableIRQ(SysTick_IRQn);
	//Activamos la interrupcion debido al conteo a cero del SysTick
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	//Actimos el timer
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	//Activamos las interrupciones globales
	__enable_irq();

}

//Funcion que retorna el tiempo en ms transcurrido desde que que inicio el SysTick
uint64_t getTicksUs(void)
{
	return ticks;
}

//Funcion que genera un delay de us
void delay_us(uint32_t wait_time_us)
{
	//Guardamos el valor de ticks transcurridos hasta el momento
	ticks_start = getTicksUs();
	//Guardamos tambien el valor de ticks transcurridos hasta el momento en una nueva variable
	ticks_counting = getTicksUs();

	//Realizamos un while que espera que se cumpla el tiempo asignado
	while(ticks_counting<(ticks_start+(uint64_t)wait_time_us))
	{
		//Actualizamos el valor
		ticks_counting = getTicksUs();
	}

}

/* Cuando se produce una interrupcion en el NVIC debido SysTick, apuntara a esta
 * funcion en el vector de interrupciones
 */
void SysTick_Handler(void)
{
	//Verificamos que la interrupcion se lance
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
	{
		//Limpiamos la bandera
		SysTick->CTRL &= ~ SysTick_CTRL_COUNTFLAG_Msk;

		//Incrementamos en 1 el contador
		ticks++;
	}
}
