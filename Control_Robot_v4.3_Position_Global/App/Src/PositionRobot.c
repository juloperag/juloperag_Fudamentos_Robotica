/*
 * PositionRobot.c
 *
 *  Created on: Sep 1, 2024
 *      Author: julil
 */


#include "PositionRobot.h"
#include <math.h>


//-----------------Funciones para definir los parametros de la poscion teorica--------------
void change_position(Parameters_Path_t *ptrParameterPath, int distance)
{
	//Definimos la distancia
	 ptrParameterPath->line_Distance = distance;                  //[mm]
	//Calculamos la posicicion
	double pot_x = ptrParameterPath->line_Distance*cos((ptrParameterPath->rotative_Grad*M_PI)/180);
	double pot_y = ptrParameterPath->line_Distance*sin((ptrParameterPath->rotative_Grad*M_PI)/180);
	//Guardamos la posicion del Goal como la posicion de Start
	ptrParameterPath->start_position_x = ptrParameterPath->goal_Position_x;
	ptrParameterPath->start_position_y = ptrParameterPath->goal_Position_y;
	//Definimos la nueva posicion de llegada
	ptrParameterPath->goal_Position_x += pot_x;
	ptrParameterPath->goal_Position_y += pot_y;
}

void change_coordinates_position(Parameters_Path_t *ptrParameterPath, double coor_x, double coor_y)
{
	//Guardamos la posicion del Goal como la posicion de Start
	ptrParameterPath->start_position_x = ptrParameterPath->goal_Position_x;
	ptrParameterPath->start_position_y = ptrParameterPath->goal_Position_y;
	//Definimos la nueva posicion de llegada
	ptrParameterPath->goal_Position_x = coor_x;
	ptrParameterPath->goal_Position_y = coor_y;
	//Definimos la distancia
	ptrParameterPath->line_Distance = sqrt(pow((ptrParameterPath->goal_Position_x - ptrParameterPath->start_position_x),2)+
			pow(ptrParameterPath->goal_Position_y - ptrParameterPath->start_position_y,2));;                  //[mm]

}

//-------------Funcion para calcular los parametros del calculo de la distancia---------------
void calculation_parameter_distance(Parameters_Path_t  *ptrParameterPath)
{
	//Calculo del vector director de la recta
	double a = ptrParameterPath->goal_Position_x - ptrParameterPath->start_position_x;
	double b = ptrParameterPath->goal_Position_y - ptrParameterPath->start_position_y;
	//definicion de la magnitud de director
	ptrParameterPath->parametersStraight.magnitude_d = sqrt(pow(a,2)+pow(b,2));
	//definicion de los parametros para el calculo de la magnitud de la proyeccion sobre la recta
	ptrParameterPath->parametersStraight.proy_A = a;
	ptrParameterPath->parametersStraight.proy_B = b;
	ptrParameterPath->parametersStraight.proy_C = -1*a*ptrParameterPath->start_position_x - b*ptrParameterPath->start_position_y;
	//definicion de los parametros para el calculo de la distancia del punto a la recta
	ptrParameterPath->parametersStraight.dis_point_A = b;
	ptrParameterPath->parametersStraight.dis_point_B = -1*a;
	ptrParameterPath->parametersStraight.dis_point_C = -1*b*ptrParameterPath->start_position_x + a*ptrParameterPath->start_position_y;
}

//------------Funciones para el calculo de la respectiva distancia------------------------
double distance_to_straight_line(Parameters_Path_t  *ptrParameterPath, double position_x, double position_y)
{
	//Calculo
	double distance = -1*((ptrParameterPath->parametersStraight.dis_point_A*position_x + ptrParameterPath->parametersStraight.dis_point_B*position_y
			+ ptrParameterPath->parametersStraight.dis_point_C)/ptrParameterPath->parametersStraight.magnitude_d);
	//Retornar
	return distance;
}

double distance_traveled(Parameters_Path_t  *ptrParameterPath, double position_x, double position_y)
{
	//Calculo
	double distance_Tra = (ptrParameterPath->parametersStraight.proy_A*position_x + ptrParameterPath->parametersStraight.proy_B*position_y
			+ ptrParameterPath->parametersStraight.proy_C)/ptrParameterPath->parametersStraight.magnitude_d;
	//Retornar
	return distance_Tra;
}

