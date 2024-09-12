#include "grid_map.h"

// Posicion vecinos de la celda unidad 
int8_t ne_pos_x[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
int8_t ne_pos_y[8] = {1, 1, 1, 0, -1, -1, -1, 0};

// Función para crear la malla de celdas
void build_grid_map(Cell_map_t grid[20][20], uint8_t row, uint8_t colum, float separation)
{  
    //Se definen variables
    float rel_x;
    float rel_y;

    //---------------------Asignacion de informacion------------------------
    // Llenar la matriz de celdas con valores predeterminados
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < colum; j++) {
            // Asignar valores a la celda
            grid[i][j].id[0] = 'A' + j;             // Asignar un identificador simple, 'A', 'B', ...
            grid[i][j].id[1] = '0' + i;             // Asignar una cifra simple, '0', '1', '2', ...
            grid[i][j].index_row = i;               // Asignar el indice de la fila
            grid[i][j].index_col = j;               // Asignar el indice de la columna
            grid[i][j].coor_x = separation * j;     // Asignar la posición x de la celda
            grid[i][j].coor_y = separation * i;     // Asignar la posición y de la celda
            grid[i][j].status = 0;                  // Inicializar el estado de la celda;             
            // Asignacion de vecinos a la celda actual
            for (uint8_t k = 0; k < 8; k++) {
                int8_t index_row_ne = i + ne_pos_x[k]; 
                int8_t index_colum_ne = j + ne_pos_y[k]; 
                // Se evalua si el indice esta dentro de la malla
                if (index_row_ne >= 0 && index_row_ne < row && index_colum_ne >= 0 && index_colum_ne < colum) 
                {
                    grid[i][j].neighbors.ptrCellMap[k] = &grid[index_row_ne][index_colum_ne];
                } 
                else 
                {
                    grid[i][j].neighbors.ptrCellMap[k] = NULL;                    
                }
            }
        }
    }
    
    //------------------Distancias entre celdas------------------------
    //Se calcula la distancia de cada celda vecina
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < colum; j++) {
            for (uint8_t k = 0; k < 8; k++)
            {    
                if(grid[i][j].neighbors.ptrCellMap[k] != NULL)
                {
                    //Calculo vector relativo
                    rel_x = grid[i][j].neighbors.ptrCellMap[k]->coor_x - grid[i][j].coor_x;
                    rel_y = grid[i][j].neighbors.ptrCellMap[k]->coor_y - grid[i][j].coor_y;
                    //Ingresamos la distancia
                    grid[i][j].neighbors.distance_neigh[k] = sqrtf(rel_x * rel_x + rel_y * rel_y);
                }
                else
                {
                    //Ingreamos la distancia
                    grid[i][j].neighbors.distance_neigh[k] = -1.0f;
                }                
            }
        }
    }
}
