//Importacio de librerias
#include "MPUAccel.h"
#include <SysTickDriver.h>

void configMPUAccel (MPUAccel_Handler_t *ptrMPUAccel){

	uint8_t rdy  = 0;
	uint8_t byte = 0;

	//------------Verificamos comunicacion con el MPU----------------
	while(!rdy){

		rdy = WHOIAM(ptrMPUAccel);
	}

	//------------------Reiniciamos el MPU--------------------------
	i2c_WriteSingleRegister(ptrMPUAccel->ptrI2Chandler, PWR_MGMT_l, 0x00);
	//Pausa
	delay_ms(1);

	//---------------Configuracion Accel----------------------------
	byte =  i2c_ReadSingleRegister(ptrMPUAccel->ptrI2Chandler, ACCEL_CONFIG);
	byte &= ~(0b00011000);
	//Pausa
	delay_ms(1);
	//Deacuerdo al valor predeterminado se carga una configuracion
	i2c_WriteSingleRegister(ptrMPUAccel->ptrI2Chandler, ACCEL_CONFIG, (byte) | (ptrMPUAccel->fullScaleACCEL<<3));

	//---------------Configuracion Giro----------------------------
	byte =  i2c_ReadSingleRegister(ptrMPUAccel->ptrI2Chandler, GIRO_CONFIG);
	byte &= ~(0b00011000);
	//Pausa
	delay_ms(1);
	//Deacuerdo al valor predeterminado se carga una configuracion
	i2c_WriteSingleRegister(ptrMPUAccel->ptrI2Chandler, GIRO_CONFIG, (byte) | (ptrMPUAccel->fullScaleACCEL <<3));
}



//Esta funcion permite verificar comunicacion correcta con el MPU
uint8_t WHOIAM (MPUAccel_Handler_t *ptrMPUAccel){
	//Variables auxiliares
	uint8_t whoami = 0;
	uint8_t rdy    = 0;
	//leemos el registro WHO_AM_I correspondiente
	whoami =  i2c_ReadSingleRegister(ptrMPUAccel->ptrI2Chandler, WHO_AM_I);
	//verificamos la transacción
	whoami &= ~(0b10000001);
	rdy = (ADDRESS_DOWN >> 1) && (whoami >> 1);

	return rdy;
}


//Funcion para la lectura de dos registros que compone uno de los ejes de acelerometro o del giroscopio
float readMPU(MPUAccel_Handler_t *ptrMPUAccel, uint8_t elementRead, int16_t offset)
{
	//Variable para guardar la  direccion de los dos registros a leer
	uint8_t address_H = 0;
	uint8_t address_L = 0;
	//Creamos las variables donde almacenamos todos los datos
	uint16_t aux_H = 0;
	uint16_t aux_L = 0;
	int16_t   aux  = 0;
	float res  = 0;
	//Seleccionamos la direccion de los registros a Leer
	switch(elementRead)
	{
		case READ_ACCEL_X:{address_H = ACCEL_XOUT_H, address_L = ACCEL_XOUT_L; break;}
		case READ_ACCEL_Y:{address_H = ACCEL_YOUT_H, address_L = ACCEL_YOUT_L; break;}
		case READ_ACCEL_Z:{address_H = ACCEL_ZOUT_H, address_L = ACCEL_ZOUT_L; break;}
		case READ_GYRO_X:{address_H = GIRO_XOUT_H, address_L = GIRO_XOUT_L; break;}
		case READ_GYRO_Y:{address_H = GIRO_YOUT_H, address_L = GIRO_YOUT_L; break;}
		case READ_GYRO_Z:{address_H = GIRO_ZOUT_H, address_L = GIRO_ZOUT_L; break;}
		default:{ break; }
	}
	//Realizamos la medicion de la magnitud fisica en el respectivo eje
	aux_H = i2c_ReadSingleRegister(ptrMPUAccel->ptrI2Chandler, address_H);
	aux_L = i2c_ReadSingleRegister(ptrMPUAccel->ptrI2Chandler, address_L);
	//Juntamos ambos bytes en un solo numero para tener la lectura completa
	aux = ((int16_t) ((aux_H << 8) | (aux_L))) - offset;
	//Realizamos la conversion de los bytes al valor de magnitud fisica en el respectivo eje
	switch(elementRead)
	{
		//Verificamos el eje del sensor a medir
		case READ_ACCEL_X:
		case READ_ACCEL_Y:
		case READ_ACCEL_Z:
		{	//Deacuerdo a la escala se realiza la conversion
			switch (ptrMPUAccel->fullScaleACCEL)
			{
				case ACCEL_2G :{res = (aux*9.77)/ACCEL_2G_SENS; break;}
				case ACCEL_4G :{res = (aux*9.77)/ACCEL_4G_SENS; break;}
				case ACCEL_8G :{res = (aux*9.77)/ACCEL_8G_SENS; break;}
				case ACCEL_16G :{res = (aux*9.77)/ACCEL_16G_SENS; break;}
				default:{ break; }
			}
			break;
		}
		case READ_GYRO_X:
		case READ_GYRO_Y:
		case READ_GYRO_Z:
		{	//Deacuerdo a la escala se realiza la conversion
			switch (ptrMPUAccel->fullScaleGYRO)
			{
				case GYRO_250 :{ res = aux/GYRO_250_SENS; break;}
				case GYRO_500 :{ res = aux/GYRO_500_SENS; break;}
				case GYRO_1000 :{ res = aux/GYRO_1000_SENS; break;}
				case GYRO_2000 :{ res = aux/GYRO_2000_SENS; break;}
				default:{ break; }
			}
			break;
		}
		default:{ break; }
	}
	//Retornamos valor
	return res;
}

//Funcion para en el modo de calibracion para la lectura de dos registros que compone uno de los ejes de acelerometro o del giroscopio
int16_t readCalibrationMPU(MPUAccel_Handler_t *ptrMPUAccel, uint8_t elementRead)
{
	//Variable para guardar la  direccion de los dos registros a leer
	uint8_t address_H = 0;
	uint8_t address_L = 0;
	//Creamos las variables donde almacenamos todos los datos
	uint16_t aux_H = 0;
	uint16_t aux_L = 0;
	int16_t   aux  = 0;
	//Seleccionamos la direccion de los registros a Leer
	switch(elementRead)
	{
		case READ_ACCEL_X:{address_H = ACCEL_XOUT_H, address_L = ACCEL_XOUT_L; break;}
		case READ_ACCEL_Y:{address_H = ACCEL_YOUT_H, address_L = ACCEL_YOUT_L; break;}
		case READ_ACCEL_Z:{address_H = ACCEL_ZOUT_H, address_L = ACCEL_ZOUT_L; break;}
		case READ_GYRO_X:{address_H = GIRO_XOUT_H, address_L = GIRO_XOUT_L; break;}
		case READ_GYRO_Y:{address_H = GIRO_YOUT_H, address_L = GIRO_YOUT_L; break;}
		case READ_GYRO_Z:{address_H = GIRO_ZOUT_H, address_L = GIRO_ZOUT_L; break;}
		default:{ break; }
	}
	//Realizamos la medicion de la magnitud fisica en el respectivo eje
	aux_H = i2c_ReadSingleRegister(ptrMPUAccel->ptrI2Chandler, address_H);
	aux_L = i2c_ReadSingleRegister(ptrMPUAccel->ptrI2Chandler, address_L);
	//Juntamos ambos bytes en un solo numero para tener la lectura completa
	aux = (int16_t) ((aux_H << 8) | (aux_L));
	//Realizamos la conversion de los bytes al valor de magnitud fisica en el respectivo eje

	//Retornamos valor
	return aux;
}


//Funcion para la calibracion de las mediciones realizadas
float calibrationMPU(MPUAccel_Handler_t *ptrMPUAccel, uint8_t elementCalibration)
{
	//Variables la ejecucion de la calibracion
	int64_t aux_sum = 0;
	uint16_t num_samples = 200;
	float offset = 0;
	//Realizacion acumulacion de valores
	for (int i = 0; i < num_samples; i++)
	{
		//Lectura del registro respectivo y acumulacion
		aux_sum += readCalibrationMPU(ptrMPUAccel, elementCalibration);
		//Pausa
		delay_ms(1);
	}
	//Promedio del eje medido
	 offset = aux_sum/num_samples;
	//Retornar valor
	return offset;
}


//Funcion para calcular el nuevo angulo de uno de los ejes del girsocopio
float getAngle(MPUAccel_Handler_t *ptrMPUAccel, uint64_t *time_pre, float ang_init, uint8_t axis, int16_t offset_Axis)
{
	//Lectura velocidad angular
	float w_Angular= readMPU(ptrMPUAccel, axis, offset_Axis);
	//Calculo del tiempo trascurrido tras el ultima leida
	uint64_t time_now = getTicksMs();
	uint16_t dt =  time_now - *time_pre;
	*time_pre = time_now;
	//Calculo angulo
	float ang = ang_init + (w_Angular*dt)/1000;

	return ang;
}

