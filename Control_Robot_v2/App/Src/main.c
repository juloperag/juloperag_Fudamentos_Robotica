/**
 ******************************************************************************
 * @file           : main.c
 * @author         : juloperag
 * @brief          : Configuracion Basica Proyecto
 ******************************************************************************
 */

//----------------------------------Inicio de definicion de librerias-------------------------------------------
//----Sistema--------
#include <stdint.h>
#include <stm32f411xe.h>
#include <stdio.h>
//----Perifericos
#include <GPIOxDriver.h>
#include <BasicTimer.h>
#include <ExtiDriver.h>
#include <USARTxDriver.h>
#include <PwmDriver.h>
#include <PLLDriver.h>
#include <SysTickDriver.h>
//------CMSIS------
#include <arm_math.h>
//-----Proyecto-------
#include <MotorDriver.h>


//-----------------------------------Fin de definicion de librerias------------------------------------------


//---------------------------Inicio de definicion de funciones y variables base----------------------------------
GPIO_Handler_t handler_BlinkyPin = {0};         //Definimos un elemento del tipo GPIO_Handler_t (Struct) para el LED
BasicTimer_Handler_t handler_BlinkyTimer ={0};  //Definimos un elemento del tipo BasicTimer_Handler_t (Struct)
void int_Hardware(void);                        //Definimos la cabecera para la configuracion
//---------------------------Fin de definicion de funciones y variables base----------------------------------


//-------------------------PIN MCO2--------------------------------
GPIO_Handler_t handler_GPIO_MCO2 = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct) para utilizar el pin MCO1 con el fin de muestrear las frecuencias de los osciladores

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
char bufferMsg[64] = {0};


//--------------------------Motor-------------------------------
//------------Derecho--------------
GPIO_Handler_t handler_GPIO_MotorR = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct), BasicTimer_Config_t  y PWM_Handler_t para el uso del Motor R
GPIO_Handler_t handler_GPIO_MotorR_IN = {0};
GPIO_Handler_t handler_GPIO_MotorR_EN = {0};
PWM_Handler_t handler_PWM_MotorR = {0};
Motor_Handler_t handler_Motor_R;
//------------Izquierdo-----------------
GPIO_Handler_t handler_GPIO_MotorL = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct), BasicTimer_Config_t  y PWM_Handler_t para el uso del Motor L
GPIO_Handler_t handler_GPIO_MotorL_IN = {0};
GPIO_Handler_t handler_GPIO_MotorL_EN = {0};
PWM_Handler_t handler_PWM_MotorL = {0};
Motor_Handler_t handler_Motor_L;
//-----------General-------------------
BasicTimer_Handler_t handler_TIMER_Motor = {0};
BasicTimer_Handler_t handler_TIMER_Count = {0};   //Definimos un elementos de tipo BasicTimer_Handler_t para relizar el test
uint16_t value_period = 4000;                 //Valor del periodo del timer, que sera luego se apuntada


//------------------Definiciones generales----------------------------------
//-----Cabeceras de funciones----
void int_MCO2(void);                                                                  //Funcion para la configuracion inicail del MCO1
void recepcionCommand(void);                                                          //Funcion que recibe los caracteres del comando recibido
void runCommand(char *prtcommand);                                                    //Funcion que ejecuta el comando ingresando
void status_motor(uint8_t status);                                                    //Funcion para definir el estado del motor
void int_Config_Motor(void);                                                    	  //Funcion que carga la configuracion inicail de los motores
void config_motor(uint8_t status, int firth, float second, float third, int forth) ;  //Funcion para configurar el estudio
void change_study(void);                                                        	  //Reiniciamos los parametros
//-----Variables PID-----------
//Variables Globales
uint16_t distance = 0;                 //Variable que guarda la distancia a recorrer
float32_t setpoint = 0;                //Variable que define el setpoint
float32_t T1,aux;                       //Temperatura del Heater 1
float32_t r1=40.0;                      //Referencia del Heater 1
#define Ts  8                           //Periodo de muestreo
//Parámetros del PID
volatile float32_t u=0.0,u_1=0.0;        //Acción de Control
volatile float e=0.0,e_1=0.0,e_2=0.0;    //Errores
float32_t kp,ti,td;
float32_t q0,q1,q2;
float32_t k=1.04,tau=160,theta=10+Ts/2;   //Parámetros del Modelo del sistema
//-----Variables Del Conteo---
Motor_Handler_t *handler_Motor_Execute = {0};     //Handler que se refiere a uno de los motores
uint8_t flag_motor = 0;                           //Bandera que indica el tipo de ejecucion del motor
uint8_t flag_turn = 0;                            //Bandera que indica la direccion del giro
uint16_t periodo_TIMER_Count = 1000;              //Frecuencia del timer contador
uint16_t frequency_PWM_Motor = 20;                //Frecuencia del timer del PWM
uint16_t count_time = 0;                          //Variable para contar el tiempo trascurrido
uint16_t limit_count_turn = 0;                    //Limite de cuentas del giro
volatile float32_t velocity_L =  0.0;			  //Velocidad de la rueda Izquierda      [m/s]
volatile float32_t velocity_R =  0.0;             //Velocidad de la rueda Derecha        [m/s]
float32_t cm_L = 0;                               //Factor de conversion rueda Izquierda [mm/cuentas]
float32_t cm_R = 0;                               //Factor de conversion rueda Derecha   [mm/cuentas]
uint64_t timeBackL = 1;                           //Variable para guardar el tiempo de la interrupcion pasada del encoder Izquierdo
uint64_t timeBackR = 1;                           //Variable para guardar el tiempo de la interrupcion pasada del encoder Derecho
#define PI 3.14159265358979323846
#define b  10430
#define DL 5170
#define DR 5145
#define Ce 72
#define duttymax  40                               //Limites de dutty
#define duttymin  8


//-----Variables de la recepcion de comandos----
uint8_t commandComplete = 1;           //Bandera que indica si el comando esta completo
uint8_t counterRecepcion = 0;          //Variable para la posicion del arrelgo
char bufferRecepcion[64] = {0};        //Arreglo que almacena el comando ingresado

int main(void)
{
	//-----------------------Configuracion inicial del sistema---------------------------------
	//Incrementamos la velocidad de reloj del sistema
	uint8_t clock = CLOCK_SPEED_100MHZ;    //Velocidad de reloj entre 25 o 100 MHz
	configPLL(clock);
	//Realizamos la configuracuion inicial
	int_Hardware();
	//Activamos el SysTick
	config_SysTick_us();
	//Activamos el punto flotante por medio del registro especifico
	SCB->CPACR |= 0xF <<20;
	//Definimos la configuracion inicail del MCO1
	 int_MCO2();

	//-----------------------Configuracion inicial de los Motores---------------------------------
	//Cargar configuracion de los motores
	int_Config_Motor();
	GPIO_writePin (&handler_GPIO_MotorR_IN, SET);
	GPIO_writePin (&handler_GPIO_MotorL_IN, SET);
	GPIO_writePin (&handler_GPIO_MotorR_EN, SET);
	GPIO_writePin (&handler_GPIO_MotorL_EN, SET);
	//Definimos el motor derecho para ser ejecutado
	handler_Motor_Execute = &handler_Motor_R;
	//Calculo inicial de parametro
	cm_L = ((PI*DL)/(100*Ce));  //[mm/cuentas]
	cm_R =	((PI*DR)/(100*Ce));  //[mm/cuentas]

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
	//-------------------------PIN_MCO2--------------------------------
	//---------------PIN: PC9----------------
	//------------AF0: MCO_2----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_MCO2.pGPIOx = GPIOC;
	//Definimos el pin a utilizar
	handler_GPIO_MCO2.GPIO_PinConfig.GPIO_PinNumber = PIN_9; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_MCO2, GPIO_MODE_ALTFN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF0);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_MCO2);

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
	//---------------PIN: PA9----------------
	//------------AF7: USART1_TX----------------
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

	//---------------PIN: PA10----------------
	//------------AF7: USART1_RX----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_USB_RX.pGPIOx = GPIOA;
	//Definimos el pin a utiliza
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

	//---------------USART1----------------
	//Definimos el periferico USARTx a utilizar
	handler_USART_USB.ptrUSARTx = USART2;
	//Definimos la configuracion del USART seleccionado
	handler_USART_USB.USART_Config.USART_mode = USART_MODE_RXTX ;           //USART_MODE_x  x-> TX, RX, RXTX, DISABLE
	handler_USART_USB.USART_Config.USART_baudrate = USART_BAUDRATE_19200;  //USART_BAUDRATE_x  x->9600, 19200, 115200
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
	handler_TIMER_Count.ptrTIMx = TIM3;
	//Definimos la configuracion del TIMER seleccionado
	handler_TIMER_Count.TIMx_Config.TIMx_periodcnt = BTIMER_PCNT_1ms; //BTIMER_PCNT_xus x->10,100/ BTIMER_PCNT_1ms
	handler_TIMER_Count.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; // BTIMER_MODE_x x->UP, DOWN
	handler_TIMER_Count.TIMx_Config.TIMX_period = 2000;//Al definir 10us,100us el valor un multiplo de ellos, si es 1ms el valor es en ms
	handler_TIMER_Count.TIMx_Config.TIMx_interruptEnable = INTERRUPTION_DISABLE; //INTERRUPTION_x  x->DISABLE, ENABLE
	//Cargamos la configuracion del TIMER especifico
	BasicTimer_Config(&handler_TIMER_Count);

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
	 handler_EXTI_FotoR.edgeType = EXTERNAL_INTERRUPP_RISING_FALLING_EDGE;
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
	 handler_EXTI_FotoL.edgeType = EXTERNAL_INTERRUPP_RISING_FALLING_EDGE;
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

//-----------------------------Inicio configuracion MCO1------------------------------------------

//Funcion para la configuracion inicial del MCO1
void int_MCO2(void)
{
	//Configuracion inicial del MCO2
	configMCO2(0);
	configMCO2PRE(4);
}
//------------------------------Fin configuracion MCO1------------------------------------------


//------------------------------Inicio configuracion de lso motores------------------------------------------
void int_Config_Motor(void)
{
	//---------------Motor Derecho----------------
	//Parametro de la señal del dutty
	handler_Motor_R.configMotor.dutty =  20;
	handler_Motor_R.configMotor.frecuency = &value_period;
	handler_Motor_R.configMotor.dir = SET;
	//handler de los perifericos
	handler_Motor_R.phandlerGPIOEN = &handler_GPIO_MotorR_EN;
	handler_Motor_R.phandlerGPIOIN = &handler_GPIO_MotorR_IN;
	handler_Motor_R.phandlerPWM = &handler_PWM_MotorR;
	//Tiempo de conteo
	handler_Motor_R.configMotor.timeCount = 1;

	//---------------Motor Izquierdo----------------
	//Parametro de la señal del dutty
	handler_Motor_L.configMotor.dutty =  20;
	handler_Motor_L.configMotor.frecuency = &value_period;
	handler_Motor_L.configMotor.dir = SET;
	//handler de los perifericos
	handler_Motor_L.phandlerGPIOEN = &handler_GPIO_MotorL_EN;
	handler_Motor_L.phandlerGPIOIN = &handler_GPIO_MotorL_IN;
	handler_Motor_L.phandlerPWM = &handler_PWM_MotorL;
	//Tiempo de conteo
	handler_Motor_L.configMotor.timeCount = 1;

};

//------------------------------Fin configuracion de lso motores------------------------------------------




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
	if(flag_motor==1)
	{
		//Calculamos la velocidad
		velocity_L = (cm_L*1000)/handler_Motor_L.configMotor.timeCount;   //[m/s]
		velocity_R = (cm_R*1000)/handler_Motor_R.configMotor.timeCount;   //[m/s]
		//Aumentamos el contador de tiempo
		count_time = count_time + periodo_TIMER_Count;       //Tiempo en ms
		//Convertimos el valor y imprimemos
		sprintf(bufferMsg,"%u\t%#.4f\t%#.4f\n", count_time, velocity_L , velocity_R);
		writeMsgForTXE(&handler_USART_USB, bufferMsg);
		//Reiniciamos el numero de conteos
		handler_Motor_R.configMotor.count = 0;
		handler_Motor_L.configMotor.count = 0;
	}
	else if(flag_motor==3)
	{
		//Aumentamos el contador de tiempo
		count_time = count_time + periodo_TIMER_Count;       //Tiempo en ms
		//Convertimos el valor y imprimemos
		sprintf(bufferMsg,"%u\t%u\t%u\n", count_time,(handler_Motor_R.configMotor.count), (handler_Motor_L.configMotor.count));
		writeMsgForTXE(&handler_USART_USB, bufferMsg);
		//Reiniciamos el numero de conteos
		handler_Motor_R.configMotor.count = 0;
		handler_Motor_L.configMotor.count = 0;
	}
	else if(flag_motor==4)
	{
		//Calculamos la velocidad
		velocity_L = (cm_L*1000)/handler_Motor_L.configMotor.timeCount;   //[m/s]
		velocity_R = (cm_R*1000)/handler_Motor_R.configMotor.timeCount;   //[m/s]
		//Aumentamos el contador de tiempo
		count_time = count_time + periodo_TIMER_Count;       //Tiempo en ms
		//Convertimos el valor y imprimemos
		sprintf(bufferMsg,"%u\t%#.4f\t%#.4f\n", count_time, velocity_L , velocity_R);
		writeMsgForTXE(&handler_USART_USB, bufferMsg);
		//Reiniciamos el numero de conteos
		handler_Motor_R.configMotor.count = 0;
		handler_Motor_L.configMotor.count = 0;
	}

}

//-------------------------USARTRX--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el USART2
void BasicUSART2_Callback(void)
{
	//Guardamos el caracter recibido
	charRead = getRxData();
	//Funcion que almacena los caracteres del comando recibido
	recepcionCommand();
}


//-------------------------Fotocompuerta--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el EXTI13 y EXTI13
void callback_extInt1(void)
{
	handler_Motor_R.configMotor.count++;
	//calculo del tiempo entre interrupcion
	uint64_t timeNow = getTicksUs();
	handler_Motor_R.configMotor.timeCount = timeNow-timeBackR;
	timeBackR = timeNow;
	//Verificamos que la bandera este arriba
	if(flag_motor==1 && flag_turn==1 && (limit_count_turn-2)<(handler_Motor_R.configMotor.count))
	{
		//Desactivamos los motores
		status_motor(RESET);
		//Actualizamos la direccion del motor
		updateDirMotor(handler_Motor_Execute);
	}

}
void callback_extInt3(void)
{
	handler_Motor_L.configMotor.count++;
	//calculo del tiempo entre interrupcion
	uint64_t timeNow = getTicksUs();
	handler_Motor_L.configMotor.timeCount = timeNow-timeBackL;
	timeBackL = timeNow;
	//Verificamos que la bandera este arriba
	if(flag_motor==1 && flag_turn==2 && (limit_count_turn-2)<(handler_Motor_L.configMotor.count))
	{
		//Desactivamos los motores
		status_motor(RESET);
		//Actualizamos la direccion del motor
		updateDirMotor(handler_Motor_Execute);
	}
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
	int forthParameter = 0;
	//char bufferMsg[64]= {0};

	//Funcion que lee la cadena de caracteres y la divide en los elementos definidos
	sscanf(prtcommand, "%s %u %u %u %u", cmd, &firtsParameter, &secondParameter, &thirdParameter, &forthParameter);

	//Imprime lista que muestra los comandos que tiene el dispositivo
	if(strcmp(cmd, "help")==0)
	{
		writeMsgForTXE(&handler_USART_USB, "Help Menu: \n");
		writeMsgForTXE(&handler_USART_USB, "1) help  ---Imprime lista de comandos. \n");
		writeMsgForTXE(&handler_USART_USB, "2) frequency # --- Cambiar el valor de la frecuenencia de las pruebas, [HZ] \n");
		writeMsgForTXE(&handler_USART_USB, "3) line # # ---Inicia Linea recta, #: dist [mm], #: vel [m/s] tal forma: 10.23-->1023 \n");
		writeMsgForTXE(&handler_USART_USB, "4) turn # # # # ---Iniciamos el giro del robot #: ang #:dir #:dutty_L #:dutty_R \n");
		writeMsgForTXE(&handler_USART_USB, "5) start # # # # ---Inicia movimiento, #: perTC [ms], #: dutty_L, #dutty_R, #: freqTP [Hz] \n");
		writeMsgForTXE(&handler_USART_USB, "6) reaction # # # ---Inicia Curva de reaccion,#: perTC [ms], #: dutty_L y dutty_R, #: freqTP [Hz] \n");
		writeMsgForTXE(&handler_USART_USB, "7) stop ---Para el estudio en medio de la ejecucion \n");

	}

	//----------------------Operacion de movimiento--------------------
	//Definimos el valor de las frecuencias para el study
	else if (strcmp(cmd, "frequency") == 0)
	{
		frequency_PWM_Motor = 100000/firtsParameter;
	}
	else if (strcmp(cmd, "line") == 0)
	{
		//Definimos la frecuencia del Timer contador
		periodo_TIMER_Count = 200;
		//Defimos la distancia a recorrer
		distance = firtsParameter;
		//Definimos la Setpoint de la velocidad
		setpoint = secondParameter/100;
		//Cargamos la configuracion
		config_motor(1, periodo_TIMER_Count,  30, 30, 30);//Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
	}
	else if (strcmp(cmd, "turn") == 0)
	{
		//Guardamos la direccion del giro
		flag_turn = secondParameter;
		//Cambiamso la direccion del motor
		if(flag_turn==1)
		{
			//Definimos el limite de cuentas
			limit_count_turn = (b/DR)*((Ce*firtsParameter)/(2*180));
			//Seleccionamos el motor derecho
			handler_Motor_Execute = &handler_Motor_R;
			//Actualizamos la direccion del motor
			updateDirMotor(handler_Motor_Execute);
		}
		else
		{
			//Definimos el limite de cuentas
			limit_count_turn = (b/DL)*((Ce*firtsParameter)/(2*180));
			//Seleccionamos el motor izquierdo
			handler_Motor_Execute = &handler_Motor_L;
			//Actualizamos la direccion del motor
			updateDirMotor(handler_Motor_Execute);
		}
		//Cargamos configuracion
		config_motor(2, periodo_TIMER_Count, (thirdParameter/100), (forthParameter/100), frequency_PWM_Motor);//Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
	}
	//------------------------------test--------------------------------
	//Inicializa el estudio de la velocidad
	else if (strcmp(cmd, "start") == 0)
	{
		//Definimos la frecuencia del timer contador
		periodo_TIMER_Count = firtsParameter;
		//Cargamos la configuracion
		config_motor(3, periodo_TIMER_Count,  (secondParameter/100), (thirdParameter/100), forthParameter);//Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
		//Imprimimos mensaje
		writeMsg(&handler_USART_USB, "Inicio conteo... \n");
	}
	//Inicia el estudio de la curva de reaccion
	else if (strcmp(cmd, "reaction") == 0)
	{
		//Definimos la frecuencia del timer contador
		periodo_TIMER_Count = firtsParameter;
		//Cargamos la configuracion
		config_motor(4, periodo_TIMER_Count, (secondParameter/100), (secondParameter/100), thirdParameter);//Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
		//Imprimimos mensaje
		writeMsg(&handler_USART_USB, "Inicio conteo... \n");
	}
	//para el study en ejecucion
	else if (strcmp(cmd, "stop") == 0)
	{
		//Paramos el study
		status_motor(RESET);
		//Imprimimos mensaje
		writeMsgForTXE(&handler_USART_USB, "Estudio finalizado \n");
	}

	else
	{
		//Se imprime que el comando no fue valido
		writeMsgForTXE(&handler_USART_USB, "Comando no correcto \n");
	}


}


//----------------------------Fin de la definicion de las funciones de los comandos----------------------------------------




//----------------------------Inicio de la definicion de las funciones-----------------------------------------


//--------------------PID----------------------
void PID(Motor_Handler_t *ptrMotorHandler)
{
    e=(r1-T1);
    // Controle PID
    u = u_1 + q0*e + q1*e_1 + q2*e_2;        //Ley del controlador PID discreto
    //Saturo la accion de control 'uT' en un tope maximo y minimo
    if (u >= 100.0)
    {
    	 u = 100.0;
    }
    else if(u <= 0.0)
    {
    	u = 0.0;
    }
    else
    {
    	__NOP();
    }
     //Retorno a los valores reales
     e_2=e_1;
     e_1=e;
     u_1=u;
     //Actualizamoe el valor del dutty
     updateDuttyMotor(ptrMotorHandler, u);
}

void status_motor(uint8_t status)
{
	if(status == 1)
	{
		//Activamos la interrupcion
		if(flag_motor==3 || flag_motor==4)
		{
			statusiInterruptionTimer(&handler_TIMER_Count, INTERRUPTION_ENABLE);
		}
		//Activamos el motor
		statusInOutPWM(handler_Motor_L.phandlerPWM, CHANNEL_ENABLE);
		statusInOutPWM(handler_Motor_R.phandlerPWM, CHANNEL_ENABLE);
		GPIO_writePin(handler_Motor_L.phandlerGPIOIN, (handler_Motor_L.configMotor.dir)&SET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOIN, (handler_Motor_R.configMotor.dir)&SET);
		GPIO_writePin(handler_Motor_L.phandlerGPIOEN, RESET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOEN, RESET);
	}
	else
	{
		//Desactivamos interrupcion
		if(flag_motor==3 || flag_motor==4)
		{
			statusiInterruptionTimer(&handler_TIMER_Count, INTERRUPTION_DISABLE);
		}
		//Desactivamos el motor
		statusInOutPWM(handler_Motor_L.phandlerPWM, CHANNEL_DISABLE);
		statusInOutPWM(handler_Motor_R.phandlerPWM, CHANNEL_DISABLE);
		GPIO_writePin(handler_Motor_L.phandlerGPIOIN, (handler_Motor_L.configMotor.dir)&RESET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOIN, (handler_Motor_R.configMotor.dir)&RESET);
		GPIO_writePin(handler_Motor_L.phandlerGPIOEN, SET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOEN, SET);
		//Reiniciamos Bandera
		flag_motor = 0;
	}
}

//Funcion para al configuracion de los motores
void config_motor(uint8_t status, int firth, float second, float third, int forth)  //Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
{
	//Actualizacion de la frecuencia del timer
	updateFrequencyTimer(&handler_TIMER_Count, firth);
	//Establecer valores
	handler_Motor_R.configMotor.count=0;
	handler_Motor_L.configMotor.count=0;
	//Establecemos valroes iniciales
	timeBackR = timeBackL = getTicksUs();
	count_time = 0;
	//Actualizamos el valor del dutty y frecuencia
	updateFrequencyTimer(&handler_TIMER_Motor, 100000/forth);
	updateDuttyMotor(&handler_Motor_R, third);
	updateDuttyMotor(&handler_Motor_L, second);
	//Cambio valor bandera
	flag_motor=status;
	//Iniciamos el study
	status_motor(SET);
}


void change_study(void)
{
	handler_Motor_R.configMotor.count=0;
	handler_Motor_L.configMotor.count=0;
	//Actualizamos el valor del dutty y frecuencia
//	updateDuttyMotor(&handler_Motor_R, value_dutty);
//	updateDuttyMotor(&handler_Motor_L, value_dutty);
}


//--------------------------  --Fin de la definicion de las funciones------------------------------------------
