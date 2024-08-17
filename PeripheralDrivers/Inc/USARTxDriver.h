/*
 * USARTxDriver.h
 *
 *  Created on: 21/04/2023
 *      Author: julian
 */

#include <stdio.h>
#include <stm32f4xx.h>

#ifndef USARTXDRIVER_H_
#define USARTXDRIVER_H_

//Definicion de macros

#define USART_MODE_TX 				0
#define USART_MODE_RX 				1
#define USART_MODE_RXTX 			2
#define USART_MODE_DISABLE 			3

#define USART_BAUDRATE_9600			0
#define USART_BAUDRATE_19200		1
#define USART_BAUDRATE_115200		2
/*
#define USART_DATASIZE_8BIT    	 	0
#define USART_DATASIZE_9BIT    		1
*/

/*EN PS
 * 0: Even parity
 * 1: Odd parity
 */
#define USART_PARITY_NONE			0
#define USART_PARITY_ODD			1
#define USART_PARITY_EVEN			2

#define USART_STOPBIT_1				0
#define USART_STOPBIT_0_5			1
#define USART_STOPBIT_2				2
#define USART_STOPBIT_1_5			3

#define USART_TX_INTERRUP	0
#define USART_RX_INTERRUP	1

#define USART_RX_INTERRUP_DISABLE	0
#define USART_RX_INTERRUP_ENABLE	1

#define USART_TX_INTERRUP_DISABLE	0
#define USART_TX_INTERRUP_ENABLE	1

//Macros que definen la prioridad del USART
enum
{
	e_USART_PRIOPITY_6 = 6,
	e_USART_PRIOPITY_7,
	e_USART_PRIOPITY_8,
	e_USART_PRIOPITY_9,
	e_USART_PRIOPITY_10,
	e_USART_PRIOPITY_11,
	e_USART_PRIOPITY_12,
	e_USART_PRIOPITY_13,
	e_USART_PRIOPITY_14,
	e_USART_PRIOPITY_15
};


/*Estructura que contiene la configuracion minima para el uso del protocolo de comunicacion
 * Velocidad(baudrate)
 * Tamaño de los datos
 * Control de errores
 * Bit de parada
 */

typedef struct
{
	uint8_t USART_mode;
	uint8_t USART_baudrate;
	//uint8_t USART_datasize;
	uint8_t USART_parity;
	uint8_t USART_stopbits;
	uint8_t USART_enableIntRX;
	uint8_t USART_enableIntTX;
}USART_Config_t;

//Handler para el uso de USART

typedef struct
{
	USART_Config_t USART_Config;
	USART_TypeDef *ptrUSARTx;
	/*
	//Buffer de recepcion de datos
	uint8_t receptionBuffer[64];
	//Elemento que indica cuantos datos se reciben
	uint8_t dataInputSize;
	//Buffer de transmision de datos
	uint8_t transmisionBuffer[64];
	//Elemento que indica cuantos datos se deben transmitir
	uint8_t dataOutputSize;
	*/
}USART_Handler_t;

//Definicion de las cabeceras de las funciones para el uso del USART
void USART_Config(USART_Handler_t *ptrUsartHandler);
void usart_Config_Init_Priority(USART_Handler_t *ptrUsartHandler, uint8_t newPriority);
uint16_t getValueBaudRate(uint8_t fck, uint32_t baudRate);
void writeChar(USART_Handler_t *ptrUsartHandlerWrite, uint8_t datatoSend);
void writeMsg(USART_Handler_t *ptrUsartHandlerString, char *MsgtoSend);
void writeCharForTXE(USART_Handler_t *ptrUsartHandler, uint8_t datatoSend);
void writeMsgForTXE(USART_Handler_t *ptrUsartHandlerString, char *MsgtoSend);
uint8_t getRxData(void);
void interruptionTX(USART_TypeDef *ptrUSARTxUsed, uint8_t interrupEnable);

void BasicUSART1_Callback(void);
void BasicUSART2_Callback(void);
void BasicUSART6_Callback(void);

#endif /* USARTXDRIVER_H_ */
