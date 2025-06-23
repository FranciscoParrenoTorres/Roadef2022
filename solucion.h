#include "encabezados.h"
#pragma once
#ifndef solucion_H
#define solucion_H

class truck_sol
{
public:
	string id_truc;
	int load_length;
	double weight_loaded;
	double vol_loaded;
	double weight_middle_axle;
	double weigth_rare_axle;
	double peso_max;//Peso máximo que puede llevar
	double volumen_max;//volumen máximo que puede llevar
	double tmt; //peso de todas las pilas de la solucion
	double volumen_temp_ocupado; //volumen que he colocado en la solución
	double peso_x_medio; //peso de la pila por punto medio
	double eje; //distancia cog y el principio camión
	double ejr; //distancia cog y el eje trasero.
	double emh; //peso en el harness eje
	double emr; //peso en el eje trasero
	double emm; //peso en el eje medio
	double emr_anterior; //peso en el eje trasero
	double emm_anterior; //peso en el eje medio
	int coste;
	bool es_extra;
	int orden_por_el_que_voy;
	void Peso_max(double peso)
	{
		peso_max = peso;
	}
	void Vol_max(double vol)
	{
		volumen_max = vol;
	}
	void reset()
	{
		load_length = 0;
		weight_loaded = 0;
		vol_loaded = 0;
		weight_middle_axle = 0;
		weigth_rare_axle = 0;
		tmt = 0;
		volumen_temp_ocupado = 0;
		peso_x_medio = 0;
		eje = 0;
		ejr = 0;
		emh = 0;
		emr = 0;
		emm = 0;
		emr_anterior = 0;
		emm_anterior = 0;
		orden_por_el_que_voy = 0;

	}
	truck_sol(string id)
	{
		id_truc = id;
		load_length = 0;
		weight_loaded = 0;
		vol_loaded = 0;
		weight_middle_axle = 0;
		weigth_rare_axle = 0;
		tmt = 0;
		volumen_temp_ocupado = 0;
		peso_x_medio = 0;
		eje = 0;
		ejr = 0;
		emh = 0;
		emr = 0;
		emm = 0;
		emr_anterior = 0;
		emm_anterior = 0;
		peso_max = 0;
		orden_por_el_que_voy = 0;
		coste = 0;
		if (id[0] == 'Q') es_extra = true;
		else es_extra = false;
		
	};
    

    


	void escribe(string file)
	{
		ofstream f(file, ios_base::app);
		f << id_truc << ";" << load_length << ";" << weight_loaded << ";" << vol_loaded << ";" << emm<< ";" << emr << endl;
		f.close();
	}
	void escribe( ofstream &f)
	{
//		ofstream f(file, ios_base::app);
		f << id_truc << ";" << load_length << ";" << weight_loaded << ";" << vol_loaded << ";" << emm << ";" << emr << "\n";
//		f.close();
	}
};
class stack_sol
{
public:
	string id_truck;
	string id_stack;
	string stack_code;
	int x_ini;
	int y_ini;
	int z_ini;
	int x_fin;
	int y_fin;
	int z_fin;
	float peso;
	float densidad=0;
	int max_stack_item_abajo;
	int orientacion;
	int num_items;
	double inventory_cost;
	double volumen;
	double max_weight_item_abajo;
	double peso_encima_item_abajo;
	vector<char> letras = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

	stack_sol()
	{
		x_ini = 0;
		y_ini = 0;
		z_ini = 0;
		x_fin = 0;
		y_fin = 0;
		z_fin = 0;
		peso = 0;
		densidad = 0;
		max_stack_item_abajo = 0;
		orientacion = 0;
		num_items = 0;
		inventory_cost = 0;
		volumen = 0;
		max_weight_item_abajo = 0;
		peso_encima_item_abajo=0;
	};
	~stack_sol()
	{
	};
	stack_sol(string tru, int id_st, string st_code, int x1, int y1, int z1, int x2, int y2, int z2, float peso_pila, int max_stack, int ori,int nit, double coste,double vol,double mwia)
	{
		id_truck = tru;
		x_ini = x1;
		y_ini = y1;
		z_ini = z1;
		x_fin = x2;
		y_fin = y2;
		z_fin = z2;
		id_stack = tru;
		id_stack += "_";
		id_stack += to_string(id_st+1);
		stack_code = "";
		if (id_st > 25)
		{
			int pel=id_st/((int)letras.size());
			int pdl = (id_st-(pel* ((int)letras.size())));

			for (int i = 0; i < pel; i++)stack_code += letras[0];
			stack_code += letras[pdl];
		}
		else stack_code += letras[id_st];
		
		peso = peso_pila;
		densidad = peso / (((float)(x2 - x1)/ (float)1000) * ((float)(y2 - y1)/ (float)1000));
		//Cambiado 27012023
		max_stack_item_abajo = max_stack;
		orientacion = ori;
		num_items = nit;
		inventory_cost = coste;
		volumen= vol;
		if (z1 == 0)
			max_weight_item_abajo = mwia;
	};
	
	void escribe(string file)
	{
		ofstream f(file, ios_base::app);
		f<< id_truck << ";" << id_stack << ";" << stack_code << ";" << x_ini << ";" << y_ini << ";" << z_ini << ";" << x_fin << ";" << y_fin << ";" << z_fin << endl;
		f.close();
	};
	void escribe(ofstream& f)
	{

		f << id_truck << ";" << id_stack << ";" << stack_code << ";" << x_ini << ";" << y_ini << ";" << z_ini << ";" << x_fin << ";" << y_fin << ";" << z_fin << endl;

	};
};
class item_sol
{
public:
	int id_CodId;
	string id_item;
	string id_truck;
	string id_stack;
	string item_code;
	item* Item;
	int x_ini;
	int y_ini;
	int z_ini;
	int x_fin;
	int y_fin;
	int z_fin;
	float peso;
	int pos_vector; //posición de la lista donde estaba
	int inventory_cost;
	double volumen;
	vector<char> letras = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
	item_sol(item* Ititem,int CodId, string Id, string id_tru, int np, string codi, int  indice, int xi, int yi, int zi, int Xf, int Yf, int zf, float peso1, int pos, double vol)
	{
		Item=Ititem;
		id_CodId = CodId;
		id_item = Id;
		id_truck = id_tru;
		id_stack = id_truck;
		id_stack += "_";
		id_stack += to_string(np+1);
		item_code = "";
		if (np > 25)
		{
			int pel = np / ((int)letras.size());
			int pdl = (np - (pel * (int)letras.size())) ;

			for (int i = 0; i < pel; i++)item_code += letras[0];
			item_code += letras[pdl];
		}
		else item_code += letras[np];
		item_code+= to_string(indice);
		x_ini = xi;
		y_ini = yi;
		z_ini = zi;
		x_fin = Xf;
		y_fin = Yf;
		z_fin = zf;
		peso = peso1;
		pos_vector = pos;
		volumen = vol;
		inventory_cost = 0;
	};
	void escribe(string file)
	{
		ofstream f(file, ios_base::app);
		f<<id_item<<";"<<id_truck << ";" <<id_stack << ";" <<item_code << ";" <<x_ini << ";" <<y_ini << ";" 
			<< z_ini << " ;" << x_fin << " ;" << y_fin << " ;" << z_fin << endl;
		f.close();
	};
	void escribe(ofstream &f)
	{
		f << id_item << ";" << id_truck << ";" << id_stack << ";" << item_code << ";" << x_ini << ";" << y_ini << ";"
			<< z_ini << " ;" << x_fin << " ;" << y_fin << " ;" << z_fin << endl;

	};
	
};
class solucion
{
public:
	list<item_sol> listado_items;
	list<truck_sol> listado_camiones;
	list<stack_sol> listado_pilas;
	int coste_total;
	int coste_inventario;
	int coste_transporte;
	solucion()
	{
		coste_total = 0;
		coste_inventario = 0;
		coste_transporte = 0;
	};
	~solucion()
	{
	};
	void reset()
	{
		listado_camiones.clear();
		listado_items.clear();
		listado_pilas.clear();
		coste_total = 0;
		coste_inventario = 0;
		coste_transporte = 0;
	};
	void escribe(string file_to_write)
	{
		ofstream f(file_to_write + "output_trucks.csv");
		f << "Id truck; Loaded length; Weight of loaded items; Volumevof loaded items; emm; emr" << endl;
		f.close();
		ofstream f2(file_to_write + "output_stacks.csv");
		f2 << "Id truck; Id stack; Stack code; X origin; Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;
		f2.close();
		ofstream f3(file_to_write + "output_items.csv");
		f3 << "Item ident;Id truck;Id stack;Item code;X origin;Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;
		f3.close();
		for (auto it = listado_items.begin(); it != listado_items.end(); it++)
			(*it).escribe(file_to_write);
		for (auto it = listado_camiones.begin(); it != listado_camiones.end(); it++)
			(*it).escribe(file_to_write);
		for (auto it = listado_pilas.begin(); it != listado_pilas.end(); it++)
			(*it).escribe(file_to_write);
	};
};
class solucion_general
{
public:
	vector<solucion> soluciones;
	int coste_total;
	int coste_inventario;
	int coste_transporte;
	
	solucion_general()
	{
		coste_total = 0;
		coste_inventario = 0;
		coste_transporte = 0;
	};
	void reset()
	{
		soluciones.clear();
		coste_total = 0;
		coste_inventario = 0;
		coste_transporte = 0;
	};
	void escribe(string file)
	{
		for (auto it = soluciones.begin(); it != soluciones.end(); it++)
		{
			(*it).escribe(file);
		}
	};
};
#endif