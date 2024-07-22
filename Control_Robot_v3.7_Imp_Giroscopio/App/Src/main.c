/**
 ******************************************************************************
 * @file           : main.c
 * @author         : juloperag
 * @brief          : Control de Robot similar a la version V3.5 pero con Giroscopio
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
#include <math.h>
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
void int_MCO2(void);                                                                  		 //Funcion para la configuracion inicail del MCO1
void recepcionCommand(void);                                                          		 //Funcion que recibe los caracteres del comando recibido
void runCommand(char *prtcommand);                                                    		 //Funcion que ejecuta el comando ingresando
void status_motor(uint8_t status);                                                   		 //Funcion para definir el estado del motor
void int_Config_Motor(void);                                                    	  		 //Funcion que carga la configuracion inicail de los motores
void config_motor(uint8_t status, int firth, float second, float third, int forth); 		 //Funcion para configurar el estudio
void constains_calculator(Parameters_PID_t *ptrPIDHandler,float k, float tau, float theta);	  //Funcion para el calculo de las onstantes
void PID(Parameters_PID_t *ptrPIDHandler, float setpoint, float measure);           		  //Funcion para la implementacion del PID
void correction(Motor_Handler_t *ptrMotorHandler);                                           //Funcion para corregir el dutty
//-----Variables PID-----------
Parameters_PID_t parameter_PID_phi = {0}; 	//Estructura que almacena los elementos del PID para la distancia recta-punto
Parameters_PID_t parameter_PID_distace = {0};
float setpoint_phi = 0;						//Variable que define el setpoint ---> phi
float setpoint_dutty = 0;              		//Variable que define el setpoint ---> Dutty
float Ts = 400;                        		//Periodo de muestreo [ms]
uint8_t flag_PID = 0;                  		//Bandera para el uso del PID
//-------Odometria-------------
Parameters_Position_t parameter_Posicion_Robot = {0}; 	//Estructura que almacena la posicion del robot
float cos_cal = 0;
float sin_cal = 0;
float turn = 0;
uint8_t flag_count_odometry = 0;
float delta_distance_L = 0;
float delta_distance_R = 0;
float32_t cm_L = 0;                               //Factor de conversion rueda Izquierda [mm/cuentas]
float32_t cm_R = 0;                               //Factor de conversion rueda Derecha   [mm/cuentas]
float distance_c = 0;                             //Distancia recorrida medida indirectamente [mm]
float velocity_c = 0;                             //Velocidad medida indirectamente [m/s]
float w_angular_c = 0;                            //Velocidad angular medida indirectamente [rad/s]
#define R2 1.41421356237

//-----Variables de los modos---
Motor_Handler_t *handler_Motor_Execute = {0};     //Handler que se refiere a uno de los motores
uint8_t flag_motor = 0;                           //Bandera que indica el tipo de ejecucion del motor
uint8_t flag_turn = 0;                            //Bandera que indica la direccion del giro
uint8_t flag_Modo_Control = 1;                    //Varibable para el modo de control en el modo line
uint16_t periodo_TIMER_Count = 1000;              //Frecuencia del timer contador
uint16_t frequency_PWM_Motor = 30;                //Frecuencia del timer del PWM
uint16_t count_time = 0;                          //Variable para contar el tiempo trascurrido
uint8_t count_equi = 0;							  //Variable para almacenar las cuentas que se esta en el equilibrio
uint16_t limit_count_turn = 0;                    //Limite de cuentas del giro
int16_t distance = 0;                             //Variable que guarda la distancia a recorrer
float porVel_L = 0;
float porVel_R = 0;
float duttyInicial = 0;                           //Dutty inicial del modo line
float m = 0.06;
float bl = 0;
#define PI 3.14159265358979323846
#define b  10430
#define DL 5170
#define DR 5145
#define Ce 72

float distance_punto_recta = 0;
float line_center = 0;
uint16_t flag_line_center = 1;
float correction_velocity_c = 0;
float correction_dutty_c = 0;

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
	//config_SysTick_us();
	//Activamos el punto flotante por medio del registro especifico
	SCB->CPACR |= 0xF <<20;
	//Definimos la configuracion inicail del MCO1
	int_MCO2();

	//-----------------------Configuracion inicial de los Motores---------------------------------
	//Cargar configuracion de los motores
	GPIO_writePin (&handler_GPIO_MotorR_IN, SET);
	GPIO_writePin (&handler_GPIO_MotorL_IN, SET);
	GPIO_writePin (&handler_GPIO_MotorR_EN, SET);
	GPIO_writePin (&handler_GPIO_MotorL_EN, SET);
	int_Config_Motor();
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
	handler_GPIO_USB_TX.GPIO_PinConfig.GPIO_PinNumber = PIN_9; 						//PIN_x, 0-15
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
	handler_GPIO_USB_RX.GPIO_PinConfig.GPIO_PinNumber = PIN_10; 						//PIN_x, 0-15
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
	handler_USART_USB.ptrUSARTx = USART1;
	//Definimos la configuracion del USART seleccionado
	handler_USART_USB.USART_Config.USART_mode = USART_MODE_RXTX;           //USART_MODE_x  x-> TX, RX, RXTX, DISABLE
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
	//definicion de parametros
	handler_Motor_R.parametersMotor.parametersPID.e = handler_Motor_R.parametersMotor.parametersPID.e_1 = handler_Motor_R.parametersMotor.parametersPID.e_2 = 0;
	handler_Motor_R.parametersMotor.parametersPID.u = handler_Motor_R.parametersMotor.parametersPID.u_1 = 0;
	//Calculo de Constantes PID
	float theta=2+Ts/2;
	constains_calculator(&handler_Motor_R.parametersMotor.parametersPID, 2, 120, theta);   //k,tau,theta

	//---------------Motor Izquierdo----------------
	//Parametro de la señal del dutty
	handler_Motor_L.configMotor.dutty =  20;
	handler_Motor_L.configMotor.frecuency = &value_period;
	handler_Motor_L.configMotor.dir = SET;
	//handler de los perifericos
	handler_Motor_L.phandlerGPIOEN = &handler_GPIO_MotorL_EN;
	handler_Motor_L.phandlerGPIOIN = &handler_GPIO_MotorL_IN;
	handler_Motor_L.phandlerPWM = &handler_PWM_MotorL;
	//definicion de parametros
	handler_Motor_L.parametersMotor.parametersPID.e = handler_Motor_L.parametersMotor.parametersPID.e_1 = handler_Motor_L.parametersMotor.parametersPID.e_2 = 0;
	handler_Motor_L.parametersMotor.parametersPID.u = handler_Motor_L.parametersMotor.parametersPID.u_1 = 0;
	//Calculo de Constantes PID
	constains_calculator(&handler_Motor_L.parametersMotor.parametersPID, 2, 120, theta);   //k,tau,theta

	//---------------PID del la distancia-----------------
	//definicion de parametros
	parameter_PID_distace.e = parameter_PID_distace.e_1 = parameter_PID_distace.e_2 = 0;
	parameter_PID_distace.u = parameter_PID_distace.u_1 = 0;
	//Calculo de Constantes PID
	constains_calculator(&parameter_PID_distace, 2, 150, theta);   //k,tau,theta

	//---------------PID al angulo-----------------
	//definicion de parametros
	parameter_PID_phi.e = parameter_PID_phi.e_1 = parameter_PID_phi.e_2 = 0;
	parameter_PID_phi.u = parameter_PID_phi.u_1 = 0;
	//Calculo de Constantes PID
	constains_calculator(&parameter_PID_phi, 2, 200, theta);   //k,tau,theta

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
		handler_Motor_L.parametersMotor.distance = (cm_L*handler_Motor_L.parametersMotor.count);                   //[mm]
		handler_Motor_R.parametersMotor.distance = (cm_R*handler_Motor_R.parametersMotor.count);				   //[mm]
		handler_Motor_L.parametersMotor.velocity = handler_Motor_L.parametersMotor.distance/periodo_TIMER_Count;   //[m/s]
		handler_Motor_R.parametersMotor.velocity = handler_Motor_R.parametersMotor.distance/periodo_TIMER_Count;   //[m/s]
		//Reiniciamos el numero de conteos
		handler_Motor_R.parametersMotor.count = 0;
		handler_Motor_L.parametersMotor.count = 0;
		//Calculo odometria
		distance_c = (handler_Motor_L.parametersMotor.distance+handler_Motor_R.parametersMotor.distance)/2;  	   //[mm]
		velocity_c = (handler_Motor_R.parametersMotor.velocity+handler_Motor_L.parametersMotor.velocity)/2;        //[m/s]
		w_angular_c = ((handler_Motor_R.parametersMotor.velocity-handler_Motor_L.parametersMotor.velocity)*100000)/b; //[rad/s]
		parameter_Posicion_Robot.xr_position = parameter_Posicion_Robot.xr_position + (distance_c*(cos(parameter_Posicion_Robot.phi_relativo)));        //[mm]
		parameter_Posicion_Robot.yr_position  = parameter_Posicion_Robot.yr_position  + (distance_c*(sin(parameter_Posicion_Robot.phi_relativo)));        //[mm]
		parameter_Posicion_Robot.phi_relativo = parameter_Posicion_Robot.phi_relativo + ((handler_Motor_R.parametersMotor.distance-handler_Motor_L.parametersMotor.distance)*100)/b;   //[rad]
		parameter_Posicion_Robot.phi_relativo = atan2(sin(parameter_Posicion_Robot.phi_relativo),cos(parameter_Posicion_Robot.phi_relativo));
		//Paso de c.relativa a c.globales
		parameter_Posicion_Robot.xg_position = parameter_Posicion_Robot.xg_position_inicial + parameter_Posicion_Robot.xr_position*cos_cal + parameter_Posicion_Robot.xr_position*sin_cal;
		parameter_Posicion_Robot.yg_position = parameter_Posicion_Robot.yg_position_inicial - parameter_Posicion_Robot.xr_position*sin_cal + parameter_Posicion_Robot.yr_position*cos_cal;
		//Convertimos el valor y imprimemos
		sprintf(bufferMsg,"%#.4f\t%#.4f\n", parameter_Posicion_Robot.xg_position , parameter_Posicion_Robot.yg_position);
		writeMsgForTXE(&handler_USART_USB, bufferMsg);
		//Restablecemos valores
		flag_count_odometry = 0;
		delta_distance_L = 0;
		delta_distance_R = 0;
		//calculo la distancia de la recta
		distance_punto_recta = (parameter_Posicion_Robot.xr_position - parameter_Posicion_Robot.yr_position)/R2;

		if(flag_Modo_Control==1)
		{
			//Cambio de modo y establecimiento de line center
			if(fabs(distance_punto_recta)>16)
			{
				flag_Modo_Control = 1;
				line_center = distance_punto_recta/2;
			}
			//correccion
			correction_dutty_c = 0;
			//Convertirmos los valores de velocidad a porcentaje
			porVel_L = 590*(handler_Motor_L.parametersMotor.velocity) - 36.53;
			porVel_R = 590*(handler_Motor_R.parametersMotor.velocity) - 36.53;
			//Aplicamos el PID
			PID(&handler_Motor_L.parametersMotor.parametersPID,setpoint_dutty,porVel_L);
			PID(&handler_Motor_R.parametersMotor.parametersPID,setpoint_dutty,porVel_R);
			//Correccion del dutty
			correction(&handler_Motor_L);
			correction(&handler_Motor_R);

		}
		else
		{
			//sumar al contador
			flag_line_center++;
			//Calculamos variable auxiliar
			float ang_measure = parameter_Posicion_Robot.phi_relativo-setpoint_phi;
			//Cambio de line center
			if(ang_measure<(0.05) && ang_measure>(-0.05) && flag_line_center>2)
			{
				line_center = distance_punto_recta/2;
				//Cambiamos bandera
				flag_line_center = 0;
			}
			//Varificamos la variacion del angulo
			if(distance_punto_recta<(8) && distance_punto_recta>(-8))
			{
				//Sumamos al contador
				count_equi++;
				//cambiamos modo de control
				if(count_equi==10)
				{
					flag_Modo_Control = 1;
					count_equi = 0;
				}
			}
			else
			{
				count_equi=0;
			}
			//Aplicamos PID
			//PID(&parameter_PID_phi, parameter_Posicion_Robot.phi_relativo, setpoint_phi);
			//PID(&parameter_PID_distace, distance_punto_recta*m*(parameter_Posicion_Robot.phi_relativo-setpoint_phi), 0);
			//Correcion dutty
			//correction_velocity_c = (distance_punto_recta/1000);
			//correction_velocity_c = parameter_PID_phi.u*(((w_angular_c)*b)/20000);
			//fabs(sin(ang_measure))
			float auxDistance = distance_punto_recta - line_center;
			float corr = 0;
			if (auxDistance>0)
			{

				corr = 50*(1-exp(-auxDistance/10));
			}
			else
			{
				corr = -50*(1-exp(auxDistance/10));
			}
			correction_dutty_c = corr*m+bl;
			//Correccion del dutty
			correction(&handler_Motor_L);
			correction(&handler_Motor_R);
		}

	}
	else if(flag_motor==3)
	{
		//Aumentamos el contador de tiempo
		count_time = count_time + periodo_TIMER_Count;       //Tiempo en ms
		//Convertimos el valor y imprimemos
		sprintf(bufferMsg,"%u\t%u\t%u\n", count_time,(handler_Motor_R.parametersMotor.count), (handler_Motor_L.parametersMotor.count));
		writeMsgForTXE(&handler_USART_USB, bufferMsg);
		//Reiniciamos el numero de conteos
		handler_Motor_R.parametersMotor.count = 0;
		handler_Motor_L.parametersMotor.count = 0;
	}
	else if(flag_motor==4)
	{
		//Calculamos la velocidad
		handler_Motor_L.parametersMotor.velocity = handler_Motor_L.parametersMotor.distance/periodo_TIMER_Count;   //[m/s]
		handler_Motor_R.parametersMotor.velocity = handler_Motor_R.parametersMotor.distance/periodo_TIMER_Count;   //[m/s]
		//Aumentamos el contador de tiempo
		count_time = count_time + periodo_TIMER_Count;       //Tiempo en ms
		//Convertimos el valor y imprimemos
		sprintf(bufferMsg,"%u\t%#.4f\t%#.4f\n", count_time, handler_Motor_L.parametersMotor.velocity , handler_Motor_R.parametersMotor.velocity);
		writeMsgForTXE(&handler_USART_USB, bufferMsg);
		//Reiniciamos el numero de conteos
		handler_Motor_R.parametersMotor.count = 0;
		handler_Motor_L.parametersMotor.count = 0;
	}

}

//-------------------------USARTRX--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el USART2
void BasicUSART1_Callback(void)
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
	handler_Motor_R.parametersMotor.count++;
	handler_Motor_R.parametersMotor.countCotinuous++;
	//calculo del tiempo entre interrupcion
//	uint64_t timeNow = getTicksUs();
//	handler_Motor_R.parametersMotor.timeCount = timeNow-timeBackR;
//	timeBackR = timeNow;
	//Verificamos que la bandera este arriba
	if(flag_motor==2 && flag_turn==1 && (limit_count_turn)<(handler_Motor_R.parametersMotor.count))
	{
		//Desactivamos los motores
		status_motor(RESET);
		//Actualizamos la direccion del motor
		updateDirMotor(handler_Motor_Execute);
	}
	else if(flag_motor==1 && distance<(cm_R*handler_Motor_R.parametersMotor.countCotinuous))
	{
		//Desactivamos los motores
		status_motor(RESET);
	}

}
void callback_extInt3(void)
{
	handler_Motor_L.parametersMotor.count++;
	handler_Motor_L.parametersMotor.countCotinuous++;
	//calculo del tiempo entre interrupcion
//	uint64_t timeNow = getTicksUs();
//	handler_Motor_L.parametersMotor.timeCount = timeNow-timeBackL;
//	timeBackL = timeNow;
	//Verificamos que la bandera este arriba
	if(flag_motor==2 && flag_turn==2 && (limit_count_turn)<(handler_Motor_L.parametersMotor.count))
	{
		//Desactivamos los motores
		status_motor(RESET);
		//Actualizamos la direccion del motor
		updateDirMotor(handler_Motor_Execute);
	}
	else if(flag_motor==1 && distance<(cm_L*handler_Motor_L.parametersMotor.countCotinuous))
	{
		//Desactivamos los motores
		status_motor(RESET);
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
		writeMsgForTXE(&handler_USART_USB, "3) line # # #---Inicia Linea recta, #: dist [mm], #: dutty inicial , #: dutty estable \n");
		writeMsgForTXE(&handler_USART_USB, "4) turn # # # # ---Iniciamos el giro del robot #: ang #:dir #:dutty_R \n");
		writeMsgForTXE(&handler_USART_USB, "5) start # # # # ---Inicia movimiento, #: perTC [ms], #: dutty_L, #dutty_R, #: freqTP [Hz] \n");
		writeMsgForTXE(&handler_USART_USB, "6) reaction # # # ---Inicia Curva de reaccion,#: perTC [ms], #: dutty_L y dutty_R, #: freqTP [Hz] \n");
		writeMsgForTXE(&handler_USART_USB, "7) stop ---Para el estudio en medio de la ejecucion \n");
		writeMsgForTXE(&handler_USART_USB, "0) const # # # ---Constantes del PID #: L,k,tau \n");
		writeMsgForTXE(&handler_USART_USB, "0) equation # #  ---Constantes de la ecuacion lineal #: m,b  591.91-->59191 \n");
	}

	//----------------------Operacion de movimiento--------------------
	//Definimos el valor de las frecuencias para el study
	else if (strcmp(cmd, "frequency") == 0)
	{
		frequency_PWM_Motor = 100000/firtsParameter;
	}
	else if (strcmp(cmd, "line") == 0)
	{
		//---------Configuracion coordenadas---------
		//Coordenadas Globales
		parameter_Posicion_Robot.xg_position_inicial = parameter_Posicion_Robot.xg_position;
		parameter_Posicion_Robot.yg_position_inicial = parameter_Posicion_Robot.yg_position;
		parameter_Posicion_Robot.ang_Giro += turn;
		//Coordenadas relativas
		parameter_Posicion_Robot.xr_position = 0;
		parameter_Posicion_Robot.yr_position = 0;
		parameter_Posicion_Robot.phi_relativo = (PI/4);
		//Calculos extra
		cos_cal = cos((PI/4)+parameter_Posicion_Robot.ang_Giro);
		sin_cal = sin((PI/4)+parameter_Posicion_Robot.ang_Giro);
		//-------------Configruacion Modo-------------
		//Definimos el modo de control y la line center
		flag_Modo_Control = 1;
		line_center = -10;
		//Definimos la frecuencia del Timer contador
		periodo_TIMER_Count = Ts;
		//Defimos la distancia a recorrer
		distance = firtsParameter;
		//Definimos el dutty inicial
		duttyInicial = secondParameter;
		//Definimos la Setpoint del dutty y del phi
		setpoint_dutty = thirdParameter;
		setpoint_phi = PI/4;
		//Establecemos valores iniciales
		//handler_Motor_L.parametersMotor.parametersPID.u_1 = handler_Motor_R.parametersMotor.parametersPID.u_1 = duttyInicial;
		handler_Motor_L.parametersMotor.parametersPID.e = handler_Motor_L.parametersMotor.parametersPID.e_1 = handler_Motor_L.parametersMotor.parametersPID.e_2 = 0;
		handler_Motor_R.parametersMotor.parametersPID.e = handler_Motor_R.parametersMotor.parametersPID.e_1 = handler_Motor_R.parametersMotor.parametersPID.e_2 = 0;
		//Cargamos la configuracion
		config_motor(1, periodo_TIMER_Count,  0.9234*(duttyInicial)-3, duttyInicial, frequency_PWM_Motor);//Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
	}
	else if (strcmp(cmd, "const") == 0)
	{
			//Cargamos la configuracion
			float theta=firtsParameter+Ts/2;
			constains_calculator(&handler_Motor_L.parametersMotor.parametersPID, secondParameter, thirdParameter, theta);   //k,tau,theta
			constains_calculator(&handler_Motor_R.parametersMotor.parametersPID, secondParameter, thirdParameter, theta);   //k,tau,theta
	}
	else if (strcmp(cmd, "equation") == 0)
	{
			//Cargamos la configuracion
			m = ((float) firtsParameter)/100;
			bl = ((float) secondParameter)/100;
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
		config_motor(2, periodo_TIMER_Count, 0.9234*((float) thirdParameter)-3, thirdParameter, frequency_PWM_Motor);//Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
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
		writeMsgForTXE(&handler_USART_USB, "Inicio conteo... \n");
	}
	//Inicia el estudio de la curva de reaccion
	else if (strcmp(cmd, "reaction") == 0)
	{
		//Definimos la frecuencia del timer contador
		periodo_TIMER_Count = firtsParameter;
		//Cargamos la configuracion
		config_motor(4, periodo_TIMER_Count, (secondParameter/100), (secondParameter/100), thirdParameter);//Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
		//Imprimimos mensaje
		writeMsgForTXE(&handler_USART_USB, "Inicio conteo... \n");
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
void PID(Parameters_PID_t *ptrPIDHandler, float setpoint, float measure)
{
	//Calculo del error
	ptrPIDHandler->e = setpoint-measure;
    // Controle PID
	float p0 =  ptrPIDHandler->q0*ptrPIDHandler->e;
	float p1 =  ptrPIDHandler->q1*ptrPIDHandler->e_1;
	float p2 =  ptrPIDHandler->q2*ptrPIDHandler->e_2;
	ptrPIDHandler->u = ptrPIDHandler->u_1 + p0 + p1 + p2;        //Ley del controlador PID discreto

     //Retorno a los valores reales
	ptrPIDHandler->e_2 = ptrPIDHandler->e_1;
	ptrPIDHandler->e_1 = ptrPIDHandler->e;
	ptrPIDHandler->u_1 = ptrPIDHandler->u;
}

void correction(Motor_Handler_t *ptrMotorHandler)
{

	//Definimos variables auxiliares
	float port_dutty = 0;
	//Conversion ley de control->velocidad->valor dutty
	if(ptrMotorHandler == &handler_Motor_L)
	{
		//Conversion a dutty
		//correction_velocity_c
		port_dutty = ptrMotorHandler->parametersMotor.parametersPID.u - correction_dutty_c;
		//port_dutty = ptrMotorHandler->parametersMotor.parametersPID.u;
	}
	else
	{
		//Conversion a dutty
		port_dutty = ptrMotorHandler->parametersMotor.parametersPID.u + correction_dutty_c;
	}

    //Saturo el porcentaje de dutty en un tope maximo y minimo
    if (port_dutty >= 100.0)
    {
    	port_dutty = 100.0;
    }
    else if(port_dutty <= duttyInicial-5)
    {
    	port_dutty = duttyInicial-5;
    }
    else
    {
    	__NOP();
    }

    //Actualizamoe el valor del dutty
    updateDuttyMotor(ptrMotorHandler, port_dutty);
}

void status_motor(uint8_t status)
{
	if(status == 1)
	{
		//Activamos el motor
		statusInOutPWM(handler_Motor_L.phandlerPWM, CHANNEL_ENABLE);
		statusInOutPWM(handler_Motor_R.phandlerPWM, CHANNEL_ENABLE);
		GPIO_writePin(handler_Motor_L.phandlerGPIOIN, (handler_Motor_L.configMotor.dir)&SET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOIN, (handler_Motor_R.configMotor.dir)&SET);
		GPIO_writePin(handler_Motor_L.phandlerGPIOEN, RESET);
		GPIO_writePin(handler_Motor_R.phandlerGPIOEN, RESET);
		//Activamos la interrupcion
		if(flag_motor!=2)
		{
			statusiInterruptionTimer(&handler_TIMER_Count, INTERRUPTION_ENABLE);
		}
	}
	else
	{
		//Desactivamos interrupcion
		if(flag_motor!=2)
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
	handler_Motor_R.parametersMotor.count = 0;
	handler_Motor_L.parametersMotor.count = 0;
	handler_Motor_R.parametersMotor.countCotinuous = 0;
	handler_Motor_L.parametersMotor.countCotinuous = 0;
	//Establecemos valroes iniciales
	//timeBackR = timeBackL = getTicksUs();
	count_time = 0;
	//Actualizamos el valor del dutty y frecuencia
	value_period = 100000/forth;
	updateFrequencyTimer(&handler_TIMER_Motor, value_period);
	updateDuttyMotor(&handler_Motor_R, third);
	updateDuttyMotor(&handler_Motor_L, second);
	//Cambio valor bandera
	flag_motor=status;
	//Iniciamos el study
	status_motor(SET);
}


void constains_calculator(Parameters_PID_t *ptrPIDHandler,float k, float tau, float theta)   //k,tau,theta
{
	   //Calculo de constantes de porcentaje, integracion y derivacion por metodo de Ziegler y Nichols
	   float kp=(1.2*tau)/(k*theta);
	   float ti=2.0*theta;
	   float td=0.5*theta;
	   //Calculo do controle PID digital
	   ptrPIDHandler->q0 = kp*(1+Ts/(2.0*ti)+td/Ts);
	   ptrPIDHandler->q1 = -kp*(1-Ts/(2.0*ti)+(2.0*td)/Ts);
	   ptrPIDHandler->q2 = (kp*td)/Ts;
}


//--------------------------  --Fin de la definicion de las funciones------------------------------------------

