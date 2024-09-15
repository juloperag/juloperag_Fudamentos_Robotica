/*
 * PositionRobot.h
 *
 *  Created on: Sep 1, 2024
 *      Author: julil
 */

#ifndef INC_POSITIONROBOT_H_
#define INC_POSITIONROBOT_H_

#include <stdint.h>

//----------Macros-----------
#define NULL_OPERATION 0
#define LINE 1
#define TURN 2

//---------Structs para las la ejecucion de operaciones------------
typedef struct
{
	double initline_x;           //coordenadas de inicio de la recta [mm]
	double initline_y;
	double delta_before[2];       //Vector director anterior [mm]
	double grad_vector_init;     //Grado de inclinacion de la linea recta[grados]
	uint8_t routelist;            //Recorrido de la lista
	uint8_t number_operation;    //numero de operacion
}Parameter_build_t;

typedef struct
{
	uint8_t operacion;          //Definicion de operacion
	double x_destination;       //coordenadas destino           [mm]
	double y_destination;
	double grad_Rotative;       //Grados de rotacion     [°]
}Parameters_Operation_t;



//-----------------Parametros medidos-----------------------
//Estructura que contiene la posicion del robot
typedef struct
{
	double grad_relativo;                                 //Angulo relativo      [grados]
	double phi_relativo;                                                      // [rad]
	double grad_grobal;                                   //Angulo acumulado     [grados]
	double xr_position, yr_position;        		      //Coordenadas relativas [mm]
	double xg_position_inicial, yg_position_inicial;      //Coordenadas gloabales iniciales [mm]
	double xg_position,yg_position;         			  //Coordenadas gloabales [mm]
}Parameters_Position_t;


//-----------------Parametros Teoricos-----------------------
typedef struct
{
	double magnitude_d;          //magnitud del vector director de la recta
	double proy_A;               //Parametros  para el calculo de la magnitud de la proyeccion sobre la recta
	double proy_B;
	double proy_C;
	double dis_point_A;          //Parametros para el calculo de la dsitancai entre la posicion global y la recta
	double dis_point_B;
	double dis_point_C;

}Parameters_Distance_t;


typedef struct
{
	int8_t direction_turn;                                //Direccion de giro
	Parameters_Distance_t parametersStraight;  //Parametros para el calculo de la distancia del punto a la recta como tambien a lo largo de la misma
	int rotative_Grad_Relative;                //Rotacion relativa en grados
	int rotative_Grad;                         //Rotacion en grados
	int line_Distance;                         //Distancia a recorrer
	double start_position_x;                   //Posicion de inicial al ejecutar la linea recta
	double start_position_y;
	double goal_Position_x;                    //Posicion destino al ejecutar la linea recta
	double goal_Position_y;
}Parameters_Path_t;

//Definicion de cabeceras de las funciones
void build_Operation(Parameters_Operation_t *prtList, Parameter_build_t *prtbuild, double finishline_x, double finishline_y);
void add_Operation(Parameters_Operation_t *prtList, uint8_t num_operation, uint8_t type_operation, double coor_x, double coor_y, double grad_turn);
void change_position(Parameters_Path_t *ptrParameterPath, int distance, double starcoor_x, double starcoor_y);
void change_coordinates_position(Parameters_Path_t *ptrParameterPath, double goalcoor_x, double goalcoor_y, double starcoor_x, double starcoor_y);
void calculation_parameter_distance(Parameters_Path_t  *ptrParameterPath);
double distance_to_straight_line(Parameters_Path_t  *ptrParameterPath, double position_x, double position_y);
double distance_traveled(Parameters_Path_t  *ptrParameterPath, double position_x, double position_y);
//Definicion de cabeceras de funciones auxiliares
double calculed_ang_turn(double vector_a[2], double vector_b[2]);

#endif /* INC_POSITIONROBOT_H_ */
