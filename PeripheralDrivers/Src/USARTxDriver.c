/*
 * USARTxDriver.c
 *
 *  Created on: 21/04/2023
 *      Author: julian
 */

#include <USARTxDriver.h>
#include <PLLDriver.h>
#include <stdio.h>

//Se cuenta con 3 USART: USART1, USART2, USART6

//Variable que guarda la referencia del periferico a utilizar
USART_TypeDef *ptrUSART1Used;
USART_TypeDef *ptrUSART2Used;
USART_TypeDef *ptrUSART6Used;


uint8_t auxRxData = 0;                               //Variable que almacena el dato recibido
char datatoSendForTXE = '\0';                        //Variable que indica el estado de transmision
char bufferMsgForTXE[100] = {0};                      //Variable que almacena un strings
char bufferMsgForTXE_inLine[20][100] = {0};           //Variable que almacena varios strings
uint8_t inLine = 0;                                  //Variable que cuenta los string que estan en la cola
uint8_t inLineTxe = 0;                               //Variable que define el string que sigue en la cola
uint8_t posChar = 0;                                 //Variable para recorrer el String
uint8_t typeWriteTXE = 0;                            //Variable que selecciona el tipo entre string y caracter

//Funcion para cargar la configuracion del periferico USART
void USART_Config(USART_Handler_t *ptrUsartHandler)
{
	//---------------------------------1) Activamos el periferico------------------------------------------
	//Registro: APB1ENR
	//Registro: APB2ENR

	if(ptrUsartHandler->ptrUSARTx == USART1)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB2ENR = RCC_APB2ENR_USART1EN;
		//Guardamos una referencia al periferico que estamos utilizando
		ptrUSART1Used = ptrUsartHandler->ptrUSARTx;
	}

	else if(ptrUsartHandler->ptrUSARTx == USART2)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB1ENR = RCC_APB1ENR_USART2EN;
		//Guardamos una referencia al periferico que estamos utilizando
		ptrUSART2Used = ptrUsartHandler->ptrUSARTx;
	}

	else if(ptrUsartHandler->ptrUSARTx == USART6)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB2ENR = RCC_APB2ENR_USART6EN;
		//Guardamos una referencia al periferico que estamos utilizando
		ptrUSART6Used = ptrUsartHandler->ptrUSARTx;
	}

	//-------------------------------2) Limpieza de registros ------------------------------------------
	//Registro: CR1
	//Registro: CR2

	ptrUsartHandler->ptrUSARTx->CR1 = 0;
	ptrUsartHandler->ptrUSARTx->CR2 = 0;

	//-------------------------3) Configuracion del parity y tamaño de dato ------------------------------
	//Registro: CR1

	//Verificamos si la paridad esta activada
	if(ptrUsartHandler->USART_Config.USART_parity != USART_PARITY_NONE)
	{
		//Activamos Parity control Enable PCE
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PCE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PCE;

		//Definimos un tamaño de datos de 9 bits M = 1
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_M;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_M;

		//Verificamos si se selecciona ODD o EVEN
		if(ptrUsartHandler->USART_Config.USART_parity == USART_PARITY_ODD)
		{
			ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PS;
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PS;
		}
		else
		{
			ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PS;
		}

	}
	else
	{
		//Desactivamos la Parity control Enable PCE
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PCE;
		//Definimos un tamaño de datos de 8 bits M = 0
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_M;
	}

	//--------------------------------4) Configuracion del Stop bits--------------------------------------
	//Registro: CR2

	//Se verifica que stop bits se selecciono
	switch(ptrUsartHandler->USART_Config.USART_stopbits)
	{
	//Para el 1 Stop bit se escribe 00
	case USART_STOPBIT_1:
	{
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(0b11<<USART_CR2_STOP_Pos);
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b00<<USART_CR2_STOP_Pos);
		break;
	}
	//Para el 0.5 Stop bit se escribe 01
	case USART_STOPBIT_0_5:
	{
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(0b11<<USART_CR2_STOP_Pos);
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b01<<USART_CR2_STOP_Pos);
		break;
	}
	//Para el 2 Stop bit se escribe 10
	case USART_STOPBIT_2:
	{
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(0b11<<USART_CR2_STOP_Pos);
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b10<<USART_CR2_STOP_Pos);
		break;
	}
	//Para el 1.5 Stop bit se escribe 11
	case USART_STOPBIT_1_5:
	{
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(0b11<<USART_CR2_STOP_Pos);
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b11<<USART_CR2_STOP_Pos);
		break;
	}
	//En caso por defecto selsccionamos un bit de paraba
	default:
	{
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(0b11<<USART_CR2_STOP_Pos);
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b00<<USART_CR2_STOP_Pos);
		break;
	}
	}

	//-------------------5) Configuracion del Baudrate(Velocidad de Trans o Rec)-------------------------
	//Registro: BRR       //Numero de 32 bits

	uint8_t clock = 0;

	if(ptrUsartHandler->ptrUSARTx == USART2)
	{
		clock = getClockAPB1();          //Guardamos la velocidad de reloj entregada al bus APB1
	}
	else
	{
		clock = getConfigPLL();	         //Guardamos la velocidad de reloj del sistema
	}

	//Verificamos el Baud Rate selecionado
	if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_9600)
	{
		//Se carga el valor de la velocidad en el registro
		ptrUsartHandler->ptrUSARTx->BRR = getValueBaudRate(clock, 9600);
	}
	else if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_19200)
	{
		//Se carga el valor de la velocidad en el registro
		ptrUsartHandler->ptrUSARTx->BRR = getValueBaudRate(clock, 19200);
	}
	else if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_115200)
		{
		//Se carga el valor de la velocidad en el registro
		ptrUsartHandler->ptrUSARTx->BRR = getValueBaudRate(clock, 115200);
	}
	else
	{
		__NOP();
	}
	//-------------------6) Configuramos el MODO-------------------------
	//Registro: CR1

	//MODOS: TX only, RX only, RXTX, disable

	switch(ptrUsartHandler->USART_Config.USART_mode)
	{
	//Activamos la parte del sistema encargada de enviar
	case USART_MODE_TX :
	{
		//Activamos el Baud rate clock para la transmision
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TE;

		break;
	}
	//Activamos la parte del sistema encargada de recibir
	case USART_MODE_RX:
	{
		//Activamos el Baud rate clock para la recepcion
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RE;

		break;
	}
	//Activamos ambas  parte del sistema encargadas de enviar y recibir
	case USART_MODE_RXTX:
	{
		//Activamos el Baud rate clock para la transmision
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TE;
		//Activamos el Baud rate clock para la recepcion
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RE;


		break;
	}
	//Desativamos ambos canales
	case USART_MODE_DISABLE:
	{
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		break;
	}
	default:
	{
		//Actuando por defecto, desactivamos ambos canales
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		break;
	}
	}

	//-------------------6) Activacion de las interrupciones------------------------
	//Registro: CR1

	//Desactivamos las interupciones globales
	__disable_irq();

	//Se selecciono la interrupcion para RX
	if(ptrUsartHandler->USART_Config.USART_enableIntRX ==  USART_RX_INTERRUP_ENABLE)
	{
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RXNEIE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RXNEIE;
	}
	else
	{
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RXNEIE;
	}

	//Verificamos si se selecciono alguna interrupcion
	if(ptrUsartHandler->USART_Config.USART_enableIntRX ==  USART_RX_INTERRUP_ENABLE || ptrUsartHandler->USART_Config.USART_enableIntTX ==  USART_TX_INTERRUP_ENABLE)
	{
		//Matriculamos la interrupcion en el NVIC
		if(ptrUsartHandler->ptrUSARTx == USART1)
		{
			NVIC_EnableIRQ(USART1_IRQn);
		}
		else if(ptrUsartHandler->ptrUSARTx == USART2)
		{
			NVIC_EnableIRQ(USART2_IRQn);
		}
		else if(ptrUsartHandler->ptrUSARTx == USART6)
		{
			NVIC_EnableIRQ(USART6_IRQn);
		}
	}
	else
	{
		__NOP();
	}
	//Activamos las interupciones globales
	__enable_irq();

	//-------------------7) Activacion del modulo Serial------------------------
	//Registro: CR1

	if(ptrUsartHandler->USART_Config.USART_mode != USART_MODE_DISABLE)
	{
		//Activamos el modulo Serial
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_UE;
	}
}

//---------------Funcion para calcular el valor correspondiente a ingresar en el BRR----------
uint16_t getValueBaudRate(uint8_t fck, uint32_t baudRate)
{
    uint32_t usartDiv = (fck*10000000000)/(16*baudRate);
    uint32_t mantiza = usartDiv/10000;
    uint32_t decimal = usartDiv-mantiza*10000;
    uint8_t div_Fraction = (decimal-1000)/625;  //1000
    uint16_t value  = mantiza<<USART_BRR_DIV_Mantissa_Pos | div_Fraction;

    return value;
}

//---------------Funciones para la transmision de datos------------------------------
//Funcion para escribir un solo char
void writeChar(USART_Handler_t *ptrUsartHandler, uint8_t datatoSend)
{
	//Verificamos que no se este enviando un mensaje
	while(!(ptrUsartHandler->ptrUSARTx->SR & USART_SR_TXE))
	{
		__NOP();
	}
	//Almacenamos un elemento char en el registro USART_DR
	ptrUsartHandler->ptrUSARTx->DR = datatoSend;

}

//Funcion para escribir un string
void writeMsg(USART_Handler_t *ptrUsartHandlerString, char *MsgtoSend)
{
	uint8_t i = 0;

	while(MsgtoSend[i] != '\0')
	{
		writeChar(ptrUsartHandlerString, MsgtoSend[i]);
		i++;
	}
}

//----------------Funciones para la transmision de datos por interrupcion------------------------------
//Funcion para escribir un solo char
void writeCharForTXE(USART_Handler_t *ptrUsartHandler, uint8_t datatoSend)
{
	if (posChar==0)
	{
		//Guardamos el caracter que se desea enviar en una variable
		datatoSendForTXE = datatoSend;
		//cambiamos el tipo
		typeWriteTXE = 0;
		//Activo la interrupcion
		interruptionTX(ptrUsartHandler->ptrUSARTx, USART_TX_INTERRUP_ENABLE);
	}
	else
	{
		__NOP();
	}
}

//Funcion para escribir un string
void writeMsgForTXE(USART_Handler_t *ptrUsartHandlerString, char *MsgtoSend)
{
	if (posChar == 0 && inLineTxe == 0)
	{
		//Guardamos el string que se desea enviar en un arreglo
		sprintf(bufferMsgForTXE, MsgtoSend);
		//cambiamos el tipo
		typeWriteTXE = 1;
		//Activo la interrupcion
		interruptionTX(ptrUsartHandlerString->ptrUSARTx, USART_TX_INTERRUP_ENABLE);
	}
	else
	{
		sprintf(bufferMsgForTXE_inLine[inLine], MsgtoSend);
		inLine++;
	}
}

//Funcion para leer un char
uint8_t getRxData(void)
{
	return auxRxData;
}

//Funcion para desactivar o activar las interrupciones por  TX
void interruptionTX(USART_TypeDef *ptrUSARTxUsed, uint8_t interrupEnable)
{
	//Se selecciono la interrupcion para TX
	if(interrupEnable ==  USART_TX_INTERRUP_ENABLE)
	{
		ptrUSARTxUsed->CR1 &= ~USART_CR1_TXEIE;
		ptrUSARTxUsed->CR1 |= USART_CR1_TXEIE;
	}
	else
	{
		ptrUSARTxUsed->CR1  &= ~USART_CR1_TXEIE;
	}
}


//Definimos las funciones para cuando se genera una interrupcion del USART1-2 y 6
__attribute__((weak)) void BasicUSART1_Callback()
{
	__NOP();
}


__attribute__((weak)) void BasicUSART2_Callback()
{
	__NOP();
}


__attribute__((weak)) void BasicUSART6_Callback()
{
	__NOP();
}


/* Cuando se produce una interrupcion en el NVIC debido a uno de los USART apuntara a una de
 * estas funciones en el vector de interrupciones respectivamente.
 * Con ello Guardamos el elemento char recibido
 */

void USART1_IRQHandler(void)
{
	//Confirmamos que el registro RXNE esta activo
	if(ptrUSART1Used->SR & USART_SR_RXNE)
	{
		//Leemos el registro DR del respectivo USART
		auxRxData = (uint8_t) ptrUSART1Used->DR;
		//Llamanos a la funcion de interrupcion
		BasicUSART1_Callback();
	}
	else if (ptrUSART1Used->SR & USART_SR_TXE)
	{
		//Verificamos entre Sting o Caracter para enviar
		if(typeWriteTXE == 0)
		{
			//transmitimos el caracter
			ptrUSART1Used->DR = datatoSendForTXE;
			//Desactivo la interrupcion
			interruptionTX(ptrUSART1Used, USART_TX_INTERRUP_DISABLE);
		}
		else if(typeWriteTXE == 1)
		{
			if (bufferMsgForTXE[posChar] != '\0')
			{
				//transmitimos el caracter
				ptrUSART1Used->DR = bufferMsgForTXE[posChar];
				//aunmentamos la variable
				posChar++;
			}
			else
			{
				if (inLineTxe<inLine)
				{
//					//Limpiamos el String
//					bufferMsgForTXE[] = {0};
					//Guardamos el string que se desea enviar en un arreglo
					sprintf(bufferMsgForTXE, bufferMsgForTXE_inLine[inLineTxe]);
					//reiniciamos la variable
					posChar = 0;
					//aunmentamos la variable
					inLineTxe++;
				}
				else
				{
					//Desactivo la interrupcion
					interruptionTX(ptrUSART1Used, USART_TX_INTERRUP_DISABLE);
					//reiniciamos la variable
					posChar = 0;
					inLineTxe = 0;
					inLine = 0;
				}
			}
		}
	}
	else
	{
		__NOP();
	}
}

void USART2_IRQHandler(void)
{
	//Confirmamos que el registro RXNE esta activo
	if(ptrUSART2Used->SR & USART_SR_RXNE)
	{
		//Leemos el registro DR del respectivo USART
		auxRxData = (uint8_t) ptrUSART2Used->DR;
		//Llamanos a la funcion de interrupcion
		BasicUSART2_Callback();
	}
	else if (ptrUSART2Used->SR & USART_SR_TXE)
	{
		//Verificamos entre Sting o Caracter para enviar
		if(typeWriteTXE == 0)
		{
			//transmitimos el caracter
			ptrUSART2Used->DR = datatoSendForTXE;
			//Desactivo la interrupcion
			interruptionTX(ptrUSART2Used, USART_TX_INTERRUP_DISABLE);
		}
		else if(typeWriteTXE == 1)
		{
			if (bufferMsgForTXE[posChar] != '\0')
			{
				//transmitimos el caracter
				ptrUSART2Used->DR = bufferMsgForTXE[posChar];
				//aunmentamos la variable
				posChar++;
			}
			else
			{
				if (inLineTxe<inLine)
				{
//					//Limpiamos el String
//					bufferMsgForTXE[] = {0};
					//Guardamos el string que se desea enviar en un arreglo
					sprintf(bufferMsgForTXE, bufferMsgForTXE_inLine[inLineTxe]);
					//reiniciamos la variable
					posChar = 0;
					//aunmentamos la variable
					inLineTxe++;
				}
				else
				{
					//Desactivo la interrupcion
					interruptionTX(ptrUSART2Used, USART_TX_INTERRUP_DISABLE);
					//reiniciamos la variable
					posChar = 0;
					inLineTxe = 0;
					inLine = 0;
				}
			}
		}
	}
	else
	{
		__NOP();
	}
}

void USART6_IRQHandler(void)
{
	//Confirmamos que el registro RXNE esta activo
	if(ptrUSART6Used->SR & USART_SR_RXNE)
	{
		//Leemos el registro DR del respectivo USART
		auxRxData = (uint8_t) ptrUSART6Used->DR;
		//Llamanos a la funcion de interrupcion
		BasicUSART6_Callback();
	}
	else if (ptrUSART6Used->SR & USART_SR_TXE)
	{
		//Verificamos entre Sting o Caracter para enviar
		if(typeWriteTXE == 0)
		{
			//transmitimos el caracter
			ptrUSART6Used->DR = datatoSendForTXE;
			//Desactivo la interrupcion
			interruptionTX(ptrUSART6Used, USART_TX_INTERRUP_DISABLE);
		}
		else if(typeWriteTXE == 1)
		{
			if (bufferMsgForTXE[posChar] != '\0')
			{
				//transmitimos el caracter
				ptrUSART6Used->DR = bufferMsgForTXE[posChar];
				//aunmentamos la variable
				posChar++;
			}
			else
			{
				if (inLineTxe<inLine)
				{
//					//Limpiamos el String
//					bufferMsgForTXE[] = {0};
					//Guardamos el string que se desea enviar en un arreglo
					sprintf(bufferMsgForTXE, bufferMsgForTXE_inLine[inLineTxe]);
					//reiniciamos la variable
					posChar = 0;
					//aunmentamos la variable
					inLineTxe++;
				}
				else
				{
					//Desactivo la interrupcion
					interruptionTX(ptrUSART6Used, USART_TX_INTERRUP_DISABLE);
					//reiniciamos la variable
					posChar = 0;
					inLineTxe = 0;
					inLine = 0;
				}
			}
		}
	}
	else
	{
		__NOP();
	}
}

