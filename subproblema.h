#include "encabezados.h"
#pragma once
#ifndef subroblema_H
#define subroblema_H
class Subproblema
{
public:
	list <truck*> lista_camiones;
	list <item*> lista_items;
	list <item_modelo> lista_items_ventana;
	int num_items_ventana = 0;
	//Tipo de problema
	int Tipo;
	vector<item_modelo> vector_items_modelo_escribir;
	string param_file;
	string input_items;
	string input_trucks;
	string output_items;
	string output_stack;
	string output_trucks;
	int p_inventario;
	int p_transporte;
	map<string, list<item*>> mapa_camiones; //para saber que item va en cada camion. 
	map<string, list<truck*>> mapa_item_camion; //para saber que camion va en cada item.
	solucion sol;
	int total_items; //Total de items a colocar teniendo en cuenta las unidades de cada uno
	int total_extra;
	int num_variables = 0;
	int no_colocados = 0;
	int inventory_cost = 0;
	int tranportation_cost = 0;
	double media_cam_extra;
	double media_inventory_item;
	int max_cam_extra;
	int total_dias_cambios = 0;
	int ventana_temporal ;
	//Cuando hay no mete obligados el primero que mete
	int PrimeroNoMeteObligados = -1;
	int max_invetory_item;
	int NumCamiones = 0;
	int Baja = 0;
	bool MejoraCotas=false;
	bool NoMeteObligados = false;
	
	double ObjBound_Original; //Valor solución posible modelo original
	double ObjMip; //Valor solución posible modelo
	double ObjBound; //Valor cota inferior modelo
	int TiempoModelo;//Tiempo para el modelo en esta componente
	int Total_Iter_Packing=50; //Total de iteraciones del modelo de packing
	int num_camiones_modelo;
	bool Algoritmo_Paralelo = false;
	int best_iteracion = (- 1);
	int num_camiones_extra_modelo;
	int max_cam_extra_tipo_modelo;
	bool En_Medio=false ;
	bool Pruebas=true; //True prueba, false competición
	//Resuelve formulacion Cplex
	int coste_transporte;
	int coste_inventario;
	int min_dim_width;
	int min_dim_length;
	int Iter_Todos_Camiones = 0;
	int Prob_Saltar_No_Obligado = 10;
	//MAtriz indicando que item puede ir en que camión
	vector<std::vector<bool>> matriz_variables;
	//vector que indica en cuantos camiones puede ir este item;
	vector<int> cuantos_item;
	//Este vector me dice que items tiene muchas unidades y lo podría poner entero
	vector<bool> items_muchas_unidades;
	//Best Solucion Obtenida con el modelo
	double Best_Sol_Modelo;
	//Variable que me indica si tenemos todo los items o no
	bool Todos_Los_Camiones = false;
	int coste_total;
	int best_coste_total=999999999;
	bool optimo;
	bool cplex_trozo; //Si es true, se resuelve con cplex tiempo inicial, si es false, se resuelve 1/5 del tiempo;
	int cam_llenos;
	int porcentage_peso;
	int porcentage_vol;
	int Iteracion_Modelo;
	int CodS=0; //Código de subproblema
	int Total_Posibles_Asignaciones=10000; //Es el número de items posibles por los posibles camiones
	int Iter_Resuelve_Subproblema;
	int veces_pila_cambia;
	int veces_pila;
	bool Todas_Enteras = false;
	bool Last_Ramon = false; //LA ventana temporal son todas y no fija las xs pero les pone un valor en cada iteracion
	bool escribe_componente = false;//Me dice si la escribo aislada
	//Veces que se ha cargado un camion
	int contador_infeasibles = 0;
	vector<pair<int,int>> veces_cargocamion;
	//Peso % y volumen % de los camiones cargados por el heuristico, sobre el total del camion y sobre el heuristico
	vector<vector<double>> cargocamion_char;
	bool TieneSol; //Si tiene solución del cplex
	int TiempoExtra; //Tiempo extra para el modelo
	//Solucion inicial del modelo 
	vector<vector<int>> X_ij;
	vector<bool> Z_j;
	vector < vector<vector<int>>> XP_ijk;
	vector<vector<bool>> ZP_jk;
	int planificados;
	int extras;
	map<string, int> camiones_solu;
	int contador_modelo;
	int gap_max;
	long long int ini_anterior;
	bool NoSePuedeDividir = false;


	Subproblema();
	~Subproblema();
	int EstaItemCamion(string id_str);
	bool EstaItemCamion(int id);
	bool EstaTruck(int id);
	bool EstaTruck(string id_str);
	bool EstaItem(int id);
	bool EstaItem(string id_str);
	bool cabe_item_en_espacio(Tripleta<int> esp, item* it, int& orienta, truck* tr);
	string obtenerTextoEntreBarras(const std::string& str);
	int Cuantos_apilar(truck* itc, item* it, double alto, int max_apila, truck_sol& ts, double densi_pila, Tripleta<int> esp, int orienta, double peso_pila, int max_num, double mwia, double volp);
	//int Cuantos_apilar(truck* itc, item* it, double alto, int max_apila, truck_sol& ts, float densi_pila, Tripleta<int> esp, int orienta, float peso_pila, int max_num, double mwia);
//	int Cuantos_apilar(truck* itc, item* it, double alto, int max_apila, truck_sol& ts, float densi_pila, Tripleta<int> esp, int orienta,float peso_pila);
	//bool apilar(item_modelo itm, truck* tr, int orienta, int i, Tripleta<int> esp, truck_sol& ts, int cpila);
	void Crear_pila(int xi, int yi, int zi, int Xf, int Yf, int num_items, item_modelo& itm, truck* itc, int pos, int orienta, truck_sol& ts, int cpila, bool actualiza);
	bool apilar(vector<item_modelo>& vitm, truck* itc, int orienta, int i, Tripleta<int> esp, truck_sol& ts, int cpila);
	bool apilar_old(vector<item_modelo>& vitm, truck* itc, int orienta, int i, Tripleta<int> esp, truck_sol& ts, int cpila);

	//Devuelve 0 si no ha podido colocar por espacio
	//Devuelve 1 si ha podido colocar
	//Devuelve 2 si no ha podido colocar por orden
	//Devuelve 3 si no ha podido colocar por peso
	int Busco_pila(Tripleta<int> its, truck* itc, truck_sol& ts, vector<item_modelo> &vec_item, int cpila, string idcam);
	bool ComprobarPesos();
	bool ObligadosSinColocar(vector<item_modelo>& vec_items);
	bool PackingModelo(int opcion,string dir, parameters param);
	void ModificarYStackItems(stack_sol &);
	void CalcularMinimo(truck* camion, vector<item_modelo>& vec_items);
	void CargoCamion(truck* ,  vector<item_modelo> &, string ,int& , int );
	void PonerCantidadesOriginal(vector<item_modelo>& vect);
	double Volumen(vector<item_modelo>& vect);
	double VolumenP(vector<item_modelo>& vect);
	double VolumenPO(vector<item_modelo>& vect);
	void CambiarCantidadesOriginal(vector<item_modelo>& vect);
	void MejorarNumExtras();
	bool Todos_Relax=false;
	bool Por_Camion = false;
	bool Fijar = true;
	void ComprobarObligadosyClientes(vector<item_modelo>& );
	bool QuedaAlguno(vector<item_modelo>& vec_items);
	void CargoCamion(truck* camion, vector<item_modelo>& vec_items, string idcamion, int& num_items_camion, int numcamion, int tipo_carga);
	void CargoCamionN(truck* camion, vector<item_modelo>& vec_items, string idcamion, int& num_items_camion, int numcamion, int tipo_carga);
	void CargoCamionIni(truck* camion, vector<item_modelo>& vec_items, string idcamion, int& num_items_camion, int numcamion);
	void ActualizarEstadisticas(truck* camion);
	void ArreglarEspacio(truck* camion, int numcamion);
	void QuitarRestos(truck* camion, int numcamion, Tripleta<int>* its);
	bool comprobarPesoEjesActualizarIntensidad(truck* camion, solucion& sol, bool checkAnterior);
	void escribe(string file);
	bool ComprobarCostes();
	int BuscarIDCC(int);
	void ComprobarItemsColocados();
	void QuitarPerfiles(list<Tripleta<int>>& lista, stack_sol& ultima, Tripleta<int> &triplet);
	void LimpiarPerfiles(list<Tripleta<int>>& lista);
	void PintarVector(vector<item_modelo>& vec_items, int kiter);
	void AleatorizarVector(vector<item_modelo>& vec_items);
	bool orden_items(const item_modelo& a, const item_modelo& b);

	//Esta función pasa por camiones e items y reduce el problema
	//Los items solamente van a ir en camiones de un número de periodos
	
	void FiltroItems(int periodos);
	void PonerMasExtras();
	void EscribeComponente(parameters param);
	void EscribeCamion( int contador, string );

	void PonerParams(parameters& params);
	void ReducirExtras();
	void ReducirExtras(parameters param);
	bool compare_trucks(const truck& first, const truck& second)
	{
		if (first.arrival_time_tm < second.arrival_time_tm)
			return true;
		else
			return false;
	};
	struct asignaciones
	{
		bool operator() (const Subproblema* a, const Subproblema* b)
		{
			if (a->Total_Posibles_Asignaciones == b->Total_Posibles_Asignaciones)
			{
				if (a->lista_camiones.size() == b->lista_camiones.size())
				{

						return  (a->lista_items.size() > b->lista_items.size()) ? true : false;

				}
				else return (a->lista_camiones.size() > b->lista_camiones.size()) ? true : false;
			}
			else return  (a->Total_Posibles_Asignaciones > b->Total_Posibles_Asignaciones) ? true : false;

		}
	};
	struct tamcam
	{
		bool operator() (const Subproblema* a, const Subproblema* b)
		{
			if (a->lista_camiones.size()== b->lista_camiones.size())
			{
				if (a->lista_items.size() == b->lista_items.size())
				{
					return  (a->Total_Posibles_Asignaciones > b->Total_Posibles_Asignaciones) ? true : false;
				}
				else
					
				return  (a->lista_items.size() > b->lista_items.size()) ? true : false;
			}
			else return (a->lista_camiones.size() > b->lista_camiones.size()) ? true : false;
		}
	};
	void ComprobarPesosCamion(truck_sol& ts);

	void reset()
	{
		ObjBound = 0;
		ObjMip = 0;
		TiempoModelo = 0;
		TiempoExtra = 0;
		num_camiones_modelo = 0;
		num_camiones_extra_modelo = 0;
		max_cam_extra_tipo_modelo = 0;
		//total_items = 0;
		coste_transporte = 0;
		coste_inventario = 0;
		coste_total = 0;
		porcentage_peso = 0;
		porcentage_vol = 0;
		cplex_trozo = false;
		sol.reset();
		Iteracion_Modelo = 0;
		
		for (auto it = lista_items.begin(); it != lista_items.end(); it++)
		{
			(*it)->num_items = (*it)->num_items_original;
		}
		for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
		{
			(*it)->reset();
		}
		cam_llenos = 0;
		planificados = 0;
		extras = 0;
		camiones_solu.clear();
	};
	
	void max_med_trucks()
	{
		max_cam_extra = 0;
		total_extra = 0;
		for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
		{
			total_extra += (*it)->cota_extra;
			if (max_cam_extra < (*it)->cota_extra)
				max_cam_extra = (*it)->cota_extra;

		}
		media_cam_extra = (double)total_extra / (double)lista_camiones.size();
	};

	void max_med_items()
	{
		max_invetory_item = 0;
		media_inventory_item = 0;
		for (auto it = lista_items.begin(); it != lista_items.end(); it++)
		{
			media_inventory_item += (*it)->inventory_cost;
			if (max_invetory_item < (*it)->inventory_cost)
			{
				max_invetory_item = (*it)->inventory_cost;
			}
		}
		media_inventory_item = media_inventory_item / lista_items.size();
	};
	void PonerAlgunosItemsABinarios();
	void CrearMatrizPosibles();
};
#endif