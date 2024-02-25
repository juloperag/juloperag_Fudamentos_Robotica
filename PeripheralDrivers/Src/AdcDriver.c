/*
 * AdcDriver.c
 *
 *  Created on: 2/06/2023
 *      Author: julian
 */

#include <AdcDriver.h>
#include <GPIOxDriver.h>


GPIO_Handler_t handlerAdcPin = {0};   //Definimos una struct para para configurar el GPIO a usar
uint8_t channelSequence = 0;          //Definimos una variable para guardar el canal
uint16_t samplingPeriodChannel = 0;   //Definimos una variable para almacenar el tiempo de muestre
uint16_t	adcRawData = 0;           //Creamos una variable auxiliar

void adc_Config(ADC_Config_t *adcConfig)
{
	//---------------------------------1) Configuracion del Pin analogo------------------------------------------

	configAnalogPin(adcConfig->conversion.channelSequence_0);

	//---------------------------------2) Activamos la señal de reloj------------------------------------------
	//Registro: APB2ENR

	RCC->APB2ENR &= ~RCC_APB2ENR_ADC1EN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	//---------------------------------3) Limpiamos los registros------------------------------------------
	//Registro: CR1
	//Registro: CR2

	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	//----------------------------------4) Resolucion del ADC-----------------------------------------
	//Registro: CR1

	//Verificamos la resolucion escogida
	switch(adcConfig->resolution)
	{
		case ADC_RESOLUTION_12_BIT:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b00<<ADC_CR1_RES_Pos;

			break;
		}

		case ADC_RESOLUTION_10_BIT:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b01<<ADC_CR1_RES_Pos;
			break;
		}

		case ADC_RESOLUTION_8_BIT:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b10<<ADC_CR1_RES_Pos;

			break;
		}

		case ADC_RESOLUTION_6_BIT:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b11<<ADC_CR1_RES_Pos;
			break;
		}

		default:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b00<<ADC_CR1_RES_Pos;
			break;
		}
	}

	//---------------------------------5) Desactivamos el modo Scan -----------------------------------------
	//Registro: CR1

	ADC1->CR1 &= ~ADC_CR1_SCAN;

	//---------------------------------6) Alineacion de los datos----------------------------------------
	//Registro: CR2

	//Verificamos la alineacion escogida
	if(adcConfig->dataAlignment == ADC_ALIGNMENT_RIGHT)
	{
		ADC1->CR2 &= ~ADC_CR2_ALIGN;

	}
	else
	{
		ADC1->CR2 &= ~ADC_CR2_ALIGN;
		ADC1->CR2 |= ADC_CR2_ALIGN;
	}

	//---------------------------------7) Desactivamos el modo continuo---------------------------------------
	//Registro: CR2

	ADC1->CR2 &= ~ADC_CR2_CONT;

	//---------------------------------8) Definimos el Sampling---------------------------------------
	//Registro: SMPR2
	//Registro: SMPR1

	if(adcConfig->conversion.channelSequence_0<=ADC_CHANNEL_9)
	{
		ADC1->SMPR2 |= (adcConfig->sampling.samplingPeriodChannel_0<<(3*(adcConfig->conversion.channelSequence_0)));
	}
	else
	{
		ADC1->SMPR1 |= (adcConfig->sampling.samplingPeriodChannel_0<<(3*(adcConfig->conversion.channelSequence_0-10)));
	}

	//---------------------------------9) Configuracion elementos en la secuencias---------------------------------------
	//Registro: SQR1
	//Registro: SQR3

	ADC1->SQR1 = 0; //Definimos un solo elemento en la secuancia, impricitamente se especifica un solo elemento L
	ADC1->SQR3 |= (adcConfig->conversion.channelSequence_0 << 0); //Definimos que el canal seleccionado sea la primera conversion

	//---------------------------------10) Configuramos el preescaler del ADC---------------------------------------
	//Registro: CCR

	uint8_t clocKAPB2 = getConfigPLL();

	//Verificamos que la señal de reloj entrante al ADC no supere los 36MHz
	if(clocKAPB2/2<=36)
	{
		ADC->CCR &= ~(0b11<<ADC_CCR_ADCPRE_Pos);   //Preescaler	de 2
	}
	else
	{
		ADC->CCR &= ~(0b11<<ADC_CCR_ADCPRE_Pos);
		ADC->CCR |= (0b01<<ADC_CCR_ADCPRE_Pos);   //Preescaler	de 4
	}

	//---------------------------------11) Activamos las interruciones---------------------------------------
	//Registro: CR1

	//Desactivamos las interupciones globales
	__disable_irq();

	//Activamos la interupcion debido a finalizar una conversion
	ADC1->CR1 |= ADC_CR1_EOCIE;

	//Matriculamos la interrupcion en el NVIC
	NVIC_EnableIRQ(ADC_IRQn);

	//Configuramos la prioridad para la interrupción ADC
	NVIC_SetPriority(ADC_IRQn, 4);

	//Activamos las interupciones globales
	__enable_irq();

	//---------------------------------14) Configuracion de eventos externos---------------------------------------
	//Registro: CR2

	//Seleccionamos el tipo de evento externo
	ADC1->CR2 &= ~(0b1111<<ADC_CR2_EXTSEL_Pos);
	ADC1->CR2 |= (adcConfig->typeEXTEvent<<ADC_CR2_EXTSEL_Pos);

	//definimos como sera el evento externo
	ADC1->CR2 &= ~(0b11<<ADC_CR2_EXTEN_Pos);
	ADC1->CR2 |= (adcConfig->externalEvent<<ADC_CR2_EXTEN_Pos);

	//---------------------------------13) Activamos el modulo ADC---------------------------------------
	//Registro: CR2

	ADC1->CR2 |= ADC_CR2_ADON;
}


void  ADC_ConfigMultichannel (ADC_Config_t *adcConfig, uint8_t numeroDeCanales)
{

	uint8_t auxCounter = 0; //Variable auiliar para hacer el recorrido por los diferentes canales

	//---------------------------------1) Configuracion del Pin analogo------------------------------------------

	while(auxCounter<numeroDeCanales)
	{
		//Almacenamos en variables auxiliares el respectivo canal y tiempo de muestreo
		numberChannel(adcConfig, auxCounter);
		//Definimos el pin como analogo
		configAnalogPin(channelSequence);
		//Aumentamos en 1 la variable
		auxCounter++;
	}

	auxCounter=0; //Reiniciamos el contador

	//---------------------------------2) Activamos la señal de reloj------------------------------------------
	//Registro: APB2ENR

	RCC->APB2ENR &= ~RCC_APB2ENR_ADC1EN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	//---------------------------------3) Limpiamos los registros------------------------------------------
	//Registro: CR1
	//Registro: CR2

	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	//----------------------------------4) Resolucion del ADC-----------------------------------------
	//Registro: CR1

	//Verificamos la resolucion escogida
	switch(adcConfig->resolution)
	{
		case ADC_RESOLUTION_12_BIT:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b00<<ADC_CR1_RES_Pos;

			break;
		}

		case ADC_RESOLUTION_10_BIT:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b01<<ADC_CR1_RES_Pos;
			break;
		}

		case ADC_RESOLUTION_8_BIT:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b10<<ADC_CR1_RES_Pos;

			break;
		}

		case ADC_RESOLUTION_6_BIT:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b11<<ADC_CR1_RES_Pos;
			break;
		}

		default:
		{
			ADC1->CR1 &= ~(0b11<<ADC_CR1_RES_Pos);
			ADC1->CR1 |= 0b00<<ADC_CR1_RES_Pos;
			break;
		}
	}

	//---------------------------------5) Activamos el modo Scan -----------------------------------------
	//Registro: CR1

	ADC1->CR1 &= ~ADC_CR1_SCAN;
	ADC1->CR1 |= ADC_CR1_SCAN;

	//---------------------------------6) Alineacion de los datos----------------------------------------
	//Registro: CR2

	//Verificamos la alineacion escogida
	if(adcConfig->dataAlignment == ADC_ALIGNMENT_RIGHT)
	{
		ADC1->CR2 &= ~ADC_CR2_ALIGN;

	}
	else
	{
		ADC1->CR2 &= ~ADC_CR2_ALIGN;
		ADC1->CR2 |= ADC_CR2_ALIGN;
	}

	//---------------------------------7) Desactivamos el modo continuo---------------------------------------
	//Registro: CR2

	ADC1->CR2 &= ~ADC_CR2_CONT;

	//---------------------------------8) Definimos el Sampling---------------------------------------
	//Registro: SMPR2
	//Registro: SMPR1

	while(auxCounter<numeroDeCanales)
	{
		//Almacenamos en variables auxiliares el respectivo canal y tiempo de muestreo
		numberChannel(adcConfig, auxCounter);

		//Cargamos el valor del tiempo de meustreo
		if(channelSequence<=ADC_CHANNEL_9)
		{
			ADC1->SMPR2 |= (samplingPeriodChannel<<(3*(channelSequence)));
		}
		else
		{
			ADC1->SMPR1 |= (samplingPeriodChannel<<(3*(channelSequence-10)));
		}
		//Aumentamos en 1 la variable
		auxCounter++;
	}
	//Reiniciamos la variable
	auxCounter = 0;

	//---------------------------------9) Configuracion elementos en la secuencias---------------------------------------
	//Registro: SQR1
	//Registro: SQR3

	//Definimos el numero de canales a usar para el ADC
	ADC1->SQR1 |= (numeroDeCanales-1)<<ADC_SQR1_L_Pos;

	while(auxCounter<numeroDeCanales)
	{
		//Almacenamos en variables auxiliares el respectivo canal y tiempo de muestreo
		numberChannel(adcConfig, auxCounter);

		//Definimos la prioridad de la conversion
		if((auxCounter+1)<=6)
		{
			ADC1->SQR3 |= (channelSequence << (5*auxCounter));
		}
		else if ((auxCounter+1)>6 && (auxCounter+1)<=12)
		{
			ADC1->SQR2 |= (channelSequence << (5*(auxCounter-6)));
		}
		else
		{
			ADC1->SQR1 |= (channelSequence << (5*(auxCounter-12)));
		}
		//Aumentamos en 1 la variable
		auxCounter++;
	}
	//Reiniciamos la variable
	auxCounter = 0;

	//---------------------------------10) Configuramos el preescaler del ADC en 2:1---------------------------------------
	//Registro: CCR

	uint8_t clocKAPB2 = getConfigPLL();

	//Verificamos que la señal de reloj entrante al ADC no supere los 36MHz
	if(clocKAPB2/2<=36)
	{
		ADC->CCR &= ~(0b11<<ADC_CCR_ADCPRE_Pos);   //Preescaler	de 2
	}
	else
	{
		ADC->CCR &= ~(0b11<<ADC_CCR_ADCPRE_Pos);
		ADC->CCR |= (0b01<<ADC_CCR_ADCPRE_Pos);   //Preescaler	de 4
	}

	//---------------------------------11) Activamos las interruciones---------------------------------------
	//Registro: CR1
	//Registro: CR2

	//Desactivamos las interupciones globales
	__disable_irq();

	//Activamos la interupcion debido a finalizar completa de las conversiones
	ADC1->CR1 |= ADC_CR1_EOCIE;

	//Activamos la interupcion debido a finalizar de una conversion regular
	ADC1->CR2 |= ADC_CR2_EOCS;

	//Matriculamos la interrupcion en el NVIC
	NVIC_EnableIRQ(ADC_IRQn);

	//Configuramos la prioridad para la interrupción ADC
	NVIC_SetPriority(ADC_IRQn, 4);

	//Activamos las interupciones globales
	__enable_irq();

	//---------------------------------14) Configuracion de eventos externos---------------------------------------
	//Registro: CR2

	//Seleccionamos el tipo de evento externo
	ADC1->CR2 &= ~(0b1111<<ADC_CR2_EXTSEL_Pos);
	ADC1->CR2 |= (adcConfig->typeEXTEvent<<ADC_CR2_EXTSEL_Pos);

	//definimos como sera el evento externo
	ADC1->CR2 &= ~(0b11<<ADC_CR2_EXTEN_Pos);
	ADC1->CR2 |= (adcConfig->externalEvent<<ADC_CR2_EXTEN_Pos);

	//---------------------------------12) Activamos el modulo ADC---------------------------------------
	//Registro: CR2

	ADC1->CR2 |= ADC_CR2_ADON;

}


//Funcion para realizar solo una conversion ADC, El dato es leido en el Callback empleando getADC()
void startSingleADC(void)
{
	//Limpiamos el bit del overrun
	ADC1->CR1 &= ~ADC_CR1_OVRIE;
	// Iniciamos un ciclo de conversión ADC
	ADC1->CR2 |= ADC_CR2_SWSTART;
}


//Funcion que retorna el ultimo dato arquirido por el ADC
uint16_t getADC(void)
{
	//Retorna el ultimo valor del ADC
	return adcRawData;
}


//Función debil, que debe ser sobreescrita en el main.
__attribute__ ((weak)) void adcComplete_Callback(void){
	__NOP();
}

//ISR de la interrupción por conversión ADC
void ADC_IRQHandler(void)
{
	if(ADC1->SR & ADC_SR_EOC)
	{
		// Evaluamos que se dio la interrupción por conversión ADC
		adcRawData = ADC1->DR;
		//llamamos la funcion de la interupcion
		adcComplete_Callback();
	}
	else
	{
		__NOP();
	}
}


//--------------------------------Funciones especificas para el uso del ADC -------------------

//Función que configura que pin deseamos que funcione como canal ADC
void configAnalogPin(uint8_t adcChannel) {

	// Seleccionamos el canal y lo configuramos como análogo.
	switch (adcChannel) {

	case ADC_CHANNEL_0: {

		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_0;

		break;
	}
	case ADC_CHANNEL_1: {

		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_1;

		break;
	}

	case ADC_CHANNEL_2: {

		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_2;
		break;
	}

	case ADC_CHANNEL_3: {

		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_3;
		break;
	}

	case ADC_CHANNEL_4: {

		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_4;
		break;
	}

	case ADC_CHANNEL_5: {

		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_5;
		break;
	}
	case ADC_CHANNEL_6: {

		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_6;
		break;
	}
	case ADC_CHANNEL_7: {

		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_7;
		break;
	}
	case ADC_CHANNEL_8: {

		handlerAdcPin.pGPIOx 						= GPIOB;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_0;
		break;
	}
	case ADC_CHANNEL_9: {

		handlerAdcPin.pGPIOx 						= GPIOB;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_1;
		break;
	}
	case ADC_CHANNEL_10: {

		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_0;
		break;
	}
	case ADC_CHANNEL_11: {
		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_1;
		break;
	}
	case ADC_CHANNEL_12: {

		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_2;
		break;
	}
	case ADC_CHANNEL_13: {

		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_3;
		break;
	}
	case ADC_CHANNEL_14: {

		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_4;
		break;
	}
	case ADC_CHANNEL_15: {

		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_5;
		break;
	}
	default: {
		break;
	}
	}

	//Definimos el modo analogo para el pin definido
	handlerAdcPin.GPIO_PinConfig.GPIO_PinModer = GPIO_MODE_ANALOG;
	GPIO_Config(&handlerAdcPin);
}

//Funcion que almacena  en variables auxiliares el respectivo canal y tiempo de muestreo
void numberChannel(ADC_Config_t *adcConfig, uint8_t numberChannel)
{

	switch (numberChannel)
	{
	case 0:
	{
		channelSequence  = adcConfig->conversion.channelSequence_0;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_0;

		break;
	}
	case 1:
	{
		channelSequence  = adcConfig->conversion.channelSequence_1;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_1;

		break;
	}
	case 2:
	{
		channelSequence  = adcConfig->conversion.channelSequence_2;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_2;

		break;
	}
	case 3:
	{
		channelSequence  = adcConfig->conversion.channelSequence_3;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_3;

		break;
	}
	case 4:
	{
		channelSequence  = adcConfig->conversion.channelSequence_4;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_4;

		break;
	}
	case 5:
	{
		channelSequence  = adcConfig->conversion.channelSequence_5;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_5;

		break;
	}
	case 6:
	{
		channelSequence  = adcConfig->conversion.channelSequence_6;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_6;

		break;
	}
	case 7:
	{
		channelSequence  = adcConfig->conversion.channelSequence_7;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_7;

		break;
	}
	case 8:
	{
		channelSequence  = adcConfig->conversion.channelSequence_8;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_8;

		break;
	}
	case 9:
	{
		channelSequence  = adcConfig->conversion.channelSequence_9;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_9;

		break;
	}
	case 10:
	{
		channelSequence  = adcConfig->conversion.channelSequence_10;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_10;

		break;
	}
	case 11:
	{
		channelSequence  = adcConfig->conversion.channelSequence_11;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_11;

		break;
	}
	case 12:
	{
		channelSequence  = adcConfig->conversion.channelSequence_12;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_12;

		break;
	}
	case 13:
	{
		channelSequence  = adcConfig->conversion.channelSequence_13;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_13;

		break;
	}
	case 14:
	{
		channelSequence  = adcConfig->conversion.channelSequence_14;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_14;

		break;
	}
	case 15:
	{
		channelSequence  = adcConfig->conversion.channelSequence_15;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_15;

		break;
	}
	case 16:
	{
		channelSequence  = adcConfig->conversion.channelSequence_16;
		samplingPeriodChannel = adcConfig->sampling.samplingPeriodChannel_16;

		break;
	}

	default:
	{
		__NOP();
		break;
	}
	}
}


