/*
 * PositionRobot.h
 *
 *  Created on: Sep 1, 2024
 *      Author: julil
 */

#ifndef INC_POSITIONROBOT_H_
#define INC_POSITIONROBOT_H_

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
void change_position(Parameters_Path_t *ptrParameterPath, int distance);
void change_coordinates_position(Parameters_Path_t *ptrParameterPath, double coor_x, double coor_y);
void calculation_parameter_distance(Parameters_Path_t  *ptrParameterPath);
double distance_to_straight_line(Parameters_Path_t  *ptrParameterPath, double position_x, double position_y);
double distance_traveled(Parameters_Path_t  *ptrParameterPath, double position_x, double position_y);

#endif /* INC_POSITIONROBOT_H_ */
