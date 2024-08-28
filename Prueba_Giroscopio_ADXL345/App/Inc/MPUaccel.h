/*
 * MPUAccel.h
 *
 *  Created on: 23/07/2024
 *      Author: Juan Esteban Rodriguez
 *      Mofied : Julian Lopera
 */

#ifndef MPUACCEL_H_
#define MPUACCEL_H_

#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>
#include <I2CDriver.h>


//---------------------Definicion de macros---------------------------
#define ADDRESS_UP          0b1101001         //Direccion del sclave
#define ADDRESS_DOWN        0b1101000

#define ACELERACION  0                        //Magnitud fisica a medir
#define GIROSCOPIO   1

#define RESET (0b1 << 7)                     //bit de reset

//--------Elementos de calibracion--------
#define CAL_ACCEL_X     0
#define CAL_ACCEL_Y     1
#define CAL_ACCEL_Z     2
#define CAL_GYRO_X      3
#define CAL_GYRO_Y      4
#define CAL_GYRO_Z      5

//---------Elemento de lectura-----------
#define READ_ACCEL_X     0
#define READ_ACCEL_Y     1
#define READ_ACCEL_Z     2
#define READ_GYRO_X      3
#define READ_GYRO_Y      4
#define READ_GYRO_Z      5

//---Direccion de los registros del MPU-------
#define ACCEL_CONFIG  0x1C                         //Direccion de los Registros para la configuracion de los elementos que miden.
#define GIRO_CONFIG   0x1B

#define ACCEL_XOUT_H  0x32                         //Direccion de los Registros que mide las las diferentes magnitudes fisicas
#define ACCEL_XOUT_L  0x33
#define ACCEL_YOUT_H  0x3D
#define ACCEL_YOUT_L  0x3E
#define ACCEL_ZOUT_H  0x3F
#define ACCEL_ZOUT_L  0x40

#define GIRO_XOUT_H  0x43
#define GIRO_XOUT_L  0x44
#define GIRO_YOUT_H  0x45
#define GIRO_YOUT_L  0x46
#define GIRO_ZOUT_H  0x47
#define GIRO_ZOUT_L  0x48

#define PWR_MGMT_l  107                            //Hay un bit en dicho registro para aplicar reset
#define WHO_AM_I    117                            //Registro para verificar la identidad del equipo

//Escalas y sensibilidad de las diferentes magnitudes
#define ACCEL_2G  0                          //Escalas para la aceleracion
#define ACCEL_4G  1
#define ACCEL_8G  2
#define ACCEL_16G 3

#define ACCEL_2G_SENS  16384                //Sensibilidades por rango de aceleración
#define ACCEL_4G_SENS  8192
#define ACCEL_8G_SENS  4096
#define ACCEL_16G_SENS 2048

#define GYRO_250   0                        //Escalas para el GYRO
#define GYRO_500   1
#define GYRO_1000  2
#define GYRO_2000  3

#define GYRO_250_SENS  131                  //Sensibilidades respectivamente por rango de GYRO
#define GYRO_500_SENS  66
#define GYRO_1000_SENS 33
#define GYRO_2000_SENS 16


//Estructura para definir los elementos correspondientes al MPU
typedef struct
{
	uint8_t 		fullScaleACCEL;
	uint8_t 		fullScaleGYRO;
	I2C_Handler_t   *ptrI2Chandler;

}MPUAccel_Handler_t;


//Cabeceras de las funciones la configuracion y adquisicion de datos del MPU
void configMPUAccel(MPUAccel_Handler_t *ptrMPUAccel);
float calibrationMPU(MPUAccel_Handler_t *ptrMPUAccel, uint8_t elementCalibration);
int16_t readCalibrationMPU(MPUAccel_Handler_t *ptrMPUAccel, uint8_t elementRead);
float readMPU(MPUAccel_Handler_t *ptrMPUAccel, uint8_t elementRead, int16_t offset);

float getAngle(MPUAccel_Handler_t *ptrMPUAccel, uint64_t *time_pre, float ang_init, uint8_t axis, int16_t offset_Axis);

uint8_t WHOIAM (MPUAccel_Handler_t *ptrMPUAccel);

#endif /* MPUACCEL_H_ */
