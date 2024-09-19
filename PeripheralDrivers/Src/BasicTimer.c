
 /*
 *  Created on: 18/03/2023
 *      Author: julian
 */

#include <BasicTimer.h>
#include <PLLDriver.h>

//Variable que guarda la referencia del periferico a utilizar
TIM_TypeDef *ptrTimer2Used;
TIM_TypeDef *ptrTimer3Used;
TIM_TypeDef *ptrTimer4Used;
TIM_TypeDef *ptrTimer5Used;

//Funcion para cargar la configuracion del Timer
void BasicTimer_Config(BasicTimer_Handler_t *ptrBTimerHandler)
{

	//---------------------------------1) Activamos el periferico------------------------------------------
	//Registro: APB1ENR

	//Verificamos para TIM2
	if(ptrBTimerHandler->ptrTIMx==TIM2)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

		//Guardamos una referencia al periferico que estamos utilizando
		ptrTimer2Used = ptrBTimerHandler->ptrTIMx;

	}
	//Verificamos para TIM3
	else if(ptrBTimerHandler->ptrTIMx==TIM3)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

		//Guardamos una referencia al periferico que estamos utilizando
		ptrTimer3Used = ptrBTimerHandler->ptrTIMx;
	}
	//Verificamos para TIM4
	else if(ptrBTimerHandler->ptrTIMx==TIM4)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

		//Guardamos una referencia al periferico que estamos utilizando
		ptrTimer4Used = ptrBTimerHandler->ptrTIMx;
	}
	//Verificamos para TIM5
	else if(ptrBTimerHandler->ptrTIMx==TIM5)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;

		//Guardamos una referencia al periferico que estamos utilizando
		ptrTimer5Used = ptrBTimerHandler->ptrTIMx;
	}
	//------------------------------2) Configurando el pre-escaler-----------------------------------------
	//Registro:TIMx_PSC		//Es un valor de 32 bit

	uint8_t clockSystem = getConfigPLL();          //Guardamos la velocidad de reloj entregada al bus APB1
	uint8_t clock = getClockAPB1();          //Guardamos la velocidad de reloj entregada al bus APB1

	//Verificamos si el multiplicador del Timer esta activado por el preescaler
	if(clockSystem>=50)
	{
		clock = clock*2;
	}
	else
	{
		__NOP();
	}

	/*La frecuencia de reloj contador CK_CNT es igual a fck_psc/(psc[15:0]+1)
	 * por tanto define la velocidad a la que incrementa el counter*/
	ptrBTimerHandler->ptrTIMx->PSC = (clock)*(ptrBTimerHandler->TIMx_Config.TIMx_periodcnt)-1; //(min:0, max:65536)

	//----------------------3) Configurando de la direccion del counter(up/down)---------------------------
	//Registro:TIMx_CR1		Es un registro de configuracion del TIMx
	//Registro:TIMx_ARR		Es un valor de 32 bit
	//Registro:TIMx_CNT/	Es un valor de 32 bit

	//verificamos si el timer se configuro como up o dowm
	if(ptrBTimerHandler->TIMx_Config.TIMx_mode==BTIMER_MODE_UP)
	{
		//-------a)Definimos la direccion para el conteo-------------
		ptrBTimerHandler->ptrTIMx->CR1 &= ~(0b1<<4);
		//-------b)Configuracion del Auto-Reload---------------------
		ptrBTimerHandler->ptrTIMx->ARR = ptrBTimerHandler->TIMx_Config.TIMX_period+1;
		//-------c)Reinicio del registro counter----------------------
		ptrBTimerHandler->ptrTIMx->CNT = 0;
	}
	else
	{
		//-------a)Definimos la direccion para el conteo-------------
		ptrBTimerHandler->ptrTIMx->CR1 &= ~(0b1<<4); //limpiamos
		ptrBTimerHandler->ptrTIMx->CR1 |= (0b1<<4);
		//-------b)Configuracion del Auto-Reload---------------------
		ptrBTimerHandler->ptrTIMx->ARR = 0;
		//-------c)Reinicio del registro counter----------------------
		ptrBTimerHandler->ptrTIMx->CNT = ptrBTimerHandler->TIMx_Config.TIMX_period;
	}


	//----------------------4) Matriculamos la interrupcion generada por el timer---------------------------------
	//Registro:TIMx_DIER

	//Desactivamos las interrupciones Globales
	__disable_irq();

	//Matriculamos la interrupcion en el NVCI
	if(ptrBTimerHandler->ptrTIMx==TIM2)
	{
		//Activamos el NVIC para la interrupcion del TIM2
		NVIC_EnableIRQ(TIM2_IRQn);
	}
	else if(ptrBTimerHandler->ptrTIMx==TIM3)
	{
		//Activamos el NVIC para la interrupcion del TIM3
		NVIC_EnableIRQ(TIM3_IRQn);
	}
	else if(ptrBTimerHandler->ptrTIMx==TIM4)
	{
		//Activamos el NVIC para la interrupcion del TIM4
		NVIC_EnableIRQ(TIM4_IRQn);
	}
	else if(ptrBTimerHandler->ptrTIMx==TIM5)
	{
		//Activamos el NVIC para la interrupcion del TIM5
		NVIC_EnableIRQ(TIM5_IRQn);
	}

	//Activo las interrupciones Globales
	__enable_irq();

	//----------------------5)Definimos el estado de la interrupcion---------------------------------

	statusiInterruptionTimer(ptrBTimerHandler, (ptrBTimerHandler->TIMx_Config.TIMx_interruptEnable));

	//----------------------6) Activamos el Timer---------------------------------
	//Registro:TIMx_CR1

	ptrBTimerHandler->ptrTIMx->CR1 &= ~TIM_CR1_CEN;
	ptrBTimerHandler->ptrTIMx->CR1 |= TIM_CR1_CEN;
}

//------------Funcion para configurar las interrupciones------------------
void timer_Config_Init_Priority(BasicTimer_Handler_t *ptrBTimerHandler, uint8_t newPriority)
{
	//Desactivamos las interupciones globales
	__disable_irq();
	//Matriculamos la interrupcion en el NVIC
	if(ptrBTimerHandler->ptrTIMx == TIM2)
	{
		NVIC_SetPriority(TIM2_IRQn, newPriority);
	}
	else if(ptrBTimerHandler->ptrTIMx == TIM3)
	{
		NVIC_SetPriority(TIM3_IRQn, newPriority);
	}
	else if(ptrBTimerHandler->ptrTIMx == TIM4)
	{
		NVIC_SetPriority(TIM4_IRQn, newPriority);
	}
	else if(ptrBTimerHandler->ptrTIMx == TIM5)
	{
		NVIC_SetPriority(TIM5_IRQn, newPriority);
	}
	//Activamos las interupciones globales
	__enable_irq();
}

//Definimos las funciones para cuando se genera una interrupcion del TIM2-3
__attribute__((weak)) void BasicTimer2_Callback(void)
{
	__NOP();
}

__attribute__((weak)) void BasicTimer3_Callback(void)
{
	__NOP();
}

__attribute__((weak)) void BasicTimer4_Callback(void)
{
	__NOP();
}

__attribute__((weak)) void BasicTimer5_Callback(void)
{
	__NOP();
}


/* Cuando se produce una interrupcion en el NVIC debido a uno de los TIMER apuntara a una de
 * estas funciones en el vector de interrupciones respectivamente
 */
void TIM2_IRQHandler(void)
{
	//Registro:TIMx_SR    Es un registro de almacenamiento del TIMx
	//limpiamos la bandera que indica que la interrupcion se a generado
	ptrTimer2Used->SR &= ~TIM_SR_UIF;

	//Ejecute la funcion correspondiente a la interupccion
	BasicTimer2_Callback();

}

void TIM3_IRQHandler(void)
{
	//Registro:TIMx_SR    Es un registro de almacenamiento del TIMx
	//limpiamos la bandera que indica que la interrupcion se a generado
	ptrTimer3Used->SR &= ~TIM_SR_UIF;

	//Ejecute la funcion correspondiente a la interupccion
	BasicTimer3_Callback();

}

void TIM4_IRQHandler(void)
{
	//Registro:TIMx_SR    Es un registro de almacenamiento del TIMx
	//limpiamos la bandera que indica que la interrupcion se a generado
	ptrTimer4Used->SR &= ~TIM_SR_UIF;

	//Ejecute la funcion correspondiente a la interupccion
	BasicTimer4_Callback();

}

void TIM5_IRQHandler(void)
{
	//Registro:TIMx_SR    Es un registro de almacenamiento del TIMx
	//limpiamos la bandera que indica que la interrupcion se a generado
	ptrTimer5Used->SR &= ~TIM_SR_UIF;

	//Ejecute la funcion correspondiente a la interupccion
	BasicTimer5_Callback();

}


//Definir la interrupcion por el timer
void statusiInterruptionTimer(BasicTimer_Handler_t *ptrBTimerHandler, uint8_t status)
{
	//Verificar el estado que se desea con definir
	if(status == INTERRUPTION_ENABLE)
	{
		//Activamos las interrupciones
		ptrBTimerHandler->ptrTIMx->DIER |=TIM_DIER_UIE;
	}
	else
	{
		//Desactivamos las interrupciones
		ptrBTimerHandler->ptrTIMx->DIER &= ~TIM_DIER_UIE;
	}
}


//Actualizamos la frecuencia del TIMER
void updateFrequencyTimer(BasicTimer_Handler_t *ptrBTimerHandler, uint16_t newPer)
{
	//Establecemos el nuevo valor del periodo en la configuracion del PWM
	ptrBTimerHandler->TIMx_Config.TIMX_period = newPer;
	//Reiniamos el contador
	ptrBTimerHandler->ptrTIMx->CNT = 0;
	/*Cargamos el valor del ARR el cual es e limite de incrementos del TIMER
	 */
	ptrBTimerHandler->ptrTIMx->ARR = ptrBTimerHandler->TIMx_Config.TIMX_period;
}

//Funcion para realizar un delay con un timer
void timer_delay(BasicTimer_Handler_t *ptrBTimerHandler,uint16_t *countingTimer, uint16_t maxCountinegTimer)
{
	//Aseguramos que el valor de las cuestas se reinicio
	*countingTimer = 0;
	//Activamos la interrupcion del Timer
	statusiInterruptionTimer(ptrBTimerHandler, SET);
	//El programa se queda aqui hasta que se cumple la condiccion
	while(*countingTimer<maxCountinegTimer)
	{
		__NOP();
	}
	//Desactivamos la interrupcion del Timer
	statusiInterruptionTimer(ptrBTimerHandler, RESET);
}

