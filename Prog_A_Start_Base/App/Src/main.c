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
//----Perifericos-----
#include <GPIOxDriver.h>
#include <BasicTimer.h>
#include <USARTxDriver.h>
#include <PLLDriver.h>
//------CMSIS------
#include <arm_math.h>
#include <math.h>
//-----Proyecto-------
#include "A_Star.h"

//-----------------------------------Fin de definicion de librerias------------------------------------------


//---------------------------Inicio de definicion de funciones y variables base----------------------------------

GPIO_Handler_t handler_BlinkyPin = {0};         //Definimos un elemento del tipo GPIO_Handler_t (Struct) para el LED
BasicTimer_Handler_t handler_BlinkyTimer ={0};  //Definimos un elemento del tipo BasicTimer_Handler_t (Struct)
void int_Hardware(void);                        //Definimos la cabecera para la configuracion

//---------------------------Fin de definicion de funciones y variables base----------------------------------


//--------------------------USART-------------------------------
GPIO_Handler_t handler_GPIO_USB_TX = {0};       //Definimos un elemento del tipo GPIO_Handler_t (Struct) y USART_Handler_t para el uso del USB
GPIO_Handler_t handler_GPIO_USB_RX = {0};
USART_Handler_t handler_USART_USB = {0};
char charRead = 'w';                        //Variable que almacena el caracter leido
char bufferMsg[64] = {0};

//------------------Definiciones generales----------------------------------
//-----Cabeceras de funciones----
void recepcion_Sring_Parameter_Grid_Map(char newchar);                                                     //Funcion para recolectar los caracteres provenientes e la interfaz grafica
void Separate_parameters(Cell_map_t array_string[20][20], char *parameter_string);	                 //Funcion para separar los parametros presentes en el string
void send_path(file_cell_t *file_cell, Cell_map_t array_string[20][20], uint8_t row, uint8_t colum); //Funcion para imprimir la ruta encontrada
void recepcionCommand(void);                                                          		 //Funcion que recibe los caracteres del comando recibido
void runCommand(char *prtcommand);                                                    		 //Funcion que ejecuta el comando ingresando

//---Recepcion de string del grid map----
uint8_t	status_A_Star = 0;              	//Variable que indica el estado de la recoleccion de caracteres
char bufferParameterStringGM[450];          //Buffer para almacenar el string Grid Map
uint16_t index_String_GM = 0;              //Variable que indica el indice del buffer
//---------------Definicion Grid map----------------
Cell_map_t grid_map[20][20] = {0};
//---Parametros de la celda-------------
//String que indica los obstaculos dentro de la malla
uint8_t grid_map_row = 0;                                     //filas de la malla
uint8_t grid_map_colum = 0;                                   //columnas de la malla
float cell_separation = 0;                                    //sepacion entre las celdas, en cm
//--------Parametros iniciales del recorrido------------
float start_x = 0;                                            //posicion inicial en x
float start_y = 0;                                            //posicion inicial en y
float goal_x = 0;                                             //posicion final en x
float goal_y = 0;                                             //posicion final en y
//-----Variables de la ejecucion de A Star---------------
uint8_t string_aStar_Complete = 0;
uint8_t flag_A_Star = 0;
//-----Variables de la recepcion de comandos----
uint8_t commandComplete = 1;                     //Bandera que indica si el comando esta completo
uint8_t counterRecepcion = 0;                    //Variable para la posicion del arrelgo
char bufferRecepcion[64] = {0};                  //Arreglo que almacena el comando ingresado

void acelerometro_I2C(void);                       //Cabecera para la comunicacion I2C

int main(void)
{
	//-----------------------Configuracion inicial del sistema---------------------------------
	//Incrementamos la velocidad de reloj del sistema
	uint8_t clock = CLOCK_SPEED_100MHZ;    //Velocidad de reloj entre 25 o 100 MHz
	configPLL(clock);
	//Realizamos la configuracuion inicial
	int_Hardware();
	//Activamos el punto flotante por medio del registro especifico
	SCB->CPACR |= 0xF <<20;

	while(1)
	{
		//--------------------Comandos-----------------------
		if(commandComplete==1)
		{
			runCommand(bufferRecepcion);
			//Reniciamos la variable
			commandComplete=0;
		}
		else
		{ __NOP(); }

		//---------------A-Star-----------------
		if(flag_A_Star==1 && string_aStar_Complete==1)
		{
		 //-------------Separar el String en los parametros del grid m-------------
		  Separate_parameters(grid_map, bufferParameterStringGM);
		  //---------Creacion de la malla con cada una de sus celdas-----------
		  build_grid_map(grid_map, grid_map_row, grid_map_colum, cell_separation);
		  //-------Calculo de la heuristica de la celda de acuerdo a la posicion objetivo-------
		  heuristic_cell_map(grid_map, grid_map_row, grid_map_colum, goal_x, goal_y);
		  //------------------Aplicacion del algoritmo A star------------------
		  file_cell_t *file_path = aplicattion_A_Star(grid_map, grid_map_row, grid_map_colum, start_x, start_y, goal_x, goal_y);
		  //-----------------Impresion de la ruta encontrada--------------------
		  send_path(file_path, grid_map, grid_map_row, grid_map_colum);
		  //Reinicamos banderas
		  flag_A_Star = 0;
		  string_aStar_Complete = 0;
		}
		else{  __NOP(); }
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



	//-------------------Fin de Configuracion GPIOx-----------------------

	//-------------------Inicio de Configuracion USARTx-----------------------

	//---------------USART2----------------
	//Definimos el periferico USARTx a utilizar
	handler_USART_USB.ptrUSARTx = USART2;
	//Definimos la configuracion del USART seleccionado
	handler_USART_USB.USART_Config.USART_mode = USART_MODE_RXTX ;           //USART_MODE_x  x-> TX, RX, RXTX, DISABLE
	handler_USART_USB.USART_Config.USART_baudrate = USART_BAUDRATE_19200;  //USART_BAUDRATE_x  x->9600, 19200, 115200
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

	//-------------------Fin de Configuracion TIMx-----------------------



}
//------------------------------Fin Configuracion del microcontrolador------------------------------------------


//----------------------------Inicio de la definicion de las funciones ISR---------------------------------------

//-------------------------BlinkyLed--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el TIM2
void BasicTimer2_Callback(void)
{
	GPIOxTooglePin(&handler_BlinkyPin);
}

//-------------------------USARTRX--------------------------------
//Definimos la funcion que se desea ejecutar cuando se genera la interrupcion por el USART2
void BasicUSART2_Callback(void)
{
	//Guardamos el caracter recibido
	charRead = getRxData();
	if(flag_A_Star){ recepcion_Sring_Parameter_Grid_Map(charRead); }
	else{ recepcionCommand(); }
}


//----------------------------Fin de la definicion de las funciones ISR----------------------------------------

//----------------------------Inicio de la definicion de las funciones de los comandos----------------------------------------

void recepcion_Sring_Parameter_Grid_Map(char newchar)
{
	if(string_aStar_Complete!=1)
		if(newchar == '$')
		{
			//Almacenamos el elemento nulo
			bufferParameterStringGM[index_String_GM]= '\0';
			//Establecemos la bandera como alta
			string_aStar_Complete = 1;
			//Reiniciamos contador
			index_String_GM = 0;
		}
		else
		{
			//Almacenamos los caracteres del comando en un arrelgo
			bufferParameterStringGM[index_String_GM] = newchar;
			//Aumentamos en uno la posicion del arreglo
			index_String_GM++;
		}
	else{__NOP();}
}

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

	//char bufferMsg[64]= {0};

	//Funcion que lee la cadena de caracteres y la divide en los elementos definidos
	sscanf(prtcommand, "%s %u ", cmd, &firtsParameter);

	//Imprime lista que muestra los comandos que tiene el dispositivo
	if(strcmp(cmd, "help")==0)
	{
		writeMsgForTXE(&handler_USART_USB, "Help Menu: \n");
		writeMsgForTXE(&handler_USART_USB, "1) help  ---Imprime lista de comandos. \n");
		writeMsgForTXE(&handler_USART_USB, "2) aStar --- Indicador de inicio de aStar \n");
	}

	//----------------------Operacion de movimiento--------------------
	//Definimos el valor de las frecuencias para el study
	else if (strcmp(cmd,"applyastar") == 0)
	{
		//Levantamos bandera
		flag_A_Star = 1;
	}
}
//----------------------------Fin de la definicion de las funciones de los comandos----------------------------------------



//---------------------Funciones Auxiliares-----------------------
//Funcion para separar los diferentes parametros del string
void Separate_parameters(Cell_map_t array_string[20][20], char *parameter_string)
{
  //Definicion de variables
  char buffercharSeparate[10];
  uint8_t index_charSeparate = 0;
  uint8_t flag_Separate = 0;
  uint8_t status_parameter = 0;
  uint8_t index_init_Grid_map = 0;

  //---------------Definicion de los parametros iniciales---------------
  for (uint16_t i = 0; parameter_string[i] != '\0'; i++)
  {
    //Verificacion del estado
    if(status_parameter<3)
    {
      //Busqueda de la separacion dentro del string
      if (parameter_string[i] == ':')
      {
        buffercharSeparate[index_charSeparate] = '\0';
        index_charSeparate = 0;
        flag_Separate = 1;
      }
      else
      {
        buffercharSeparate[index_charSeparate] = parameter_string[i];
        index_charSeparate++;
      }
      //Si la bandera se levanta se asigna el valor correspondiente al parametro
      if(flag_Separate == 1)
      {
        switch(status_parameter)
        {
          case 0:
          {
            grid_map_row = atoi(buffercharSeparate);
            break;
          }
          case 1:
          {
            grid_map_colum = atoi(buffercharSeparate);
            break;
          }
          case 2:
          {
            cell_separation = atof(buffercharSeparate);
            break;
          }
        }
        status_parameter++;
        flag_Separate = 0;
      }
    }
    else
    {
      index_init_Grid_map = i;
      break;
    }
  }
  //---------------transformacion del string grid map en un array---------------
  //Variables para los indices
  uint8_t index_row = 0;
  uint8_t index_col = 0;

  //Separacion de cada caracter
  for (uint16_t i = index_init_Grid_map; parameter_string[i] != '\0'; i++)
  {
    if (parameter_string[i] == ';')
    {
      //Se aumenta el indice de la fila y se reinicia el indice de la columna
      index_row++;
      index_col = 0;
    }
    else
    {
      //Se guarda el caracter
      array_string[index_row][index_col].feature = parameter_string[i];
      //se aumenta el indice de la columna
      index_col++;
    }
  }
  //---------------Definicion del start y del goal---------------
  //Recorremo el array creado para encontrar la posicion de los objetivos
  for(int i = 0; i < grid_map_row; i++)
  {
    for(int j = 0; j < grid_map_colum; j++)
    {
      if(array_string[i][j].feature == 'S')
      {
        //Definimos su posicion
        start_x = j*cell_separation;
        start_y = i*cell_separation;
      }
      else if (array_string[i][j].feature == 'G')
      {
      //Definimos su posicion
        goal_x= j*cell_separation;
        goal_y= i*cell_separation;
      }
    }
  }
}


//Funcion para imprimir la ruta encontrada
void send_path(file_cell_t *file_cell, Cell_map_t array_string[20][20], uint8_t row, uint8_t colum)
{
  //Variables
  uint8_t index = 0;
  char buffermsg[22] = {0};

  //Cambiamos los caracteres de la malla de strings por caracteres que indican la ruta establecida con A Star
  while(1)
  {
	if(file_cell->ptrCell_parent[index] != NULL)
	{
	   array_string[file_cell->ptrCell_parent[index]->index_row][file_cell->ptrCell_parent[index]->index_col].feature = '+';
	  index++;
	}
	else
	{
	  break;
	}
  }
  //Indica de nuevo el inicio del recorrido
   array_string[file_cell->ptrCell_parent[0]->index_row][file_cell->ptrCell_parent[0]->index_col].feature = 'S';

  //Envio de caracter para indicar que se trata del grid map
  writeChar(&handler_USART_USB, '$');
  //Imprimir la malla modificada
  for(int i=0;i<row;i++)
  {
	for(int j=0;j<colum;j++)
	{
	  //Agregamos las caracteristicas por fila a un buffer
	  buffermsg[j] = array_string[i][j].feature;
	  index = j;
	}
	//Agragamos el valor nullo al final del string
	buffermsg[index+1] = '\0';
	//imprimimos el string
	sprintf(bufferMsg, "%s;",buffermsg);
	writeMsg(&handler_USART_USB, bufferMsg);
  }
}

//--------------------------  --Fin de la definicion de las funciones------------------------------------------

