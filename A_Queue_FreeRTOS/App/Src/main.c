/**
 ******************************************************************************
 * @file           : main.c
 * @author         : juloperag
 * @brief          : Configuracion Basica Proyecto
 ******************************************************************************
 */

//----------------------------------Inicio de definicion de librerias-------------------------------------------
#include <stdint.h>
#include <stm32f411xe.h>
#include <stdio.h>
#include <GPIOxDriver.h>
#include <BasicTimer.h>
#include <ExtiDriver.h>
#include <USARTxDriver.h>
#include <SysTickDriver.h>
#include <PwmDriver.h>
#include <I2CDriver.h>
#include <AdcDriver.h>
#include <dsp/basic_math_functions.h>

//-----------------------------------Fin de definicion de librerias------------------------------------------


//---------------------------Inicio de definicion de funciones y variables base----------------------------------

GPIO_Handler_t handler_BlinkyPin = {0};         //Definimos un elemento del tipo GPIO_Handler_t (Struct) para el LED
BasicTimer_Handler_t handler_BlinkyTimer ={0};  //Definimos un elemento del tipo BasicTimer_Handler_t (Struct)
void int_Hardware(void);                        //Definimos la cabecera para la configuracion

//---------------------------Fin de definicion de funciones y variables base----------------------------------


//--------------------------EXTI-------------------------------
GPIO_Handler_t handler_GPIO_Button = {0};     //Definicion un elemento del tipo EXTI_Config_t y PIO_Handler_t para el user boton
EXTI_Config_t handler_EXTI_Button ={0};

//--------------------------USART-------------------------------
GPIO_Handler_t handler_GPIO_USB_TX = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct) y USART_Handler_t para el uso del USB
GPIO_Handler_t handler_GPIO_USB_RX = {0};
USART_Handler_t handler_USART_USB = {0};
char charRead = 'w';                        //Variable que almacena el caracter leido
char sendMg[] = "Boton presionado \n";       //Definimos string
char bufferMsg[64] = {0};

//--------------------------PWM-------------------------------
GPIO_Handler_t handler_GPIO_PWM = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct), BasicTimer_Config_t  y PWM_Handler_t para el uso del PWM
BasicTimer_Handler_t handler_TIMER_PWM = {0};
PWM_Handler_t handler_PWM = {0};
uint8_t duttyporc = 10;                      //Variables para cambiar el duttycicle
uint8_t estado = 0;

//--------------------------ADC------------------------------
ADC_Config_t  handler_ADC_Channel0 = {0};     //Definimos un elemento del tipo ADC_Config_t (Struct) para la conversion ADC
uint16_t adcData = 0;                         //Variable que almacena el valor del ADC leido

//-------------------------I2C--------------------------
GPIO_Handler_t handler_GPIO_SCL_Acelerometro = {0};   //Definimos un elemento del tipo GPIO_Handler_t (Struct) y I2C_Handler_t para la comunicacion I2C
GPIO_Handler_t handler_GPIO_SDA_Acelerometro = {0};
I2C_Handler_t handler_I2C_Acelerometro = {0};
//-------Acelerometro
#define ACCEL_ADDRESSS  0b1101000;                   //Definicion de la direccion del Sclave
#define ACCEL_XOUT_H  0x3B                           //Definicion de la direccion de los registros del Sclave a usar
#define ACCEL_XOUT_L  0x3C
#define ACCEL_YOUT_H  0x3D
#define ACCEL_YOUT_L  0x3E
#define ACCEL_ZOUT_H  0x3F
#define ACCEL_ZOUT_L  0x40

#define PWR_MGMT_l  107                            //Hay un bit en dicho registro para aplicar reset
#define WHO_AM_I    117                            //Registro para verificar la identidad del equipo

void acelerometro_I2C(void);                       //Cabecera para la comunicacion I2C

int main(void)
{
	//Realizamos la configuracuion inicial
	int_Hardware();
	//Activamos el SysTick
	config_SysTick_ms();
	//Activamos el punto flotante por medio del registro especifico
	SCB->CPACR |= 0xF <<20;
	//Definimos para el PIN un 1 logico,
	GPIO_writePin (&handler_BlinkyPin, SET);

	while(1)
	{
		if(charRead != '\0')
		{
			if(charRead == '1')
			{
				//------------------------------Implementacion USART-Systick--------------------------------
				//Definimos el tiempo que a pasado desde que el programa empezo a ejecutarse
				uint32_t tiempo = (uint32_t) getTicksMs();
				//ese tiempo lo volvemos un string
				sprintf(bufferMsg,"Tiempo Transcurrido: %lu ms \n",tiempo);
				//Enviamos por puerto serial dicho string
				writeMsg(&handler_USART_USB, bufferMsg);

				//-----------------------------Implementacion USART-Librerias Externas--------------------------------
				//Definimos un arreglo que contiene numeros decimales
				float32_t value[2] = {-12.12, 2.14};
				float32_t valueAbs[2] = {0.0,0.0};
				//Aplicamos la funcion que se importo de la librerias CMSIS
				arm_abs_f32(value,valueAbs,2);
				//Enviamos un mensaje por el puerto Serial
				sprintf(bufferMsg,"El valor abs de %#.2f = %#.2f \n", value[0], valueAbs[0]);
				//Enviamos por puerto serial dicho string
				writeMsg(&handler_USART_USB, bufferMsg);

				//-----------------------------Implementacion USART-ADC--------------------------------
				sprintf(bufferMsg, "Data Canal 1: %u \n", adcData);
				writeMsg(&handler_USART_USB, bufferMsg);

				//reniciamos
				charRead = '\0';
			}
			else
			{
				acelerometro_I2C();
			}
		}
		else if(estado==1)
		{
			//Cambiamos el porcentaje del dutty
			updateDuttyCycle(&handler_PWM, duttyporc);
			estado = 0;
			//Leemos la coordenada en x del Giroscopio;
		}
		else
		{
			__NOP();
		}
	}

	return 0;
}



//------------------------------Inicio Configuracion del microcontrolador------------------------------------------
void int_Hardware(void)
{

	//-------------------Inicio de Configuracion GPIOx-----------------------

	//---------------------------BlinkyLed--------------------------------
	//---------------PIN: PA5----------------
	//Definimos el periferico GPIOx a usar.
	handler_BlinkyPin.pGPIOx = GPIOA;
	//Definimos el pin a utilizar
	handler_BlinkyPin.GPIO_PinConfig.GPIO_PinNumber = PIN_5; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_BlinkyPin, GPIO_MODE_OUT, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF0);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_BlinkyPin);

	//---------------------------USART--------------------------------
	//---------------PIN: PA2----------------
	//------------AF7: USART2_TX----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_USB_TX.pGPIOx = GPIOA;
	//Definimos el pin a utilizar
	handler_GPIO_USB_TX.GPIO_PinConfig.GPIO_PinNumber = PIN_2; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_USB_TX, GPIO_MODE_ALTFN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF7);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_USB_TX);

	//---------------PIN: PA3----------------
	//------------AF7: USART2_RX----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_USB_RX.pGPIOx = GPIOA;
	//Definimos el pin a utilizar
	handler_GPIO_USB_RX.GPIO_PinConfig.GPIO_PinNumber = PIN_3; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_USB_RX, GPIO_MODE_ALTFN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF7);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_USB_RX);

	//---------------------------PWM--------------------------------
	//---------------PIN: PB6----------------
	//------------AF2: TIM4_CH1----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_PWM.pGPIOx = GPIOB;
	//Definimos el pin a utilizar
	handler_GPIO_PWM.GPIO_PinConfig.GPIO_PinNumber = PIN_6; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_PWM, GPIO_MODE_ALTFN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF2);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_PWM);

	//---------------------------I2C--------------------------------
	//---------------PIN: PB8----------------
	//------------AF4: I2C1_SCL----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_SCL_Acelerometro.pGPIOx = GPIOB;
	//Definimos el pin a utilizar
	handler_GPIO_SCL_Acelerometro.GPIO_PinConfig.GPIO_PinNumber = PIN_8; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_SCL_Acelerometro, GPIO_MODE_ALTFN, GPIO_OTYPER_OPENDRAIN, GPIO_OSPEEDR_FAST, GPIO_PUPDR_NOTHING, AF4);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_SCL_Acelerometro);

	//---------------PIN: PB9----------------
	//------------AF4: I2C1_SDA----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_SDA_Acelerometro.pGPIOx = GPIOB;
	//Definimos el pin a utilizar
	handler_GPIO_SDA_Acelerometro.GPIO_PinConfig.GPIO_PinNumber = PIN_9; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_SDA_Acelerometro, GPIO_MODE_ALTFN, GPIO_OTYPER_OPENDRAIN, GPIO_OSPEEDR_FAST, GPIO_PUPDR_NOTHING, AF4);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_SDA_Acelerometro);

	//-------------------Fin de Configuracion GPIOx-----------------------

	//-------------------Inicio de Configuracion USARTx-----------------------

	//---------------USART2----------------
	//Definimos el periferico USARTx a utilizar
	handler_USART_USB.ptrUSARTx = USART2;
	//Definimos la configuracion del USART seleccionado
	handler_USART_USB.USART_Config.USART_mode = USART_MODE_RXTX ;           //USART_MODE_x  x-> TX, RX, RXTX, DISABLE
	handler_USART_USB.USART_Config.USART_baudrate = USART_BAUDRATE_9600;  //USART_BAUDRATE_x  x->9600, 19200, 115200
	handler_USART_USB.USART_Config.USART_parity= USART_PARITY_NONE;       //USART_PARITY_x   x->NONE, ODD, EVEN
	handler_USART_USB.USART_Config.USART_stopbits=USART_STOPBIT_1;         //USART_STOPBIT_x  x->1, 0_5, 2, 1_5
	handler_USART_USB.USART_Config.USART_enableIntRX = USART_RX_INTERRUP_ENABLE;   //USART_RX_INTERRUP_x  x-> DISABLE, ENABLE
	handler_USART_USB.USART_Config.USART_enableIntTX = USART_TX_INTERRUP_DISABLE;   //USART_TX_INTERRUP_x  x-> DISABLE, ENABLE
	//Cargamos la configuracion del USART especifico
	USART_Config(&handler_USART_USB);

	//-------------------Fin de Configuracion USARTx-----------------------

	//-------------------Inicio de Configuracion TIMx-----------------------

	//---------------TIM2----------------
	//Definimos el TIMx a usar
	handler_BlinkyTimer.ptrTIMx = TIM2;
	//Definimos la configuracion del TIMER seleccionado
	handler_BlinkyTimer.TIMx_Config.TIMx_periodcnt = BTIMER_PCNT_1ms; //BTIMER_PCNT_xus x->10,100/ BTIMER_PCNT_1ms
	handler_BlinkyTimer.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; // BTIMER_MODE_x x->UP, DOWN
	handler_BlinkyTimer.TIMx_Config.TIMX_period = 250;//Al definir 10us,100us el valor un multiplo de ellos, si es 1ms el valor es en ms
	handler_BlinkyTimer.TIMx_Config.TIMx_interruptEnable = INTERRUPTION_ENABLE; //INTERRUPTION_x  x->DISABLE, ENABLE
	//Cargamos la configuracion del TIMER especifico
	BasicTimer_Config(&handler_BlinkyTimer);

	//---------------TIM4----------------
	//Definimos el TIMx a usar
	handler_TIMER_PWM.ptrTIMx = TIM4;
	//Definimos la configuracion del TIMER seleccionado
	handler_TIMER_PWM.TIMx_Config.TIMx_periodcnt = BTIMER_PCNT_1ms; //BTIMER_PCNT_xus x->10,100/ BTIMER_PCNT_1ms
	handler_TIMER_PWM.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; // BTIMER_MODE_x x->UP, DOWN
	handler_TIMER_PWM.TIMx_Config.TIMX_period = 20;//Al definir 10us,100us el valor un multiplo de ellos, si es 1ms el valor es en ms
	handler_TIMER_PWM.TIMx_Config.TIMx_interruptEnable = INTERRUPTION_DISABLE; //INTERRUPTION_x  x->DISABLE, ENABLE
	//Cargamos la configuracion del TIMER especifico
	BasicTimer_Config(&handler_TIMER_PWM);

	//-------------------Fin de Configuracion TIMx-----------------------


	//-------------------Inicio de Configuracion EXTIx -----------------------

	//---------------PIN: PC13----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_Button.pGPIOx = GPIOC;
	//Definimos el pin a utilizar
	handler_GPIO_Button.GPIO_PinConfig.GPIO_PinNumber = PIN_13;
	//Definimos la posicion del elemento pGIOHandler.
	handler_EXTI_Button.pGPIOHandler = &handler_GPIO_Button;
	//Definimos el tipo de flanco
	handler_EXTI_Button.edgeType = EXTERNAL_INTERRUPP_FALLING_EDGE;
	//Cargamos la configuracion del EXTIx
	extInt_Config(&handler_EXTI_Button);

	//-------------------Fin de Configuracion EXTIx-----------------------

	//-------------------Inicio de Configuracion PWM_Channelx----------------------

	//---------------TIM4_Channel_1----------------
	//Definimos el TIMx a usar
	handler_PWM.ptrTIMx = TIM4;
	//Definimos la configuracion para el PWM
	handler_PWM.config.channel = PWM_CHANNEL_1;     //PWM_CHANNEL_x x->1,2,3,4
	handler_PWM.config.duttyCicle = 10;             //Valor del Dutty como multiplo del periodo del CNT
	handler_PWM.config.polarity = POLARITY_HIGH;    //POLARITY_x x-> HIGH, LOW
	//Cargamos la configuracion
	pwm_Config(&handler_PWM);
	//Activamos el PWM
	statusInOutPWM(&handler_PWM, CHANNEL_ENABLE);

	//---------------------Fin de Configuracion PWM_Channelx-----------------------

	//-------------------Inicio de Configuracion I2Cx----------------------

	//---------------I2C1----------------
	//Definimos el I2Cx a usar
	handler_I2C_Acelerometro.prtI2Cx = I2C1;
	//Definimos la configuracion para el I2C
	handler_I2C_Acelerometro.modeI2C = I2C_MODE_FM;               //I2C_MODE_x  x->SM,FM
	handler_I2C_Acelerometro.slaveAddress = ACCEL_ADDRESSS;       //Direccion del Sclave
	//Cargamos la configuracion
	i2c_Config(&handler_I2C_Acelerometro);

	//---------------------Fin de Configuracion I2Cx----------------------

	//-------------------Inicio configuracion ADC-----------------------

	//---------------Channel_0----------------
	 handler_ADC_Channel0.conversion.channelSequence_0 =  ADC_CHANNEL_0;
	 handler_ADC_Channel0.sampling.samplingPeriodChannel_0 = ADC_SAMPLING_PERIOD_84_CYCLES;      //ADC_SAMPLING_PERIOD_x_CYCLES  x-> 3, 15, 28, 56, 84, 112, 144, 480
	 handler_ADC_Channel0.resolution = ADC_RESOLUTION_12_BIT;                                    //ADC_RESOLUTION_x_BIT          x-> 8,6,10,12
	 handler_ADC_Channel0.dataAlignment =  ADC_ALIGNMENT_RIGHT;                                  //ADC_ALIGNMENT_x               x-> RIGHT, LEFT
	 handler_ADC_Channel0.externalEvent = ADC_EXTERNAL_EVENT_DISABLE;                            //ADC_EXTERNAL_EVENT_x          x-> DISABLE, RISING, FALLING, BOTH
	 handler_ADC_Channel0.typeEXTEvent =  ADC_EVENT_TIMER_1_CC1;                                 //ADC_EVENT_TIMER_x_y           x-> 1, 2, 3, 4, 5 | y-> Depende
	 adc_Config(&handler_ADC_Channel0);

	//-------------------Fin configuracion ADC-----------------------

}
//------------------------------Fin Configuracion del microcontrolador------------------------------------------


//----------------------------Inicio de la definicion de las funciones ISR---------------------------------------

//-------------------------BlinkyLed--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el TIM2
void BasicTimer2_Callback(void)
{
	GPIOxTooglePin(&handler_BlinkyPin);
	startSingleADC();
}

//-------------------------USARTRX--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el USART2
void BasicUSART2_Callback(void)
{
	charRead = getRxData();
}

//-------------------------ADC--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el ADC
void adcComplete_Callback(void)
{
	adcData = getADC();
}

//-------------------------UserButton--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el EXTI13
void callback_extInt13(void)
{
	writeMsg(&handler_USART_USB, sendMg);
	if(duttyporc<100)
	{
		duttyporc++;
	}
	else
	{
		__NOP();
	}
	estado = 1;
}

//----------------------------Fin de la definicion de las funciones ISR----------------------------------------

//----------------------------Inicio de la definicion de las funciones-----------------------------------------

void acelerometro_I2C(void)
{
	uint8_t i2cBuffer = 0;

	switch(charRead)
	{
	case 'w':
	{
		//Definimos un string
		sprintf(bufferMsg,"WHO AM I (r)?\n");
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);
		//Leemos el registro deseado del Sclave
		i2cBuffer = i2c_ReadSingleRegister(&handler_I2C_Acelerometro, WHO_AM_I);
		//Definimos un string
		sprintf(bufferMsg,"dataRead = 0x%x \n", (unsigned int) i2cBuffer);
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);
		charRead = '\0';
		break;
	}
	case 'p':
	{
		//Definimos un string
		sprintf(bufferMsg,"PWR_MGMT_l state (r)\n");
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);
		//Leemos el registro deseado del Sclave
		i2cBuffer = i2c_ReadSingleRegister(&handler_I2C_Acelerometro, PWR_MGMT_l);
		//Definimos un string
		sprintf(bufferMsg,"dataRead = 0x%x \n", (unsigned int) i2cBuffer);
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);
		charRead = '\0';
		break;
	}
	case 'r':
	{
		//Definimos un string
		sprintf(bufferMsg,"PWR_MGMT_l reset (r)\n");
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);
		//Escribimos en el registro deseado del Sclave
		i2c_WriteSingleRegister(&handler_I2C_Acelerometro, PWR_MGMT_l, 0x00);

		charRead = '\0';
		break;
	}
	case 'x':
	{
		//Definimos un string
		sprintf(bufferMsg,"Axis X data (r)\n");
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);
		//Leemos el registro correspondiente a la poscicion en X del acelerometro
		uint8_t AccelX_low = i2c_ReadSingleRegister(&handler_I2C_Acelerometro, ACCEL_XOUT_L);
		uint8_t AccelX_high = i2c_ReadSingleRegister(&handler_I2C_Acelerometro, ACCEL_XOUT_H);
		uint16_t AccelX = AccelX_high<<8 | AccelX_low;
		//Definimos un string
		sprintf(bufferMsg,"AccelX = %d \n", (int) AccelX );
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);

		charRead = '\0';
		break;
	}
	case 'y':
	{
		//Definimos un string
		sprintf(bufferMsg,"Axis Y data (r)\n");
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);
		//Leemos el registro correspondiente a la poscicion en X del acelerometro
		uint8_t AccelY_low = i2c_ReadSingleRegister(&handler_I2C_Acelerometro, ACCEL_YOUT_L);
		uint8_t AccelY_high = i2c_ReadSingleRegister(&handler_I2C_Acelerometro, ACCEL_YOUT_H);
		uint16_t AccelY = AccelY_high<<8 | AccelY_low;
		//Definimos un string
		sprintf(bufferMsg,"AccelY = %d \n", (int) AccelY);
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);

		charRead = '\0';
		break;
	}
	case 'z':
	{
		//Definimos un string
		sprintf(bufferMsg,"Axis Z data (r)\n");
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);
		//Leemos el registro correspondiente a la poscicion en X del acelerometro
		uint8_t AccelZ_low = i2c_ReadSingleRegister(&handler_I2C_Acelerometro, ACCEL_ZOUT_L);
		uint8_t AccelZ_high = i2c_ReadSingleRegister(&handler_I2C_Acelerometro, ACCEL_ZOUT_H);
		uint16_t AccelZ = AccelZ_high<<8 | AccelZ_low;
		//Definimos un string
		sprintf(bufferMsg,"AccelZ = %d \n", (int) AccelZ );
		//Enviamos por puerto serial dicho string
		writeMsg(&handler_USART_USB, bufferMsg);

		charRead = '\0';
		break;
	}
	default:
	{
		charRead = '\0';
		break;
	}
	}
}
//--------------------------  --Fin de la definicion de las funciones------------------------------------------

