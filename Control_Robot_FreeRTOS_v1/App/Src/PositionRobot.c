/*
 * PositionRobot.c
 *
 *  Created on: Sep 1, 2024
 *      Author: julil
 */


#include "PositionRobot.h"
#include <math.h>


//-------------------------Funcion para la definicion de operaciones--------------------------
void build_Operation(Parameters_Operation_t *prtList, Parameter_build_t *prtbuild, double finishline_x, double finishline_y)
{
	//Definicion el vector director
	double delta[2] = {finishline_x - prtbuild->initline_x, finishline_y - prtbuild->initline_y};
	//Calculo angululo entre vectores directores
	double grad_turn_res = calculed_ang_turn(prtbuild->delta_before, delta);
	//condicional de cambio de angulo
	if(grad_turn_res == 0)
	{
		//agregar operacion de linea recta
		 add_Operation(prtList, prtbuild->number_operation, LINE, finishline_x, finishline_y, 0);
	}
	else
	{
		//Agregar operacion de rotacion
		if(prtbuild->routelist>0){ prtbuild->number_operation++; }
		add_Operation(prtList, prtbuild->number_operation, TURN, 0, 0, grad_turn_res);
		//agregar operacion de linea recta
		prtbuild->number_operation++;
		add_Operation(prtList, prtbuild->number_operation, LINE, finishline_x, finishline_y, 0);
	}
	//Se redefine los valores iniciales
	prtbuild->delta_before[0] = delta[0];
	prtbuild->delta_before[1] = delta[1];
	prtbuild->initline_x = finishline_x;
	prtbuild->initline_y = finishline_y;
}

void add_Operation(Parameters_Operation_t *prtList, uint8_t num_operation, uint8_t type_operation, double coor_x, double coor_y, double grad_turn)
{
	prtList[num_operation].operacion = type_operation;
	prtList[num_operation].x_destination = coor_x;
	prtList[num_operation].y_destination = coor_y;
	prtList[num_operation].grad_Rotative = grad_turn;
}


//-----------------Funciones para definir los parametros de la poscion teorica--------------
void change_position(Parameters_Path_t *ptrParameterPath, int distance, double starcoor_x, double starcoor_y)
{
	//Definimos la distancia
	 ptrParameterPath->line_Distance = distance;                  //[mm]
	//Calculamos la posicicion
	double pot_x = ptrParameterPath->line_Distance*cos((ptrParameterPath->rotative_Grad*M_PI)/180);
	double pot_y = ptrParameterPath->line_Distance*sin((ptrParameterPath->rotative_Grad*M_PI)/180);
	//Guardamos Coordenadas iniciales
	ptrParameterPath->start_position_x = starcoor_x;
	ptrParameterPath->start_position_y = starcoor_y;
	//Definimos la nueva posicion de llegada
	ptrParameterPath->goal_Position_x += pot_x;
	ptrParameterPath->goal_Position_y += pot_y;
}

void change_coordinates_position(Parameters_Path_t *ptrParameterPath, double goalcoor_x, double goalcoor_y, double starcoor_x, double starcoor_y)
{
	//Guardamos Coordenadas iniciales
	ptrParameterPath->start_position_x = starcoor_x;
	ptrParameterPath->start_position_y = starcoor_y;
	//Definimos la nueva posicion de llegada
	ptrParameterPath->goal_Position_x = goalcoor_x;
	ptrParameterPath->goal_Position_y = goalcoor_y;
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




//------------------------------Funciones auxiliares-----------------------------

double calculed_ang_turn(double vector_a[2], double vector_b[2])
{
    //Calculo de los diferentes elementos
    double dot = vector_a[0]*vector_b[1]-vector_b[0]*vector_a[1];
    double magvector_a = sqrt(pow(vector_a[0],2)+pow(vector_a[1],2));
    double magvector_b = sqrt(pow(vector_b[0],2)+pow(vector_b[1],2));
    double ang_between_vector = acos((vector_a[0]*vector_b[0]+vector_a[1]*vector_b[1])/(magvector_b*magvector_a));
    //conversion a grados
    ang_between_vector = (ang_between_vector*180)/M_PI;
    //agregamos la direccion de giro
    if(dot<0){ ang_between_vector = -ang_between_vector;}
    //Aproximamos a cero si el angulo es menor a un valor
    if((ang_between_vector>-0.1) && (ang_between_vector<0.1)){ ang_between_vector = 0; }
    //Retornar valor
    return ang_between_vector;
}
