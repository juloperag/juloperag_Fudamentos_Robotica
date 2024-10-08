#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

//Declaración adelantada de Cell_map_t
typedef struct Cell_map_t Cell_map_t;

//Estructura que contiene punteros a las estructuras de las celdas vecinas de la celda actual
typedef struct
{
    Cell_map_t *ptrCellMap[8];               //Punturos a las estructuras de las celdas vecinas de la celda actual
    float distance_neigh[8];                 //Distancia entre la celda actual y la celda vecina

}Cell_map_neighbors_t;


//Estructura que contiene la informacion de una celda
typedef struct Cell_map_t
{
    char id[2];                                //Identificador de la celda
    uint8_t index_row;                         //Indice de la fila en la malla
    uint8_t index_col;                         //Indice de la columna en la malla
    float coor_x;                              //Posicion x de la celda    
    float coor_y;                              //Posicion y de la celda
    Cell_map_neighbors_t neighbors;            //Punteros a la estructuras de las celdas vecinas de la celda actual
    //parametros para A-Star
    char feature;                              //Caracter que indica la celda
    float h;                                   //Heuristica de la celda
    uint8_t status;                            //Estado de la celda
    
}Cell_map_t;

//Definimos las cabeceras de las funciones
void build_grid_map(Cell_map_t grid[20][20], uint8_t row, uint8_t colum, float separation);
