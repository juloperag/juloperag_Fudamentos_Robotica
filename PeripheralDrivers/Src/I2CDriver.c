/*
 * I2CDriver.c
 *
 *  Created on: 10/05/2023
 *      Author: julian
 */

#include <I2CDriver.h>
#include <PLLDriver.h>

/*
 * Recordar de configurar los pines GPIOx por medio de las funciones alternativas,
 * Ademas de configurar dichos pines como open_drain
 */

void i2c_Config(I2C_Handler_t *ptrHandlerI2C)
{
	uint8_t clockAPB1 = getClockAPB1();     //Variable que guarda la velocidad de reloj entregada al bus APB1

	//---------------------------------1) Activamos el periferico------------------------------------------
	//Registro: APB1ENR

	if(ptrHandlerI2C->prtI2Cx == I2C1)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	}
	else if(ptrHandlerI2C->prtI2Cx == I2C2)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
	}
	else if(ptrHandlerI2C->prtI2Cx == I2C3)
	{
		/*Activamos el periferico escribiendo un 1 deacuerdo a la posicion
		 * del periferico en el registro*/
		RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
	}

	//---------------------------------2) Reiniciamos el periferico------------------------------------------
	//Registro: CR1

	ptrHandlerI2C->prtI2Cx->CR1 |= I2C_CR1_SWRST;
	__NOP();
	ptrHandlerI2C->prtI2Cx->CR1 &= ~I2C_CR1_SWRST;

	//-----------------------------------3) Valor del reloj principal------------------------------------------
	//Registro: CR2

	ptrHandlerI2C->prtI2Cx->CR2 &= ~(0b111111<<I2C_CR2_FREQ_Pos);
	ptrHandlerI2C->prtI2Cx->CR2 |= (clockAPB1<<I2C_CR2_FREQ_Pos);

	//-------------------------------------4) Configuracion del I2C------------------------------------------
	//Registro: CCR
	//Registro: TRISE

	//Variables que almacenan el respectivo valor a cargar en los registros CRR y Trise
	uint16_t valueCRR = 0;
	uint8_t valueTrise = 0;

	//definimos inicialmente los registro en 0
	ptrHandlerI2C->prtI2Cx->CCR = 0;
	ptrHandlerI2C->prtI2Cx->TRISE = 0;

	//Configuramos el I2C deacuerdo al modo selsecionado
	if(ptrHandlerI2C->modeI2C == I2C_MODE_SM)
	{
		//Seleccionamos el modo estandar
		ptrHandlerI2C->prtI2Cx->CCR &= ~I2C_CCR_FS;

		//Definimos la señal de reloj
		valueCRR = (5000*clockAPB1)/1000;
		ptrHandlerI2C->prtI2Cx->CCR |= (valueCRR<<I2C_CCR_CCR_Pos);

		//Definimos el tiempo maximo en el T-RIse
		valueTrise = ((1000*clockAPB1)/1000)+1;
		ptrHandlerI2C->prtI2Cx->TRISE |= valueTrise;
	}
	else
	{
		//Seleccionamos el modo Fast
		ptrHandlerI2C->prtI2Cx->CCR &= ~I2C_CCR_FS;
		ptrHandlerI2C->prtI2Cx->CCR |= I2C_CCR_FS;

		//Definimos la señal de reloj
		valueCRR = (2500*clockAPB1)/3000;
		ptrHandlerI2C->prtI2Cx->CCR |= (valueCRR<<I2C_CCR_CCR_Pos);

		//Definimos el tiempo maximo en el T-RIse
		valueTrise = ((300*clockAPB1)/1000)+1;
		ptrHandlerI2C->prtI2Cx->TRISE |= valueTrise;
	}

	//-----------------------------------5) Activamos el modulo I2C------------------------------------------
	//Registro: CR1

	ptrHandlerI2C->prtI2Cx->CR1 &= ~I2C_CR1_PE;
	ptrHandlerI2C->prtI2Cx->CR1 |= I2C_CR1_PE;
}

//----------------------------------------Funciones para la Transaccion----------------------------------------
//Funcion para empezar la Transaccion
void i2c_StartTrasaction(I2C_Handler_t *ptrHandlerI2C)
{
	/*verificamos que la linea no esta ocupada
	 * bit "busy" en I2C
	 */
	while (ptrHandlerI2C->prtI2Cx->SR2 & I2C_SR2_BUSY)
	{
		__NOP();
	}
	//Generamos un Start bit
	ptrHandlerI2C->prtI2Cx->CR1 |= I2C_CR1_START;
	//Esperamos a que la bandera "SB" del evento "Start" se levante
	//Se limpia la bandera "SB" leyendo SR1->escribiendo en DR
	while (!(ptrHandlerI2C->prtI2Cx->SR1 & I2C_SR1_SB))
	{
		__NOP();
	}
}

//Generar de nuevo una señal para empazar la Transaccion
void i2c_ReStartTrasaction(I2C_Handler_t *ptrHandlerI2C)
{
	//Generamos un Start bit
	ptrHandlerI2C->prtI2Cx->CR1 |= I2C_CR1_START;
	//Esperamos a que la bandera "SB" del evento "Start" se levante
	//Se limpia la bandera "SB" leyendo SR1->escribiendo en DR
	while (!(ptrHandlerI2C->prtI2Cx->SR1 & I2C_SR1_SB))
	{
		__NOP();
	}
}

//Funcion para enviar la direcion de Slave ademas de definir la lectura o escritura del Sclave
void i2c_SendSlaveAddressRW(I2C_Handler_t *ptrHandlerI2C, uint8_t slaveAddress, uint8_t readOrWrite)
{
	//Definimos una variable auxiliar
	uint8_t auxByte = 0;
	(void) auxByte;

	//Cargamos la direccion del Sclave, ademas cargamos el bit de lectura o escritura
	ptrHandlerI2C->prtI2Cx->DR = (slaveAddress<<1) | readOrWrite;
	//Esperamos a que la bandera "ADDR" del evento "Address" se levante
	//Se limpia la bandera "ADDR" leyendo SR1->leyendo en SR2
	while (!(ptrHandlerI2C->prtI2Cx->SR1 & I2C_SR1_ADDR))
	{
		__NOP();
	}
	//Se limpia la bandera "ADDR"
	auxByte = ptrHandlerI2C->prtI2Cx->SR1;
	auxByte = ptrHandlerI2C->prtI2Cx->SR2;
}

//Enviamos la direccion de memoria que deseamos leer o escribir
void i2c_SendMemoryAddress(I2C_Handler_t *ptrHandlerI2C, uint8_t memAddr)
{
	//Enviamos la direccion de memoria que desamos leer
	ptrHandlerI2C->prtI2Cx->DR = memAddr;
	//Esperamos a que la bandera "TXE" del evento "Data1" se levante
	//Se limpia la bandera "TxE" y "BTF" con la condicion Stop
	while (!(ptrHandlerI2C->prtI2Cx->SR1 & I2C_SR1_TXE))
	{
		__NOP();
	}
}

//Escritura de datos
void i2c_SendDataByte(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite)
{
	//Cargamos el valor que desasmos escribir
	ptrHandlerI2C->prtI2Cx->DR = dataToWrite;
	//Esperamos a que la bandera "BTF" del evento "Transmision" se levante
	//Se limpia la bandera "TxE" y "BTF" con la condicion Stop
	while (!(ptrHandlerI2C->prtI2Cx->SR1 & I2C_SR1_BTF))
	{
		__NOP();
	}
}

//Lectura de datos
uint8_t i2c_ReadDataByte(I2C_Handler_t *ptrHandlerI2C)
{
	//Esperamos a que la bandera "RxNE" del evento "Recepcion" se levante
	//Se limpia la bandera "RxNE" leyendo el registro DR
	while (!(ptrHandlerI2C->prtI2Cx->SR1 & I2C_SR1_RXNE))
	{
		__NOP();
	}
	//guardamos los datos recibimos
	ptrHandlerI2C->dataI2C = ptrHandlerI2C->prtI2Cx->DR;
	return ptrHandlerI2C->dataI2C ;
}

//Funcion para activar la indicaicon de NoACK(indicacion para el Sclave de terminar la transmision de datos)
void i2c_SendNoAck(I2C_Handler_t *ptrHandlerI2C)
{
	//Esribimos cero en el bit Ack del registro CR1
	ptrHandlerI2C->prtI2Cx->CR1 &= ~I2C_CR1_ACK;

}

//Funcion para activar la indicaicon de ACK(indicacion para el Sclave de continuar la transmision de datos)
void i2c_SendAck(I2C_Handler_t *ptrHandlerI2C)
{
	//Esribimos 1 en el bit Ack del registro CR1
	ptrHandlerI2C->prtI2Cx->CR1 |= I2C_CR1_ACK;
}

//Funcion para generar el Stop bit
void i2c_StopTrasaction(I2C_Handler_t *ptrHandlerI2C)
{
	ptrHandlerI2C->prtI2Cx->CR1 |= I2C_CR1_STOP;
}

//------------------------------------Funciones para la comunicacion por I2C----------------------------------------

uint8_t i2c_ReadSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead)
{
	//0. Creacion de una variable auxiliara para recibir el dato leido
	uint8_t auxRead = 0;
	//1. Generacion de Start bit
	i2c_StartTrasaction(ptrHandlerI2C);
	//2. Enviamos la direccion del Sclavo e indicamos que se desea escribir el Sclave
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_WRITE_DATA);
	//3. Enviamos la direccion de memoria que deseamos leer
	i2c_SendMemoryAddress(ptrHandlerI2C, regToRead);
	//4. Creamos una condicion de re start
	i2c_ReStartTrasaction(ptrHandlerI2C);
	//5. Enviamos la direccion del Sclavo e indicamos que se desea leer el Sclave
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_READ_DATA);
	//6. Leemos el dato que envia el Sclavo
	auxRead = i2c_ReadDataByte(ptrHandlerI2C);
	//7. Generamso la condicio de NoAck
	i2c_SendNoAck(ptrHandlerI2C);
	//8. Generamos la condicion de Stop
	i2c_StopTrasaction(ptrHandlerI2C);

	return auxRead;
}

void i2c_WriteSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead, uint8_t newValue)
{
	//1. Generacion de Start bit
	i2c_StartTrasaction(ptrHandlerI2C);
	//2. Enviamos la direccion del Sclavo e indicamos que se desea escribir el Sclave
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_WRITE_DATA);
	//3. Enviamos la direccion de memoria que deseamos escribir
	i2c_SendMemoryAddress(ptrHandlerI2C, regToRead);
	//4. Enviamos el dato que se desea escribir en el Sclave
	i2c_SendDataByte(ptrHandlerI2C, newValue);
	//5. Generamos la condicion de Stop
	i2c_StopTrasaction(ptrHandlerI2C);
}

void i2c_WriteSingle(I2C_Handler_t *ptrHandlerI2C, uint8_t newValue)
{
	//1. Generacion de Start bit
	i2c_StartTrasaction(ptrHandlerI2C);
	//2. Enviamos la direccion del Sclavo e indicamos que se desea escribir el Sclave
	i2c_SendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_WRITE_DATA);
	//3. Enviamos la direccion de memoria que deseamos escribir NO SE ESPECIFICA
	//4. Enviamos el dato que se desea escribir en el Sclave
	i2c_SendDataByte(ptrHandlerI2C, newValue);
	//5. Generamos la condicion de Stop
	i2c_StopTrasaction(ptrHandlerI2C);
}

