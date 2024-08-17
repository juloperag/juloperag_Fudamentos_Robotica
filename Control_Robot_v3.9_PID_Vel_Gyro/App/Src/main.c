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
#include "MPUAccel.h"


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

//-------------------------MPU6050--------------------------
GPIO_Handler_t handler_GPIO_SCL_MPU6050 = {0};         //Definimos un elemento del tipo GPIO_Handler_t (Struct) y I2C_Handler_t para la comunicacion I2C
GPIO_Handler_t handler_GPIO_SDA_MPU6050 = {0};
I2C_Handler_t handler_I2C_MPU6050 = {0};
MPUAccel_Handler_t handler_MPUAccel_MPU6050 = {0};     //Se crea un handler para guardar la configuracion


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
BasicTimer_Handler_t handler_TIMER_Sampling = {0};   //Definimos un elementos de tipo BasicTimer_Handler_t para relizar el test
uint16_t value_period = 4000;                 //Valor del periodo del timer, que sera luego se apuntada


//------------------Definiciones generales----------------------------------
//-----Cabeceras de funciones----
void int_MCO2(void);                                                                  		 //Funcion para la configuracion inicail del MCO1
void int_MPU(void);                                                                          //Funcion para configurar el MPU
void recepcionCommand(void);                                                          		 //Funcion que recibe los caracteres del comando recibido
void runCommand(char *prtcommand);                                                    		 //Funcion que ejecuta el comando ingresando
void status_motor(uint8_t status);                                                   		 //Funcion para definir el estado del motor
void int_Config_Motor(void);                                                    	  		 //Funcion que carga la configuracion inicail de los motores
void config_motor(uint8_t status, int firth, float second, float third, int forth); 		 //Funcion para configurar el estudio
void constains_calculator(Parameters_PID_t *ptrPIDHandler,float kp, float ti, float td);	 //Funcion para el calculo de las constantes
void PID(Parameters_PID_t *ptrPIDHandler, float setpoint, float measure);           		 //Funcion para la implementacion del PID
void correction(Motor_Handler_t *ptrMotorHandler);                                           //Funcion para corregir el dutty
//-----Variables del MPU
float ang_Row = 0;                               //Variable para almacenar el angulo del eje Z
int16_t gyro_offset = 0;                         //Variable para guardad el offset de la calibracion
uint64_t time_preview = 0;                       //Variable para guardar el tiempo anterior medido
//-----Variables PID-----------
Parameters_PID_t parameter_PID_phi = {0}; 	//Estructura que almacena los elementos del PID para la distancia recta-punto
Parameters_PID_t parameter_PID_distace = {0};
float setpoint_phi = 0;						//Variable que define el setpoint ---> phi
float setpoint_dutty = 0;              		//Variable que define el setpoint ---> Dutty
float Ts = 400;                        		//Periodo de muestreo [ms]
uint8_t flag_PID = 0;                  		//Bandera para el uso del PID
//-------Odometria-------------
Parameters_Position_t parameter_Posicion_Robot = {0}; 	//Estructura que almacena la posicion del robot
MPUTimerSample_t sample_Gyro = {0};                     //Estructura para almacenar los datos del muestreo
float sum_ang  = 0;                              //Angulo acumulado
float cos_cal = 0;
float sin_cal = 0;
float turn = 0;
uint8_t flag_count_odometry = 0;
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
uint16_t timeAction_TIMER_Sampling = 400;               //Frecuencia del timer contador
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

double velRef_L = 0;
double velRef_R = 0;
double uRef = 0;
double wRef = 0;

float distance_punto_recta = 0;
float line_center = 0;
uint16_t flag_line_center = 1;
float correction_velocity_c = 0;
float correction_dutty_c = 0;

//--------Variables del timerSampling---------
uint16_t time_accumulated = 0;
uint16_t counting_action = 0;

//-----Variables de la recepcion de comandos----
uint8_t commandComplete = 1;           //Bandera que indica si el comando esta completo
uint8_t counterRecepcion = 0;          //Variable para la posicion del arrelgo
char bufferRecepcion[64] = {0};        //Arreglo que almacena el comando ingresado


float corr=0;

int main(void)
{
	//-----------------------Configuracion inicial del sistema---------------------------------
	//Incrementamos la velocidad de reloj del sistema

	uint8_t clock = CLOCK_SPEED_100MHZ;    //Velocidad de reloj entre 25 o 100 MHz
	configPLL(clock);
	//Realizamos la configuracuion inicial
	int_Hardware();
	//Activamos el Systick
	config_SysTick_ms();
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

	//--------------------------Configuramos inicia el MPU----------------------
	//Configuracion MPU
	int_MPU();


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
		{ __NOP(); }
		//Calculo de odometry y aplicacion PID
		if(flag_count_odometry == 1)
		{
			//Convertirmos los valores de velocidad a porcentaje
			porVel_L = 563*(handler_Motor_L.parametersMotor.velocity) - 26;
			porVel_R = 563*(handler_Motor_R.parametersMotor.velocity) - 26;
			//Calculo odometria
			distance_c = (handler_Motor_L.parametersMotor.distance+handler_Motor_L.parametersMotor.distance)/2;  	   //[mm]
			//w_angular_c = ((handler_Motor_R.parametersMotor.velocity-handler_Motor_L.parametersMotor.velocity)*100000)/b; //[rad/s]
			parameter_Posicion_Robot.xr_position = parameter_Posicion_Robot.xr_position + (distance_c*(cos(parameter_Posicion_Robot.phi_relativo)));        //[mm]
			parameter_Posicion_Robot.yr_position  = parameter_Posicion_Robot.yr_position  + (distance_c*(sin(parameter_Posicion_Robot.phi_relativo)));       //[mm]
			//Paso de c.relativa a c.globales
			parameter_Posicion_Robot.xg_position = parameter_Posicion_Robot.xg_position_inicial + parameter_Posicion_Robot.xr_position*cos_cal + parameter_Posicion_Robot.xr_position*sin_cal;
			parameter_Posicion_Robot.yg_position = parameter_Posicion_Robot.yg_position_inicial - parameter_Posicion_Robot.xr_position*sin_cal + parameter_Posicion_Robot.yr_position*cos_cal;
			//Convertimos el valor y imprimemos
			sprintf(bufferMsg,"%#.4f\t%#.4f\n", parameter_Posicion_Robot.xg_position , parameter_Posicion_Robot.yg_position);
			writeMsgForTXE(&handler_USART_USB, bufferMsg);
			//calculo a la distancia de la recta
			//distance_punto_recta = (parameter_Posicion_Robot.yr_position/1000);
			//Control PID para la distancia
			//PID(&parameter_PID_distace, 0, distance_punto_recta);
			//planteamos una suafizacion al error
//			if (parameter_PID_distace.u>0)
//			{
//
//				corr = 20*(1-exp(-parameter_PID_distace.u/20));
//			}
//			else
//			{
//				corr = -20*(1-exp(parameter_PID_distace.u/20));
//			}
			//wRef = parameter_PID_distace.u*1;
			//Calculamos velocidades para cada rueda
			//velRef_L = (2*uRef-(wRef*b)/100000)/2;
			//velRef_R = 2*uRef-velRef_L;
			//Conversion de velocidad a dutty
			//handler_Motor_L.configMotor.dutty= 478*velRef_L  - 18;
			//handler_Motor_R.configMotor.dutty = 563*velRef_R  - 26;
			//Correccion del dutty
			//Aplicamos el PID
			PID(&handler_Motor_L.parametersMotor.parametersPID, setpoint_dutty,porVel_L);
			PID(&handler_Motor_R.parametersMotor.parametersPID,setpoint_dutty,porVel_R);
			//Correccion del dutty
			correction(&handler_Motor_L);
			correction(&handler_Motor_R);
			//Restablecemos valores
			flag_count_odometry = 0;
		}
		else
		{ __NOP(); }


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

	//---------------------------I2C--------------------------------
	//---------------PIN: PB8----------------
	//------------AF4: I2C1_SCL----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_SCL_MPU6050.pGPIOx = GPIOB;
	//Definimos el pin a utilizar
	handler_GPIO_SCL_MPU6050.GPIO_PinConfig.GPIO_PinNumber = PIN_8; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_SCL_MPU6050, GPIO_MODE_ALTFN, GPIO_OTYPER_OPENDRAIN, GPIO_OSPEEDR_FAST, GPIO_PUPDR_NOTHING, AF4);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_SCL_MPU6050);

	//---------------PIN: PB9----------------
	//------------AF4: I2C1_SDA----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_SDA_MPU6050.pGPIOx = GPIOB;
	//Definimos el pin a utilizar
	handler_GPIO_SDA_MPU6050.GPIO_PinConfig.GPIO_PinNumber = PIN_9; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_SDA_MPU6050, GPIO_MODE_ALTFN, GPIO_OTYPER_OPENDRAIN, GPIO_OSPEEDR_FAST, GPIO_PUPDR_NOTHING, AF4);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_SDA_MPU6050);

	//-------------------Fin de Configuracion GPIOx-----------------------

	//-------------------Inicio de Configuracion USARTx-----------------------

	//---------------USART1----------------
	//Definimos el periferico USARTx a utilizar
	handler_USART_USB.ptrUSARTx = USART2;
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
	handler_TIMER_Sampling.ptrTIMx = TIM3;
	//Definimos la configuracion del TIMER seleccionado
	handler_TIMER_Sampling.TIMx_Config.TIMx_periodcnt = BTIMER_PCNT_1ms; //BTIMER_PCNT_xus x->10,100/ BTIMER_PCNT_1ms
	handler_TIMER_Sampling.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; // BTIMER_MODE_x x->UP, DOWN
	handler_TIMER_Sampling.TIMx_Config.TIMX_period = 20;   //Al definir 10us,100us el valor un multiplo de ellos, si es 1ms el valor es en ms
	handler_TIMER_Sampling.TIMx_Config.TIMx_interruptEnable = INTERRUPTION_DISABLE; //INTERRUPTION_x  x->DISABLE, ENABLE
	//Cargamos la configuracion del TIMER especifico
	BasicTimer_Config(&handler_TIMER_Sampling);

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

	//-------------------Inicio de Configuracion I2Cx----------------------

	//---------------I2C1----------------
	//Definimos el I2Cx a usar
	handler_I2C_MPU6050.prtI2Cx = I2C1;
	//Definimos la configuracion para el I2C
	handler_I2C_MPU6050.modeI2C = I2C_MODE_FM;               //I2C_MODE_x  x->SM,FM
	handler_I2C_MPU6050.slaveAddress = ADDRESS_DOWN;         //Direccion del Sclave
	//Cargamos la configuracion
	i2c_Config(&handler_I2C_MPU6050);

	//---------------------Fin de Configuracion I2Cx----------------------

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

//------------------------------Inicio configuracion del MPU------------------------------------------
void int_MPU(void)
{
	//Definimos la escala de las diferentes magnitudes fisicas
	handler_MPUAccel_MPU6050.fullScaleACCEL = ACCEL_2G;
	handler_MPUAccel_MPU6050.fullScaleGYRO = GYRO_250;
	//Definimos el handler correspondiente al I2C
	handler_MPUAccel_MPU6050.ptrI2Chandler = &handler_I2C_MPU6050;
	//Cargamos configuracion
	configMPUAccel(&handler_MPUAccel_MPU6050);

}

//------------------------------Fin configuracion del MPU------------------------------------------



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
	constains_calculator(&handler_Motor_R.parametersMotor.parametersPID, 2, 150, theta);   //k,tau,theta

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
	constains_calculator(&handler_Motor_L.parametersMotor.parametersPID, 2, 150, theta);   //k,tau,theta

	//---------------PID del la distancia-----------------
	//definicion de parametros
	parameter_PID_distace.e = parameter_PID_distace.e_1 = parameter_PID_distace.e_2 = 0;
	parameter_PID_distace.u = parameter_PID_distace.u_1 = 0;
	//Calculo de Constantes PID
	constains_calculator(&parameter_PID_distace, 2, 200, theta);    //k,tau,theta

};


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

//------------------------------Fin configuracion de los motores------------------------------------------





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
	//----------------Accion a Realizar con el tiempo del TIMER--------------------
	if(flag_motor==1)
	{
		//Calculamos el angulo
		ang_Row = getAngle(&handler_MPUAccel_MPU6050, &sample_Gyro, ang_Row, READ_GYRO_Z, gyro_offset);
		//Acumulamos los angulos
		sum_ang += ang_Row;
		//Se acumula el tiempo
		time_accumulated += sample_Gyro.delta_timer;
	}
	else{ __NOP(); }

	//----------------Accion a realizar con un tiempo especifico--------------------
	if(counting_action>=timeAction_TIMER_Sampling)
	{
		if(flag_motor==1)
		{
			//Calculamos el angulo promedio y la establecemis como el angulo relativo
			float prom_ang = sum_ang/counting_action;
			parameter_Posicion_Robot.phi_relativo = (prom_ang*PI)/180;          //[rad]
			parameter_Posicion_Robot.phi_relativo = atan2(sin(parameter_Posicion_Robot.phi_relativo),cos(parameter_Posicion_Robot.phi_relativo));
			//Calculamos la velocidad
			handler_Motor_L.parametersMotor.distance = (cm_L*handler_Motor_L.parametersMotor.count);                   //[mm]
			handler_Motor_R.parametersMotor.distance = (cm_R*handler_Motor_R.parametersMotor.count);				   //[mm]
			handler_Motor_L.parametersMotor.velocity = handler_Motor_L.parametersMotor.distance/time_accumulated;      //[m/s]
			handler_Motor_R.parametersMotor.velocity = handler_Motor_R.parametersMotor.distance/time_accumulated;      //[m/s]
			//Reiniciamos el numero de conteos
			handler_Motor_R.parametersMotor.count = 0;
			handler_Motor_L.parametersMotor.count = 0;
			//Subimos Bandera
			flag_count_odometry = 1;
			//Reiniciamos tiempo
			time_accumulated = 0;
			//Reiniciamos variable
			sum_ang = 0;
		}
		else if(flag_motor==3)
		{
			//Aumentamos el contador de tiempo
			count_time = count_time + timeAction_TIMER_Sampling;       //Tiempo en ms
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
			handler_Motor_L.parametersMotor.velocity = handler_Motor_L.parametersMotor.distance/timeAction_TIMER_Sampling;   //[m/s]
			handler_Motor_R.parametersMotor.velocity = handler_Motor_R.parametersMotor.distance/timeAction_TIMER_Sampling;   //[m/s]
			//Aumentamos el contador de tiempo
			count_time = count_time + timeAction_TIMER_Sampling;       //Tiempo en ms
			//Convertimos el valor y imprimemos
			sprintf(bufferMsg,"%u\t%#.4f\t%#.4f\n", count_time, handler_Motor_L.parametersMotor.velocity , handler_Motor_R.parametersMotor.velocity);
			writeMsgForTXE(&handler_USART_USB, bufferMsg);
			//Reiniciamos el numero de conteos
			handler_Motor_R.parametersMotor.count = 0;
			handler_Motor_L.parametersMotor.count = 0;
		}
		//Reiniciamos el contador de accion
		counting_action = 0;

	}
	else{ counting_action++; }

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
	handler_Motor_R.parametersMotor.count++;
	handler_Motor_R.parametersMotor.countCotinuous++;
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
		writeMsgForTXE(&handler_USART_USB, "3) line # # ---Inicia Linea recta, #: dist [mm], #: dutty inicial \n");
		writeMsgForTXE(&handler_USART_USB, "4) turn # # # # ---Iniciamos el giro del robot #: ang #:dir #:dutty_R \n");
		writeMsgForTXE(&handler_USART_USB, "6) stop ---Para el estudio en medio de la ejecucion \n");
		writeMsgForTXE(&handler_USART_USB, "0) const # # # ---Constantes del PID #: kp,ti,td \n");
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
		//parameter_Posicion_Robot.xg_position_inicial = parameter_Posicion_Robot.xg_position;
		//parameter_Posicion_Robot.yg_position_inicial = parameter_Posicion_Robot.yg_position;
		parameter_Posicion_Robot.xg_position = 0;
		parameter_Posicion_Robot.yg_position = 0;
		parameter_Posicion_Robot.ang_Giro += turn;
		//Coordenadas relativas
		parameter_Posicion_Robot.xr_position = 0;
		parameter_Posicion_Robot.yr_position = 0;
		ang_Row = 0;
		//Calculos extra
		cos_cal = cos(parameter_Posicion_Robot.ang_Giro);
		sin_cal = sin(parameter_Posicion_Robot.ang_Giro);
		//-------------Configruacion Modo-------------
		//Definimos la frecuencia del Timer contador
		timeAction_TIMER_Sampling = (Ts/20);
		//Defimos la distancia a recorrer
		distance = firtsParameter;
		//Definimos el dutty inicial
		duttyInicial = setpoint_dutty = secondParameter;
		//uRef = (duttyInicial+26)/563;
		//Definimos la Setpoint del dutty y del phi
		//setpoint_dutty = thirdParameter;
		//Establecemos valores iniciales
		//handler_Motor_L.parametersMotor.parametersPID.u_1 = handler_Motor_R.parametersMotor.parametersPID.u_1 = duttyInicial;
		handler_Motor_L.parametersMotor.parametersPID.e = handler_Motor_L.parametersMotor.parametersPID.e_1 = handler_Motor_L.parametersMotor.parametersPID.e_2 = 0;
		handler_Motor_R.parametersMotor.parametersPID.e = handler_Motor_R.parametersMotor.parametersPID.e_1 = handler_Motor_R.parametersMotor.parametersPID.e_2 = 0;
		//parameter_PID_distace.e = parameter_PID_distace.e_1 = parameter_PID_distace.e_2 = parameter_PID_distace.u_1 = 0;
		//Cargamos la configuracion
		config_motor(1, timeAction_TIMER_Sampling, duttyInicial, duttyInicial, frequency_PWM_Motor);//Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
		//Iniciamos los motores
		status_motor(SET);
		//Medimos el tiempo inicial
		sample_Gyro.timer_prev = getTicksMs();
		//-----------------------------Variables a organizar--------------
		//Calibracion del eje Z del giroscopio
		gyro_offset = calibrationMPU(&handler_MPUAccel_MPU6050, CAL_GYRO_Z);
		flag_PID = 0;
		time_accumulated = 0;
		counting_action = 0;

	}
	else if (strcmp(cmd, "const") == 0)
	{
			constains_calculator(&parameter_PID_distace, firtsParameter, secondParameter, thirdParameter);   //kp,ti,td
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
		config_motor(2, timeAction_TIMER_Sampling, thirdParameter, thirdParameter, frequency_PWM_Motor);//Tipo de Estudio, periodo [ms], por dutty L, por dutty R, fre pwm [hz]
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
		port_dutty = (0.9234*(ptrMotorHandler->parametersMotor.parametersPID.u) - 3) ;
		//port_dutty = ptrMotorHandler->parametersMotor.parametersPID.u ;
	}
	else
	{
		//Conversion a dutty
		port_dutty = ptrMotorHandler->parametersMotor.parametersPID.u ;
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
			statusiInterruptionTimer(&handler_TIMER_Sampling, INTERRUPTION_ENABLE);
		}
	}
	else
	{
		//Desactivamos interrupcion
		if(flag_motor!=2)
		{
			statusiInterruptionTimer(&handler_TIMER_Sampling, INTERRUPTION_DISABLE);
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
	//updateFrequencyTimer(&handler_TIMER_Sampling, firth);
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
}


//--------------------------  --Fin de la definicion de las funciones------------------------------------------

