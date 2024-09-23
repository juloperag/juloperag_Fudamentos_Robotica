/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Control de Robot con FreeRTOS
 ******************************************************************************
 */

//Incluir archivo punto h para el main
#include "main.h"

//---------------------------Inicio de definicion de funciones y variables base----------------------------------
GPIO_Handler_t handler_GPIO_BlinkyPin = {0};    //Definimos un elemento del tipo GPIO_Handler_t (Struct) para el LED
void initSystem(void);                          //Definimos la cabecera para la configuracion
//---------------------------Fin de definicion de funciones y variables base----------------------------------

//--------------------------USART-------------------------------
GPIO_Handler_t handler_GPIO_CommTerm_TX = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct) y USART_Handler_t para el uso del USB
GPIO_Handler_t handler_GPIO_CommTerm_RX = {0};
USART_Handler_t handler_USART_CommTerm = {0};
char usartData = 'w';                           //Variable que almacena el caracter leido

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
BasicTimer_Handler_t handler_TIMER_Delay = {0};       //Se define Timer para el delay
uint8_t period_sampling = 16;                        //[mm]
uint16_t timeAction_TIMER_Sampling = 13;            //Frecuencia del timer contador    // 16 ms  9-->144
uint16_t value_period = 4000;                        //Valor del periodo del timer, que sera luego se apunta

//------------------Definiciones generales----------------------------------
//-----Macros----
#define DL 5170                                 //Diametro rueda izquierda
#define DR 5145                                 //Diametro rueda Derecha
#define Ce 72                                   //Numero de interrupciones en el incoder
//-----Cabeceras de funciones----                                                                 		 //Funcion para la configuracion inicail del MCO1
void int_MPU(void);
void int_Config_Motor(void);
//------Variables para el delay-----
uint16_t countingTimer = 0;
//-----Variables del MPU-----
int16_t gyro_offset = 0;                         //Variable para guardad el offset de la calibracion
//-----Variables de los modos y operacion---
uint16_t frequency_PWM_Motor = 30;                //Frecuencia del timer del PWM
float velSetPoint = 0;
uint8_t duttySetPoint = 28;                       //Dutty setpoint de line
//---------Odometria---------
float cm_L = 0;                               //Factor de conversion rueda Izquierda [mm/cuentas]
float cm_R = 0;                               //Factor de conversion rueda Derecha   [mm/cuentas]
//-----Variables PID-----------
Parameters_PID_t parameter_PID_distace = {0};        //Structurapara los parametros del PID
//--------Variables del timerSampling---------
uint8_t time_accion = 0;                        //Variable para guardar el Tiempo entre acciones
uint16_t counting_action = 0;                     //Contador para la accion
//-----Variables de la recepcion de comandos----
uint8_t commandComplete = 1;                     //Bandera que indica si el comando esta completo
uint8_t counterRecepcion = 0;                    //Variable para la posicion del arrelgo
char bufferRecepcion[64] = {0};                  //Arreglo que almacena el comando ingresado

//----------------------------------FreeRTOS-----------------------------------------
//Definicion de variables para la configuracion inicial del FreeRTOS
uint32_t SystemCoreClock = 100E6;
#define STACK_SIZE 200
//Variable para comprobar la creacion de la tarea
BaseType_t xReturned;

//Handler de las funciones de las Tareas
extern void vTask_Menu(void * pvParameters);
extern void vTask_Print(void * pvParameters);
extern void vTask_Commands(void * pvParameters);
extern void vTask_Line(void * pvParameters);
extern void vTask_Measure(void * pvParameters);
extern void vTask_Line_PID(void * pvParameters);
extern void vTask_Turn(void *pvParameters);
extern void vTask_Square(void *pvParameters);
extern void vTask_Execute_AStar(void * pvParameters);
extern void vTask_Separate_GripMap(void * pvParameters);
extern void vTask_Apply_Astar(void * pvParameters);
extern void vTask_Execute_Operation(void *pvParameters);
extern void vTask_Stop_Execute(void * pvParameters);
extern void vTask_Stop(void * pvParameters);
//Handler de las Tareas
TaskHandle_t xHandleTask_Menu = NULL;
TaskHandle_t xHandleTask_Print = NULL;
TaskHandle_t xHandleTask_Commands = NULL;
TaskHandle_t xHandleTask_Line = NULL;
TaskHandle_t xHandleTask_Stop = NULL;
TaskHandle_t xHandleTask_Measure = NULL;
TaskHandle_t xHandleTask_Line_PID = NULL;
TaskHandle_t xHandleTask_Turn_itself = NULL;
TaskHandle_t xHandleTask_Square = NULL;
TaskHandle_t xHandleTask_Execute_Astar = NULL;
TaskHandle_t xHandleTask_Separate_GridMap = NULL;
TaskHandle_t xHandleTask_Apply_Astar = NULL;
TaskHandle_t xHandleTask_Execute_Operation = NULL;
TaskHandle_t xHandleTask_Stop_Execute = NULL;
//Handler de las Queue
QueueHandle_t xQueue_Print;
QueueHandle_t xQueue_StructCommand;
QueueHandle_t xQueue_InputData;
QueueHandle_t xQueue_Operation;
QueueHandle_t xMailbox_Mode;
QueueHandle_t xMailbox_Path;
//Handler de grupo de eventos
EventGroupHandle_t xEventGroup_Execute_Operation;
EventGroupHandle_t xEventGroup_Execute_Astar;

//Handler para el Software Timer
TimerHandle_t handler_led_timer;

//Variables adicionales aplicacion
state_t next_state = sMenuOperation;


int main(void)
{
	//Incrementamos la velocidad de reloj del sistema
	uint8_t clock = CLOCK_SPEED_100MHZ;    //Velocidad de reloj entre 25 o 100 MHz
	configPLL(clock);
	//Configuracion inicial del sistema
	initSystem();
	//Activamos la unidad de punto flotante (FPU)
	SCB->CPACR    |= (0xF << 20);
	//Activamos del contador de Ticks
	DWT->CTRL    |= (1 << 0);
	//-----------------------Configuracion inicial de los Motores---------------------------------
	//Cargar configuracion de los motores
	GPIO_writePin (&handler_GPIO_MotorR_IN, SET);
	GPIO_writePin (&handler_GPIO_MotorL_IN, SET);
	GPIO_writePin (&handler_GPIO_MotorR_EN, SET);
	GPIO_writePin (&handler_GPIO_MotorL_EN, SET);
	int_Config_Motor();
	//Calculo inicial de parametro
	cm_L = ((M_PI*DL)/(100*Ce));  //[mm/cuentas]
	cm_R =	((M_PI*DR)/(100*Ce));  //[mm/cuentas]
	//Calculamos el setpoint
	velSetPoint = (0.00169*duttySetPoint + 0.0619);
	//Calculo periodo de accion
	time_accion = period_sampling*timeAction_TIMER_Sampling;
	//--------------------------Configuramos inicia el MPU----------------------
	//Configuracion MPU
	int_MPU();
	//Calibracion del eje Z del giroscopio
	gyro_offset = calibrationMPU(&handler_MPUAccel_MPU6050, &handler_TIMER_Delay, &countingTimer, CAL_GYRO_Z);


	//-----------------------Inicio cofiguracion de los elemntos del kernel de FreeRTOS----------------------

	//-------------------Configuracion Task--------------
	//Tarea de mediciones de la linea recta
	xReturned = xTaskCreate(
						vTask_Measure,       /* Function that implements the task. */
	                    "Task_Measure",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    3,/* Priority at which the task is created. */
	                    &xHandleTask_Measure);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea de la aplicaciones de las medicciones PID de la linea recta
	xReturned = xTaskCreate(
						vTask_Line_PID,       /* Function that implements the task. */
	                    "Task_Line_PID",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    3,/* Priority at which the task is created. */
	                    &xHandleTask_Line_PID);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea que verifica si se llego a la condiccion de parada
	xReturned = xTaskCreate(
						vTask_Stop_Execute,       /* Function that implements the task. */
	                    "Task_Stop_Execute",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    3,/* Priority at which the task is created. */
	                    &xHandleTask_Stop_Execute);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);

	//Tarea comandos
	xReturned = xTaskCreate(
						vTask_Commands,       /* Function that implements the task. */
	                    "Task_Commands",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    4,/* Priority at which the task is created. */
	                    &xHandleTask_Commands);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea Print
	xReturned = xTaskCreate(
						vTask_Print,       /* Function that implements the task. */
	                    "Task_Print",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    4,/* Priority at which the task is created. */
	                    &xHandleTask_Print);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);

	//Tarea Menu
	xReturned = xTaskCreate(
						vTask_Menu,       /* Function that implements the task. */
	                    "Task_Menu",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Menu);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea execute Operation
	xReturned = xTaskCreate(
						vTask_Execute_Operation,       /* Function that implements the task. */
	                    "Task_Execute_Operation",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Execute_Operation);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea Line
	xReturned = xTaskCreate(
						vTask_Line,       /* Function that implements the task. */
	                    "Task_Line",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Line);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea turn itself
	xReturned = xTaskCreate(
						vTask_Turn,       /* Function that implements the task. */
	                    "Task_Turn",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Turn_itself);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea Square
	xReturned = xTaskCreate(
						vTask_Square,       /* Function that implements the task. */
	                    "Task_Square",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Square);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea execute Operation
	xReturned = xTaskCreate(
						vTask_Apply_Astar,       /* Function that implements the task. */
	                    "Task_Apply_Astar",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Apply_Astar);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea execute Operation
	xReturned = xTaskCreate(
						vTask_Separate_GripMap,       /* Function that implements the task. */
	                    "Task_Separate_GripMap",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Separate_GridMap);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea Execute_AStar
	xReturned = xTaskCreate(
						vTask_Execute_AStar,       /* Function that implements the task. */
	                    "Task_Execute_AStar",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Execute_Astar);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea Stop
	xReturned = xTaskCreate(
						vTask_Stop,       /* Function that implements the task. */
	                    "Task_Stop",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Stop);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);

	//-------------------Configuracion Queue--------------
	//Cola para recibir datos por consola
	xQueue_InputData = xQueueCreate(500, sizeof(char ) );
	configASSERT(xQueue_InputData != NULL);
	//Cola para recibir datos por consola
	xQueue_StructCommand = xQueueCreate(10, sizeof(command_t));
	configASSERT(xQueue_StructCommand != NULL);
	//cola para enviar datos por consola
	xQueue_Print = xQueueCreate(10, sizeof(char *) );
	configASSERT(xQueue_Print != NULL);
	//Buzon para definir el modo de operacion
	xMailbox_Mode = xQueueCreate(1, sizeof( uint8_t ));
	configASSERT(xMailbox_Mode != NULL);
	//Buzon para pasar el string del grid map
	xMailbox_Path = xQueueCreate(1, sizeof(file_cell_t * ));
	configASSERT(xMailbox_Path  != NULL);
	//Cola para almacenar las operaciones
	xQueue_Operation = xQueueCreate(30, sizeof(Parameters_Operation_t) );
	configASSERT(xQueue_Operation != NULL);

	//-------------------Configuracion Even Group-------------
	//Grupo de eventos para ejecutar las operaciones
	xEventGroup_Execute_Operation = xEventGroupCreate();
	//Grupo de eventos para ejecutar de A star
	xEventGroup_Execute_Astar = xEventGroupCreate();

	//-------------------Configuracion Timer--------------
	//Software Timer para el blink
	handler_led_timer = xTimerCreate("led_timer", pdMS_TO_TICKS(500), pdTRUE, 0, led_state_callback);
	xTimerStart(handler_led_timer, portMAX_DELAY);

	//-------------------Inicializacion Scheduler--------------
	//Inicia le Scheduler a funcionar
	vTaskStartScheduler();

	//-----------------------Fin cofiguracion de los elemntos del kernel de FreeRTOS----------------------

	//Si el scheduler se inicia correctamente no se ejecutada este while
	while(1)
	{
		__NOP();
	}
}


//------------------------------Inicio Configuracion del microcontrolador------------------------------------------
void initSystem(void)
{
	//---------------------------------Inicio de Configuracion GPIOx---------------------------------

	//---------------------------BlinkyLed--------------------------------
	//---------------PIN: PA5----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_BlinkyPin.pGPIOx = GPIOA;
	//Definimos el pin a utilizar
	handler_GPIO_BlinkyPin.GPIO_PinConfig.GPIO_PinNumber = PIN_5; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_BlinkyPin, GPIO_MODE_OUT, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF0);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_BlinkyPin);


	//---------------------------USART--------------------------------
	//---------------PIN: PA2----------------
	//------------AF7: USART2_TX----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_CommTerm_TX.pGPIOx = GPIOA;
	//Definimos el pin a utilizar
	handler_GPIO_CommTerm_TX.GPIO_PinConfig.GPIO_PinNumber = PIN_9; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_CommTerm_TX, GPIO_MODE_ALTFN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF7);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_CommTerm_TX);

	//---------------PIN: PA3----------------
	//------------AF7: USART2_RX----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_CommTerm_RX.pGPIOx = GPIOA;
	//Definimos el pin a utilizar
	handler_GPIO_CommTerm_RX.GPIO_PinConfig.GPIO_PinNumber = PIN_10; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_CommTerm_RX, GPIO_MODE_ALTFN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF7);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_CommTerm_RX);


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

	//---------------USART2----------------
	//Definimos el periferico USARTx a utilizar
	handler_USART_CommTerm.ptrUSARTx = USART1;
	//Definimos la configuracion del USART seleccionado
	handler_USART_CommTerm.USART_Config.USART_mode = USART_MODE_RXTX ;           //USART_MODE_x  x-> TX, RX, RXTX, DISABLE
	handler_USART_CommTerm.USART_Config.USART_baudrate = USART_BAUDRATE_19200;  //USART_BAUDRATE_x  x->9600, 19200, 115200
	handler_USART_CommTerm.USART_Config.USART_parity= USART_PARITY_NONE;       //USART_PARITY_x   x->NONE, ODD, EVEN
	handler_USART_CommTerm.USART_Config.USART_stopbits=USART_STOPBIT_1;         //USART_STOPBIT_x  x->1, 0_5, 2, 1_5
	handler_USART_CommTerm.USART_Config.USART_enableIntRX = USART_RX_INTERRUP_ENABLE;   //USART_RX_INTERRUP_x  x-> DISABLE, ENABLE
	handler_USART_CommTerm.USART_Config.USART_enableIntTX = USART_TX_INTERRUP_DISABLE;   //USART_TX_INTERRUP_x  x-> DISABLE, ENABLE
	//Cargamos la configuracion del USART especifico
	usart_Config_Init_Priority(&handler_USART_CommTerm, e_USART_PRIOPITY_6);
	USART_Config(&handler_USART_CommTerm);

	//-------------------Fin de Configuracion USARTx-----------------------

	//---------------TIM3----------------
	//Definimos el TIMx a usar
	handler_TIMER_Sampling.ptrTIMx = TIM3;
	//Definimos la configuracion del TIMER seleccionado
	handler_TIMER_Sampling.TIMx_Config.TIMx_periodcnt = BTIMER_PCNT_1ms; //BTIMER_PCNT_xus x->10,100/ BTIMER_PCNT_1ms
	handler_TIMER_Sampling.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; // BTIMER_MODE_x x->UP, DOWN
	handler_TIMER_Sampling.TIMx_Config.TIMX_period = period_sampling;   //Al definir 10us,100us el valor un multiplo de ellos, si es 1ms el valor es en ms
	handler_TIMER_Sampling.TIMx_Config.TIMx_interruptEnable = INTERRUPTION_DISABLE; //INTERRUPTION_x  x->DISABLE, ENABLE
	//Cargamos la configuracion del TIMER especifico
	timer_Config_Init_Priority(&handler_TIMER_Sampling, e_TIMER_PRIOPITY_6);
	BasicTimer_Config(&handler_TIMER_Sampling);

	//---------------TIM3----------------
	//Definimos el TIMx a usar
	handler_TIMER_Delay.ptrTIMx = TIM4;
	//Definimos la configuracion del TIMER seleccionado
	handler_TIMER_Delay.TIMx_Config.TIMx_periodcnt = BTIMER_PCNT_100us; //BTIMER_PCNT_xus x->10,100/ BTIMER_PCNT_1ms
	handler_TIMER_Delay.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; // BTIMER_MODE_x x->UP, DOWN
	handler_TIMER_Delay.TIMx_Config.TIMX_period = 10;   //Al definir 10us,100us el valor un multiplo de ellos, si es 1ms el valor es en ms
	handler_TIMER_Delay.TIMx_Config.TIMx_interruptEnable = INTERRUPTION_DISABLE; //INTERRUPTION_x  x->DISABLE, ENABLE
	//Cargamos la configuracion del TIMER especifico
	timer_Config_Init_Priority(&handler_TIMER_Delay, e_TIMER_PRIOPITY_6);
	BasicTimer_Config(&handler_TIMER_Delay);

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
	exti_Config_Int_Priority(&handler_EXTI_FotoR, e_EXTI_PRIOPITY_7);
	extInt_Config(&handler_EXTI_FotoR);

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
	exti_Config_Int_Priority(&handler_EXTI_FotoL, e_EXTI_PRIOPITY_7);
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






//------------------------------Inicio configuracion del MPU------------------------------------------
void int_MPU(void)
{
	//Definimos la escala de las diferentes magnitudes fisicas
	handler_MPUAccel_MPU6050.fullScaleACCEL = ACCEL_2G;
	handler_MPUAccel_MPU6050.fullScaleGYRO = GYRO_250;
	//Definimos el handler correspondiente al I2C
	handler_MPUAccel_MPU6050.ptrI2Chandler = &handler_I2C_MPU6050;
	//Cargamos configuracion
	configMPUAccel(&handler_MPUAccel_MPU6050, &handler_TIMER_Delay, &countingTimer);

}

//------------------------------Fin configuracion del MPU------------------------------------------

//------------------------------Inicio configuracion de lso motores------------------------------------------
void int_Config_Motor(void)
{
	//---------------Motor Derecho----------------
	//Parametro de la señal del dutty
	handler_Motor_R.configMotor.dutty =  28;
	handler_Motor_R.configMotor.frecuency = &value_period;
	handler_Motor_R.configMotor.dir = SET;
	//handler de los perifericos
	handler_Motor_R.phandlerGPIOEN = &handler_GPIO_MotorR_EN;
	handler_Motor_R.phandlerGPIOIN = &handler_GPIO_MotorR_IN;
	handler_Motor_R.phandlerPWM = &handler_PWM_MotorR;
	//definicion de parametros
	handler_Motor_R.parametersMotor.parametersPID.e = handler_Motor_R.parametersMotor.parametersPID.e_prev = 0;
	handler_Motor_R.parametersMotor.parametersPID.u =  handler_Motor_R.parametersMotor.parametersPID.e_intel = 0;
	//Calculo de Constantes PID
	handler_Motor_R.parametersMotor.parametersPID.kp = 250;
	handler_Motor_R.parametersMotor.parametersPID.ki = 0;
	handler_Motor_R.parametersMotor.parametersPID.kd = 100;

	//---------------Motor Izquierdo----------------
	//Parametro de la señal del dutty
	handler_Motor_L.configMotor.dutty =  28;
	handler_Motor_L.configMotor.frecuency = &value_period;
	handler_Motor_L.configMotor.dir = SET;
	//handler de los perifericos
	handler_Motor_L.phandlerGPIOEN = &handler_GPIO_MotorL_EN;
	handler_Motor_L.phandlerGPIOIN = &handler_GPIO_MotorL_IN;
	handler_Motor_L.phandlerPWM = &handler_PWM_MotorL;
	//definicion de parametros
	handler_Motor_L.parametersMotor.parametersPID.e = handler_Motor_L.parametersMotor.parametersPID.e_prev = 0;
	handler_Motor_L.parametersMotor.parametersPID.u =  handler_Motor_L.parametersMotor.parametersPID.e_intel = 0;
	//Calculo de Constantes PID
	handler_Motor_L.parametersMotor.parametersPID.kp = 250;
	handler_Motor_L.parametersMotor.parametersPID.ki = 0;
	handler_Motor_L.parametersMotor.parametersPID.kd = 100;

	//---------------PID del la distancia-----------------
	//definicion de parametros
	parameter_PID_distace.e = parameter_PID_distace.e_prev = 0;
	parameter_PID_distace.u =  parameter_PID_distace.e_intel = 0;
	//Calculo de Constantes PID
	parameter_PID_distace.kp = 1.0;
	parameter_PID_distace.ki = 0.1;
	parameter_PID_distace.kd = 0.8;
};

//------------------------------Fin configuracion de los motores------------------------------------------







//----------------------------Inicio de la definicion de las funciones ISR---------------------------------------

//-------------------------USARTRX--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el USART2
void BasicUSART1_Callback(void)
{
	usartData = getRxData();

	//Se define variable para verificar si una tarea de mayor proridad esta lista para Running
	BaseType_t pxHigherPriorityTaskWoken;
	(void) pxHigherPriorityTaskWoken;
	pxHigherPriorityTaskWoken = pdFALSE;

	//Verificamos que la cola aun no se encuentra llena
	xReturned = xQueueIsQueueFullFromISR(xQueue_InputData);

	//Por tanto si es True entonces aun no hay espacio
	if(xReturned != pdTRUE)
	{
		if(usartData != '\r' && usartData != '\n')
		{
			//Se envia caracter a la cola
			xQueueSendToBackFromISR(xQueue_InputData, (void *)&usartData, NULL);
		}
	}
	else
	{
		if(usartData == '@')
		{
			//Se recibe mensaje de la cola
			xQueueReceiveFromISR(xQueue_InputData, (void *)&usartData, NULL);
			//Se envia mensaje a la cola
			xQueueSendToBackFromISR(xQueue_InputData, (void *)&usartData, NULL);
		}
	}
	//Se envia notificacion al command Task
	if(usartData == '@')
	{
		xTaskNotifyFromISR(xHandleTask_Commands, 0, eNoAction, NULL);
	}
}

//-------------------------Muestreo--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el TIM2
void BasicTimer3_Callback(void)
{
	//Se define variable para verificar si una tarea de mayor proridad esta lista para Running
	BaseType_t pxHigherPriorityTaskWoken;
	pxHigherPriorityTaskWoken = pdFALSE;
	//Notificamos a la tarea respectiva
	xTaskNotifyFromISR(xHandleTask_Measure, 0, eNoAction, &pxHigherPriorityTaskWoken);
   	//Realizamos cambio de contexto
   	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

//-------------------------Muestreo--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el TIM2
void BasicTimer4_Callback(void)
{
	countingTimer++;
}

//----------------------------Fin de la definicion de las funciones ISR----------------------------------------





