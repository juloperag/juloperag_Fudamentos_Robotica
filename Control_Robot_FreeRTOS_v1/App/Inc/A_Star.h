#include <stdio.h>
#include <stdint.h>
#include "grid_map.h"

//Definicion de macros
#define OPEN 1
#define CLOSED 2

typedef struct
{
	uint8_t grid_map_row;                      //filas de la malla
	uint8_t grid_map_colum;                    //columnas de la malla
	float cell_separation;                     //sepacion entre las celdas, en cm
	float start_x;                             //posicion inicial en x
	float start_y;                             //posicion inicial en y
	float goal_x;                              //posicion final en x
	float goal_y;                              //posicion final en y
	Cell_map_t grid_map[20][20];               //Grid map
}item_A_Star_t;


//Estructura que contiene la informacion de una ficha 
typedef struct
{
  Cell_map_t *ptrCell_file;          //Puntero de la celda de la cual se crea la ficha
  Cell_map_t *ptrCell_parent[64];    //Punteros de los diferentes padres de la celda que se creo la ficha
  uint8_t num_parent;                //Numero de padres de la celda que se creo la ficha
  float cost_g;                      //Costo acumulado del viaje desde el inicio hasta la celda actual
  float function_F;                  //Funcion F = g + h
  
}file_cell_t;
//Definimos las cabeceras de las funciones
file_cell_t* aplicattion_A_Star(Cell_map_t grid[20][20], uint8_t row, uint8_t colum, float start_x, float start_y, float  goal_x, float goal_y);
uint8_t search_position_file_Open(file_cell_t list_file[64], uint64_t avan_file);
void heuristic_cell_map(Cell_map_t grid[20][20], uint8_t row, uint8_t colum, float goal_x, float goal_y);
float heuristic_single_cell(Cell_map_t *cell, float  goal_x, float goal_y);
