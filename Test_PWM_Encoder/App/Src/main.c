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
#include <PwmDriver.h>
#include <arm_math.h>


//-----------------------------------Fin de definicion de librerias------------------------------------------


//---------------------------Inicio de definicion de funciones y variables base----------------------------------

GPIO_Handler_t handler_BlinkyPin = {0};         //Definimos un elemento del tipo GPIO_Handler_t (Struct) para el LED
BasicTimer_Handler_t handler_BlinkyTimer ={0};  //Definimos un elemento del tipo BasicTimer_Handler_t (Struct)
void int_Hardware(void);                        //Definimos la cabecera para la configuracion

//---------------------------Fin de definicion de funciones y variables base----------------------------------


//--------------------------Fotocopuerta-------------------------------
//----------Derecho--------------
GPIO_Handler_t handler_GPIO_FotoR = {0};     //Definicion un elemento del tipo EXTI_Config_t y GPIO_Handler_t para el user boton
EXTI_Config_t  handler_EXTI_FotoR ={0};
//----------Izquierdo------------
GPIO_Handler_t handler_GPIO_FotoL = {0};     //Definicion un elemento del tipo EXTI_Config_t y GPIO_Handler_t para el user boton
EXTI_Config_t  handler_EXTI_FotoL ={0};

//--------------------------USART-------------------------------
GPIO_Handler_t handler_GPIO_USB_TX = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct) y USART_Handler_t para el uso del USB
GPIO_Handler_t handler_GPIO_USB_RX = {0};
USART_Handler_t handler_USART_USB = {0};
char charRead = 'w';                        //Variable que almacena el caracter leido
char sendMg[] = "Boton presionado \n";       //Definimos string
char bufferMsg[64] = {0};

//--------------------------Motor-------------------------------
//------------Derecho--------------
GPIO_Handler_t handler_GPIO_MotorR = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct), BasicTimer_Config_t  y PWM_Handler_t para el uso del Motor R
GPIO_Handler_t handler_GPIO_MotorR_IN = {0};
GPIO_Handler_t handler_GPIO_MotorR_EN = {0};
PWM_Handler_t handler_PWM_MotorR = {0};
//------------Izquierdo-----------------
GPIO_Handler_t handler_GPIO_MotorL = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct), BasicTimer_Config_t  y PWM_Handler_t para el uso del Motor L
GPIO_Handler_t handler_GPIO_MotorL_IN = {0};
GPIO_Handler_t handler_GPIO_MotorL_EN = {0};
PWM_Handler_t handler_PWM_MotorL = {0};
//-----------General-------------------
BasicTimer_Handler_t handler_TIMER_Motor = {0};
BasicTimer_Handler_t handler_TIMER_study = {0}; //Definimos un elementos de tipo BasicTimer_Handler_t para relizar el test
uint8_t duttyporc = 10;                      //Variables para cambiar el duttycicle
uint8_t estado = 0;

//------------------Definiciones generales----------------------------------
//-----Cabeceras de funciones----
void recepcionCommand(void);             //Funcion que recibe los caracteres del comando recibido
void runCommand(char *prtcommand);       //Funcion que ejecuta el comando ingresando
void status_study(uint8_t status);
void change_dutty(void);
//-----Variables study encoder---
GPIO_Handler_t *handler_GPIO_Motor_EN_Test ={0}; //Handler que se refiere a los motores para el test
GPIO_Handler_t *handler_GPIO_Motor_IN_Test ={0};
PWM_Handler_t *handler_PWM_Motor_Test = {0};
uint8_t flag_study = 0;                     //Bandera que indica la ejecucion del study
uint16_t frequency[3] = {4000, 2000, 1000}; //Arreglo de frecuencias para el desarrollo del study, estandar: 25,50,100 Hz
uint8_t index_count = 0;                  //
uint8_t index_fre = 0;                    //Indice del arreglo de las frecuencias
uint16_t count_for_dutty[3][21]={0};      //Definimos una matriz para almacenar conteos por valor de dutty
uint16_t count_foto = 0;                  //Variable que almacena las interrupciones de la forocompuerata
uint16_t count_foto_R = 0;
uint16_t count_foto_L = 0;
uint8_t value_dutty = 0;                  //Porcentaje del dutty del pwm en el test del encoder
uint8_t value_dutty_estatico = 50;        //Valor de dutty que se utiliza en la prueba estatica
uint16_t value_frequency_estatico = 4000;  //Valor de frecuencia que se utiuliza en la prueba estatica
uint8_t flag_dir = 1;                      //Variable para configurar la direccion de los motores
uint8_t flag_dir_M1 = 1;				  //Variable que guarda del motor
uint8_t flag_dir_M2 = 1;				  //Variable que guarda del motor

//-----Variables de la recepcion de comandos----
uint8_t commandComplete = 1;           //Bandera que indica si el comando esta completo
uint8_t counterRecepcion = 0;          //Variable para la posicion del arrelgo
char bufferRecepcion[64] = {0};        //Arreglo que almacena el comando ingresado


int main(void)
{
	//Realizamos la configuracuion inicial
	int_Hardware();
	//Configuracion inicial Motor
	GPIO_writePin (&handler_GPIO_MotorR_IN, SET);
	GPIO_writePin (&handler_GPIO_MotorL_IN, SET);
	GPIO_writePin (&handler_GPIO_MotorR_EN, SET);
	GPIO_writePin (&handler_GPIO_MotorL_EN, SET);
	//Definimos el motor derecho como el que se movera
	handler_GPIO_Motor_EN_Test = &handler_GPIO_MotorR_EN;
	handler_PWM_Motor_Test = &handler_PWM_MotorR;
	handler_GPIO_Motor_IN_Test = &handler_GPIO_MotorR_IN;
	flag_dir  = flag_dir_M1;

	while(1)
	{
		///Verificamos para ejecuta el comando ingresando
		if(commandComplete==1)
		{
			runCommand(bufferRecepcion);
			//Reniciamos la variable
			commandComplete=0;
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

	//---------------------------Motor Derecho--------------------------------
	//----------------------PWM--------------------
	//---------------PIN: PA0----------------
	//------------AF2: TIM5_CH1----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_MotorR.pGPIOx = GPIOA;
	//Definimos el pin a utilizar
	handler_GPIO_MotorR.GPIO_PinConfig.GPIO_PinNumber = PIN_0; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_MotorR, GPIO_MODE_ALTFN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF2);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_MotorR);
	//---------------Direccion--------------------
	//---------------PIN: PC12----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_MotorR_IN.pGPIOx = GPIOC;
	//Definimos el pin a utilizar
	handler_GPIO_MotorR_IN.GPIO_PinConfig.GPIO_PinNumber = PIN_12; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_MotorR_IN, GPIO_MODE_OUT, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF0);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_MotorR_IN);
	//------------Enable------------------------
	//---------------PIN: PC10----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_MotorR_EN.pGPIOx = GPIOC;
	//Definimos el pin a utilizar
	handler_GPIO_MotorR_EN.GPIO_PinConfig.GPIO_PinNumber = PIN_10; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_MotorR_EN, GPIO_MODE_OUT, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF0);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_MotorR_EN);

	//---------------------------Motor Izquierdo--------------------------------
	//----------------------PWM--------------------
	//---------------PIN: PA1----------------
	//------------AF2: TIM5_CH2----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_MotorL.pGPIOx = GPIOA;
	//Definimos el pin a utilizar
	handler_GPIO_MotorL.GPIO_PinConfig.GPIO_PinNumber = PIN_1; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_MotorL, GPIO_MODE_ALTFN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF2);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_MotorL);
	//---------------Direccion--------------------
	//---------------PIN: PD2----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_MotorL_IN.pGPIOx = GPIOD;
	//Definimos el pin a utilizar
	handler_GPIO_MotorL_IN.GPIO_PinConfig.GPIO_PinNumber = PIN_2; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_MotorL_IN, GPIO_MODE_OUT, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF0);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_MotorL_IN);
	//------------Enable------------------------
	//---------------PIN: PC11----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_MotorL_EN.pGPIOx = GPIOC;
	//Definimos el pin a utilizar
	handler_GPIO_MotorL_EN.GPIO_PinConfig.GPIO_PinNumber = PIN_11; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_MotorL_EN, GPIO_MODE_OUT, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF0);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_MotorL_EN);



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
	handler_USART_USB.USART_Config.USART_enableIntTX = USART_TX_INTERRUP_ENABLE;   //USART_TX_INTERRUP_x  x-> DISABLE, ENABLE
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

	//---------------TIM3----------------
	//Definimos el TIMx a usar
	handler_TIMER_study.ptrTIMx = TIM3;
	//Definimos la configuracion del TIMER seleccionado
	handler_TIMER_study.TIMx_Config.TIMx_periodcnt = BTIMER_PCNT_1ms; //BTIMER_PCNT_xus x->10,100/ BTIMER_PCNT_1ms
	handler_TIMER_study.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; // BTIMER_MODE_x x->UP, DOWN
	handler_TIMER_study.TIMx_Config.TIMX_period = 10000;//Al definir 10us,100us el valor un multiplo de ellos, si es 1ms el valor es en ms
	handler_TIMER_study.TIMx_Config.TIMx_interruptEnable = INTERRUPTION_DISABLE; //INTERRUPTION_x  x->DISABLE, ENABLE
	//Cargamos la configuracion del TIMER especifico
	BasicTimer_Config(&handler_TIMER_study);

	//---------------TIM5----------------
	//Definimos el TIMx a usar
	handler_TIMER_Motor.ptrTIMx = TIM5;
	//Definimos la configuracion del TIMER seleccionado
	handler_TIMER_Motor.TIMx_Config.TIMx_periodcnt = BTIMER_PCNT_10us; //BTIMER_PCNT_xus x->10,100/ BTIMER_PCNT_1ms
	handler_TIMER_Motor.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; // BTIMER_MODE_x x->UP, DOWN
	handler_TIMER_Motor.TIMx_Config.TIMX_period = 100;//Al definir 10us,100us el valor un multiplo de ellos, si es 1ms el valor es en ms
	handler_TIMER_Motor.TIMx_Config.TIMx_interruptEnable = INTERRUPTION_DISABLE; //INTERRUPTION_x  x->DISABLE, ENABLE
	//Cargamos la configuracion del TIMER especifico
	BasicTimer_Config(&handler_TIMER_Motor);

	//-------------------Fin de Configuracion TIMx-----------------------


	//-------------------Inicio de Configuracion EXTIx -----------------------

	//---------------PIN: PC1----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_FotoR.pGPIOx = GPIOC;
	//Definimos el pin a utilizar
	handler_GPIO_FotoR.GPIO_PinConfig.GPIO_PinNumber = PIN_1;
	//Definimos la posicion del elemento pGIOHandler.
	 handler_EXTI_FotoR.pGPIOHandler = &handler_GPIO_FotoR;
	//Definimos el tipo de flanco
	 handler_EXTI_FotoR.edgeType = EXTERNAL_INTERRUPP_FALLING_EDGE;
	//Cargamos la configuracion del EXTIx
	extInt_Config(& handler_EXTI_FotoR);

	//---------------PIN: PC3----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_FotoL.pGPIOx = GPIOC;
	//Definimos el pin a utilizar
	handler_GPIO_FotoL.GPIO_PinConfig.GPIO_PinNumber = PIN_3;
	//Definimos la posicion del elemento pGIOHandler.
	 handler_EXTI_FotoL.pGPIOHandler = &handler_GPIO_FotoL;
	//Definimos el tipo de flanco
	 handler_EXTI_FotoL.edgeType = EXTERNAL_INTERRUPP_FALLING_EDGE;
	//Cargamos la configuracion del EXTIx
	extInt_Config(& handler_EXTI_FotoL);


	//-------------------Fin de Configuracion EXTIx-----------------------

	//-------------------Inicio de Configuracion PWM_Channelx----------------------

	//---------------TIM5_Channel_1----------------
	//Definimos el TIMx a usar
	handler_PWM_MotorR.ptrTIMx = TIM5;
	//Definimos la configuracion para el PWM
	handler_PWM_MotorR.config.channel = PWM_CHANNEL_1;     //PWM_CHANNEL_x x->1,2,3,4
	handler_PWM_MotorR.config.duttyCicle = 10;             //Valor del Dutty como multiplo del periodo del CNT
	handler_PWM_MotorR.config.polarity = POLARITY_LOW;    //POLARITY_x x-> HIGH, LOW
	//Cargamos la configuracion
	pwm_Config(&handler_PWM_MotorR);
	//Activamos el PWM
	statusInOutPWM(&handler_PWM_MotorR, CHANNEL_ENABLE);

	//---------------TIM5_Channel_2----------------
	//Definimos el TIMx a usar
	handler_PWM_MotorL.ptrTIMx = TIM5;
	//Definimos la configuracion para el PWM
	handler_PWM_MotorL.config.channel = PWM_CHANNEL_2;     //PWM_CHANNEL_x x->1,2,3,4
	handler_PWM_MotorL.config.duttyCicle = 10;             //Valor del Dutty como multiplo del periodo del CNT
	handler_PWM_MotorL.config.polarity = POLARITY_LOW;    //POLARITY_x x-> HIGH, LOW
	//Cargamos la configuracion
	pwm_Config(&handler_PWM_MotorL);
	//Activamos el PWM
	statusInOutPWM(&handler_PWM_MotorL, CHANNEL_ENABLE);

	//---------------------Fin de Configuracion PWM_Channelx-----------------------


}
//------------------------------Fin Configuracion del microcontrolador------------------------------------------


//----------------------------Inicio de la definicion de las funciones ISR---------------------------------------

//-------------------------BlinkyLed--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el TIM2
void BasicTimer2_Callback(void)
{
	GPIOxTooglePin(&handler_BlinkyPin);
}

//-------------------------study--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el TIM2
void BasicTimer3_Callback(void)
{
	if (value_dutty<20)
	{
		//Guardamos el numero de conteos de la fotocompuerta
		count_for_dutty[index_fre][index_count] = count_foto;
		//Reiniciamos la variable
		count_foto = 0;
		//Aumentamos el valor de las variables
		index_count++;
		value_dutty++;
		//Actualizamos el valor del dutty
		updateDuttyCyclePercentage(handler_PWM_Motor_Test, value_dutty);
	}
	else
	{
		//Guardamos el numero de conteos de la fotocompuerta
		count_for_dutty[index_fre][index_count] = count_foto;
		//Aumentamos e indice de las freuencias
		index_fre++;
		if(index_fre<3)
		{
			//Reiniciamos el valor de las variabels
			value_dutty = 0;
			index_count = 0;
			count_foto = 0;
			//Actualizamos el valor del dutty
			updateDuttyCyclePercentage(handler_PWM_Motor_Test, value_dutty);
			//Actualizamos la frecuencia del timer
			updateFrequencyTimer(&handler_TIMER_Motor, frequency[index_fre]);
		}
		else
		{
			status_study(RESET);
		}
	}
}

//-------------------------USARTRX--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el USART2
void BasicUSART2_Callback(void)
{
	//Guardamos el caracter recibido
	charRead = getRxData();
	//Verificar
	if(charRead=='-' || charRead=='+')
	{
		//Cambiar dutty de a uno
		change_dutty();
	}
	else
	{
		//Funcion que almacena los caracteres del comando recibido
		recepcionCommand();
	}
}


//-------------------------Fotocompuerta--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el EXTI13 y EXTI13
void callback_extInt1(void)
{
	count_foto_R++;
}
void callback_extInt3(void)
{
	count_foto_L++;
}

//----------------------------Fin de la definicion de las funciones ISR----------------------------------------

//----------------------------Inicio de la definicion de las funciones de los comandos----------------------------------------

//Funcion que recibe los caracteres del comando recibido
void recepcionCommand(void)
{
	if(charRead == '@')
	{
		//Almacenamos el elemento nulo
		bufferRecepcion[counterRecepcion] = '\0';
		//Establecemos la bandera como alta
		commandComplete = 1;
		//Reiniciamos la variable
		counterRecepcion = 0;
		//Reiniciamos la variable
	}
	else
	{
		//Almacenamos los caracteres del comando en un arrelgo
		bufferRecepcion[counterRecepcion] = charRead;
		//Aumentamos en uno la posicion del arreglo
		counterRecepcion++;
		//Reiniciamos la variable
	}

}

//Funcion que ejecuta el comando ingresando
void runCommand(char *prtcommand)
{
	//Variables para almacenar los elmentos que entrega el comando luego de ser divididos por la funcion sscanf
	char cmd[64]= {0};
	int firtsParameter = 0;
	int secondParameter = 0;
	int thirdParameter = 0;
	char bufferMsg[64]= {0};

	//Funcion que lee la cadena de caracteres y la divide en los elementos definidos
	sscanf(prtcommand, "%s %u %u %u", cmd, &firtsParameter, &secondParameter, &thirdParameter);

	//Imprime lista que muestra los comandos que tiene el dispositivo
	if(strcmp(cmd, "help")==0)
	{
		writeMsgForTXE(&handler_USART_USB, "Help Menu: \n");
		writeMsgForTXE(&handler_USART_USB, "1) help  ---Imprime lista de comandos. \n");
		writeMsgForTXE(&handler_USART_USB, "2) Frequency # # # ---Definir los 3 valores de las frecuencias para el study en Hz. \n");
		writeMsgForTXE(&handler_USART_USB, "3) Motor # ---Elige el tipo de motor para el test, 1:Derecho, 2:Izquierdo \n");
		writeMsgForTXE(&handler_USART_USB, "4) sON --- Enciende el motor seleccionado en la prueba estatica \n");
		writeMsgForTXE(&handler_USART_USB, "5) sOFF --- Apaga el motor seleccionado prueba estatica \n");
		writeMsgForTXE(&handler_USART_USB, "6) sDireccion --- Cambiar la direccion en la prueba estatica \n");
		writeMsgForTXE(&handler_USART_USB, "7) sDutty # --- Cambiar el valor del dutty en la prueba estatica #/0-100 \n");
		writeMsgForTXE(&handler_USART_USB, "8) sFrequency # --- cambiar el valor de la frecuenciea en la prueba estatica en HZ \n");
		writeMsgForTXE(&handler_USART_USB, "9) sCount --- Mostrar el numero de cuentas \n");
		writeMsgForTXE(&handler_USART_USB, "10) sCountInit--- Iniciar el numero de cuentas \n");
		writeMsgForTXE(&handler_USART_USB, "11) Start ---Inicializa el estudio \n");
		writeMsgForTXE(&handler_USART_USB, "12) Stop ---Para el estudio en medio de la ejecucion \n");
		writeMsgForTXE(&handler_USART_USB, "13) Print ---Imprime los valores del conteo de interrupciones en el estudio \n");
	}


	//Definimos el valor de las frecuencias para el study
	else if (strcmp(cmd, "Frequency") == 0)
	{

		frequency[0] = 100000/firtsParameter;
		frequency[1] = 100000/secondParameter;
		frequency[2] = 100000/thirdParameter;
	}
	//Defino el tipo de motor a usar para la prueba
	else if (strcmp(cmd, "Motor") == 0)
	{
		//Guardamos el handler de forma global
		if(firtsParameter==1)
		{
			handler_GPIO_Motor_EN_Test = &handler_GPIO_MotorR_EN;
			handler_PWM_Motor_Test = &handler_PWM_MotorR;
			handler_GPIO_Motor_IN_Test = &handler_GPIO_MotorR_IN;
			flag_dir  = flag_dir_M1;
			//Imprimimos mensaje
			writeMsgForTXE(&handler_USART_USB, "Motor 1 seleccionado \n");
		}
		else if(firtsParameter==2)
		{
			handler_GPIO_Motor_EN_Test = &handler_GPIO_MotorL_EN;
			handler_PWM_Motor_Test = &handler_PWM_MotorL;
			handler_GPIO_Motor_IN_Test = &handler_GPIO_MotorL_IN;
			flag_dir  = flag_dir_M2;
			//Imprimimos mensaje
			writeMsgForTXE(&handler_USART_USB, "Motor 2 seleccionado \n");
		}
		else
		{
			//Imprimimos mensaje
			writeMsgForTXE(&handler_USART_USB, "Valor no valido de motor \n");
		}
	}
	//Encendemos el motor
	else if (strcmp(cmd, "sON") == 0)
	{
		//Actualizamos el valor del dutty y frecuencia
		updateDuttyCyclePercentage(handler_PWM_Motor_Test, value_dutty_estatico);
		updateFrequencyTimer(&handler_TIMER_Motor, value_frequency_estatico);
		//Activamos el motor
		GPIO_writePin (handler_GPIO_Motor_EN_Test, RESET);
		//Imprimimos mensaje
		writeMsgForTXE(&handler_USART_USB, "Motor Encendido \n");
	}
	//Apagamos el motor
	else if (strcmp(cmd, "sOFF") == 0)
	{
		//Desactivamos el motor
		GPIO_writePin (handler_GPIO_Motor_EN_Test, SET);
		//Imprimimos mensaje
		writeMsgForTXE(&handler_USART_USB, "Motor Apagado \n");
	}
	//Cambiar direccion
	else if (strcmp(cmd, "sDireccion") == 0)
	{
		if(handler_GPIO_Motor_IN_Test==&handler_GPIO_MotorR_IN)
		{
			//Cambiamos el valor de la bandera
			flag_dir_M1 = (~flag_dir_M1)&(0x01);
			//Guardamos el valor
			flag_dir = flag_dir_M1;
		}
		else
		{
			//Cambiamos el valor de la bandera
			flag_dir_M2 = (~flag_dir_M2)&(0x01);
			//Guardamos el valor
			flag_dir = flag_dir_M2;
		}

		//Cambiamos la direccion del motor
		GPIO_writePin(handler_GPIO_Motor_IN_Test, flag_dir&SET);
		statusPolarityPWM(handler_PWM_Motor_Test, flag_dir&SET);
	}
	//Cambiar dutty en prueba estatica
	else if (strcmp(cmd, "sDutty") == 0)
	{
		//Guardamos valor
		value_dutty_estatico = firtsParameter;
		//Actualizamos el valor del dutty
		updateDuttyCyclePercentage(handler_PWM_Motor_Test, value_dutty_estatico);
		//Imprimimos mensaje
		writeMsgForTXE(&handler_USART_USB, "Dutty Actualizado \n");
	}
	//Cambiar frecuencia en prueba estatica
	else if (strcmp(cmd, "sFrequency") == 0)
	{
		//Guardamos valor
		value_frequency_estatico = 100000/firtsParameter;
		//Actualizamos el valor del dutty y de la frecuencia
		updateFrequencyTimer(&handler_TIMER_Motor, value_frequency_estatico);
		updateDuttyCyclePercentage(handler_PWM_Motor_Test, value_dutty_estatico);
		//Imprimimos mensaje
		writeMsgForTXE(&handler_USART_USB, "Frecuencia Actualizado \n");
	}
	//Imprimimos count
	else if (strcmp(cmd, "sCount") == 0)
	{
		sprintf(bufferMsg,"Cuentas_R: %u \n", count_foto_R);
		//imprimimos mensajes
		writeMsg(&handler_USART_USB, bufferMsg);
		sprintf(bufferMsg,"Cuentas_L: %u \n", count_foto_L);
		//imprimimos mensajes
		writeMsg(&handler_USART_USB, bufferMsg);
	}
	//Inicializamos count
	else if (strcmp(cmd, "sCountInit") == 0)
	{
		if(flag_study==0)
		{
			//Inicializamos conteo
			count_foto_L = 0;
			count_foto_R = 0;
			//Imprimimos mensaje
			writeMsgForTXE(&handler_USART_USB, "Reinicio exitoso. \n");

		}
		else
		{
			//Imprimimos mensaje
			writeMsgForTXE(&handler_USART_USB, "Prueba de encoder en curso, no es posible. \n");
		}
	}
	//Inicializa el study
	else if (strcmp(cmd, "Start") == 0)
	{
		//Establecemos valores iniciales
		index_count=0;
		value_dutty=0;
		index_fre=0;
		//count_for_dutty[][]={0};
		//Actualizamos el valor del dutty y frecuencia
		updateDuttyCyclePercentage(handler_PWM_Motor_Test, value_dutty);
		updateFrequencyTimer(&handler_TIMER_Motor, frequency[index_fre]);
		//Imprimimos mensaje
		writeMsgForTXE(&handler_USART_USB, "Inicio del estudio... \n");
		//Iniciamos el study
		status_study(flag_dir&SET);
	}
	//para el study en ejecucion
	else if (strcmp(cmd, "Stop") == 0)
	{
		//Paramos el study
		status_study(flag_dir&RESET);
		//Imprimimos mensaje
		writeMsgForTXE(&handler_USART_USB, "Estudio finalizado... \n");
	}
	//imprime los valores del study
	else if (strcmp(cmd, "Print") == 0)
	{
		if(flag_study==0)
		{
			//Imprimimos los valores recolectados de lestudio
			for(uint8_t i=0; i<3;i++)
			{
				sprintf(bufferMsg,"Estudio para Frecuencia: %u Hz \n", 100000/frequency[i]);
				//imprimimos mensajes
				writeMsg(&handler_USART_USB, bufferMsg);
				for(uint8_t e=0; e<21;e++)
				{
					sprintf(bufferMsg,"Porcentaje de Dutty %u : %u Cuentas \n",  e, count_for_dutty[i][e]);
					//Enviamos por puerto serial dicho string
					//imprimimos mensaje
					writeMsg(&handler_USART_USB, bufferMsg);
				}
			}
		}
		else
		{
			writeMsgForTXE(&handler_USART_USB, "Estudio no finalizado \n");
		}

	}

	else
	{
		//Se imprime que el comando no fue valido
		writeMsgForTXE(&handler_USART_USB, "Comando no correcto \n");
	}


}


//----------------------------Fin de la definicion de las funciones de los comandos----------------------------------------




//----------------------------Inicio de la definicion de las funciones-----------------------------------------

void status_study(uint8_t status)
{
	if(status == 1)
	{
		//Activamos el PWM
		statusInOutPWM(handler_PWM_Motor_Test, CHANNEL_ENABLE);
		//Activamos la interrupcion
		statusiInterruptionTimer(&handler_TIMER_study, INTERRUPTION_ENABLE);
		//Activamos el motor
		GPIO_writePin (handler_GPIO_Motor_EN_Test, flag_dir&RESET);
		//levantamos Bandera
		flag_study = 1;
	}
	else
	{
		//Desactivamos el PWM
		statusInOutPWM(handler_PWM_Motor_Test, CHANNEL_DISABLE);
		//Desactivamos interrupcion
		statusiInterruptionTimer(&handler_TIMER_study, INTERRUPTION_DISABLE);
		//Desactivamos el motor
		GPIO_writePin (handler_GPIO_Motor_EN_Test, flag_dir&SET);
		//Reiniciamos Bandera
		flag_study = 0;
	}
}

void change_dutty(void)
{
	//Variable que guarda texto
	char bufferMsg[64]= {0};
	//Verificamos el caracter
	if(charRead=='+')
	{
		if(value_dutty_estatico<100)
		{
			value_dutty_estatico++;
		}
		//Actualizamos el valor del dutty y frecuencia
		updateDuttyCyclePercentage(handler_PWM_Motor_Test, value_dutty_estatico);
		//Convertimos texto
		sprintf(bufferMsg,"Valor dutty : %u \n", value_dutty_estatico);
		//imprimimos mensaje
		writeMsg(&handler_USART_USB, bufferMsg);
	}
	else if(charRead=='-')
	{
		if(value_dutty_estatico>0)
		{
			value_dutty_estatico--;
		}
		//Actualizamos el valor del dutty y frecuencia
		updateDuttyCyclePercentage(handler_PWM_Motor_Test, value_dutty_estatico);
		//Convertimos texto
		sprintf(bufferMsg,"Valor dutty : %u \n", value_dutty_estatico);
		//imprimimos mensaje
		writeMsg(&handler_USART_USB, bufferMsg);
	}
	else
	{
		__NOP();
	}
}

//--------------------------  --Fin de la definicion de las funciones------------------------------------------

