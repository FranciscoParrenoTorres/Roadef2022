#pragma once
#ifndef PARAMETERS_H
#define PARAMETERS_H
class parameters
{
	
public:
	
	float inventory_cost; //coste de inventario alpha^I
	float transportation_cost; //coste de transporte alpha^t
	float extra_truck_cost; //coste de camion extra alpha^e
	int time_limit; //limite de tiempo
	double cam_x_peso;
	double cam_x_vol;
	string param_file ;
	string input_items ;
	string input_trucks ;
	string output_items ;
	string output_stack ;
	string output_trucks ;
	string report;
	double porcentaje;
	timeb tini;

	int tqueda();
	int tllevo();
};
#endif
