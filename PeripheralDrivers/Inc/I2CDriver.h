/*
 * I2CDriver.h
 *
 *  Created on: 10/05/2023
 *      Author: julian
 */

#ifndef I2CDRIVER_H_
#define I2CDRIVER_H_

#include <stm32f4xx.h>

//Definicion de macros

#define I2C_WRITE_DATA  0               //Modo de comunicacion con el esclavo
#define I2C_READ_DATA   1

#define MAIN_CLOCK_4_MHZ_FOR_I2C    4   //Relog interno utilizado para el I2C, en nuestro caso seria inicialente 16MHz
#define MAIN_CLOCK_16_MHZ_FOR_I2C  16
#define MAIN_CLOCK_20_MHZ_FOR_I2C  20

#define I2C_MODE_SM    0                //Modo de la velocidad
#define I2C_MODE_FM    1

#define I2C_MODE_SM_SPEED_100KHz  80    //Represente el valor del CCR, previamente calculado
#define I2C_MODE_FM_SPEED_400KHz  13

#define I2C_MAX_RISE_TIME_SM      17    //Representa el valor del Trise, previamente calculado
#define I2C_MAX_RISE_TIME_FM       5

//Handler para el uso del del I2C
typedef struct
{
	I2C_TypeDef  *prtI2Cx;
	uint8_t       slaveAddress;
	uint8_t       modeI2C;
	uint8_t       dataI2C;
}I2C_Handler_t;

//Cabeceras de las funciones a usar para el I2C
void i2c_Config(I2C_Handler_t *ptrHandlerI2C);
void i2c_StartTrasaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_ReStartTrasaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_SendSlaveAddressRW(I2C_Handler_t *ptrHandlerI2C, uint8_t slaveAddress, uint8_t readOrWrite);
void i2c_SendMemoryAddress(I2C_Handler_t *ptrHandlerI2C, uint8_t memAddr);
void i2c_SendDataByte(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite);
uint8_t i2c_ReadDataByte(I2C_Handler_t *ptrHandlerI2C);
void i2c_StopTrasaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_SendAck(I2C_Handler_t *ptrHandlerI2C);
void i2c_SendNoAck(I2C_Handler_t *ptrHandlerI2C);

uint8_t i2c_ReadSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead);
void i2c_WriteSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead, uint8_t newValue);
void i2c_WriteSingle(I2C_Handler_t *ptrHandlerI2C, uint8_t newValue);

#endif /* I2CDRIVER_H_ */
