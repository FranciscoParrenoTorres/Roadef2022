
#include "encabezados.h"

#ifndef ITEM_H
#define ITEM_H
class truck;
class item
{
public:
	char Id[30];// id del item
	int CodIdCC; //número de identificación del item de 0 a el número de items en la CC
	char suplier_code[30]; // codigo del proveedor
	int suplier_code_int;
	char suplier_dock[30]; // codigo del muelle del proveedor
	int suplier_dock_int;
	char sup_cod_dock[60];
	long int sup_cod_dock_int;
	char plant_code[30]; // codigo de la planta
	int plant_code_int;
	char plant_dock[30]; // codigo del muelle de la planta
	int plant_dock_int;
	char product_code[30]; // codigo del producto
	int product_code_int;
	char package_code[30]; //codigo de empaquetado
	int num_items; // numero de items
	int CodId; //número de identificación del item de 0 a el número de items
	int Aleatorizado_inventory_cost; //VAlor aleatorizado para ordenaciones random
	int Aleatorizado_weight; //VAlor aleatorizado para ordenaciones random
	int Aleatorizado_densidad; //VAlor aleatorizado para ordenaciones random
	int Aleatorizado_volumen; //VAlor aleatorizado para ordenaciones random
	int length;	// longitud
	int width; // anchura
	int height; // altura
	float weight; // peso del item sin dividir entre 100
	int nesting_height; // altura de anidamiento
	char stackability_code[30]; // codigo de apilabilidad
	int stackability_code_int;  // codigo de apilabilidad en formato int
	char forced_orientation[30]; // orientacion forzada
	int forced_orientation_int; // 0 none, 1 lengthwise, 2 widthwise
	long long int earliest_arrival; // llegada mas temprana
	time_t earliest_arrival_tm; // llegada mas tardia en formato tm
	tm earliest_arrival_tm_ptr;
	long long int latest_arrival; // llegada mas tardia 
	time_t latest_arrival_tm; // llegada mas tardia en formato tm
	tm latest_arrival_tm_ptr;
	int inventory_cost; // coste de inventario CI_i
	int max_stackability; // maxima apilabilidad
	double volumen; // volumen de un item
	list<string> lista_camiones; //camiones en los que puede ir el item, porque el camión puede llevarlo y por ventana temporal.
	list<truck*> camiones; //punteros a camiones
	list<truck*> camiones_originales; //punteros a camiones originales
	int max_weight_on_bottom_item; //peso máximo que soporta este item si está debajo
	bool colocado;
	int num_items_original;
	double densidad;
	double vol_con_nest;
	
	double Volumen(int, int);
	static bool comparador(item* a, item* b) {
		return a->latest_arrival < b->latest_arrival;
	};
	/*
	bool operator < (item& a)const
	{
		
		if (this->latest_arrival > a.latest_arrival)
			return true;
		else
			return false;
	}*/
};
#endif
