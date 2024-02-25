/*
 * RTCDriver.c
 *
 *  Created on: 4/06/2023
 *      Author: julian
 */

#include <RTCDriver.h>

//Variables auxiliares que almacenan lso resgistros sombras leidos
uint32_t auxregTR = 0;
uint32_t auxregDR = 0;

void configRTC(RTC_Handler_t *rtcConfig)
{
	//---------------------------------1) Activamos la señal de Reloj------------------------------------------
	//Registro: APB1ENR

	//Activamos la señal de reloj para acceder a los registros del RTC
	RCC->APB1ENR &= ~RCC_APB1ENR_PWREN;
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	//-------------------2) Activamos el acceso a los registros de RTC Y RTC BR--------------------------------
	//Registro: PWR_CR

	PWR->CR &= ~PWR_CR_DBP;
	PWR->CR |= PWR_CR_DBP;

	//---------------------3) Seleccionamos LSE como la fuente de reloj del RTC---------------------------------
	//Registro: BDCR

	RCC->BDCR &= ~RCC_BDCR_RTCSEL_Msk;
	RCC->BDCR |= 0b01<<RCC_BDCR_RTCSEL_Pos;

	//---------------------------------4) Activamos el clock LSE--------------------------------------------
	//Registro: BDCR

	RCC->BDCR &= ~RCC_BDCR_LSEON;
	RCC->BDCR |= RCC_BDCR_LSEON;

	//Espereamos a que se active el LSE
	while(!(RCC->BDCR & RCC_BDCR_LSERDY))
	{
		__NOP();
	}

	//---------------------------------5) Activamos el RTC--------------------------------------------
	//Registro: BDCR

	RCC->BDCR &= ~RCC_BDCR_RTCEN;
	RCC->BDCR |= RCC_BDCR_RTCEN;

	//----------------------5) Desbloqueo de la proteccion de los registros--------------------------------------------
	//Registro: WPR

	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	//-----------------------------6) Configuracion del Calender--------------------------------------------
	//Registro: ISR
	//Registro: PRER
	//Registro: TR
	//Registro: DR

	//-------a) Inicializamos el modo de configuracion------
	RTC->ISR &= ~RTC_ISR_INIT;
	RTC->ISR |= RTC_ISR_INIT;
	//Esperamos que se inicialice el modo
	while(!(RTC->ISR & RTC_ISR_INITF))
	{
		__NOP();
	}
	//-------b) Configuramos los preescales------
	RTC->PRER = 0; //Limpiamos el registro
//	RTC->PRER |= 256<<RTC_PRER_PREDIV_S_Pos;
//	RTC->PRER |= 128<<RTC_PRER_PREDIV_A_Pos;
	RTC->PRER |= RTC_PRER_PREDIV_A;
	RTC->PRER |= 0xFF<<RTC_PRER_PREDIV_S_Pos;

	//-------c) Especificamos el formato de horas------
	RTC->CR &= ~RTC_CR_FMT;	 //Definimos el formato de 24 horas
	//-------d) Actviamos el acceso al los registros sombra------
	RTC->CR |= RTC_CR_BYPSHAD;
	//-------e) Definimos los minutos,horas,segundos, etc ------
	timeConfigRTC(rtcConfig);
	dateConfigRTC(rtcConfig);
	//-------f) desactivamos el acceso al los registros sombra------
	RTC->CR &= ~RTC_CR_BYPSHAD;
	//-------g) Finalizamos el modo de configuracion------
	RTC->ISR &= ~RTC_ISR_INIT;
	//----------------------5) Activamos la proteccion de los registros--------------------------------------------
	//Registro: WPR
	RTC->WPR = 0xFF;
}


//----------------------Funciones de configuracion--------------------------------------------
//Funcion que cambia los registros corrrespondientes al tiempo
void timeConfigRTC(RTC_Handler_t *rtcConfig)
{
	//Reiniciamos el registro
	RTC->TR = 0;
	//segundos
	uint8_t st = (rtcConfig->seg)/10;
	RTC->TR |= st<<RTC_TR_ST_Pos;
	RTC->TR |= (rtcConfig->seg-st*10)<<RTC_TR_SU_Pos;
	//minutos
	uint8_t mnt = (rtcConfig->min)/10;
	RTC->TR |= mnt<<RTC_TR_MNT_Pos;
	RTC->TR |= (rtcConfig->min-mnt*10)<<RTC_TR_MNU_Pos;
	//horas
	uint8_t ht = (rtcConfig->hour)/10;
	RTC->TR |= ht<<RTC_TR_HT_Pos;
	RTC->TR |= (rtcConfig->hour-ht*10)<<RTC_TR_HU_Pos;
}

//Funcion que cambia los registros corrrespondientes a la fecha
void dateConfigRTC(RTC_Handler_t *rtcConfig)
{
	//Reiniciamos el registro
	RTC->DR = 0;
	//dia del mes
	uint8_t dt = (rtcConfig->date)/10;
	RTC->DR |= dt<<RTC_DR_DT_Pos;
	RTC->DR |= (rtcConfig->date-dt*10)<<RTC_DR_DU_Pos;
	//mes
	uint8_t mt = (rtcConfig->month)/10;
	RTC->DR |= mt<<RTC_DR_MT_Pos;
	RTC->DR |= (rtcConfig->month-mt*10)<<RTC_DR_MU_Pos;
	//dias de la semana
	RTC->DR |= (rtcConfig->dayWeek)<<RTC_DR_WDU_Pos;
	//año
	uint8_t yt = (rtcConfig->year)/10;
	RTC->DR |= yt<<RTC_DR_YT_Pos;
	RTC->DR |= (rtcConfig->year-yt*10)<<RTC_DR_YU_Pos;
}


//Funcion que actualiza el tiempo
void updatetimeConfigRTC(RTC_Handler_t *rtcConfig, uint8_t newhour, uint8_t newmin, uint8_t newseg)
{
	//Desbloqueo de la proteccion de los registros
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	//Inicializamos el modo de configuracion
	RTC->ISR &= ~RTC_ISR_INIT;
	RTC->ISR |= RTC_ISR_INIT;
	//Esperamos que se inicialice el modo
	while(!(RTC->ISR & RTC_ISR_INITF))
	{
		__NOP();
	}
	//Establecemos los nuevos valores
	rtcConfig->hour = newhour;
	rtcConfig->min = newmin;
	rtcConfig->seg = newseg;
	//Actviamos el acceso al los registros sombra
	RTC->CR |= RTC_CR_BYPSHAD;
	//Cargamos los nuevos valores
	timeConfigRTC(rtcConfig);
	//Desactivamos el acceso al los registros sombra
	RTC->CR &= ~RTC_CR_BYPSHAD;
	//Finalizamos el modo de configuracion
	RTC->ISR &= ~RTC_ISR_INIT;
	//Activamos la proteccion de los registros
	RTC->WPR = 0xFF;
}

//Funcion que actualiza la fecha
void updateDateConfigRTC(RTC_Handler_t *rtcConfig, uint8_t newyear, uint8_t newmonth, uint8_t newdate, uint8_t newdayweek)
{
	//Desbloqueo de la proteccion de los registros
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	//Inicializamos el modo de configuracion
	RTC->ISR &= ~RTC_ISR_INIT;
	RTC->ISR |= RTC_ISR_INIT;
	//Esperamos que se inicialice el modo
	while(!(RTC->ISR & RTC_ISR_INITF))
	{
		__NOP();
	}
	//Establecemos los nuevos valores
	rtcConfig->year = newyear;
	rtcConfig->month = newmonth;
	rtcConfig->date = newdate;
	rtcConfig->dayWeek = newdayweek;
	//Actviamos el acceso al los registros sombra
	RTC->CR |= RTC_CR_BYPSHAD;
	//Cargamos los nuevos valores
	dateConfigRTC(rtcConfig);
	//Desactivamos el acceso al los registros sombra
	RTC->CR &= ~RTC_CR_BYPSHAD;
	//Finalizamos el modo de configuracion
	RTC->ISR &= ~RTC_ISR_INIT;
	//Activamos la proteccion de los registros
	RTC->WPR = 0xFF;
}

//----------------------Funciones de lectura del tiempo y fecha--------------------------------------------

//Funcion que lee el registro TR
void initTimeShowRTC(void)
{
	//Bandera que garantiza que se a realizado una copia a los registros sombra
	while(!(RTC->ISR & RTC_ISR_RSF))
	{
		__NOP();
	}
	//Almacenamos el registro en una variable auxiliar
	auxregTR = RTC->TR;
	//Limpiamos la bandera
	RTC->ISR &= ~RTC_ISR_RSF;
}

//Funcion que lee el registro DR
void initDateShowRTC(void)
{
	//Bandera que garantiza que se a realizado una copia a los registros sombra
	while(!(RTC->ISR & RTC_ISR_RSF))
	{
		__NOP();
	}
	//Almacenamos el registro en una variable auxiliar
	auxregDR = RTC->DR;
	//Limpiamos la bandera
	RTC->ISR &= ~RTC_ISR_RSF;
}


//Lectura de segundos
uint8_t segShowRTC(void)
{
	uint8_t seg = 0;
	//Obtenemos los segundos transcurridos
	seg = (uint8_t) ((auxregTR>>RTC_TR_ST_Pos)&0b111)*10 + ((auxregTR>>RTC_TR_SU_Pos)&0b1111);

	return seg;
}

//Lectura de minutos
uint8_t minShowRTC(void)
{
	uint8_t min = 0;
	//Obtenemos los minutos transcurridos
	min =  (uint8_t) ((auxregTR>>RTC_TR_MNT_Pos)&0b111)*10 + ((auxregTR>>RTC_TR_MNU_Pos)&0b1111);

	return min;
}

//Lectura de horas
uint8_t hourShowRTC(void)
{
	uint8_t hour = 0;
	//Obtenemos los horas transcurridos
	hour = (uint8_t) ((auxregTR>>RTC_TR_HT_Pos)&0b11)*10 + ((auxregTR>>RTC_TR_HU_Pos)&0b1111);

	return	hour;
}


//Lectura de dias de la semana
uint8_t dayweekShowRTC(void)
{
	uint8_t dweek = 0;
	//Obtenemos los dias de la semana transcurridos
	dweek = (uint8_t) ((auxregDR >> RTC_DR_WDU_Pos)&0b111);

	return dweek;
}

//Lectura de la fecha
uint8_t dateShowRTC(void)
{
	uint8_t date = 0;
	//Obtenemos de la fecha
	date = (uint8_t) ((auxregDR>>RTC_DR_DT_Pos)&0b11)*10 + ((auxregDR>>RTC_DR_DU_Pos)&0b1111);

	return date;
}

//Lectura del mes
uint8_t monthShowRTC(void)
{
	uint8_t month = 0;
	//Obtenemos del mes
	month = (uint8_t) ((auxregDR>>RTC_DR_MT_Pos)&0b1)*10 + ((auxregDR>>RTC_DR_MU_Pos)&0b1111);

	return month;
}

//Lectura del año
uint8_t yearShowRTC(void)
{
	uint8_t year = 0;
	//Obtenemos del año
	year  = (uint8_t) ((auxregDR>>RTC_DR_YT_Pos)&0b1111)*10 + ((auxregDR>>RTC_DR_YU_Pos)&0b1111);

	return year;
}
