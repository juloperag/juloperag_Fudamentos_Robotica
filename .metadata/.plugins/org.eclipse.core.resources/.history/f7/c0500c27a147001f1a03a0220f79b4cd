#include "A_Star.h"
#include <float.h>
#include <math.h>

//-----------------------------------------------Funciones para la implementacion de A Star------------------------------------------------------------------
file_cell_t* aplicattion_A_Star(Cell_map_t **grid, uint8_t row, uint8_t colum, float start_x, float start_y, float goal_x, float goal_y) {
  //Variables
  file_cell_t *ptrFile;                           //Puntero a la ficha de la secuencia actual de A Star
  file_cell_t *file_Open[64] = {0};               //Arreglo de fichas abiertas
  uint64_t file_Open_Availability = 0b1;          //Conjunto de bits que indica que fichas abiertas estan disponibles para la comparacion
  uint8_t bit_file_cell = 0;                      //bit de una ficha que indica su disponibilidad
  uint8_t index_ptr = 0;                          //Indice de la ficha de la secuencia actual de A Star
  float min_f;                                    //Valor minimo de la funcion F            
  float min_h;                                    //Valor minimo de la heuristica H
  uint8_t index_file_Open;                        //Indice de la ficha de la ficha abierta a crear

  //----------------Se crea la ficha inicial----------------
  //Se crea la ficha inicial
  file_Open[0] = (file_cell_t *)malloc(sizeof(file_cell_t));
  file_Open[0]->num_parent = 0;
  file_Open[0]->cost_g = 0.0f; 
  //Por medio de un puntero se asigna la ficha inicial
  ptrFile = file_Open[0];
  
  // ------------- Se identifica la celda inicial -----------
  for (int i = 0; i < row; i++) 
  {
    for (int j = 0; j < colum; j++) 
    {
      if (grid[i][j].coor_x == start_x && grid[i][j].coor_y == start_y) 
      {
        ptrFile->ptrCell_file = &grid[i][j];  
        ptrFile->ptrCell_file->status  = OPEN;
        break;
      }
    }
  }

  while (1) {
    // Inicializamos variables
    min_f = DBL_MAX;
    min_h = DBL_MAX;

    //---------------------Creacion de las diferentes fichas de las celdas vecinas-----------------
    for(uint8_t k = 0; k < 8; k++) 
    {
      if(ptrFile->ptrCell_file->neighbors.ptrCellMap[k] != NULL) 
      {
        if(ptrFile->ptrCell_file->neighbors.ptrCellMap[k]->h != -1.0f && ptrFile->ptrCell_file->neighbors.ptrCellMap[k]->status < OPEN)
        {
          //Se busca la posicion que ocupada la nueva ficha en la lista de fichas abiertas
          index_file_Open = search_position_file_Open(file_Open, file_Open_Availability);
          //Se abre la celda
          ptrFile->ptrCell_file->neighbors.ptrCellMap[k]->status = OPEN;
          //Se crea la ficha
          file_Open[index_file_Open] = (file_cell_t *)malloc(sizeof(file_cell_t));
          file_Open[index_file_Open]->ptrCell_file = ptrFile->ptrCell_file->neighbors.ptrCellMap[k];
          for(uint8_t u = 0; u < ptrFile->num_parent; u++)
          {
            file_Open[index_file_Open]->ptrCell_parent[u] = ptrFile->ptrCell_parent[u];
          }
          file_Open[index_file_Open]->ptrCell_parent[ptrFile->num_parent] = ptrFile->ptrCell_file;
          file_Open[index_file_Open]->num_parent = ptrFile->num_parent+1;
          file_Open[index_file_Open]->cost_g = ptrFile->ptrCell_file->neighbors.distance_neigh[k]+ptrFile->cost_g;
          file_Open[index_file_Open]->function_F = file_Open[index_file_Open]->cost_g + ptrFile->ptrCell_file->neighbors.ptrCellMap[k]->h;
          //Indicacion de la disponibilidad de la ficha
          file_Open_Availability |= (0b1 << index_file_Open);
        }
      }
    }
    
    //-----------------Cerramos la ficha actual del puntero----------------------
    //indicamos el nuevo estado de la celda
    ptrFile->ptrCell_file->status = CLOSED;
    //Liberamos la memoria del arreglo de fichas
    free(ptrFile);
    //Indicacion de la no disponibilidad de la ficha
    file_Open_Availability &=  ~(0b1 << index_ptr);
     
    //---------------- Comparación función F ---------------------
    for (uint8_t k = 0; k < 64; k++) {
      //Recorrido
      bit_file_cell = (file_Open_Availability >> k) & 0b1;
      //Se el bit es un valor logico de 1 entonces la ficha esta disponible para su comparacion con las demas
      if(bit_file_cell==1)
      {
        // Verificar que la función F sea menor al valor anterior
        if (file_Open[k]->function_F < min_f) {
          // Reemplazamos valores
          min_f = file_Open[k]->function_F;
          min_h = file_Open[k]->ptrCell_file->h;
          index_ptr = k;
        }
        // Verificar que la función F sea igual al valor anterior, si es así se desempata con el valor h
        else if (file_Open[k]->function_F == min_f) {
          if (file_Open[k]->ptrCell_file->h < min_h) {
              // Reemplazamos valores
              min_f = file_Open[k]->function_F;
              min_h = file_Open[k]->ptrCell_file->h;
              index_ptr = k;         
          }       
        }
      }
    }
    
    //--------------- Seleccionamos nueva ficha para el puntero--------------------
    ptrFile = file_Open[index_ptr];
    
    //-----------------Verificacion si se llego al gol----------------------
    if (ptrFile->ptrCell_file->coor_x == goal_x && ptrFile->ptrCell_file->coor_y == goal_y) {
        break;
    }
  }
  return ptrFile;
}



uint8_t search_position_file_Open(file_cell_t **list_file, uint64_t avan_file)
{
  //Variables
  uint8_t bit_list = 0;
  uint8_t index = 100;
  float value_f = 0;
  //bucle para recorrer la lista de bits
  for(int i = 0; i<64; i++)
  {
    //Recorrido
    bit_list = (avan_file >> i) & 0b1;
    //Se comprueba el el bit de correspondiente a la disponibilidad de la ficha
    if(bit_list  == 0)
    {
      index = i;
      break;
    }
  }
  /*En caso que no se indico un valor diferente a 100 para el indice, se busca una ubicacion dentro 
  de la lista de fichas abiertas, seleccionando aquella ubicacion donde el valor de la funcion f
  sea la mayor*/
  if (index == 100)
  {  
    //Recorrido del arreglo de fichas abiertas
    for(int k = 0; k<64; k++)
    {  
      //Se busca el mayor valor de la funcion f
      if(list_file[k] != NULL && list_file[k]->function_F > value_f)
      {
        value_f = list_file[k]->function_F;
        index = k;
      }
    }
    //liberamos la memoria de la ficha
    free(list_file[index]);
  }
  
  return index;
}



//-----------------------------------------------Funciones calculo heuristica----------------------------------------------------------------
void heuristic_cell_map(Cell_map_t **grid, char **map_String, uint8_t row, uint8_t colum, float goal_x, float goal_y)
{
  //Recorrido por cada una de las celdas
  for (int i = 0; i < row; i++)
  {
    for (int j = 0; j < colum; j++) 
    {
      //Calculo heuristica
      if(grid[i][j].coor_x == goal_x && grid[i][j].coor_y == goal_y)
      {
        grid[i][j].h = 0.0f;
      }
      else if(map_String[i][j]=='#')
      {
        grid[i][j].h = -1.0f;
      }
      else
      {
        grid[i][j].h = heuristic_single_cell(&grid[i][j], goal_x, goal_y); 
      }   
    }
  }  
}


float heuristic_single_cell(Cell_map_t *cell, float  goal_x, float goal_y)
{
  //---------Definicion de variables--------
  //Variables del calculo heuristica
  Cell_map_t *ptrCell = cell;                 //Puntero a la celda actual
  float total_distante = 0;                   //Distancia total entre la celda actual y la celda del gol
  float rel_x;                                //Diferencia de las coordenadas x entre la celda actual y la celda vecina
  float rel_y;                                //Diferencia de las coordenadas y entre la celda actual y la celda vecina
  float ang_between_vector[8];                //Angulo entre el vector de la celda del gol y la celda vecina
  float res;                 
  float dis[8];                               //Distancia entre la celda vecina y la recta
  uint8_t best_index;                         //Mejor indice de las celdas vecinas
  float min_distance;                         //Distancia minima entre la celda vecina y la recta
  //Variables Recta
  float A_recta;                              //Parametros de la recta
  float B_recta = -1.0f;
  float C_recta;
  float delta_y;                              //Diferencia la coordenada y entre la celda inicial y la final
  float delta_x;                              //Diferencia la coordenada x entre la celda inicial y la final
  float mag_delta;                            //Magnitud del vector resultante de la diferencia de las coordenadas

  
   //---------------Calculos parametros de la recta-----------------
  delta_y = goal_y - cell->coor_y;
  delta_x = goal_x - cell->coor_x;
  mag_delta = sqrtf(delta_x * delta_x + delta_y * delta_y);
  A_recta =  delta_y / delta_x;
  C_recta = cell->coor_y - A_recta * cell->coor_x;

  while (1)
  { 
    //Inicializamos variables
    best_index = 0;
    min_distance = DBL_MAX;

    //------------------Calculo parametros de cada una de las celdas vecinas--------------------
    for (uint8_t k = 0; k < 8; k++)
    {
      //Verificamos que el puntero al vecino no es nulo
      if (ptrCell->neighbors.ptrCellMap[k] == NULL)
      {
        ang_between_vector[k] = -1.0f;
        dis[k] = -1.0f;          
      }
      else
      {
        //Calculo vector relativo
        rel_x = ptrCell->neighbors.ptrCellMap[k]->coor_x - ptrCell->coor_x;
        rel_y = ptrCell->neighbors.ptrCellMap[k]->coor_y - ptrCell->coor_y;
        //Calculo parametros
        res = round(((rel_x*delta_x + rel_y*delta_y) / (ptrCell->neighbors.distance_neigh[k] * mag_delta)) * 10000) / 10000;
        ang_between_vector[k] = acos(res);
        if(A_recta != INFINITY && A_recta != -INFINITY)
        {
          dis[k] = fabsf(A_recta * ptrCell->neighbors.ptrCellMap[k]->coor_x + B_recta * ptrCell->neighbors.ptrCellMap[k]->coor_y + C_recta) / sqrtf(A_recta * A_recta + 1);  
        }
        else
        {
          dis[k] = fabsf(ptrCell->neighbors.ptrCellMap[k]->coor_x-goal_x);
        }
      }            
    }

    //----------------Buscamos la celda vecina mas cercana a la recta y que este en la misma direccion que la recta---------------------
    //Recorremos los parametros para encontrar la mejor celda
    for (uint8_t k = 0; k < 8; k++)
    {
      //Verificamos que no sea -1
      if (ptrCell->neighbors.distance_neigh[k] != -1.0f)
      {
        //Verificamos que la diferencia entre los angulos no sea superior a 45°
        if (ang_between_vector[k] <= 1.5f)
        {
          //Verificar que la distancia sea menor a la anterior menor distancia
          if (dis[k] < min_distance)
          {
            min_distance = dis[k];
            best_index = k;
          }
        }          
      }        
    }

    
    //--------------Seleccionamos la celda vecina y sumamos su distancia--------------------
    total_distante = total_distante + ptrCell->neighbors.distance_neigh[best_index];
    ptrCell = ptrCell->neighbors.ptrCellMap[best_index];    

    
    //-----------------Verificacion si se llego al gol----------------------
    if (ptrCell->coor_x == goal_x && ptrCell->coor_y == goal_y)
    {
      break;
    }
  }

  return total_distante;
}

