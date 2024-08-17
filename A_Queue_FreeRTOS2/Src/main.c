/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 */

//Incluir archivo punto h para el main
#include "main.h"

//---------------------------Inicio de definicion de funciones y variables base----------------------------------
GPIO_Handler_t handler_GPIO_BlinkyPin = {0};    //Definimos un elemento del tipo GPIO_Handler_t (Struct) para el LED
void initSystem(void);                          //Definimos la cabecera para la configuracion
//---------------------------Fin de definicion de funciones y variables base----------------------------------

//-------------------------PIN MCO2--------------------------------
GPIO_Handler_t handler_GPIO_MCO2 = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct) para utilizar el pin MCO2 con el fin de muestrear las frecuencias de los osciladores
void int_MCO2(void);                      //Funcion para la configuracion inicail del MCO2


//--------------------------USART-------------------------------
GPIO_Handler_t handler_GPIO_CommTerm_TX = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct) y USART_Handler_t para el uso del USB
GPIO_Handler_t handler_GPIO_CommTerm_RX = {0};
USART_Handler_t handler_USART_CommTerm = {0};
char usartData = 'w';                           //Variable que almacena el caracter leido

//-------------------------SEGGER-----------------------------------
//extern void SEGGER_UART_init(uint32_t);     //Le indicamos al sistema que hay una funcion para inicio de la comunicacion del SEGGER por UART

//-------------------------FreeRTOS-----------------------------------
//Definicion de variables para la configuracion inicial del FreeRTOS
uint32_t SystemCoreClock = 100E6;
#define STACK_SIZE 200
//Variable para comprobar la creacion de la tarea
BaseType_t xReturned;

//Handler de las funciones de las Tareas
extern void vTask_Menu(void * pvParameters);
extern void vTask_Print(void * pvParameters);
extern void vTask_Commands(void * pvParameters);
//Handler de las Tareas
TaskHandle_t xHandleTask_Menu = NULL;
TaskHandle_t xHandleTask_Print = NULL;
TaskHandle_t xHandleTask_Commands = NULL;

//Handler de las Queue
QueueHandle_t xQueue_Print;
QueueHandle_t xQueue_InputData;

//Handler para el Software Timer
TimerHandle_t handler_led_timer;

//Variables adicionales aplicacion
state_t next_state = sMainMenu;


int main(void)
{
	//Incrementamos la velocidad de reloj del sistema
	uint8_t clock = CLOCK_SPEED_100MHZ;    //Velocidad de reloj entre 25 o 100 MHz
	configPLL(clock);
	//Configuracion inicial del sistema
	initSystem();
	//Definimos la configuracion inicail del MCO1
	int_MCO2();
	//Activamos la unidad de punto flotante (FPU)
	SCB->CPACR    |= (0xF << 20);
	//Activamos del contador de Ticks
	DWT->CTRL    |= (1 << 0);


	//---------------------Inicio de uso de funciones para el funcionamiento del SEGGER----------------------
	//Necesaria para el SEGGER
//	vInitPrioGroupValue();
//	//Configuramos el puerto Serial para trabajar  con el SEGGER
//	SEGGER_UART_init(500000);
//	/* Primero configuramos */
//	SEGGER_SYSVIEW_Conf();
	//-----------------------Fin de uso de funciones para el funcionamiento del SEGGER----------------------


	//-----------------------Inicio cofiguracion de los elemntos del kernel de FreeRTOS----------------------

	//-------------------Configuracion Task--------------
	//Tarea Menu
	xReturned = xTaskCreate(
						vTask_Menu,       /* Function that implements the task. */
	                    "Task_Menu",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Menu);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea Print
	xReturned = xTaskCreate(
						vTask_Print,       /* Function that implements the task. */
	                    "Task_Print",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Print);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);
	//Tarea comandos
	xReturned = xTaskCreate(
						vTask_Commands,       /* Function that implements the task. */
	                    "Task_Commands",          /* Text name for the task. */
	                    STACK_SIZE,      /* Stack size in words, not bytes. */
						NULL,    /* Parameter passed into the task. */
	                    2,/* Priority at which the task is created. */
	                    &xHandleTask_Commands);      /* Used to pass out the created task's handle. */
	configASSERT(xReturned == pdPASS);

	//-------------------Configuracion Queue--------------
	//Cola para recibir datos por consola
	xQueue_InputData = xQueueCreate(10, sizeof( char ) );
	configASSERT(xQueue_InputData != NULL);
	//cola para enviar datos por consola
	xQueue_Print = xQueueCreate(10, sizeof( size_t) );
	configASSERT(xQueue_Print != NULL);

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


	//---------------------------USART--------------------------------
	//---------------PIN: PA2----------------
	//------------AF7: USART2_TX----------------
	//Definimos el periferico GPIOx a usar.
	handler_GPIO_CommTerm_TX.pGPIOx = GPIOA;
	//Definimos el pin a utilizar
	handler_GPIO_CommTerm_TX.GPIO_PinConfig.GPIO_PinNumber = PIN_2; 						//PIN_x, 0-15
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
	handler_GPIO_CommTerm_RX.GPIO_PinConfig.GPIO_PinNumber = PIN_3; 						//PIN_x, 0-15
	//Definimos la configuracion de los registro para el pin seleccionado
	// Orden de elementos: (Struct, Mode, Otyper, Ospeedr, Pupdr, AF)
	GPIO_PIN_Config(&handler_GPIO_CommTerm_RX, GPIO_MODE_ALTFN, GPIO_OTYPER_PUSHPULL, GPIO_OSPEEDR_MEDIUM, GPIO_PUPDR_NOTHING, AF7);
	/*Opciones: GPIO_Tipo_x, donde x--->||IN, OUT, ALTFN, ANALOG ||| PUSHPULL, OPENDRAIN |||
	 * ||| LOW, MEDIUM, FAST, HIGH ||| NOTHING, PULLUP, PULLDOWN, RESERVED |||  AFx, 0-15 |||*/
	//Cargamos la configuracion del PIN especifico
	GPIO_Config(&handler_GPIO_CommTerm_RX);


	//---------------------------------Fin de Configuracion GPIOx---------------------------------


	//-------------------Inicio de Configuracion USARTx-----------------------

	//---------------USART2----------------
	//Definimos el periferico USARTx a utilizar
	handler_USART_CommTerm.ptrUSARTx = USART2;
	//Definimos la configuracion del USART seleccionado
	handler_USART_CommTerm.USART_Config.USART_mode = USART_MODE_RXTX ;           //USART_MODE_x  x-> TX, RX, RXTX, DISABLE
	handler_USART_CommTerm.USART_Config.USART_baudrate = USART_BAUDRATE_115200;  //USART_BAUDRATE_x  x->9600, 19200, 115200
	handler_USART_CommTerm.USART_Config.USART_parity= USART_PARITY_NONE;       //USART_PARITY_x   x->NONE, ODD, EVEN
	handler_USART_CommTerm.USART_Config.USART_stopbits=USART_STOPBIT_1;         //USART_STOPBIT_x  x->1, 0_5, 2, 1_5
	handler_USART_CommTerm.USART_Config.USART_enableIntRX = USART_RX_INTERRUP_ENABLE;   //USART_RX_INTERRUP_x  x-> DISABLE, ENABLE
	handler_USART_CommTerm.USART_Config.USART_enableIntTX = USART_TX_INTERRUP_DISABLE;   //USART_TX_INTERRUP_x  x-> DISABLE, ENABLE
	//Cargamos la configuracion del USART especifico
	usart_Config_Init_Priority(&handler_USART_CommTerm, e_USART_PRIOPITY_6);
	USART_Config(&handler_USART_CommTerm);

	//-------------------Fin de Configuracion USARTx-----------------------

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




//----------------------------Inicio de la definicion de las funciones ISR---------------------------------------


//-------------------------USARTRX--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el USART2
void BasicUSART2_Callback(void)
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
		if(usartData == '#')
		{
			//Se recibe mensaje de la cola
			xQueueReceiveFromISR(xQueue_InputData, (void *)&usartData, NULL);
			//Se envia mensaje a la cola
			xQueueSendToBackFromISR(xQueue_InputData, (void *)&usartData, NULL);
		}
	}
	//Se envia notificacion al command Task
	if(usartData == '#')
	{
		xTaskNotifyFromISR(xHandleTask_Commands, 0, eNoAction, NULL);
	}
}

//----------------------------Fin de la definicion de las funciones ISR----------------------------------------



//----------------------Inicio de la definicion de las funciones del Software Timer----------------------------------
void led_state_callback(TimerHandle_t xTimer)
{
	//Cambio estado opuesto Led
	GPIOxTooglePin(&handler_GPIO_BlinkyPin);
}
//---------------------Fin de la definicion de las funciones del Software Timer----------------------------------
