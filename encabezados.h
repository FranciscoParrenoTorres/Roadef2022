#pragma once


///esto quitar para servidor

#include <float.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <list>
#include <numeric>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>
//#include <values.h>
#include <string.h>
#include <map>
#include <stack>
#include <set>
#include <sys/types.h>
#include <assert.h>
#include <iomanip>
#include <sys/timeb.h>
#include <math.h>
#include <tuple>
#include <ctime>
#include <chrono>
#include <omp.h>
#include <filesystem>
#include <thread>
//#include <mutex>
#pragma warning(disable:4786)
using namespace std;

#define MAX_GAP 0.005
#define MAX_CAMIONES_POR_ITEM 100
#define MAX_VECES_MODELO 8
#define MIN_DIF_SOL_MODELO 0.01
#define MAX_ITER_GLOBAL 100
//Máximo número de periodos que va a ir un item
#define MAX_PERIODO_ITEM 1
//Para las estadísticias de cuántas veces repite el modelo
#define MAX_ITER_MODELO 50
#define get_random(min, max) ((rand() % (int)(((max)+1) - (min))) + (min))  
#define INTENSITY_MIDDLES 3 //intensidad a la hora de poner pallets en medio
//invertir una map <K,V> a <V,K>
template<typename K, typename V>
map<V, K> reverseMap(map<K, V> const& map) {
    std::map<V, K> rMap;
    for (auto const& pair : map) {
        rMap[pair.second] = pair.first;
    }
    return rMap;
}
template<typename K, typename V>
std::multimap<V, K> invertMap(std::map<K, V> const& map)
{
	std::multimap<V, K> multimap;

	for (auto const& pair : map) {
		multimap.insert(std::make_pair(pair.second, pair.first));
	}

	return multimap;
}

template<typename T>
class Tripleta
{
public:
	T y_ini, y_fin, x;


	//Lista de trozos de items que toca el espacio
	list <Tripleta<T>> perfiles;

	Tripleta(const T& x, const T& y, const T& z)
	{
		this->y_ini = x;
		this->y_fin = y;
		this->x = z;
	}
	Tripleta(const Tripleta& copia)
	{
		this->y_ini = copia.y_ini;
		this->y_fin = copia.y_fin;
		this->x = copia.x;
		this->perfiles = copia.perfiles;
	}
	int CantidadTocaPerfil(int Yi,int Yf)
	{
		int cantidad = 0;
		if (x == 0)
		{
			return cantidad = (y_fin - y_ini);
		}
		for (auto it1 = perfiles.begin(); it1 != perfiles.end(); it1++)
		{
			if (((*it1).y_fin > Yi) && ((*it1).y_fin <= Yf))
				cantidad += (*it1).y_fin - max(Yi, (*it1).y_ini);
			else
			{
				if (((*it1).y_ini >= Yi) && ((*it1).y_ini < Yf))
					cantidad += min(Yf, (*it1).y_fin) - (*it1).y_ini;
				else
				{
					if (((*it1).y_ini < Yi) && ((*it1).y_fin > Yf))
						cantidad += Yf - Yi;
				}

			}


		}
		return cantidad;
	}

};



#include "parameters.h"
#include "item.h"
#include "kosaraju.h"
int Days_0001(int y, int m, int d);


#include "Item_modelo.h"

#include "solucion.h"
struct mejor_sol
{
	vector<solucion> solucion_compo;
	int coste_total = 0;
	int coste_transporte = 0;
	int coste_inventario = 0;
	
};
#include "espacios.h"



#include "truck.h"
#include "subproblema.h"

#include "Problema.h"

#include "pilas_cargadas.h"

#include "ilcplex/ilocplex.h"
#include <ilconcert/ilomodel.h>
#ifdef _WIN32
#include <SDL.h>
#endif
ILOSTLBEGIN
typedef IloIntVarArray NumVector;
typedef IloArray<IloNumVarArray> NumVarMatrixF;
typedef IloArray<IloIntVarArray> NumVarMatrix;
typedef IloArray<IloNumVarArray> NumVarMatrix2F;
typedef IloArray<IloIntVarArray> NumVarMatrix2;
typedef IloArray<IloArray<IloNumVarArray> > NumVarMatrix3F;
typedef IloArray<IloArray<IloIntVarArray> > NumVarMatrix3;
typedef IloArray<IloArray<IloArray<IloIntVarArray> > > NumVarMatrix4;
typedef IloArray < IloArray<IloArray<IloArray<IloIntVarArray> > > > NumVarMatrix5;
typedef IloArray < IloArray < IloArray<IloArray<IloArray<IloIntVarArray> > > > > NumVarMatrix6;

typedef IloBoolVarArray BoolVector;
typedef IloArray<IloBoolVarArray> BoolVarMatrix;
typedef IloArray<IloBoolVarArray> BoolVarMatrix2;
typedef IloArray<IloArray<IloBoolVarArray> > BoolVarMatrix3;
typedef IloArray<IloArray<IloArray<IloBoolVarArray> > > BoolVarMatrix4;
typedef IloArray < IloArray<IloArray<IloArray<IloBoolVarArray> > > > BoolVarMatrix5;
typedef IloArray < IloArray < IloArray<IloArray<IloArray<IloBoolVarArray> > > > > BoolVarMatrix6;


int PintarProblema(int a);



//int Resolver_Paralelo(Problema& P, int& iter, string file_to_read, parameters param, int Gurobi);
void AddCamionSolucion(Subproblema* Sub, truck* camion_orig);
void leer_datos( parameters& params, list<item*>& lista_items,list<truck*>& lista_cam, map <string, truck*>& m_camiones, map<string, int>&proveedores, map <string, list<item*>> &proveedor_items);
void escribe_item(string dir,  map <string, truck*> mapa_camiones, parameters para, list<item*> lista_item, list<truck*>& lista_cam);
//int Roadef2022Cplex(list<truck*> lista_cam, list<item*> lista_item, parameters param);
int Roadef2022ExtraCplex(string dir, list<truck*> lista_cam, list<item*> lista_item, parameters param, int num);
int knapsack_bouknap(set<int> , int ); //Hace una mochila de los datos que le demos
void Calcula_componentes_conexas(map<string, int> proveedores, list<truck*> lista_camiones, map<int, list<string>>& comp);
void Calcula_componentes_conexas2(map<string, int> proveedores, list<truck*> lista_camiones, map<int, list<string>>& componentes, list<item*>lista_item, map <string, list<item*>>& proveedor_items, Problema& Prob);
void Calcula_componente_conex3(Problema P1, list<truck*> lista_camiones, list<item*>lista_item, Problema& Prob, map <string, truck*> mapa_camiones);
void escribe_info_componentes(Problema& Prob, string instancia, int tot_cam, int tot_item, int prove);
void estadistcias_itesms(list<item*>lista_item, string file);
//void Crear_Subproblemas(map<int, list<string>> componentes, map <string, list<item*>>& proveedor_items, Problema &Prob);
int Roadef2022ExtraCplex(string dir, parameters param, Subproblema *Sub);
int Roadef2022Fixing(string dir, parameters param, Subproblema* Sub);
int TabuAdaptativo(string dir, parameters param, Subproblema* Sub);
int Roadef2022ExtraCplex_V2(string dir, parameters param, Subproblema* Sub);
//int Roadef2022ExtraGurobi(string dir, parameters param, Subproblema* Sub);
int Roadef2022ExtraCplexSimplificado(string dir, parameters param, Subproblema* Sub);
void comprobar_componente(list<truck*> lc);
std::pair<int, int> UltimoCamion(item_modelo Item, std::list<truck*> lista_cam);
void PonerObligados(truck* Camion, item_modelo Item, std::list<truck*> lista_cam);
void Enteras(parameters param, Subproblema* Sub);
//Comprobaciones
bool cuadra_num_items(Subproblema* SU);
double VolumenTotal(Subproblema* Sub);
//relaxfix
int relaxandfix(string dir, parameters param, Subproblema* Sub);
int Modeloryf1(string dir, parameters param);
//int Modeloryf1(string dir, parameters param, Subproblema* Sub, long long int fini, long long int ffin, IloEnv& env, IloModel& model);
void restricciones_RyF(Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, IloRangeArray& restr);
void DeleteCamionSolucion(Subproblema* Sub, truck* camion_orig, bool hacia_delante);
void DeleteCamionSolucion(Subproblema* Sub, truck* camion_orig);
void CambiarCamionSolucion(Subproblema* Sub, truck* camion_orig);
int resolver_modelo_RyF(IloCplex& cplex, Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, long long int fini,long long int ffin,list<truck*>& listacam);
//void Poner_binarias_RyF(IloCplex& cplex, Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, long long int ini, long long int ffin, int ventana);
void Poner_binarias_RyF(Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, long long int ini, long long int ffin, int ventana);
void Poner_binarias_RyF(Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, long long int ini, long long int ffin, long long int ffin2, int ventana);
void Poner_binarias_RyF(Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, long long int ini, long long int ffin, long long int ini2, long long int ffin2, int ventana);

long long int CalcularFechasDiasDespues(Subproblema* Sub, list<truck*>::iterator& ultimo_camion, int dias);

//voi//fijar_variables_RyF(IloCplex& cplex, list<truck*> lista, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, Subproblema* Sub, bool &quedan);
void fijar_variables_RyF(list<truck*> lista, parameters param, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, Subproblema* Sub, bool& quedan);
void EscribirSolucion(Subproblema* Sub, IloCplex* cplex, NumVarMatrix2F* x_ij, IloNumVarArray* z_j, NumVarMatrix3F* xp_ijk, NumVarMatrix2F* zp_jk, long long int fin, list<truck*>& listacam);

void ConvertirSolucionModeloACamiones2(Subproblema* Sub, IloCplex* cplex, NumVarMatrix2F* x_ij, IloNumVarArray* z_j, NumVarMatrix3F* xp_ijk, NumVarMatrix2F* zp_jk, long long int fin, list<truck*>& listacam);
void ConvertirSolucionModeloACamiones4(Subproblema* Sub, IloCplex* cplex, NumVarMatrix2F* x_ij, IloNumVarArray* z_j, NumVarMatrix3F* xp_ijk, NumVarMatrix2F* zp_jk, long long int fin, list<truck*>& listacam);

bool PackingRF(string dir, parameters param, Subproblema* Sub, list<truck*> lista_camiones_ventana);
bool PuedeIrItemCam(item* It_choose, truck* cam);
bool PuedeIrCamItem(item* Item, truck* cam_choose);
bool puede_ir_camion(item_modelo itM, truck* cam);
void items_en_camiones(truck* cam, list<item_modelo> lista_items_ventana);
bool PackingRF2(string dir, parameters param, Subproblema* Sub, list<truck*> lista_camiones_ventana);
bool PackingRF_New(string dir, parameters param, Subproblema* Sub, list<truck*> lista_camiones_ventana);
void colocar_en_sig_cam(Subproblema* Sub, list<truck*>::iterator itc, truck*& sig_cam, bool& hay_recarga, int& cuantos_recargar, bool es_extra, truck* &plani);
void buscar_sig_cam(list<truck*> lista_camiones_ventana, list<truck*>::iterator itc, truck*& sig_cam);
void buscar_sig_cam_del_extra(list<truck*> lista_camiones_ventana, list<truck*>::iterator itc, list<truck*>::iterator itex, truck*& sig_cam,truck*&plani);
void colocar_en_sig_cam_lista(Subproblema* Sub, list<truck*>::iterator itc,  truck*& sig_cam, bool& hay_recarga, int& cuantos_recargar, bool es_extra, truck*& plani);
int NumExtra(const char codigo[30]);

bool PosibleIntercambio(truck* TR1, truck* TR2);
void BusquedaLocal(Subproblema* Sub);

void RestablecerItemsDeLista(Subproblema* Sub, std::vector<item_modelo>& listaTotalItems, std::vector<item_modelo>& itemsTR1, std::vector<item_modelo>& itemsTR2);
void EscribirInfoItems(truck* );
void ReorganizarCamiones(truck* TR1, truck* TR2, Subproblema* Sub);