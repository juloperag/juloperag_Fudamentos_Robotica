/*
 * AdcDriver.h
 *
 *  Created on: 2/06/2023
 *      Author: julian
 */

#ifndef ADCDRIVER_H_
#define ADCDRIVER_H_

#include <stm32f4xx.h>
#include <PLLDriver.h>

//Definicion de macros
#define ADC_CHANNEL_0		0
#define ADC_CHANNEL_1		1
#define ADC_CHANNEL_2		2
#define ADC_CHANNEL_3		3
#define ADC_CHANNEL_4		4
#define ADC_CHANNEL_5		5
#define ADC_CHANNEL_6		6
#define ADC_CHANNEL_7		7
#define ADC_CHANNEL_8		8
#define ADC_CHANNEL_9		9
#define ADC_CHANNEL_10		10
#define ADC_CHANNEL_11		11
#define ADC_CHANNEL_12		12
#define ADC_CHANNEL_13		13
#define ADC_CHANNEL_14		14
#define ADC_CHANNEL_15		15
#define ADC_CHANNEL_16		16

#define ADC_RESOLUTION_12_BIT	0
#define ADC_RESOLUTION_10_BIT	1
#define ADC_RESOLUTION_8_BIT	2
#define ADC_RESOLUTION_6_BIT	3

#define ADC_ALIGNMENT_RIGHT		0
#define ADC_ALIGNMENT_LEFT		1

#define ADC_EXTERNAL_EVENT_DISABLE    	    0
#define ADC_EXTERNAL_EVENT_RISING     	    1
#define ADC_EXTERNAL_EVENT_FALLING		    2
#define ADC_EXTERNAL_EVENT_BOTH  		    3

#define ADC_EVENT_TIMER_1_CC1    		    0
#define ADC_EVENT_TIMER_1_CC2    		    1
#define ADC_EVENT_TIMER_1_CC3    		    2
#define ADC_EVENT_TIMER_2_CC2    		    3
#define ADC_EVENT_TIMER_2_CC3    		    4
#define ADC_EVENT_TIMER_1_CC4    		    5
#define ADC_EVENT_TIMER_2_TRGO    		    6
#define ADC_EVENT_TIMER_3_CC1    		    7
#define ADC_EVENT_TIMER_3_TRGO   		    8
#define ADC_EVENT_TIMER_4_CC4    		    9
#define ADC_EVENT_TIMER_5_CC1    		    10
#define ADC_EVENT_TIMER_5_CC2    		    11
#define ADC_EVENT_TIMER_5_CC3    		    12
#define ADC_EVENT_EXTI_LINE11    		    15

#define ADC_SAMPLING_PERIOD_3_CYCLES	0b000;
#define ADC_SAMPLING_PERIOD_15_CYCLES	0b001;
#define ADC_SAMPLING_PERIOD_28_CYCLES	0b010;
#define ADC_SAMPLING_PERIOD_56_CYCLES	0b011;
#define ADC_SAMPLING_PERIOD_84_CYCLES	0b100;
#define ADC_SAMPLING_PERIOD_112_CYCLES	0b101;
#define ADC_SAMPLING_PERIOD_144_CYCLES	0b110;
#define ADC_SAMPLING_PERIOD_480_CYCLES	0b111;


//Estructura que especifica el canal ADC y su respectivo orden de secuencia de conversion
typedef struct
{
	uint8_t		channelSequence_0;
	uint8_t		channelSequence_1;
	uint8_t		channelSequence_2;
	uint8_t		channelSequence_3;
	uint8_t		channelSequence_4;
	uint8_t		channelSequence_5;
	uint8_t		channelSequence_6;
	uint8_t		channelSequence_7;
	uint8_t		channelSequence_8;
	uint8_t		channelSequence_9;
	uint8_t		channelSequence_10;
	uint8_t		channelSequence_11;
	uint8_t		channelSequence_12;
	uint8_t		channelSequence_13;
	uint8_t		channelSequence_14;
	uint8_t		channelSequence_15;
	uint8_t		channelSequence_16;
}Conversion_Config_t;

//Estructura que especifica el tiempo deseado para hacer la adquisición de dato
typedef struct
{
	uint16_t	samplingPeriodChannel_0;
	uint16_t	samplingPeriodChannel_1;
	uint16_t	samplingPeriodChannel_2;
	uint16_t	samplingPeriodChannel_3;
	uint16_t	samplingPeriodChannel_4;
	uint16_t	samplingPeriodChannel_5;
	uint16_t	samplingPeriodChannel_6;
	uint16_t	samplingPeriodChannel_7;
	uint16_t	samplingPeriodChannel_8;
	uint16_t	samplingPeriodChannel_9;
	uint16_t	samplingPeriodChannel_10;
	uint16_t	samplingPeriodChannel_11;
	uint16_t	samplingPeriodChannel_12;
	uint16_t	samplingPeriodChannel_13;
	uint16_t	samplingPeriodChannel_14;
	uint16_t	samplingPeriodChannel_15;
	uint16_t	samplingPeriodChannel_16;

}Sampling_Config_t;

//Definimos los elementos para la configuracion del ADC
typedef struct
{
	Conversion_Config_t  conversion;
	Sampling_Config_t	sampling;
	uint8_t		resolution;	     	// Precisión con la que el ADC hace la adquisición del dato
	uint8_t     externalEvent;      //Se define la activacion por eventos externos de la conversion
	uint8_t    	typeEXTEvent;       //Se define el tipo de evento externo
	uint8_t		dataAlignment;    	//Alineación a la izquierda o a la derecha
	uint16_t	adcData;			//Dato de la conversión
}ADC_Config_t;

//Definimos las cabeceras de las funciones para el uso del ADC
void adc_Config(ADC_Config_t *adcConfig);
void configAnalogPin(uint8_t adcChannel);
void adcComplete_Callback(void);
void startSingleADC(void);
void startContinousADC(void);
uint16_t getADC(void);

void numberChannel(ADC_Config_t *adcConfig, uint8_t numberChannel);
void ADC_ConfigMultichannel (ADC_Config_t *adcConfig, uint8_t numeroDeCanales);

#endif /* ADCDRIVER_H_ */
