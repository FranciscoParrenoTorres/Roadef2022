#include "encabezados.h"

#pragma warning(disable:4786)




#include "ilcplex/ilocplex.h"
#include <ilconcert/ilomodel.h>
ILOSTLBEGIN
using namespace std;
//#include "gurobi_c++.h"


/*
	IloIntVarArray  variable entera
	IloNumVarArray  variable real
*/
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

void ConvertirSolucionModeloACamiones( Subproblema* Sub, IloCplex* cplex, NumVarMatrix2* x_ij, IloIntVarArray* z_j, NumVarMatrix3* xp_ijk, NumVarMatrix2* zp_jk);

//int Maintenance::Roadef2022Gurobi()
//{
//	//Tenemos el numero de nodos, num_nodos
//// el numero de nodos elegidos, num_elegidos
//// la matriz de adyacencia, d
////Definimos el entorno
//	GRBEnv envGRB = GRBEnv();
//	// Definimos un modelo
//	GRBModel model = GRBModel(envGRB);
//	//Definimos las variable dependiendo de qué tipo
//	GRBVar* X = 0;
//	X = model.addVars(num_nodos, GRB_BINARY);
//	//  X = model.addVars(num_nodos,GRB_CONTINUOUS);
//	//Poner nombre a las variables
//	int nmbvar = 0;
//	std::vector<std::vector<int>> key(num_camiones, std::vector<int>(num_items, -1));
//	for (int i = 0; i < num_camiones; i++)
//	{
//
//		for (int l = 0; l < num_items; l++)
//		{
//			if (key[i][l] == -1)
//			{
//				string vname = "X_" + to_string(i) + to_string(l);
//				X[nmbvar].set(GRB_StringAttr_VarName, vname.c_str());
//				key[i][l] = nmbvar;
//				nmbvar++;
//			}
//		}
//	}
//
//
//	int nmbvar2 = 0;
//
//	int K = 0;
//	for (int i = 0; i < num_nodos; i++)
//	{
//		string vname = "X_" + i;
//		X[i].set(GRB_StringAttr_VarName, vname.c_str());
//	}
//
//	//DEfinimos una expresión
//	for (int i = 0; i < num_nodos - 1; i++)
//	{
//		for (int j = i; j < num_nodos; j++)
//		{
//			if (d[i][j] == true)
//			{
//				GRBLinExpr expr = 0;
//
//				expr += X[i];
//				expr += X[j];
//				//Añadimos la restricción al modelo
//				model.addConstr(expr <= 1, "S_" + to_string(i) + to_string(j));
//				//Puede ser expr== 1 o expr>=1
//			}
//		}
//	}
//
//	//Función objetivo
//	GRBLinExpr obj = 0;
//	for (int i = 0; i < num_nodos; i++) obj += X[i];
//
//	model.setObjective(obj, GRB_MAXIMIZE);
//	//Pool
//	model.optimize();
//	//Parámetros de optimización
//	model.set(GRB_IntParam_PoolSolutions, 20); //Tamaño de la pool
//	model.set(GRB_DoubleParam_PoolGap, 0.001); //Gap de la pool
//	model.set(GRB_IntParam_PoolSearchMode, 1); //Tipo de búsqueda
//	model.set(GRB_DoubleParam_TimeLimit, 60); //Tiempo límite
//	model.set(GRB_IntParam_Threads, 4); //Número de hilos
//	//Optimizamos	
//	model.optimize();
//	//Imprimimos el modelo en un fichero
//	model.write("prueba.lp");
//	//Estado del algoritmo	al terminar
//	int status = model.get(GRB_IntAttr_Status);
//	//número de soluciones obtenidas
//	int Nsol = model.get(GRB_IntAttr_SolCount);
//	//VAlor del mip
//	double ObjMip = model.get(GRB_DoubleAttr_ObjVal);
//	//Cota
//	double ObjBound = model.get(GRB_DoubleAttr_ObjBound);
//	for (auto i = 0; i < Nsol; i++)
//	{
//		//Pasamos la solución a X
//		model.set(GRB_IntParam_SolutionNumber, i);
//		//Extraer los valores de X
//		for (auto j = 0; j < num_nodos; j++)
//		{
//			if (X[j].get(GRB_DoubleAttr_Xn) > 0.999)
//			{
//				cout << "Posición " << j << " elegida de la solución " << i << "\n";
//			}
//		}
//
//	}
//
//	//PAra poner una solución inicial
//	for (auto i = 0; i < num_nodos; i++)
//		X[i].set(GRB_DoubleAttr_Start, 1);
//}
int Roadef2022Cplex(list<truck*> lista_camiones, list<item*> lista_items, parameters param)
{
	//Establecemos el entorno
	IloEnv env;
	//Creamos el modelo
	IloModel model = IloModel(env);
	//Creamos las variables

	NumVarMatrix2 x_ij(env, lista_items.size());
	IloIntVarArray z_j(env, lista_camiones.size(), 0, 1);
	//Si queremos que sean continuas
	//IloNumVarArray X2(env, num_nodos, 0, 1, ILOFLOAT);
	//Ponemos los nombres
	char nombre[20];
	//int num_var = 0;
	for (int i = 0; i < lista_items.size(); i++) //Recorrer la lista de tipo items
	{
		x_ij[i] = IloIntVarArray(env, lista_camiones.size()); //Recorrer la lista de camiones
	}
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			//Si existe la variable

			x_ij[(*it_i)->CodId][(*it)->CodId] = IloIntVar(env, 0, (*it_i)->num_items);

			sprintf(nombre, "X_%d_%d", (*it_i)->CodId, (*it)->CodId);
			x_ij[(*it_i)->CodId][(*it)->CodId].setName(nombre);
		}
	}
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		sprintf(nombre, "Z_%d", (*it)->CodId);
		z_j[(*it)->CodId].setName(nombre);
	}


	//Restricciones
	IloRangeArray restr(env);


	//Restricción suma de items igual a todos los items 
	//restricción 1
	
	for (auto it = lista_items.begin(); it != lista_items.end(); it++)
	{
		IloExpr expr1(env);
		for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
		{
			expr1 += x_ij[(*it)->CodId][(*it_c)->CodId];
			
		}
		restr.add(IloRange(env, (*it)->num_items, expr1, (*it)->num_items));
		expr1.end();
	}
	//Restricción 2 y 3
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		IloExpr expr2(env);
		IloExpr expr3(env);
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			expr2 += x_ij[(*it_i)->CodId][(*it)->CodId] * (*it_i)->volumen;
			expr3 += x_ij[(*it_i)->CodId][(*it)->CodId] * (*it_i)->weight;
		}
		expr2 += (-1) * z_j[(*it)->CodId]*(*it)->volumen;
		expr3 += (-1) * z_j[(*it)->CodId] * (*it)->max_loading_weight;
		restr.add(IloRange(env, -IloInfinity, expr2, 0));
		restr.add(IloRange(env, -IloInfinity, expr3, 0));
		expr2.end();
		expr3.end();
	}

	//AÑADIMOS LAS RESTRICCIONES AL MODELO
	model.add(restr);

	//Función objetivo
	IloExpr obj(env);
	//Parte de camiones
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		obj += z_j[(*it)->CodId]*(*it)->cost*param.transportation_cost;
	}
	//Parte de items
	for (auto it_c = lista_camiones.begin(); it_c != lista_camiones.end(); it_c++)
	{
		for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
		{
			double val = ((*it)->inventory_cost * param.inventory_cost) * ((*it)->latest_arrival - (*it_c)->arrival_time);
			obj += (x_ij[(*it)->CodId][(*it_c)->CodId]) *val ;
		}
	}
	//Añadimos la función objetivo al modelo

	IloObjective fobj = IloMinimize(env, obj);
	model.add(fobj);
	obj.end();
	IloCplex cplex(model);
//	cplex.exportModel("prueba.lp");
	//Parámetros de optimización
	cplex.setParam(IloCplex::Threads, 4); //numero de hilos
	cplex.setParam(IloCplex::TiLim, 60); //tiempo
	//	cplex.setParam(IloCplex::SolnPoolReplace, 2);
	//	cplex.setParam(IloCplex::SolnPoolCapacity, 100); //Capacidad de la pool
		//Optimizamos	
	//IloBool result = cplex.solve();
	//Si quisieramos generar soluciones
	// IloBool result = cplex.populate(); 
	//Estado del algoritmo	al terminar
	//IloCplex::Status estado = cplex.getCplexStatus();
	//número de soluciones obtenidas
	int Nsol = cplex.getSolnPoolNsolns();
	//VAlor del mip
	//double ObjMip = cplex.getObjValue();
	//Valor de la cota
	//double ObjBound = cplex.getBestObjValue();
	for (auto i = 0; i < Nsol; i++)
	{
		//Extraer los valores de X
		for (auto it_c = lista_camiones.begin(); it_c != lista_camiones.end(); it_c++)
		{
			for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
			{


				if (cplex.getValue(x_ij[(*it)->CodId][(*it_c)->CodId], i) > 0.999)
				{
					cout << "Posición " << (*it)->CodId << " "<< (*it_c)->CodId << " elegida de la solución " << i << "\n";
				}
			}
		}

	}
	//Cerramos el entorno
	model.end(); env.end();
	return 1;
}
int Roadef2022ExtraCplex(string dir, list<truck*> lista_camiones, list<item*> lista_items, parameters param,int num)
{
	//porcentge de peso y volumen que le ponemos al modelo
	//para la aleatorización.
	double porcent_pyv =  num/ 100;
	

	//Establecemos el entorno
	IloEnv env;
	//Creamos el modelo
	IloModel model = IloModel(env);
	//Creamos las variables

	NumVarMatrix2 x_ij(env, lista_items.size());
	IloIntVarArray z_j(env, lista_camiones.size(), 0, 1);
	NumVarMatrix3 xp_ijk(env, lista_items.size());
	NumVarMatrix2 zp_jk(env, lista_camiones.size());
	//Si queremos que sean continuas
	//IloNumVarArray X2(env, num_nodos, 0, 1, ILOFLOAT);
	//Ponemos los nombres
	char nombre[20];
	//int num_var = 0;
	for (int i = 0; i < lista_items.size(); i++) //Recorrer la lista de tipo items
	{
		x_ij[i] = IloIntVarArray(env, lista_camiones.size()); //Recorrer la lista de camiones
		xp_ijk[i] = NumVarMatrix2(env, lista_camiones.size()); //Recorrer la lista de camiones
	}
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			//Si existe la variable

			x_ij[(*it_i)->CodId][(*it)->CodId] = IloIntVar(env, 0, (*it_i)->num_items);

			sprintf(nombre, "X_%d_%d", (*it_i)->CodId, (*it)->CodId);
			x_ij[(*it_i)->CodId][(*it)->CodId].setName(nombre);
			xp_ijk[(*it_i)->CodId][(*it)->CodId]= IloIntVarArray(env, (*it)->cota_extra);
			for (int k = 0; k < (*it)->cota_extra; k++)
			{
				xp_ijk[(*it_i)->CodId][(*it)->CodId][k] = IloIntVar(env, 0, (*it_i)->num_items);

				sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodId, (*it)->CodId,k);
				xp_ijk[(*it_i)->CodId][(*it)->CodId][k].setName(nombre);
			}
		}
	}
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		sprintf(nombre, "Z_%d", (*it)->CodId);
		z_j[(*it)->CodId].setName(nombre);
		zp_jk[(*it)->CodId] = IloIntVarArray(env, (*it)->cota_extra);
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			zp_jk[(*it)->CodId][k]=IloIntVar(env, 0, 1);
			sprintf(nombre, "ZP_%d_%d", (*it)->CodId,k);
			zp_jk[(*it)->CodId][k].setName(nombre);
		}
	}


	//Restricciones
	IloRangeArray restr(env);


	//Restricción suma de items igual a todos los items 
	//restricción 1

	for (auto it = lista_items.begin(); it != lista_items.end(); it++)
	{
		IloExpr expr1(env);
		for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
		{
			expr1 += x_ij[(*it)->CodId][(*it_c)->CodId];
			for (int k = 0; k < (*it_c)->cota_extra; k++)
			{
				expr1 += xp_ijk[(*it)->CodId][(*it_c)->CodId][k];
			}

		}
		restr.add(IloRange(env, (*it)->num_items, expr1, (*it)->num_items));
		expr1.end();
	}
	//Restricción 2 y 3
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		IloExpr expr2(env);
		IloExpr expr3(env);

		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			expr2 += x_ij[(*it_i)->CodId][(*it)->CodId] * (*it_i)->volumen ;
			expr3 += x_ij[(*it_i)->CodId][(*it)->CodId] * (*it_i)->weight ;

		}
		expr2 += (-1) * z_j[(*it)->CodId] * (*it)->volumen_kp * porcent_pyv;
		expr3 += (-1) * z_j[(*it)->CodId] * (*it)->max_loading_weight * porcent_pyv;
		restr.add(IloRange(env, -IloInfinity, expr2, 0));
		restr.add(IloRange(env, -IloInfinity, expr3, 0));
		expr2.end();
		expr3.end();
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			IloExpr expr2p(env);
			IloExpr expr3p(env);
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				expr2p += xp_ijk[(*it_i)->CodId][(*it)->CodId][k] * (*it_i)->volumen;
				expr3p += xp_ijk[(*it_i)->CodId][(*it)->CodId][k] * (*it_i)->weight;
			}
			expr2p += (-1) * zp_jk[(*it)->CodId][k] * (*it)->volumen_kp * porcent_pyv;
			expr3p += (-1) * zp_jk[(*it)->CodId][k] * (*it)->max_loading_weight * porcent_pyv;

			restr.add(IloRange(env, -IloInfinity, expr2p, 0));
			restr.add(IloRange(env, -IloInfinity, expr3p, 0));

			expr2p.end();
			expr3p.end();
		}
	}

	//AÑADIMOS LAS RESTRICCIONES AL MODELO
	model.add(restr);

	//Función objetivo
	IloExpr obj(env);
	//Parte de camiones
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		obj += z_j[(*it)->CodId] * (*it)->cost * param.transportation_cost;
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			obj += zp_jk[(*it)->CodId][k] * (*it)->cost * (1+param.extra_truck_cost) * param.transportation_cost;
		}
	}
	//Parte de items
	for (auto it_c = lista_camiones.begin(); it_c != lista_camiones.end(); it_c++)
	{
		for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
		{
			//Dias de diferencia entre una fecha y otra
//			int days = difftime((*it)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
			int difday = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday) -
				Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);

			obj += (difday) * x_ij[(*it)->CodId][(*it_c)->CodId] * (*it)->inventory_cost * param.inventory_cost;

		
		}
		for (int k = 0; k < (*it_c)->cota_extra; k++)
		{
			for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
			{
//				int days = difftime((*it)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
				int difday = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday) -
					Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);

				obj += difday * xp_ijk[(*it)->CodId][(*it_c)->CodId][k] * (*it)->inventory_cost * param.inventory_cost;
			}
		}
	}
	//Añadimos la función objetivo al modelo

	IloObjective fobj = IloMinimize(env, obj);
	model.add(fobj);
	obj.end();
	IloCplex cplex(model);
//	cplex.exportModel("prueba.lp");
	//Parámetros de optimización
	cplex.setParam(IloCplex::Threads, 4); //numero de hilos
	cplex.setParam(IloCplex::TiLim, 300); //tiempo
	//	cplex.setParam(IloCplex::SolnPoolReplace, 2);
	//	cplex.setParam(IloCplex::SolnPoolCapacity, 100); //Capacidad de la pool
		//Optimizamos	
	IloBool result = cplex.solve();
	
	//Si quisieramos generar soluciones
	// IloBool result = cplex.populate(); 
	//Estado del algoritmo	al terminar
	IloCplex::Status estado = cplex.getCplexStatus();

	 cout << result << " " << estado;
	//número de soluciones obtenidas
	int Nsol = cplex.getSolnPoolNsolns();
	//VAlor del mip
	double ObjMip = cplex.getObjValue();
	//Valor de la cota
	double ObjBound = cplex.getBestObjValue();
	 cout << "Best Value " << ObjMip << " Cota " << ObjBound ;
	int num_camiones = 0;
	int num_camiones_extra = 0;
	int max_cam_extra_tipo = 0;
	//Para leer todas las soluciones la 0 es la mejor
	for (auto i = 0; i < 1; i++)
	{
		//Extraer los valores de X
		for (auto it_c = lista_camiones.begin(); it_c != lista_camiones.end(); it_c++)
		{
			if (cplex.isExtracted(z_j[(*it_c)->CodId]) && cplex.getValue(z_j[(*it_c)->CodId], i) > 0.999)
				num_camiones++;
			int num_cam_extra_tipo = 0;
			for (int k = 0; k < (*it_c)->cota_extra; k++)
			{
				if (cplex.isExtracted(zp_jk[(*it_c)->CodId][k]) && cplex.getValue(zp_jk[(*it_c)->CodId][k], i) > 0.999)
				{
					num_camiones_extra++;
					num_cam_extra_tipo++;
				}
				if (num_cam_extra_tipo > max_cam_extra_tipo)
					max_cam_extra_tipo = num_cam_extra_tipo;
			}
//			for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
//			{
//
//
//				if (cplex.getValue(x_ij[(*it)->CodId][(*it_c)->CodId], i) > 0.999)
//				{
////					cout << "Posición " << (*it)->CodId << " " << (*it_c)->CodId << " elegida de la solución " << i << "\n";
//				}
//
//
//					if (cplex.getValue(xp_ijk[(*it)->CodId][(*it_c)->CodId][k], i) > 0.999)
//					{
////						cout << "Posición " << (*it)->CodId << " " << (*it_c)->CodId << " " << k << " elegida de la solución " << i << "\n";
//					}
//				}
//			}
		}
		// cout << "camiones " << num_camiones << " Camiones extra " << num_camiones_extra << " Max Cam Extra Tipo " << max_cam_extra_tipo << endl;

		string nombreArchivo1 = "Resultados.txt";

		ofstream archivo;
		// Abrimos el archivo
		archivo.open(nombreArchivo1.c_str(), ofstream::out | ofstream::app);
		archivo << dir << ";" << lista_camiones.size() << ";" << Nsol << ";" << setprecision(8) << ObjMip << ";" << ObjBound << ";" << num_camiones << ";" << num_camiones_extra << ";" << max_cam_extra_tipo << endl;
		archivo.close();
	}

	//Cerramos el entorno
	cplex.end();
	model.end(); env.end();
	return 1;
}
int Roadef2022ExtraCplexSimplificado(string dir, parameters param, Subproblema* Sub)
{
	//Definimos un vector de variables de items
	int cantidad = 3;
	vector<bool> items_muchas_unidades(Sub->lista_items.size());
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			if (((*it_i)->num_items * (*it_i)->weight > ((*it)->max_loading_weight / cantidad)) ||
				((*it_i)->num_items * (*it_i)->volumen > ((*it)->volumen_kp / cantidad)))
			{
				items_muchas_unidades[(*it_i)->CodIdCC] = true;
//				cout << "Item " << <<"Grandes " << (*it_i)->num_items << "Peso " << (*it_i)->weight<<endl;
			}
		}
	}
	//Establecemos el entorno
	IloEnv env;
	//Creamos el modelo
	IloModel model = IloModel(env);
	//Creamos las variables
	NumVarMatrix2 z_ij(env, Sub->lista_items.size());
	NumVarMatrix3 zp_ijk(env, Sub->lista_items.size());
	
	NumVarMatrix2 x_ij(env, Sub->lista_items.size());
	IloIntVarArray z_j(env, Sub->lista_camiones.size(), 0, 1);
	
	NumVarMatrix3 xp_ijk(env, Sub->lista_items.size());
	NumVarMatrix2 zp_jk(env, Sub->lista_camiones.size());
	//Si queremos que sean continuas
	//IloNumVarArray X2(env, num_nodos, 0, 1, ILOFLOAT);
	//Ponemos los nombres
	char nombre[20];
	//int num_var = 0;
	for (int i = 0; i < Sub->lista_items.size(); i++) //Recorrer la lista de tipo items
	{
		x_ij[i] = IloIntVarArray(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
		xp_ijk[i] = NumVarMatrix2(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones

		z_ij[i] = IloIntVarArray(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
		zp_ijk[i] = NumVarMatrix2(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones

	}
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			//Si existe la variable
			if (items_muchas_unidades[(*it_i)->CodIdCC]==true)
			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);
			else
				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, 1);
			z_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, 1);
			sprintf(nombre, "ZG_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
			z_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].setName(nombre);
			
			sprintf(nombre, "X_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].setName(nombre);

			xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);

			zp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);

			for (int k = 0; k < (*it)->cota_extra; k++)
			{
				if (items_muchas_unidades[(*it_i)->CodIdCC] == true)
				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, (*it_i)->num_items);
				else
					xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);

				sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k].setName(nombre);

				zp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
				sprintf(nombre, "ZPG_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
				zp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k].setName(nombre);

				//				printf("Variable Definida %d %d %d\n", (*it_i)->CodIdCC, (*it)->CodIdCC, k);

			}
		}
	}
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		sprintf(nombre, "Z_%d", (*it)->CodIdCC);
		z_j[(*it)->CodIdCC].setName(nombre);
		zp_jk[(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			zp_jk[(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
			sprintf(nombre, "ZP_%d_%d", (*it)->CodIdCC, k);
			zp_jk[(*it)->CodIdCC][k].setName(nombre);
		}
	}


	//Restricciones
	IloRangeArray restr(env);


	//Restricción suma de items igual a todos los items 
	//restricción 1

	for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
	{
		IloExpr expr1(env);
		
		for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
		{
			IloExpr expr4(env);
			
			//			printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC,k);
			expr1 += x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
			expr4 += z_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
			if (items_muchas_unidades[(*it)->CodIdCC] == true)
			{
				IloExpr expr5(env);
				expr5 += x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
				expr5 += (-1)* (*it)->num_items * z_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
				restr.add(IloRange(env, -IloInfinity, expr5, 0));
				expr5.end();
			}
			for (int k = 0; k < (*it_c)->cota_extra; k++)
			{
				//				printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC, k);

				expr1 += xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
				expr4 += zp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
				if (items_muchas_unidades[(*it)->CodIdCC] == true)
				{
					IloExpr expr5(env);
					expr5 += xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
					expr5 += (-1) * (*it)->num_items * zp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
					restr.add(IloRange(env, -IloInfinity, expr5, 0));
					expr5.end();
				}
			}
			if (items_muchas_unidades[(*it)->CodIdCC] == true)
			{
				
				int cota=ceil(max((double)((*it)->weight * (*it)->num_items )/(double)(*it_c)->max_loading_weight, 
					(double)((*it)->volumen * (*it)->num_items) / (double)(*it_c)->volumen_kp))+1;
				restr.add(IloRange(env, -IloInfinity, expr4,cota));
			}
			expr4.end();

		}
		if (items_muchas_unidades[(*it)->CodIdCC] == true)
		{

			
			restr.add(IloRange(env, (*it)->num_items, expr1, (*it)->num_items));
			
			
//			cout << "Item "<< (*it)->CodIdCC <<"Num " <<(*it)->num_items << endl;
		}
		else
			restr.add(IloRange(env, 1, expr1, 1));
		expr1.end();
	}
	//Restricción 2 y 3
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		IloExpr expr2(env);
		IloExpr expr3(env);

		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			if (items_muchas_unidades[(*it_i)->CodIdCC] == true)
			{
				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->volumen;
				expr3 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->weight;
			}
			else
			{
				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->volumen * (*it_i)->num_items;
				expr3 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->weight * (*it_i)->num_items;
			}

		}
		expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_kp;
		expr3 += (-1) * z_j[(*it)->CodIdCC] * (*it)->max_loading_weight;
		restr.add(IloRange(env, -IloInfinity, expr2, 0));
		restr.add(IloRange(env, -IloInfinity, expr3, 0));
		expr2.end();
		expr3.end();
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			IloExpr expr2p(env);
			IloExpr expr3p(env);
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				if (items_muchas_unidades[(*it_i)->CodIdCC] == true)
				{
					expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->volumen;
					expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->weight;
				}
				else
				{
					expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->volumen * (*it_i)->num_items;
					expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->weight * (*it_i)->num_items;
				}
			}
			expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_kp;
			expr3p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->max_loading_weight;

			restr.add(IloRange(env, -IloInfinity, expr2p, 0));
			restr.add(IloRange(env, -IloInfinity, expr3p, 0));

			expr2p.end();
			expr3p.end();
		}
	}

	//AÑADIMOS LAS RESTRICCIONES AL MODELO
	model.add(restr);

	//Función objetivo
	IloExpr obj(env);
	//Parte de camiones
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		obj += z_j[(*it)->CodIdCC] * (*it)->cost * param.transportation_cost;
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			obj += zp_jk[(*it)->CodIdCC][k] * (*it)->cost * (1 + param.extra_truck_cost) * param.transportation_cost;
		}
	}
	//Parte de items
	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{
		for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
		{
			//Dias de diferencia entre una fecha y otra
//			int days = difftime((*it)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
			int difday = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday) -
				Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);

			if (items_muchas_unidades[(*it)->CodIdCC] == true)
			obj += (difday)*x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC] * (*it)->inventory_cost * param.inventory_cost;
			else
				obj += (difday)*x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC] * (*it)->num_items* (*it)->inventory_cost * param.inventory_cost;


		}
		for (int k = 0; k < (*it_c)->cota_extra; k++)
		{
			for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
			{
//				int days = difftime((*it)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
				int difday = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday) -
					Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);

				if (items_muchas_unidades[(*it)->CodIdCC] == true)

				obj += difday * xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k] * (*it)->inventory_cost * param.inventory_cost;
				else
				obj+= difday* xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k] *(*it)->num_items * (*it)->inventory_cost * param.inventory_cost;
			}
		}
	}
	//Añadimos la función objetivo al modelo

	IloObjective fobj = IloMinimize(env, obj);
	model.add(fobj);
	obj.end();
	IloCplex cplex(model);
//	cplex.exportModel("prueba.lp");
	//Parámetros de optimización
	cplex.setParam(IloCplex::Threads, 4); //numero de hilos
	cplex.setParam(IloCplex::TiLim, Sub->TiempoModelo); //tiempo
	//	cplex.setParam(IloCplex::SolnPoolReplace, 2);
	//	cplex.setParam(IloCplex::SolnPoolCapacity, 100); //Capacidad de la pool
		//Optimizamos	
	IloBool result = cplex.solve();

	//Si quisieramos generar soluciones
	// IloBool result = cplex.populate(); 
	//Estado del algoritmo	al terminar
	IloCplex::Status estado = cplex.getCplexStatus();

	if (Sub->Pruebas) cout << result << " " << estado;
	//número de soluciones obtenidas
	int Nsol = cplex.getSolnPoolNsolns();
	//VAlor del mip
	Sub->ObjMip = cplex.getObjValue();
	//Valor de la cota
	Sub->ObjBound = cplex.getBestObjValue();
	if (Sub->Pruebas) cout << "Best Value " << Sub->ObjMip << " Cota " << Sub->ObjBound;
	Sub->num_camiones_modelo = 0;
	Sub->num_camiones_extra_modelo = 0;
	Sub->max_cam_extra_tipo_modelo = 0;
	//Para leer todas las soluciones la 0 es la mejor
	for (auto i = 0; i < 1; i++)
	{
		//Extraer los valores de X
		for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
		{
			if (cplex.isExtracted(z_j[(*it_c)->CodIdCC]) && cplex.getValue(z_j[(*it_c)->CodIdCC], i) > 0.999)
				Sub->num_camiones_modelo++;
			int num_cam_extra_tipo = 0;
			for (int k = 0; k < (*it_c)->cota_extra; k++)
			{
				if (cplex.isExtracted(zp_jk[(*it_c)->CodIdCC][k]) && cplex.getValue(zp_jk[(*it_c)->CodIdCC][k], i) > 0.999)
				{
					Sub->num_camiones_extra_modelo++;
					num_cam_extra_tipo++;
				}
				if (num_cam_extra_tipo > Sub->max_cam_extra_tipo_modelo)
					Sub->max_cam_extra_tipo_modelo = num_cam_extra_tipo;
			}
			//			for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
			//			{
			//
			//
			//				if (cplex.getValue(x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC], i) > 0.999)
			//				{
			////					cout << "Posición " << (*it)->CodIdCC << " " << (*it_c)->CodIdCC << " elegida de la solución " << i << "\n";
			//				}
			//
			//
			//					if (cplex.getValue(xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k], i) > 0.999)
			//					{
			////						cout << "Posición " << (*it)->CodIdCC << " " << (*it_c)->CodIdCC << " " << k << " elegida de la solución " << i << "\n";
			//					}
			//				}
			//			}
		}
		if (Sub->Pruebas) cout << "camiones " << Sub->num_camiones_modelo << " Camiones extra " << Sub->num_camiones_extra_modelo << " Max Cam Extra Tipo " << Sub->max_cam_extra_tipo_modelo << endl;

		string nombreArchivo1 = "Resultados_cplex_items.txt";

		ofstream archivo;
		// Abrimos el archivo
		archivo.open(nombreArchivo1.c_str(), ofstream::out | ofstream::app);
		archivo << dir << ";" << Sub->lista_camiones.size() << ";" << Nsol << ";" << setprecision(8) << Sub->ObjMip << ";" << Sub->ObjBound << ";" << Sub->num_camiones_modelo << ";" << Sub->num_camiones_extra_modelo << ";" << Sub->max_cam_extra_tipo_modelo << endl;
		archivo.close();
	}

	//Cerramos el entorno
	cplex.end();
	model.end(); env.end();
	return 1;
}


//Esta función resuelve la asignación inicial y también la posterior
int Roadef2022ExtraCplex_V2(string dir, parameters param, Subproblema *Sub)
{
	
	/*if (!cuadra_num_items(Sub))
	{
		int kk = 9;
	}*/
	//Porcentge de volumen y peso.
	//para la aleatorización
//	double por_v = (double)97 /(double) 100;
//	double por_p = (double)97 / (double) 100;
	vector<int> cuantos_item(Sub->lista_items.size());
	vector<std::vector<bool>> matriz_variables(Sub->lista_items.size(), std::vector<bool>(Sub->lista_camiones.size(), false));

	double por_p = (double)Sub->porcentage_peso / 100;
	double por_v = (double)Sub->porcentage_vol / 100;
	double por_vheur = (double)1 + (double)(1 - por_v);
	//Establecemos el entorno
	IloEnv env;
	//Creamos el modelo
	IloModel model = IloModel(env);
	//Creamos las variables
	
	NumVarMatrix2 x_ij(env, Sub->lista_items.size());
	IloIntVarArray z_j(env, Sub->lista_camiones.size(), 0, 1);
	NumVarMatrix3 xp_ijk(env, Sub->lista_items.size());
	NumVarMatrix2 zp_jk(env, Sub->lista_camiones.size());
	//Si queremos que sean continuas
	//IloNumVarArray X2(env, num_nodos, 0, 1, ILOFLOAT);
	//Ponemos los nombres
	char nombre[20];
	if (Sub->Iteracion_Modelo>=2)
	Sub->PonerMasExtras();

	
	for (auto it_i = Sub->lista_items.begin(); it_i != Sub->lista_items.end(); it_i++) //Recorrer la lista de tipo items
	{
		
		if ((*it_i)->num_items <= 0)
			continue;
		//cout << (*it_i)->Id << ";" << (*it_i)->CodIdCC << ";" << (*it_i)->num_items << endl;
		x_ij[(*it_i)->CodIdCC] = IloIntVarArray(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
		xp_ijk[(*it_i)->CodIdCC] = NumVarMatrix2(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
	}
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		bool enc = false;
		if ((*it)->CodIdCC==34)
		{
			enc = true;
		}
		if ((*it)->no_puede_colocar_nada == true) continue;
		for(auto it_i=(*it)->items.begin();it_i!= (*it)->items.end();it_i++)
		{
			if (cuantos_item[(*it_i)->CodIdCC] > MAX_CAMIONES_POR_ITEM)
				continue;
			cuantos_item[(*it_i)->CodIdCC]++;
			matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] = true;
			if ((*it_i)->num_items <= 0)continue;
			if (!(*it)->lleno)
			{
				//Si existe la variable
				//cout << "cam:" << (*it)->CodIdCC << " iditem:" << (*it_i)->CodIdCC << endl;
				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);

				sprintf(nombre, "X_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].setName(nombre);
				
			}
			xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
			int i = 0;
			for (auto  itpe = (*it)->pcamiones_extra.begin(); itpe != (*it)->pcamiones_extra.end(); itpe++,i++)
			{
				if (!(*itpe)->lleno)
				{
					
					xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][i] = IloIntVar(env, 0, (*it_i)->num_items);
					sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, i);
					xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][i].setName(nombre);
				}
			}
			
			//for (int k = 0; k < (*it)->cota_extra; k++)
			for (auto k = (*it)->pcamiones_extra.size(); k < (*it)->cota_extra; k++)
			{
				
				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, (*it_i)->num_items);

				sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k].setName(nombre);
//				printf("Variable Definida %d %d %d\n", (*it_i)->CodIdCC, (*it)->CodIdCC, k);

			}
		}
	}
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{
			sprintf(nombre, "Z_%d", (*it)->CodIdCC);
			z_j[(*it)->CodIdCC].setName(nombre);
			//si el camion tiene peso cargado, la ponemos a 1 CAMBIADO
			/*
			if ((*it)->peso_cargado > 0)
				z_j[(*it)->CodIdCC].setLB(1);*/
			//Para la segunda vez, ponemos la variable a cero
			/*if ((*it)->items_modelo.size() > 0)
				z_j[(*it)->CodIdCC] = IloIntVar(env, 0, 0);*/
		}
		
		zp_jk[(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			zp_jk[(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
			sprintf(nombre, "ZP_%d_%d", (*it)->CodIdCC, k);
			zp_jk[(*it)->CodIdCC][k].setName(nombre);
		}
		
	}


	//Restricciones
	IloRangeArray restr(env);

	
	
	//Restricción suma de items igual a todos los items 
	//restricción 1

	for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
	{
		if ((*it)->num_items <= 0) continue;
		
		IloExpr expr1(env);
		bool algo = false;
		for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
		{
			/*if ((*it_c)->CodIdCC  == 19)
				int kk=0;*/
			if (matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
			if ((*it_c)->no_puede_colocar_nada == true) continue;
			if (!(*it_c)->lleno)
			{
				//			printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC,k);
				expr1 += x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
				algo = true;
			}
			int i = 0;
			for (auto itpe = (*it_c)->pcamiones_extra.begin(); itpe != (*it_c)->pcamiones_extra.end(); itpe++, i++)
			{
				if (!(*itpe)->lleno)
				{
					expr1 += xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][i];
					algo = true;
				}
			}
			for (auto k = (*it_c)->pcamiones_extra.size(); k < (*it_c)->cota_extra; k++)
			{
//				printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC, k);

				expr1 += xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
				algo = true;
			}

		}
		if (algo)
		{
			restr.add(IloRange(env, (*it)->num_items, expr1, (*it)->num_items));
			expr1.end();
		}
	}
	//Restricción 2 y 3
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		/*if ((*it)->CodIdCC == 6)
			int kk = 0;*/
		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{
			IloExpr expr2(env);
			IloExpr expr3(env);

			//for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			bool hay_items = false;
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				if (matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false) continue;
				if ((*it_i)->num_items <= 0) continue;
				//HE puesto el volumen para piezas grandes
				double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
				if ((*it)->volumen_heuristico < 1)
				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * volumen_ampliado;
				else
				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->volumen;
				expr3 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->weight;
				hay_items = true;
				
			}
			
			if (hay_items)
			{
				
				/*if ((((*it)->volumen_kp - (*it)->volumen_ocupado) < 0)||((*it)->max_loading_weight - (*it)->peso_cargado)<0)
				{
					int kk = 0;
					cout << "vol:" << (*it)->volumen_kp << " - " << (*it)->volumen_ocupado <<" = " << ((*it)->volumen_kp - (*it)->volumen_ocupado) << endl;
					cout << "peso:" << (*it)->max_loading_weight<<" - "<< (*it)->peso_cargado << " = " << ((*it)->max_loading_weight - (*it)->peso_cargado) << endl;
					PintarProblema(555);
				}*/
					
				//int delantev=((*it)->volumen_ocupado);
				//int delantep= ((*it)->peso_cargado) ;
//				expr2 += (-1) * z_j[(*it)->CodIdCC] * ((*it)->volumen_kp - (*it)->volumen_ocupado) * por_v;


				if ((*it)->volumen_heuristico < 1)
					expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_inicial * por_v;
				else
					expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_heuristico* por_vheur;
				expr3 += (-1) * z_j[(*it)->CodIdCC] * ((*it)->peso_inicial - (*it)->peso_cargado) * por_p;
				restr.add(IloRange(env, -IloInfinity, expr2, 0));
				restr.add(IloRange(env, -IloInfinity, expr3, 0));
			}
			expr2.end();
			expr3.end();
		}
		
		int indice = 0;
		if ((*it)->pcamiones_extra.size() > 0)
		{
			for (auto ite = (*it)->pcamiones_extra.begin(); ite != (*it)->pcamiones_extra.end(); ite++)
			{
				if (!(*ite)->lleno)
				{
					IloExpr expr2p(env);
					IloExpr expr3p(env);

					bool hay_items = false;
					for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
					{
						if (matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false) continue;
						if ((*it_i)->num_items <= 0) continue;
						//HE puesto el volumen para piezas grandes
						double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
						if ((*it)->volumen_heuristico < 1)
						expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][indice] * volumen_ampliado * por_vheur;
						else
						expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][indice] * (*it_i)->volumen;
						expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][indice] * (*it_i)->weight;
						hay_items = true;

					}

					if (hay_items)
					{
						/*if ((((*it)->volumen_kp - (*it)->volumen_ocupado) < 0) || ((*it)->max_loading_weight - (*it)->peso_cargado) < 0)
						{
							int kk = 0;
							PintarProblema(555);
						}*/

						//si el camion tiene peso cargado, la ponemos a 1 CAMBIADO
						/*if ((*ite)->peso_cargado > 0)
							zp_jk[(*it)->CodIdCC][indice].setLB(1);*/
		//				expr2p += (-1) * zp_jk[(*it)->CodIdCC][indice] * ((*it)->volumen_kp - (*ite)->volumen_ocupado) * por_v;

						if ((*it)->volumen_heuristico < 1)
							expr2p += (-1) * zp_jk[(*it)->CodIdCC][indice] * ((*it)->volumen_inicial - (*ite)->volumen_ocupado) * por_v;
						else
							expr2p += (-1) * zp_jk[(*it)->CodIdCC][indice] * ((*it)->volumen_heuristico - (*ite)->volumen_ocupado)* por_vheur;
						expr3p += (-1) * zp_jk[(*it)->CodIdCC][indice] * ((*it)->peso_inicial - (*ite)->peso_cargado) * por_p;

						restr.add(IloRange(env, -IloInfinity, expr2p, 0));
						restr.add(IloRange(env, -IloInfinity, expr3p, 0));
					}
					expr2p.end();
					expr3p.end();
				}
				indice++;
			}

		}
		for (int k = indice; k < (*it)->cota_extra; k++)
		{
			IloExpr expr2p(env);
			IloExpr expr3p(env);
			//Si es la primera vez que ejecuta el modelo y no ha creado ningun extra
			
			bool hay_items = false;
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				if (matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false) continue;
				if ((*it_i)->num_items <= 0) continue;
				//HE puesto el volumen para piezas grandes
				double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
				if ((*it)->volumen_heuristico < 1)
				expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * volumen_ampliado;
				else
				expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->volumen;
				expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->weight;
				hay_items = true;
				
			}

			if (hay_items)
			{
//				expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_kp * por_v;
				if ((*it)->volumen_heuristico < 1)
					expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_inicial * por_v;
				else
					expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_heuristico * por_vheur;
				expr3p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->peso_inicial * por_p;
				
				restr.add(IloRange(env, -IloInfinity, expr2p, 0));
				restr.add(IloRange(env, -IloInfinity, expr3p, 0));
			}
			
			
			expr2p.end();
			expr3p.end();
		}
		
	}

	//AÑADIMOS LAS RESTRICCIONES AL MODELO
	model.add(restr);

	//Función objetivo
	IloExpr obj(env);
	//Parte de camiones
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{
			if ((*it)->peso_cargado > 0)
			{
//				obj += z_j[(*it)->CodIdCC];
				int cambiado = 9;

			}
				
			else obj += z_j[(*it)->CodIdCC] * (*it)->cost * param.transportation_cost;
		}
		int i = 0;
		for (auto iti= (*it)->pcamiones_extra.begin(); iti!= (*it)->pcamiones_extra.end(); iti++,i++)
		{
			if (!(*iti)->lleno)
			{
				obj += zp_jk[(*it)->CodIdCC][i] * (*it)->cost * (1 + param.extra_truck_cost) * param.transportation_cost;
				int cambiado = 9;
			}
		}
		for (auto k = (*it)->pcamiones_extra.size(); k < (*it)->cota_extra; k++)
		{
			obj += zp_jk[(*it)->CodIdCC][k] *(*it)->cost * (1 + param.extra_truck_cost) * param.transportation_cost;
		}		
	}
	//Parte de items
	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{
		
		if ((*it_c)->no_puede_colocar_nada == true) continue;
		if (!(*it_c)->lleno)
		{
			//for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
			for (auto it_i = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it_i != Sub->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
			{
				if (matriz_variables[(*it_i)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
				if ((*it_i)->num_items <= 0) continue;
				//Dias de diferencia entre una fecha y otra
				//int days = difftime((*it_i)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
				int days = Days_0001((*it_i)->latest_arrival_tm_ptr.tm_year, (*it_i)->latest_arrival_tm_ptr.tm_mon + 1, (*it_i)->latest_arrival_tm_ptr.tm_mday) -
					Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);

				if (days == 0)
				{
//					obj += x_ij[(*it_i)->CodIdCC][(*it_c)->CodIdCC];
					int cambiado = 9;
				}
				else obj += (days)*x_ij[(*it_i)->CodIdCC][(*it_c)->CodIdCC] * (*it_i)->inventory_cost * param.inventory_cost;


			}
		}
		int fin = 0;
		for (auto ck=(*it_c)->pcamiones_extra.begin(); ck != (*it_c)->pcamiones_extra.end(); ck++)
		{
			if (!(*ck)->lleno)
			{
				for (auto it_i = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it_i != Sub->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
				{
					if (matriz_variables[(*it_i)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
					if ((*it_i)->num_items <= 0) continue;
					//int days = difftime((*it_i)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
					int days = Days_0001((*it_i)->latest_arrival_tm_ptr.tm_year, (*it_i)->latest_arrival_tm_ptr.tm_mon + 1, (*it_i)->latest_arrival_tm_ptr.tm_mday) -
						Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);

					if (days == 0)
					{
//						obj += xp_ijk[(*it_i)->CodIdCC][(*it_c)->CodIdCC][fin];
						int cambiado = 9;
					}
					else obj += days * xp_ijk[(*it_i)->CodIdCC][(*it_c)->CodIdCC][fin] * (*it_i)->inventory_cost * param.inventory_cost;
				}
			}
			fin++;
		}
		for (int k = fin; k < (*it_c)->cota_extra; k++)
		{
			for (auto it_i = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it_i != Sub->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
			{
				if (matriz_variables[(*it_i)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
				if ((*it_i)->num_items <= 0) continue;
				//int days = difftime((*it_i)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
				int days = Days_0001((*it_i)->latest_arrival_tm_ptr.tm_year, (*it_i)->latest_arrival_tm_ptr.tm_mon + 1, (*it_i)->latest_arrival_tm_ptr.tm_mday) -
					Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);

				if (days == 0)
				{
				//	obj += xp_ijk[(*it_i)->CodIdCC][(*it_c)->CodIdCC][k];
					int cambiado = 9;
				}
				else obj += days * xp_ijk[(*it_i)->CodIdCC][(*it_c)->CodIdCC][k] * (*it_i)->inventory_cost * param.inventory_cost;
			}
		}
	}
	//Añadimos la función objetivo al modelo

	IloObjective fobj = IloMinimize(env, obj);
	model.add(fobj);
	obj.end();
	IloCplex cplex(model);
	
	//if (Sub->Pruebas)	cplex.exportModel("prueba.lp");
	if (!Sub->Pruebas)
		cplex.setOut(env.getNullStream());
	//Parámetros de optimización
	cplex.setParam(IloCplex::MIPEmphasis, 1);//Feasibility
	cplex.setParam(IloCplex::Threads, 8); //numero de hilos
	if (Sub->cplex_trozo == false)
		cplex.setParam(IloCplex::TiLim, Sub->TiempoModelo); //tiempo
	else
		cplex.setParam(IloCplex::TiLim, Sub->TiempoModelo);
//	cplex.setParam(IloCplex::TiLim, max((double)Sub->TiempoModelo/(double)2,(double)1)); //tiempo
	//	cplex.setParam(IloCplex::SolnPoolReplace, 2);
	//	cplex.setParam(IloCplex::SolnPoolCapacity, 100); //Capacidad de la pool
		//Optimizamos	
	cplex.setParam(IloCplex::EpGap, MAX_GAP);
	IloBool result = cplex.solve();
	double gap = MAX_GAP;
	if (result)
	gap = cplex.getMIPRelativeGap();
	int cont = 1;
	while (gap > MAX_GAP && 2*cont < MAX_VECES_MODELO)
	{

		cplex.setParam(IloCplex::MIPEmphasis, 2);//Feasibility
		if (Sub->Pruebas)
			printf("Gap MAyor de Reducido 0.1 %d\n", cont);
		cplex.setParam(IloCplex::EpGap, MAX_GAP-0.001);
		result = cplex.solve();
		gap = cplex.getMIPRelativeGap();
		cont++;
	}
	//Si quisieramos generar soluciones
	// IloBool result = cplex.populate(); 
	//Estado del algoritmo	al terminar
	IloCplex::Status estado = cplex.getCplexStatus();

	if (Sub->Pruebas) cout << result << " " << estado;
	//número de soluciones obtenidas
	int Nsol = cplex.getSolnPoolNsolns();
	if (Nsol > 0)
		Sub->TieneSol = true;
	else
		Sub->TieneSol = false;
	//VAlor del mip
	Sub->ObjMip = cplex.getObjValue();
	//Valor de la cota
//	Sub->ObjBound = cplex.getBestObjValue();
	if (Sub->Pruebas) cout << "Best Value " << Sub->ObjMip << " Cota " << Sub->ObjBound;
	//Limpio todos los valores porque puede ser la segunda vez que lo llamo
	Sub->num_camiones_modelo = 0;
	Sub->num_camiones_extra_modelo = 0;
	Sub->max_cam_extra_tipo_modelo = 0;
	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{
		(*it_c)->items_modelo.clear();
		for (auto ita = (*it_c)->items_extras_modelo.begin(); ita != (*it_c)->items_extras_modelo.end(); ita++)
		{
			(*ita).clear();
		}
		//(*it_c)->num_items_extra.clear();
	}
	vector<truck*> camiones_afectados;
	
	for (auto i = 0; i < 1; i++)
	{
		Sub->num_camiones_modelo = 0;
		//Extraer los valores de X
		for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
		{	
			
			if ((*it_c)->no_puede_colocar_nada == true) continue;
			(*it_c)->Modelo_pesado = false;
			(*it_c)->index_minimo = -1;
			(*it_c)->num_items_modelo = 0;
			(*it_c)->total_items_modelos = 0;
			(*it_c)->peso_total_items_modelo = 0;
			(*it_c)->volumen_total_items_modelo = 0;
			(*it_c)->dim_minima = (*it_c)->length;
			(*it_c)->coste_invetario_items_por_colocar = 0;
			if (!(*it_c)->lleno)
			{
				if (cplex.isExtracted(z_j[(*it_c)->CodIdCC]) && cplex.getValue(z_j[(*it_c)->CodIdCC], i) > 0.999)
				{
					if (Sub->Pruebas) cout <<endl<< "Z_" << (*it_c)->CodIdCC<<": ";
					camiones_afectados.push_back((*it_c));
					(*it_c)->num_items_modelo = 0;
					(*it_c)->total_items_modelos = 0;
					(*it_c)->peso_total_items_modelo = 0;
					(*it_c)->volumen_total_items_modelo = 0;
					(*it_c)->dim_minima = (*it_c)->length;
					(*it_c)->coste_invetario_items_por_colocar = 0;
					Sub->num_camiones_modelo++;
					
					
					//for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
					for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
					{
						if (matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
						if ((*it)->num_items == 0)continue;
						//if (Sub->Pruebas) cout << "X_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC <<"="<<(*it)->num_items<< endl;
						if (cplex.isExtracted(x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC]) && cplex.getValue(x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC]) > 0.999)
						{
							// if (Sub->Pruebas) cout << "   X_" << (*it)->CodIdCC << "-" << (*it_c)->CodIdCC << "-" << (*it)->num_items << endl;
							int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
							int valor = (int)ceil(cplex.getValue(x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC]) - 0.001);
							if (Sub->Pruebas) cout << "   X_" << (*it)->CodIdCC << "-" << (*it_c)->CodIdCC << "=" << valor<<endl;
							(*it_c)->items_modelo.push_back(item_modelo(*it, valor, order,false));
							(*it_c)->num_items_modelo += valor;
							(*it_c)->peso_total_items_modelo += valor*(*it)->weight;
							(*it_c)->volumen_total_items_modelo +=valor* (*it)->volumen;
							if ((*it_c)->dim_minima > (*it)->length)(*it_c)->dim_minima = (*it)->length;
							if ((*it_c)->dim_minima > (*it)->width)(*it_c)->dim_minima = (*it)->width;
							//(*it_c)->coste_invetario_items_por_colocar += (*it)->inventory_cost * valor;
							
						}
					}
					if ((*it_c)->pilas_solucion.size() == 0)
					{
						Sub->num_camiones_modelo++;						
					}
					

				}
				(*it_c)->total_items_modelos += (*it_c)->num_items_modelo;
			}
			
			int num_cam_extra_tipo = 0;
			for (int k = 0; k < (*it_c)->cota_extra; k++)
			{
				
				if (cplex.isExtracted(zp_jk[(*it_c)->CodIdCC][k]) && cplex.getValue(zp_jk[(*it_c)->CodIdCC][k], i) > 0.999)
				{
					//crear el camion extra
					if (Sub->Pruebas) cout <<endl<< "ZP_" << (*it_c)->CodIdCC << "_" << k << ": ";
					
					
					truck* textra = NULL;
					vector<item_modelo> aux;
					double peso_total_items_modelo = 0;
					double volumen_total_items_modelo = 0;
					int cant_items = 0;
					bool nuevo = true;
					bool no_extra = false;

					//Si el camion extra ya existe
					if ((*it_c)->pcamiones_extra.size() > k)
					{
						int indice = 0;
						for (auto itt = (*it_c)->pcamiones_extra.begin(); itt != (*it_c)->pcamiones_extra.end(); itt++, indice++)
						{
							if (indice == k)
							{
								textra = (*itt);
								nuevo = false;
								break;
							}
						}
						textra->items_modelo.clear();
						camiones_afectados.push_back(textra);
						camiones_afectados.push_back((*it_c));
						
					}
					else
					{
						//Si el planificado va vacio, coloco la carga en el planificado
					//Esto se puede producir cuando el modelo no alcanza una solución optima.
						
						if ((*it_c)->num_items_modelo == 0 && (*it_c)->pilas_solucion.empty())
						{
							textra = (*it_c);
							camiones_afectados.push_back((*it_c));
							no_extra = true;
						}
						else
						{
							Sub->num_camiones_extra_modelo++;
							Sub->num_camiones_modelo++;
							num_cam_extra_tipo++;
							textra = new truck(*(*it_c));
							(*it_c)->pcamiones_extra.push_back(textra);
							textra->id_truck[0] = 'Q';
							string  num = "_" + to_string((int)(*it_c)->pcamiones_extra.size());
							strcat(textra->id_truck, num.c_str());
							camiones_afectados.push_back(textra);
							camiones_afectados.push_back((*it_c));
						}
					}
					textra->num_items_modelo = 0;
					textra->total_items_modelos = 0;
					textra->peso_total_items_modelo = 0;
					textra->volumen_total_items_modelo = 0;
					textra->dim_minima = textra->length;
					textra->coste_invetario_items_por_colocar = 0;
					
					
					//for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
					for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
					{
						if (matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
						if ((*it)->num_items == 0)continue;
						/*if (enc) 
							if (Pruebas) cout << " " << (*it)->CodIdCC << "-" << (*it_c)->CodIdCC << "-" << k << endl;*/
						if (cplex.isExtracted(xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) && cplex.getValue(xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > 0.999)
						{
							//if (Sub->Pruebas) cout << "   XP_" << (*it)->CodIdCC << "-" << (*it_c)->CodIdCC << "-" << k << endl;
							int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
							int valor = (int)ceil(cplex.getValue(xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.001);
							if (Sub->Pruebas) cout << "   XP_" << (*it)->CodIdCC << "-" << (*it_c)->CodIdCC << "-" << k << "=" << valor<< endl ;
							aux.push_back(item_modelo(*it, valor,order,false));
							textra->items_modelo.push_back(item_modelo(*it, valor, order,false));
							textra->num_items_modelo += valor;
							textra->peso_total_items_modelo+= valor * (*it)->weight;
							textra->volumen_total_items_modelo += valor * (*it)->volumen;
							//textra->coste_invetario_items_por_colocar += (*it)->inventory_cost * valor;
							cant_items += valor;
							peso_total_items_modelo +=valor* (*it)->weight;
							volumen_total_items_modelo +=valor* (*it)->volumen;
							if ((*it_c)->dim_minima > (*it)->length)(*it_c)->dim_minima = (*it)->length;
							if ((*it_c)->dim_minima > (*it)->width)(*it_c)->dim_minima = (*it)->width;
							if (textra->dim_minima > (*it)->length)textra->dim_minima = (*it)->length;
							if (textra->dim_minima > (*it)->width)textra->dim_minima = (*it)->width;
							textra->total_items_modelos += valor;
						}
					}
					if (no_extra == false)
					{
						if (nuevo)
						{

							(*it_c)->peso_total_items_modelo_extra.push_back(peso_total_items_modelo);
							(*it_c)->volumen_total_items_modelo_items_extra.push_back(volumen_total_items_modelo);
							(*it_c)->num_items_extra.push_back(cant_items);
							(*it_c)->items_extras_modelo.push_back(aux);

						}
						else
						{
							(*it_c)->num_items_extra.at(k) = cant_items;
							cont = k;
							for (auto ita = (*it_c)->items_extras_modelo.begin(); ita != (*it_c)->items_extras_modelo.end(); ita++, cont--)
							{
								if (cont == 0)
								{
									for (auto itb = aux.begin(); itb != aux.end(); itb++)
									{
										(*ita).push_back((*itb));
									}
								}
							}
						}
					}
					
					////(*it_c)->total_items_modelos += cant_items;
					//if (textra->items_modelo.size() == 0)
					//{
					//	int kk = 0;
					//}
					//int cont = k;
					//for (auto ita = (*it_c)->items_extras_modelo.begin(); ita != (*it_c)->items_extras_modelo.end(); ita++, cont--)
					//{
					//	if (cont == 0)
					//	{
					//		if ((*ita).size() != textra->items_modelo.size())
					//		{
					//			int kk = 0;
					//		}
					//	}
					//}
				}
				if (num_cam_extra_tipo > Sub->max_cam_extra_tipo_modelo)
					Sub->max_cam_extra_tipo_modelo = num_cam_extra_tipo;
			}
			
		}
		
	}
	if (Sub->Pruebas) cout << "Extra v2" << endl;
	for (auto cam = camiones_afectados.begin(); cam != camiones_afectados.end() && Nsol>0; cam++)
	{
		
		vector<item_modelo> aux;
		double peso_total_items_modelo = 0;
		double volumen_total_items_modelo = 0;
		int cant_items = 0;
		if (Sub->Pruebas) cout << "Extra v2_1" << endl;
		if ((*cam)->id_truck[0] == 'Q')
		{
			vector<truck*>::iterator itcam = cam;
			itcam++;
			int k = 0;
			
			for (auto ittt = (*itcam)->pcamiones_extra.begin(); ittt != (*itcam)->pcamiones_extra.end(); ittt++,k++)
			{
				if ((*ittt)->id_truck == (*cam)->id_truck)
				{
					break;
				}
			}
			
			//Quitar los items de pilas_colocadas y ponerlos en la lista de items modelo
			for (auto itt = (*cam)->pilas_solucion.begin(); itt != (*cam)->pilas_solucion.end(); itt++)
			{
				for (auto itt1 = (*itt).items.begin(); itt1 != (*itt).items.end(); itt1++)
				{
					for (auto itt2 = (*itcam)->items.begin(); itt2 != (*itcam)->items.end(); itt2++)
					{
						//if (Sub->Pruebas) cout << (*itt2)->Id << " " << (*itt1)->id_item << endl;
						if ((*itt2)->Id == (*itt1).id_item)
						{
							(*itt2)->num_items++;
							(*cam)->items_modelo.push_back(item_modelo(*itt2, 1, (*itcam)->orden_ruta[(*itt2)->sup_cod_dock],false));
							aux.push_back(item_modelo(*itt2, 1, (*itcam)->orden_ruta[(*itt2)->sup_cod_dock],false));
							cant_items += 1;
							peso_total_items_modelo += (*itt2)->weight;
							volumen_total_items_modelo += (*itt2)->volumen;
							break;
						}
					}
				}
				(*itt).items.clear();
			}
			(*cam)->pilas_solucion.clear();
			(*cam)->pila_multi_dock = false;
			(*cam)->vol_antes = (*cam)->volumen_ocupado;
			(*cam)->peso_antes = (*cam)->peso_cargado;
			(*cam)->peso_cargado = 0;			
			(*cam)->volumen_ocupado = 0;			
			(*cam)->volumen_a_cargar = 0; //Volumen que voy a querer cargar en el camion
			(*cam)->peso_a_cargar = 0; //Peso que voy a querer cargar en el camion
			(*cam)->Empezando_Camion = true;
			(*cam)->emm_colocado_anterior = (*cam)->emm_colocado;
			(*cam)->emr_colocado_anterior = (*cam)->emr_colocado;
			(*cam)->emm_colocado = 0;
			(*cam)->emr_colocado = 0;

			(*cam)->lleno = false;
			(*cam)->codigo_cierre = 0;
			
			(*cam)->orden_por_el_que_voy = 0;
			(*cam)->no_puede_colocar_nada = false;
		
			
			

			//lo borro del listado camiones
			for (auto itli = Sub->sol.listado_camiones.begin(); itli != Sub->sol.listado_camiones.end(); itli++)
			{
				if ((*itli).id_truc == (*cam)->id_truck)
				{
					itli=Sub->sol.listado_camiones.erase(itli);
					break;
				}
			}
			(*cam)->lesp.clear();
		
			(*itcam)->peso_total_items_modelo_extra.at(k) += peso_total_items_modelo;
			(*itcam)->volumen_total_items_modelo_items_extra.at(k) += volumen_total_items_modelo;
			(*cam)->num_items_modelo += cant_items;
			(*cam)->total_items_modelos = (*cam)->num_items_modelo;
			/*(*itcam)->num_items_extra.at(k) += cant_items;			
			(*cam)->total_items_modelos += cant_items;*/
			if ((*itcam)->items_extras_modelo.size() == 0)
			{
				(*itcam)->items_extras_modelo.push_back(aux);
			}
			else
			{
				list<vector<item_modelo>>::iterator itp = (*itcam)->items_extras_modelo.begin();
				for (int i = 0; i < k; i++)
					itp++;
				for (auto ita = aux.begin(); ita != aux.end(); ita++)
				{
					(*itp).push_back((*ita));
				}
			}
			int j = 0;
			int total = 0;
			for (auto ip = (*itcam)->items_extras_modelo.begin(); ip != (*itcam)->items_extras_modelo.end(); ip++,j++)
			{
				cont = 0;
				for (auto ip2 = (*ip).begin(); ip2 != (*ip).end(); ip2++)
				{
					cont += (*ip2).num_items;
				}
				(*itcam)->num_items_extra.at(j) = cont;
				if (j == k) //actualizo en el camion que estoy
				{
					(*cam)->total_items_modelos = cont;
					(*cam)->num_items_modelo = cont;
				}
				
				total += cont;
			}
			(*itcam)->total_items_modelos = (*itcam)->num_items_modelo + total;
			cam++;
		}
		else
		{
			if (Sub->Pruebas) cout << "Extra v2_2" << endl;
			//if (Sub->Pruebas && strcmp((*cam)->id_truck, "P314549701") == 0)
			//{
			//	int kk = 0;
			//}
			if (Sub->Pruebas)
			{
				cout << "A-"<<(*cam)->id_truck << ":" << (*cam)->volumen_ocupado << endl;
			}
			
			//Quitar los items de pilas_colocadas y ponerlos en la lista de items modelo
			for (auto itt = (*cam)->pilas_solucion.begin(); itt != (*cam)->pilas_solucion.end(); itt++)
			{
				for (auto itt1 = (*itt).items.begin(); itt1 != (*itt).items.end(); itt1++)
				{
					for (auto itt2 = (*cam)->items.begin(); itt2 != (*cam)->items.end(); itt2++)
					{
						//if (Sub->Pruebas) cout << (*itt2)->Id << " " << (*itt1)->id_item << endl;
						if ((*itt2)->Id == (*itt1).id_item)
						{
							(*cam)->items_modelo.push_back(item_modelo(*itt2, 1, (*cam)->orden_ruta[(*itt2)->sup_cod_dock],false));
							(*cam)->num_items_modelo += 1;
							(*cam)->total_items_modelos ++;
							(*itt2)->num_items++;
							(*cam)->peso_total_items_modelo += (*itt2)->weight;
							(*cam)->volumen_total_items_modelo += (*itt2)->volumen;
							if ((*cam)->dim_minima > (*itt2)->length)(*cam)->dim_minima = (*itt2)->length;
							if ((*cam)->dim_minima > (*itt2)->width)(*cam)->dim_minima = (*itt2)->width;
							break;
						}
					}
				}
				(*itt).items.clear();
			}
			(*cam)->pilas_solucion.clear();
			(*cam)->pila_multi_dock = false;
			(*cam)->peso_antes = (*cam)->peso_cargado;
			(*cam)->vol_antes = (*cam)->volumen_ocupado;
			(*cam)->peso_cargado = 0;
			(*cam)->volumen_ocupado = 0;

			(*cam)->volumen_a_cargar = 0; //Volumen que voy a querer cargar en el camion
			(*cam)->peso_a_cargar = 0; //Peso que voy a querer cargar en el camion
			(*cam)->Empezando_Camion = true;
			(*cam)->emm_colocado_anterior = (*cam)->emm_colocado;
			(*cam)->emr_colocado_anterior = (*cam)->emr_colocado;
			(*cam)->emm_colocado = 0;
			(*cam)->emr_colocado = 0;

			(*cam)->lleno = false;
			(*cam)->codigo_cierre = 0;

			(*cam)->orden_por_el_que_voy = 0;
			(*cam)->no_puede_colocar_nada = false;

			//lo borro del listado camiones
			for (auto itli = Sub->sol.listado_camiones.begin(); itli != Sub->sol.listado_camiones.end(); itli++)
			{
				if ((*itli).id_truc == (*cam)->id_truck)
				{
					itli=Sub->sol.listado_camiones.erase(itli);
					break;
				}
			}
			(*cam)->lesp.clear();
			for (auto ita = (*cam)->pcamiones_extra.begin(); ita != (*cam)->pcamiones_extra.end(); ita++)
			{
				(*cam)->total_items_modelos += (*ita)->num_items_modelo;
			}
		}
		
		
	}


	if (Sub->Pruebas) cout << "Terminado Extra v2" << endl;
	//Cerramos el entorno
	cplex.end();
	model.end();
	env.end();
	if (Sub->Pruebas) cout << "Terminado Extra v2 Parte 2" << endl;
	return 1;
}
void Subproblema::PonerAlgunosItemsABinarios()
{


	int cantidad = 1000;
	items_muchas_unidades.clear();
	items_muchas_unidades.resize(lista_items.size());
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			if (((*it_i)->num_items * (*it_i)->weight > ((*it)->peso_inicial / cantidad)) ||
				((*it_i)->num_items * (*it_i)->volumen > ((*it)->volumen_inicial / cantidad)))
			{
				items_muchas_unidades[(*it_i)->CodIdCC] = true;
				//				cout << "Item " << <<"Grandes " << (*it_i)->num_items << "Peso " << (*it_i)->weight<<endl;
			}

		}
	}
}
void Subproblema::CrearMatrizPosibles()
{

	//Esta matriz me indica que items pueden ir en cada camión
	//Podemos hacer que solamente vayan a un número de camiones
	matriz_variables.clear();
	matriz_variables.resize(lista_items.size(), std::vector<bool>(lista_camiones.size(), false));


	cuantos_item.clear();
	cuantos_item.resize(lista_items.size());
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			//int kk = (*it_i)->CodIdCC;
			//int kk2 = cuantos_item[(*it_i)->CodIdCC];
			if (cuantos_item[(*it_i)->CodIdCC] > MAX_CAMIONES_POR_ITEM)
				continue;
			cuantos_item[(*it_i)->CodIdCC]++;
			matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] = true;
		}
	}
}
int Roadef2022ExtraCplex(string dir, parameters param, Subproblema* Sub)
{

	/*if (!cuadra_num_items(Sub))
	{
		int kk = 0;
	}*/
	//Porcentge de volumen y peso.
	//para la aleatorización
	double por_p = (double)Sub->porcentage_peso / 100;
	double por_v = (double)Sub->porcentage_vol / 100;
	double por_vheur = (double)1 + (double)(1 - por_v);


	Sub->PonerAlgunosItemsABinarios();
	Sub->CrearMatrizPosibles();

	//Establecemos el entorno
	IloEnv env;
	//Creamos el modelo
	IloModel model = IloModel(env);
	//Creamos las variables

	NumVarMatrix2 x_ij(env, Sub->lista_items.size());
	IloIntVarArray z_j(env, Sub->lista_camiones.size(), 0, 1);
	NumVarMatrix3 xp_ijk(env, Sub->lista_items.size());
	NumVarMatrix2 zp_jk(env, Sub->lista_camiones.size());
	//Si queremos que sean continuas
	//IloNumVarArray X2(env, num_nodos, 0, 1, ILOFLOAT);
	//Ponemos los nombres
	char nombre[20];

	for (auto it_i = Sub->lista_items.begin(); it_i != Sub->lista_items.end(); it_i++) //Recorrer la lista de tipo items
	{

		if ((*it_i)->num_items <= 0)
			continue;
		//int ni = (*it_i)->num_items;
		//int cit = (*it_i)->CodIdCC;
		//
			x_ij[(*it_i)->CodIdCC] = IloIntVarArray(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
			xp_ijk[(*it_i)->CodIdCC] = NumVarMatrix2(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
		
	}

	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{

			if (Sub->cuantos_item[(*it_i)->CodIdCC] > MAX_CAMIONES_POR_ITEM)
				continue;

			if ((*it_i)->num_items <= 0)continue;
			if (!(*it)->lleno)
			{
				//Si existe la variable
				//cout << "cam:" << (*it)->CodIdCC << " iditem:" << (*it_i)->CodIdCC << endl;


				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == true)
				{
					//int ni = (*it_i)->num_items;
					//int cit = (*it_i)->CodIdCC;
					x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);
				}
				else
					x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, 1);
				//				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);

				sprintf(nombre, "X_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].setName(nombre);

			}
			xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
			for (int k = 0; k < (*it)->cota_extra; k++)
			{
				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == true)
					xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, (*it_i)->num_items);
				else
					xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
				sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k].setName(nombre);
				//				printf("Variable Definida %d %d %d\n", (*it_i)->CodIdCC, (*it)->CodIdCC, k);

			}
		}
	}
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{
			sprintf(nombre, "Z_%d", (*it)->CodIdCC);
			z_j[(*it)->CodIdCC].setName(nombre);
			//Para la segunda vez, ponemos la variable a cero
			/*if ((*it)->items_modelo.size() > 0)
				z_j[(*it)->CodIdCC] = IloIntVar(env, 0, 0);*/
		}
		zp_jk[(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			zp_jk[(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
			sprintf(nombre, "ZP_%d_%d", (*it)->CodIdCC, k);
			zp_jk[(*it)->CodIdCC][k].setName(nombre);
		}
	}


	//Restricciones
	IloRangeArray restr(env);


	//Restricción suma de items igual a todos los items 
	//restricción 1

	for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
	{
		if ((*it)->num_items <= 0) continue;

		IloExpr expr1(env);
		bool algo = false;
		for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
		{
			if (Sub->matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) 
				continue;
			if ((*it_c)->no_puede_colocar_nada == true) continue;
			if (!(*it_c)->lleno)
			{
				//			printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC,k);

				
				if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
					expr1 += (*it)->num_items * x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
				else
					expr1 += x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
				algo = true;
			}
			for (int k = 0; k < (*it_c)->cota_extra; k++)
			{
				//				printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC, k);
				if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
					expr1 += (*it)->num_items * xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
				else
					expr1 += xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
				algo = true;
			}

		}
		if (algo)
		{
			restr.add(IloRange(env, (*it)->num_items, expr1, (*it)->num_items));
			expr1.end();
		}
	}
	//Restricción 2 y 3
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{

		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{
			IloExpr expr2(env);
			IloExpr expr3(env);


			bool hay_items = false;
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false) 
					continue;
				if ((*it_i)->num_items <= 0) continue;
				int cuantos = 1;
				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
					cuantos = (*it_i)->num_items;
				double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
				//double dif_vol = volumen_ampliado - (*it_i)->volumen;
				if ((*it)->volumen_heuristico < 1)
					expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * volumen_ampliado * cuantos;
				else
					expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->volumen * cuantos;
				expr3 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->weight * cuantos;
				hay_items = true;

			}

			if (hay_items)
			{


				if ((*it)->volumen_heuristico < 1)
					expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_inicial * por_v;
				else
					expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_heuristico * por_vheur;
				expr3 += (-1) * z_j[(*it)->CodIdCC] * (*it)->peso_inicial * por_p;
				restr.add(IloRange(env, -IloInfinity, expr2, 0));
				restr.add(IloRange(env, -IloInfinity, expr3, 0));
			}
			expr2.end();
			expr3.end();
		}

		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			IloExpr expr2p(env);
			IloExpr expr3p(env);
			//Si es la primera vez que ejecuta el modelo y no ha creado ningun extra

			bool hay_items = false;
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false) 
					continue;
				if ((*it_i)->num_items <= 0) continue;
				int cuantos = 1;
				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
					cuantos = (*it_i)->num_items;
				double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
				if ((*it)->volumen_heuristico < 1)
					expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * volumen_ampliado * por_vheur * cuantos;
				else
					expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->volumen * cuantos;
				expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->weight * cuantos;
				hay_items = true;
			}

			if (hay_items)
			{
				//		expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_kp * por_v;
				if ((*it)->volumen_heuristico < 1)
					expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_inicial * por_v;
				else
					expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_heuristico * por_vheur;
				expr3p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->peso_inicial * por_p;

				restr.add(IloRange(env, -IloInfinity, expr2p, 0));
				restr.add(IloRange(env, -IloInfinity, expr3p, 0));
			}


			expr2p.end();
			expr3p.end();
		}
	}
	//Orden de los camiones
	/**/
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{

		if ((*it)->no_puede_colocar_nada == true) continue;
		IloExpr expr2p(env);
		expr2p += (-1) * z_j[(*it)->CodIdCC] * (*it)->cota_extra;
		//			obj += z_j[(*it)->CodIdCC] * (*it)->cost * param.transportation_cost;
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			expr2p += zp_jk[(*it)->CodIdCC][k];

		}
		restr.add(IloRange(env, -IloInfinity, expr2p, 0));
		expr2p.end();
	}
	//AÑADIMOS LAS RESTRICCIONES AL MODELO
	model.add(restr);

	//Función objetivo
	IloExpr obj(env);
	//Parte de camiones
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{

			obj += z_j[(*it)->CodIdCC] * (*it)->cost * param.transportation_cost;
		}
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			obj += zp_jk[(*it)->CodIdCC][k] * (*it)->cost * (1 + param.extra_truck_cost) * param.transportation_cost;

		}

	}
	//Parte de items
	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{

		if ((*it_c)->no_puede_colocar_nada == true) continue;
		if (!(*it_c)->lleno)
		{
			//int cuantos = Sub->mapa_camiones[(*it_c)->id_truck].size();
			for (auto it_i = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it_i != Sub->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
			{
				if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it_c)->CodIdCC] == false)
					continue;
				if ((*it_i)->num_items <= 0) continue;
				//Dias de diferencia entre una fecha y otra
				int cuantos = 1;
				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
					cuantos = (*it_i)->num_items;
				//				int days2 = difftime((*it_i)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
				int days = Days_0001((*it_i)->latest_arrival_tm_ptr.tm_year, (*it_i)->latest_arrival_tm_ptr.tm_mon + 1, (*it_i)->latest_arrival_tm_ptr.tm_mday) -
					Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);
				/*				if (days2 != days)
								{
									time_t tempt = (*it_i)->latest_arrival_tm;
									tm temp = (*it_i)->latest_arrival_tm_ptr;
									int kk = 9;
								}*/
				if (days == 0)
				{
					tm temp_i = (*it_i)->latest_arrival_tm_ptr;
					tm temp_c = (*it_c)->arrival_time_tm_ptr;
					//time_t temp_i2 = (*it_i)->latest_arrival_tm;
					//time_t temp_c2 = (*it_c)->arrival_time_tm;
					//long long int temp_i3 = (*it_i)->latest_arrival;
					//long long int temp_c3 = (*it_c)->arrival_time;
					//if ((temp_i.tm_min > temp_c.tm_min))
					//{
					//	int kk = 9;
					//}
					//int kk = 9;
				}
				else
					obj += cuantos * (days)*x_ij[(*it_i)->CodIdCC][(*it_c)->CodIdCC] * (*it_i)->inventory_cost * param.inventory_cost;


			}
		}
		for (int k = 0; k < (*it_c)->cota_extra; k++)
		{

			if (!(*it_c)->lleno)
			{
				for (auto it_i = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it_i != Sub->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
				{
					if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
					if ((*it_i)->num_items <= 0) continue;
					int cuantos = 1;
					if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
						cuantos = (*it_i)->num_items;
					//					int days2 = difftime((*it_i)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
					int days = Days_0001((*it_i)->latest_arrival_tm_ptr.tm_year, (*it_i)->latest_arrival_tm_ptr.tm_mon + 1, (*it_i)->latest_arrival_tm_ptr.tm_mday) -
						Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);
					/*					if (days2 != days)
										{
											time_t tempt = (*it_i)->latest_arrival_tm;
											tm temp = (*it_i)->latest_arrival_tm_ptr;
											int kk = 9;
										}
						*/					if (days > 0)
					obj += cuantos * days * xp_ijk[(*it_i)->CodIdCC][(*it_c)->CodIdCC][k] * (*it_i)->inventory_cost * param.inventory_cost;
				}
			}
		}
	}
	//Añadimos la función objetivo al modelo

	IloObjective fobj = IloMinimize(env, obj);
	model.add(fobj);
	obj.end();
	IloCplex cplex(model);
		//cplex.exportModel("prueba1.lp");
		//cplex.setOut(env.getNullStream());
	if (!Sub->Pruebas)
		cplex.setOut(env.getNullStream());
	//Parámetros de optimización
	cplex.setParam(IloCplex::Threads, 8); //numero de hilos
	cplex.setParam(IloCplex::MIPEmphasis, 1);//Feasibility


	if (Sub->cplex_trozo == false)
		cplex.setParam(IloCplex::TiLim, Sub->TiempoModelo); //tiempo
	else
		cplex.setParam(IloCplex::TiLim, max((double)Sub->TiempoModelo / (double)5, (double)1)); //tiempo
	//	cplex.setParam(IloCplex::SolnPoolReplace, 2);
	//	cplex.setParam(IloCplex::SolnPoolCapacity, 100); //Capacidad de la pool
		//Optimizamos	
	//Para inicializar una solución
	IloNumVarArray vars(env);
	IloNumArray vals(env);
	if (Sub->Iter_Resuelve_Subproblema > 0)
	{
		for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
		{
			for (auto ic=(*it)->camiones.begin(); ic != (*it)->camiones.end();ic++)
			{

				if (Sub->X_ij[(*it)->CodIdCC][(*ic)->CodIdCC]>0)
				{
					vars.add(x_ij[(*it)->CodIdCC][(*ic)->CodIdCC]);
					vals.add(Sub->X_ij[(*it)->CodIdCC][(*ic)->CodIdCC]);
				}
				for (int k = 0; k < (*ic)->cota_extra; k++)
				{
					if (Sub->XP_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][k] > 0)
					{
						vars.add(xp_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][k]);
						vals.add(Sub->XP_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][k]);
					}
				}

			}
		}
		for (auto ic = Sub->lista_camiones.begin(); ic != Sub->lista_camiones.end(); ic++)
		{
			if (Sub->Z_j[(*ic)->CodIdCC]==true)
			{
				vars.add(z_j[(*ic)->CodIdCC]);
				vals.add(1);
			}
			for (int k = 0; k < (*ic)->cota_extra; k++)
			{
				if (Sub->ZP_jk[(*ic)->CodIdCC][k] == true)
				{
					vars.add(zp_jk[(*ic)->CodIdCC][k]);
					vals.add(1);
				}
			}
		}
		cplex.addMIPStart(vars, vals, cplex.MIPStartRepair);

	}
	
	double tiempo = Sub->TiempoModelo;
	double gap = MAX_GAP;
	int gap_max = 1000 * MAX_GAP;
	int alea = get_random((2*gap_max) / 3, gap_max);
	cplex.setParam(IloCplex::EpGap, gap);

	IloBool result = cplex.solve();
	double GAP = (double)alea / double(1000);
	if (result)
		gap = cplex.getMIPRelativeGap();
	int cont = 1;
	while (gap > GAP && cont < MAX_VECES_MODELO)
	{
		cplex.setParam(IloCplex::MIPEmphasis, 2);//Feasibility
		if (Sub->Pruebas)
			printf("\nGap MAyor de %.3f %d\n", GAP, cont);
		cplex.setParam(IloCplex::EpGap, GAP - 0.001);
		result = cplex.solve();
		if (result)
		{
			gap = cplex.getMIPRelativeGap();
		}
		else
		{
			//cplex.exportModel("pruebam.lp");
			gap = GAP + 0.01;
			tiempo = 2 * tiempo;
			cplex.setParam(IloCplex::TiLim, tiempo);
		}
		cont++;
	}

	//Si quisieramos generar soluciones
	// IloBool result = cplex.populate(); 
	//Estado del algoritmo	al terminar
	IloCplex::Status estado = cplex.getCplexStatus();

	if (Sub->Pruebas) cout << result << " " << estado << endl;
	//número de soluciones obtenidas
	int Nsol = cplex.getSolnPoolNsolns();
	if (Nsol > 0)
		Sub->TieneSol = true;
	else
		Sub->TieneSol = false;
	//VAlor del mip
	double objbound = cplex.getBestObjValue();
	if (objbound > Sub->ObjBound_Original)
		Sub->ObjBound_Original = objbound;
	Sub->ObjMip = cplex.getObjValue();
	//Valor de la cota
	if (Sub->porcentage_peso >= 99 && Sub->porcentage_vol >= 99)
		Sub->ObjBound = cplex.getBestObjValue();
	if (Sub->Pruebas) cout << "Best Value " << Sub->ObjMip << " Cota " << Sub->ObjBound << endl;
	//Limpio todos los valores porque puede ser la segunda vez que lo llamo
	Sub->num_camiones_modelo = 0;
	Sub->num_camiones_extra_modelo = 0;
	Sub->max_cam_extra_tipo_modelo = 0;
	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{
		(*it_c)->items_modelo.clear();
		(*it_c)->items_extras_modelo.clear();
		(*it_c)->num_items_extra.clear();
	}
	ConvertirSolucionModeloACamiones(Sub, &cplex, &x_ij, &z_j, &xp_ijk, &zp_jk);
	if (Sub->Pruebas) cout << "Terminado RoadefExtra" << endl;

	//Cerramos el entorno
	cplex.end();
	model.end(); env.end();
	return 1;
}

void ConvertirSolucionModeloACamiones( Subproblema *Sub, IloCplex *cplex, NumVarMatrix2 *x_ij,IloIntVarArray *z_j, NumVarMatrix3 *xp_ijk, NumVarMatrix2 *zp_jk)
{

	if (Sub->Best_Sol_Modelo>(Sub->ObjMip+0.00001))
	{
		Sub->Best_Sol_Modelo = Sub->ObjMip;
		Sub->Z_j.clear();
		Sub->X_ij.clear();
		Sub->XP_ijk.clear();
		Sub->ZP_jk.clear();
		Sub->Z_j.resize(Sub->lista_camiones.size(), false);
		Sub->X_ij.resize(Sub->lista_items.size(), std::vector<int>(Sub->lista_camiones.size(), 0));
		Sub->XP_ijk.resize(Sub->lista_items.size(), std::vector<vector<int>>(Sub->lista_camiones.size()));
		for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
			for (auto ic = (*it)->camiones.begin(); ic != (*it)->camiones.end(); ic++)
				Sub->XP_ijk[(*it)->CodIdCC][(*ic)->CodIdCC].resize((*ic)->cota_extra, 0);

		Sub->ZP_jk.resize(Sub->lista_camiones.size());
	}
	for (auto ic = Sub->lista_camiones.begin(); ic != Sub->lista_camiones.end(); ic++)
		Sub->ZP_jk[(*ic)->CodIdCC].resize((*ic)->cota_extra, 0);
	for (auto i = 0; i < 1; i++)
	{
		//Extraer los valores de X
		for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
		{
			if ((*it_c)->no_puede_colocar_nada == true) continue;
			(*it_c)->Modelo_pesado = false;
			(*it_c)->index_minimo = -1;
			(*it_c)->num_items_modelo = 0;
			(*it_c)->total_items_modelos = 0;
			(*it_c)->peso_total_items_modelo = 0;
			(*it_c)->volumen_total_items_modelo = 0;
			(*it_c)->dim_minima = (*it_c)->length;
			(*it_c)->coste_invetario_items_por_colocar = 0;
			if (!(*it_c)->lleno)
			{
				if (cplex->isExtracted((*z_j)[(*it_c)->CodIdCC]) && cplex->getValue((*z_j)[(*it_c)->CodIdCC], i) > 0.999)
				{
					Sub->Z_j[(*it_c)->CodIdCC] = true;
					/*if (Sub->Pruebas) cout << "Z_" <<  (*it_c)->CodIdCC << endl;*/
					Sub->num_camiones_modelo++;
					(*it_c)->num_items_modelo = 0;
					(*it_c)->total_items_modelos = 0;
					(*it_c)->peso_total_items_modelo = 0;
					(*it_c)->volumen_total_items_modelo = 0;
					(*it_c)->dim_minima = (*it_c)->length;
					(*it_c)->coste_invetario_items_por_colocar = 0;
					//for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
					for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
					{
						if (Sub->matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
						if ((*it)->num_items == 0)continue;

						if (cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) > 0.999)
						{
							
							int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
							int valor = (int)ceil(cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) - 0.001);
							Sub->X_ij[(*it)->CodIdCC][(*it_c)->CodIdCC] = valor;
							int cuantos = 1;
							if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
								cuantos = (*it)->num_items;
							(*it_c)->items_modelo.push_back(item_modelo(*it, cuantos * valor, order,false));
							(*it_c)->num_items_modelo += cuantos * valor;
							(*it_c)->peso_total_items_modelo += cuantos * valor * (*it)->weight;
							(*it_c)->volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
							if ((*it_c)->dim_minima > (*it)->length)(*it_c)->dim_minima = (*it)->length;
							if ((*it_c)->dim_minima > (*it)->width)(*it_c)->dim_minima = (*it)->width;
							//(*it_c)->coste_invetario_items_por_colocar += (*it)->inventory_cost * valor;

						}
					}

				}
				(*it_c)->total_items_modelos += (*it_c)->num_items_modelo;
			}

			int num_cam_extra_tipo = 0;
			for (int k = 0; k < (*it_c)->cota_extra; k++)
			{
				if (cplex->isExtracted((*zp_jk)[(*it_c)->CodIdCC][k]) && cplex->getValue((*zp_jk)[(*it_c)->CodIdCC][k], i) > 0.999)
				{
					Sub->ZP_jk[(*it_c)->CodIdCC][k] = true;
					//crear el camion extra
					/*if (Sub->Pruebas)
						cout << "ZP_" << (*it_c)->CodIdCC << "_"<< k << endl;*/
					if ((*it_c)->items_modelo.size() == 0)
					{
						Sub->num_camiones_modelo++;
						(*it_c)->num_items_modelo = 0;
						(*it_c)->total_items_modelos = 0;
						(*it_c)->peso_total_items_modelo = 0;
						(*it_c)->volumen_total_items_modelo = 0;
						(*it_c)->dim_minima = (*it_c)->length;
						(*it_c)->coste_invetario_items_por_colocar = 0;
						for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
						{
							if (Sub->matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
							if ((*it)->num_items == 0)continue;

							if (cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > 0.999)
							{
								int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
								int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.001);
								Sub->XP_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k] = valor;
								int cuantos = 1;
								if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
									cuantos = (*it)->num_items;
								(*it_c)->items_modelo.push_back(item_modelo(*it, cuantos * valor, order,false));
								(*it_c)->num_items_modelo += cuantos * valor;
								(*it_c)->peso_total_items_modelo += cuantos * valor * (*it)->weight;
								(*it_c)->volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
								if ((*it_c)->dim_minima > (*it)->length)(*it_c)->dim_minima = (*it)->length;
								if ((*it_c)->dim_minima > (*it)->width)(*it_c)->dim_minima = (*it)->width;

							}
						}
						(*it_c)->total_items_modelos += (*it_c)->num_items_modelo;
					}
					else
					{
						if ((*it_c)->num_items_modelo == 0)
						{
							//int kk = 9;
							PintarProblema(110);
						}
						truck* textra = new truck(*(*it_c));
						(*it_c)->pcamiones_extra.push_back(textra);
						textra->num_items_modelo = 0;
						textra->total_items_modelos = 0;
						textra->peso_total_items_modelo = 0;
						textra->volumen_total_items_modelo = 0;
						textra->dim_minima = textra->length;
						textra->coste_invetario_items_por_colocar = 0;
						textra->id_truck[0] = 'Q';
						string  num = "_" + to_string((int)(*it_c)->pcamiones_extra.size());
						strcat(textra->id_truck, num.c_str());

						double peso_total_items_modelo = 0;
						double volumen_total_items_modelo = 0;
						Sub->num_camiones_extra_modelo++;
						Sub->num_camiones_modelo++;
						vector<item_modelo> aux;
						int cant_items = 0;
						//for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
						for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
						{
							if (Sub->matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
							if ((*it)->num_items == 0)continue;

							if (cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > 0.999)
							{
								int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
								int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.001);
								int cuantos = 1;
								if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
									cuantos = (*it)->num_items;
								aux.push_back(item_modelo(*it, cuantos * valor, order,false));
								textra->items_modelo.push_back(item_modelo(*it, cuantos * valor, order,false));
								textra->num_items_modelo += cuantos * valor;
								textra->peso_total_items_modelo += cuantos * valor * (*it)->weight;
								textra->volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
								//textra->coste_invetario_items_por_colocar += (*it)->inventory_cost * valor;
								cant_items += cuantos * valor;
								peso_total_items_modelo += cuantos * valor * (*it)->weight;
								volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
								if ((*it_c)->dim_minima > (*it)->length)(*it_c)->dim_minima = (*it)->length;
								if ((*it_c)->dim_minima > (*it)->width)(*it_c)->dim_minima = (*it)->width;
								if (textra->dim_minima > (*it)->length)textra->dim_minima = (*it)->length;
								if (textra->dim_minima > (*it)->width)textra->dim_minima = (*it)->width;
								textra->total_items_modelos += cuantos * valor;
							}
						}
						(*it_c)->peso_total_items_modelo_extra.push_back(peso_total_items_modelo);
						(*it_c)->volumen_total_items_modelo_items_extra.push_back(volumen_total_items_modelo);
						(*it_c)->items_extras_modelo.push_back(aux);
						(*it_c)->num_items_extra.push_back(cant_items);
						(*it_c)->total_items_modelos += cant_items;
						num_cam_extra_tipo++;
					}


				}
				if (num_cam_extra_tipo > Sub->max_cam_extra_tipo_modelo)
					Sub->max_cam_extra_tipo_modelo = num_cam_extra_tipo;
			}

		}

	}
}
//Esta es la función que va fijando va
int Roadef2022Fixing(string dir, parameters param, Subproblema* Sub)
{

	/*if (!cuadra_num_items(Sub))
	{
		int kk = 0;
	}*/
	//Porcentge de volumen y peso.
	//para la aleatorización
	double por_p = (double)Sub->porcentage_peso / 100;
	double por_v = (double)Sub->porcentage_vol / 100;
	double por_vheur = (double)1 + (double)(1 - por_v);

	//Definimos un vector de variables de items
	int cantidad = 100;
	vector<bool> items_muchas_unidades(Sub->lista_items.size());
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			if (((*it_i)->num_items * (*it_i)->weight > ((*it)->peso_inicial / cantidad)) ||
				((*it_i)->num_items * (*it_i)->volumen > ((*it)->volumen_inicial / cantidad)))
			{
				items_muchas_unidades[(*it_i)->CodIdCC] = true;
				//				cout << "Item " << <<"Grandes " << (*it_i)->num_items << "Peso " << (*it_i)->weight<<endl;
			}

		}
	}

	//Establecemos el entorno
	IloEnv env;
	//Creamos el modelo
	IloModel model = IloModel(env);
	//Creamos las variables

	NumVarMatrix2 x_ij(env, Sub->lista_items.size());
	IloIntVarArray z_j(env, Sub->lista_camiones.size(), 0, 1);
	NumVarMatrix3 xp_ijk(env, Sub->lista_items.size());
	NumVarMatrix2 zp_jk(env, Sub->lista_camiones.size());
	//Si queremos que sean continuas
	//IloNumVarArray X2(env, num_nodos, 0, 1, ILOFLOAT);
	//Ponemos los nombres
	char nombre[20];

	for (auto it_i = Sub->lista_items.begin(); it_i != Sub->lista_items.end(); it_i++) //Recorrer la lista de tipo items
	{
		
		if ((*it_i)->num_items <= 0)
			continue;
		x_ij[(*it_i)->CodIdCC] = IloIntVarArray(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
		xp_ijk[(*it_i)->CodIdCC] = NumVarMatrix2(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
	}

	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			//int kk = (*it_i)->CodIdCC;
			//int kk2 = Sub->cuantos_item[(*it_i)->CodIdCC];
			if (Sub->cuantos_item[(*it_i)->CodIdCC]>MAX_CAMIONES_POR_ITEM)
				continue;
			Sub->cuantos_item[(*it_i)->CodIdCC]++;
			Sub->matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] = true;
			if ((*it_i)->num_items <= 0)continue;
			if (!(*it)->lleno)
			{
				//Si existe la variable
				//cout << "cam:" << (*it)->CodIdCC << " iditem:" << (*it_i)->CodIdCC << endl;


				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == true)
					x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);
				else
					x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, 1);
//				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);

				sprintf(nombre, "X_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].setName(nombre);

			}
			xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
			for (int k = 0; k < (*it)->cota_extra; k++)
			{
				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == true)
				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, (*it_i)->num_items);
				else
					xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
				sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k].setName(nombre);
				//				printf("Variable Definida %d %d %d\n", (*it_i)->CodIdCC, (*it)->CodIdCC, k);

			}
		}
	}
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{
			sprintf(nombre, "Z_%d", (*it)->CodIdCC);
			z_j[(*it)->CodIdCC].setName(nombre);
			//Para la segunda vez, ponemos la variable a cero
			/*if ((*it)->items_modelo.size() > 0)
				z_j[(*it)->CodIdCC] = IloIntVar(env, 0, 0);*/
		}
		zp_jk[(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			zp_jk[(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
			sprintf(nombre, "ZP_%d_%d", (*it)->CodIdCC, k);
			zp_jk[(*it)->CodIdCC][k].setName(nombre);
		}
	}


	//Restricciones
	IloRangeArray restr(env);


	//Restricción suma de items igual a todos los items 
	//restricción 1

	for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
	{
		if ((*it)->num_items <= 0) continue;

		IloExpr expr1(env);
		bool algo = false;
		for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
		{
			if (Sub->matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
			if ((*it_c)->no_puede_colocar_nada == true) continue;
			if (!(*it_c)->lleno)
			{
				//			printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC,k);
				if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
				expr1 += (*it)->num_items* x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
				else
					expr1 += x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
				algo = true;
			}
			for (int k = 0; k < (*it_c)->cota_extra; k++)
			{
				//				printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC, k);
				if (items_muchas_unidades[(*it)->CodIdCC] == false)
					expr1 += (*it)->num_items * xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
				else
					expr1 += xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
				algo = true;
			}

		}
		if (algo)
		{
			restr.add(IloRange(env, (*it)->num_items, expr1, (*it)->num_items));
			expr1.end();
		}
	}
	//Restricción 2 y 3
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{

		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{
			IloExpr expr2(env);
			IloExpr expr3(env);

			
			bool hay_items = false;
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false) continue;
				if ((*it_i)->num_items <= 0) continue;
				int cuantos = 1;
				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
					cuantos = (*it_i)->num_items;
				double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
				//double dif_vol = volumen_ampliado - (*it_i)->volumen;
				if ((*it)->volumen_heuristico<1)
				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * volumen_ampliado*cuantos;
				else
				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->volumen * cuantos;
				expr3 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->weight * cuantos;
				hay_items = true;

			}

			if (hay_items)
			{
				
				
				if ((*it)->volumen_heuristico < 1)
					expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_inicial * por_v;
				else
					expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_heuristico * por_vheur;
				expr3 += (-1) * z_j[(*it)->CodIdCC] * (*it)->peso_inicial * por_p;
				restr.add(IloRange(env, -IloInfinity, expr2, 0));
				restr.add(IloRange(env, -IloInfinity, expr3, 0));
			}
			expr2.end();
			expr3.end();
		}
		
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			IloExpr expr2p(env);
			IloExpr expr3p(env);
			//Si es la primera vez que ejecuta el modelo y no ha creado ningun extra

			bool hay_items = false;
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false) continue;
				if ((*it_i)->num_items <= 0) continue;
				int cuantos = 1;
				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
					cuantos = (*it_i)->num_items;
				double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
				if ((*it)->volumen_heuristico < 1)
					expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * volumen_ampliado * por_vheur * cuantos;
				else
				expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->volumen * cuantos;
				expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->weight * cuantos;
				hay_items = true;
			}

			if (hay_items)
			{
		//		expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_kp * por_v;
				if ((*it)->volumen_heuristico<1)
				expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_inicial * por_v;
				else
					expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_heuristico * por_vheur;
				expr3p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->peso_inicial * por_p;
				
				restr.add(IloRange(env, -IloInfinity, expr2p, 0));
				restr.add(IloRange(env, -IloInfinity, expr3p, 0));
			}


			expr2p.end();
			expr3p.end();
		}
	}
	//Orden de los camiones
	/**/
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{

		if ((*it)->no_puede_colocar_nada == true) continue;
		IloExpr expr2p(env);
		expr2p += (-1) * z_j[(*it)->CodIdCC]*(*it)->cota_extra;
//			obj += z_j[(*it)->CodIdCC] * (*it)->cost * param.transportation_cost;
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			expr2p +=  zp_jk[(*it)->CodIdCC][k];

		}
		restr.add(IloRange(env, -IloInfinity, expr2p, 0));
		expr2p.end();
	}
	//AÑADIMOS LAS RESTRICCIONES AL MODELO
	model.add(restr);

	//Función objetivo
	IloExpr obj(env);
	//Parte de camiones
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{
			
			obj += z_j[(*it)->CodIdCC] * (*it)->cost * param.transportation_cost;
		}
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			obj += zp_jk[(*it)->CodIdCC][k] * (*it)->cost * (1 + param.extra_truck_cost) * param.transportation_cost;

		}
		
	}
	//Parte de items
	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{
		
	
		if ((*it_c)->no_puede_colocar_nada == true) continue;
		if (!(*it_c)->lleno)
		{
			//int cuantos = Sub->mapa_camiones[(*it_c)->id_truck].size();
			for (auto it_i = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it_i != Sub->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
			{
				if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
				if ((*it_i)->num_items <= 0) continue;
				//Dias de diferencia entre una fecha y otra
				int cuantos = 1;
				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] ==false)
					cuantos = (*it_i)->num_items;
//				int days2 = difftime((*it_i)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
				int days = Days_0001((*it_i)->latest_arrival_tm_ptr.tm_year, (*it_i)->latest_arrival_tm_ptr.tm_mon + 1, (*it_i)->latest_arrival_tm_ptr.tm_mday) -
					Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);
/*				if (days2 != days)
				{
					time_t tempt = (*it_i)->latest_arrival_tm;
					tm temp = (*it_i)->latest_arrival_tm_ptr;
					int kk = 9;
				}*/
				if (days == 0)
				{
					tm temp_i = (*it_i)->latest_arrival_tm_ptr;
					tm temp_c = (*it_c)->arrival_time_tm_ptr;
					//time_t temp_i2 = (*it_i)->latest_arrival_tm;
					//time_t temp_c2 = (*it_c)->arrival_time_tm;
					//long long int temp_i3 = (*it_i)->latest_arrival;
					//long long int temp_c3 = (*it_c)->arrival_time;
					//if ( ( temp_i.tm_min > temp_c.tm_min))
					//{
					//	int kk = 9;
					//}
					//int kk = 9;
				}
				else
				obj += cuantos*(days)*x_ij[(*it_i)->CodIdCC][(*it_c)->CodIdCC] * (*it_i)->inventory_cost * param.inventory_cost;


			}
		}
		for (int k = 0; k < (*it_c)->cota_extra; k++)
		{
			
			if (!(*it_c)->lleno)
			{
				for (auto it_i = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it_i != Sub->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
				{
					if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
					if ((*it_i)->num_items <= 0) continue;
					int cuantos = 1;
					if (items_muchas_unidades[(*it_i)->CodIdCC] == false)
						cuantos = (*it_i)->num_items;
//					int days2 = difftime((*it_i)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
					int days = Days_0001((*it_i)->latest_arrival_tm_ptr.tm_year, (*it_i)->latest_arrival_tm_ptr.tm_mon + 1, (*it_i)->latest_arrival_tm_ptr.tm_mday) -
						Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);
/*					if (days2 != days)
					{
						time_t tempt = (*it_i)->latest_arrival_tm;
						tm temp = (*it_i)->latest_arrival_tm_ptr;
						int kk = 9;
					}
	*/					if (days>0)
					obj += cuantos*days * xp_ijk[(*it_i)->CodIdCC][(*it_c)->CodIdCC][k] * (*it_i)->inventory_cost * param.inventory_cost;
				}
			}
		}
	}
	//Añadimos la función objetivo al modelo

	IloObjective fobj = IloMinimize(env, obj);
	model.add(fobj);
	obj.end();
	IloCplex cplex(model);
//	cplex.exportModel("prueba1.lp");
	//cplex.setOut(env.getNullStream());
	if (!Sub->Pruebas)
		cplex.setOut(env.getNullStream());
	//Parámetros de optimización
	cplex.setParam(IloCplex::Threads, 1); //numero de hilos
	cplex.setParam(IloCplex::MIPEmphasis, 1);//Feasibility

	
	if (Sub->cplex_trozo == false)
		cplex.setParam(IloCplex::TiLim, Sub->TiempoModelo); //tiempo
	else
		cplex.setParam(IloCplex::TiLim, max((double)Sub->TiempoModelo / (double)5,(double) 1)); //tiempo
	//	cplex.setParam(IloCplex::SolnPoolReplace, 2);
	//	cplex.setParam(IloCplex::SolnPoolCapacity, 100); //Capacidad de la pool
		//Optimizamos	

	double gap = MAX_GAP;
	int gap_max = 1000 * MAX_GAP;
	int alea = get_random(gap_max / 2, gap_max);
	cplex.setParam(IloCplex::EpGap, gap);
	IloBool result = cplex.solve();
	double GAP = (double)alea / double(1000);
	if (result)
		gap = cplex.getMIPRelativeGap();
	int cont = 1;
	while (gap > GAP && cont<MAX_VECES_MODELO)
	{
		cplex.setParam(IloCplex::MIPEmphasis, 2);//Feasibility
		if (Sub->Pruebas)
			printf("\nGap MAyor de %.3f %d\n",GAP, cont);
		cplex.setParam(IloCplex::EpGap, GAP- 0.001);
		result = cplex.solve();
		if (result)
			gap = cplex.getMIPRelativeGap();

		cont++;
	}
		
	//Si quisieramos generar soluciones
	// IloBool result = cplex.populate(); 
	//Estado del algoritmo	al terminar
	IloCplex::Status estado = cplex.getCplexStatus();

	if (Sub->Pruebas) cout << result << " " << estado<<endl;
	//número de soluciones obtenidas
	int Nsol = cplex.getSolnPoolNsolns();
	if (Nsol > 0)
		Sub->TieneSol = true;
	else
		Sub->TieneSol = false;
	//VAlor del mip
	Sub->ObjBound_Original = cplex.getBestObjValue();
	Sub->ObjMip = cplex.getObjValue();
	//Valor de la cota
	if (Sub->porcentage_peso >= 99 && Sub->porcentage_vol >= 99)
		Sub->ObjBound = cplex.getBestObjValue();
	if (Sub->Pruebas) cout << "Best Value " << Sub->ObjMip << " Cota " << Sub->ObjBound<<endl;
	//Limpio todos los valores porque puede ser la segunda vez que lo llamo
	Sub->num_camiones_modelo = 0;
	Sub->num_camiones_extra_modelo = 0;
	Sub->max_cam_extra_tipo_modelo = 0;
	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{
		(*it_c)->items_modelo.clear();
		(*it_c)->items_extras_modelo.clear();
		(*it_c)->num_items_extra.clear();
	}

	//Para leer todas las soluciones la 0 es la mejor
	ConvertirSolucionModeloACamiones(Sub, &cplex,&x_ij,&z_j,&xp_ijk,&zp_jk);

	if (Sub->Pruebas) cout << "Terminado RoadefExtra" << endl;
	
	//Cerramos el entorno
	cplex.end();
	model.end(); env.end();
	return 1;
}
//int Roadef2022ExtraGurobi(string dir, parameters param, Subproblema* Sub)
//{
//	int NUM_MAX_EXTRA = 10;
//	//Establecemos el entorno
//	GRBEnv env= GRBEnv();
//	//Creamos el modelo
//	GRBModel model = GRBModel(env);
//
//	//Creamos las variables
//	GRBVar** x_ij = 0;
//	GRBVar* z_j = 0;
//	GRBVar*** xp_ijk = 0;
//	GRBVar** zp_jk = 0;
//	z_j = model.addVars(Sub->lista_camiones.size(), GRB_BINARY);
//	x_ij = new GRBVar * [Sub->lista_items.size()];
//	xp_ijk = new GRBVar * *[Sub->lista_items.size()];
//	zp_jk = new GRBVar * [Sub->lista_camiones.size()];
////	NumVarMatrix2 x_ij(env, Sub->lista_items.size());
////	IloIntVarArray z_j(env, Sub->lista_camiones.size(), 0, 1);
////	NumVarMatrix3 xp_ijk(env, Sub->lista_items.size());
////	NumVarMatrix2 zp_jk(env, Sub->lista_camiones.size());
//	//Si queremos que sean continuas
//	//IloNumVarArray X2(env, num_nodos, 0, 1, ILOFLOAT);
//	//Ponemos los nombres
//	char nombre[20];
//	int num_var = 0;
//	for (int i = 0; i < Sub->lista_items.size(); i++) //Recorrer la lista de tipo items
//	{
//		x_ij[i] = new GRBVar[Sub->lista_camiones.size()];
//		xp_ijk[i] = new GRBVar * [Sub->lista_camiones.size()];
////		x_ij[i] = IloIntVarArray(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
////		xp_ijk[i] = NumVarMatrix2(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
//	}
//	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
//	{
//		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
//		{
//			//Si existe la variable
//			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = model.addVar(0.0, (*it_i)->num_items, 0.0, GRB_INTEGER);
////			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);
//			sprintf(nombre, "X_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
//			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].set(GRB_StringAttr_VarName, nombre);
////			sprintf(nombre, "X_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
////			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].setName(nombre);
//			xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC] = new GRBVar[(*it)->cota_extra];
////			xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
//			for (int k = 0; k < (*it)->cota_extra; k++)
//			{
//				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = model.addVar(0.0, (*it_i)->num_items, 0.0, GRB_INTEGER);
////				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, (*it_i)->num_items);
//				sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
//				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k].set(GRB_StringAttr_VarName, nombre);
////				sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
////				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k].setName(nombre);
//			}
//		}
//	}
//	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
//	{
//		sprintf(nombre, "Z_%d", (*it)->CodIdCC);
//		z_j[(*it)->CodIdCC].set(GRB_StringAttr_VarName, nombre);
//		//TODO Fijar a cero la variable
////		if ((*it)->items_modelo.size() > 0)
////			z_j[(*it)->CodIdCC].set(GRB_IntAttr, 0);
////		z_j[(*it)->CodIdCC].setName(nombre);
//		zp_jk[(*it)->CodIdCC] = new GRBVar[(*it)->cota_extra];
////		zp_jk[(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
//		for (int k = 0; k < (*it)->cota_extra; k++)
//		{
//			zp_jk[(*it)->CodIdCC][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
////			zp_jk[(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
//			sprintf(nombre, "ZP_%d_%d", (*it)->CodIdCC, k);
//			zp_jk[(*it)->CodIdCC][k].set(GRB_StringAttr_VarName, nombre);
////			zp_jk[(*it)->CodIdCC][k].setName(nombre);
//		}
//	}
//
//
//	//Restricciones
//
//
//
//	//Restricción suma de items igual a todos los items 
//	//restricción 1
//
//	for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
//	{
////		IloExpr expr1(env);
//		GRBLinExpr expr1 = 0;
//		for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
//		{
//			expr1 += x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
//			for (int k = 0; k < (*it_c)->cota_extra; k++)
//			{
//				expr1 += xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
//			}
//
//		}
//		model.addConstr(expr1 == (*it)->num_items, "S_" + to_string((*it)->CodIdCC));
////			model.addConstr(expr <= 1, "S_" + to_string(i) + to_string(j));
////		restr.add(IloRange(env, (*it)->num_items, expr1, (*it)->num_items));
////		expr1.end();
//	}
//	//Restricción 2 y 3
//	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
//	{
//		GRBLinExpr expr2 = 0, expr3=0;
////		IloExpr expr2(env);
////		IloExpr expr3(env);
//
//		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
//		{
//			expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->volumen;
//			expr3 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->weight;
//
//		}
//		expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_kp;
//		expr3 += (-1) * z_j[(*it)->CodIdCC] * (*it)->max_loading_weight;
//		model.addConstr(expr2 <= 0, "V1_" + to_string((*it)->CodIdCC));
//		model.addConstr(expr3 <= 0, "V2_" + to_string((*it)->CodIdCC));
////		restr.add(IloRange(env, -IloInfinity, expr2, 0));
////		restr.add(IloRange(env, -IloInfinity, expr3, 0));
////		expr2.end();
////		expr3.end();
//		for (int k = 0; k < (*it)->cota_extra; k++)
//		{
//			GRBLinExpr expr2p = 0, expr3p = 0;
////			IloExpr expr2p(env);
////			IloExpr expr3p(env);
//			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
//			{
//				expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->volumen;
//				expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->weight;
//			}
//			expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_kp;
//			expr3p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->max_loading_weight;
//			model.addConstr(expr2p <= 0, "V1p_" + to_string((*it)->CodIdCC)+"_"+to_string(k));
//			model.addConstr(expr3p <= 0, "V2p_" + to_string((*it)->CodIdCC)+"_" + to_string(k));
////			restr.add(IloRange(env, -IloInfinity, expr2p, 0));
////			restr.add(IloRange(env, -IloInfinity, expr3p, 0));
//
////			expr2p.end();
////			expr3p.end();
//		}
//	}
//
//	//AÑADIMOS LAS RESTRICCIONES AL MODELO
////	model.add(restr);
//
//	//Función objetivo
////	IloExpr obj(env);
//	GRBLinExpr obj = 0;
//	//Parte de camiones
//	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
//	{
//		obj += z_j[(*it)->CodIdCC] * (*it)->cost * param.transportation_cost;
//		for (int k = 0; k < (*it)->cota_extra; k++)
//		{
//			obj += zp_jk[(*it)->CodIdCC][k] * (*it)->cost * (1 + param.extra_truck_cost) * param.transportation_cost;
//		}
//	}
//	//Parte de items
//	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
//	{
//		for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
//		{
//			//Dias de diferencia entre una fecha y otra
//			int days = difftime((*it)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
//			obj += (days)*x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC] * (*it)->inventory_cost * param.inventory_cost;
//
//
//		}
//		for (int k = 0; k < (*it_c)->cota_extra; k++)
//		{
//			for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
//			{
//				int days = difftime((*it)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
//				obj += days * xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k] * (*it)->inventory_cost * param.inventory_cost;
//			}
//		}
//	}
//	//Añadimos la función objetivo al modelo
//	model.setObjective(obj, GRB_MINIMIZE);
////	IloObjective fobj = IloMinimize(env, obj);
////	model.add(fobj);
////	obj.end();
////	IloCplex cplex(model);
////	cplex.exportModel("prueba.lp");
//	model.write("prueba.lp");
//	//Parámetros de optimización
//	model.set(GRB_DoubleParam_TimeLimit, Sub->TiempoModelo); //Tiempo límite
//	model.set(GRB_IntParam_Threads, 4); //Número de hilos
////	cplex.setParam(IloCplex::Threads, 4); //numero de hilos
////	cplex.setParam(IloCplex::TiLim, Sub->TiempoModelo); //tiempo
//	//	cplex.setParam(IloCplex::SolnPoolReplace, 2);
//	//	cplex.setParam(IloCplex::SolnPoolCapacity, 100); //Capacidad de la pool
//		//Optimizamos	
////	IloBool result = cplex.solve();
//	model.optimize();
//	//Si quisieramos generar soluciones
//	// IloBool result = cplex.populate(); 
//	//Estado del algoritmo	al terminar
////	IloCplex::Status estado = cplex.getCplexStatus();
//	int status = model.get(GRB_IntAttr_Status);
////
////	cout << result << " " << estado;
//	//número de soluciones obtenidas
////	int Nsol = cplex.getSolnPoolNsolns();
//	int Nsol = model.get(GRB_IntAttr_SolCount);
//	//VAlor del mip
//	Sub->ObjMip = model.get(GRB_DoubleAttr_ObjVal);
//	//Valor de la cota
//	Sub->ObjBound = model.get(GRB_DoubleAttr_ObjBound);
//	cout << "Best Value " << Sub->ObjMip << " Cota " << Sub->ObjBound;
//	Sub->num_camiones_modelo = 0;
//	Sub->num_camiones_extra_modelo = 0;
//	Sub->max_cam_extra_tipo_modelo = 0;
//	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
//	{
//		(*it_c)->items_modelo.clear();
//		(*it_c)->items_extras_modelo.clear();
//		(*it_c)->num_items_extra.clear();
//	}
//	//Para leer todas las soluciones la 0 es la mejor
//	for (auto i = 0; i < 1; i++)
//	{
//		model.set(GRB_IntParam_SolutionNumber, i);
//		//Extraer los valores de X
//		for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
//		{
//			if (z_j[(*it_c)->CodIdCC].get(GRB_DoubleAttr_Xn) > 0.999)
//				Sub->num_camiones_modelo++;
//			int num_cam_extra_tipo = 0;
//			for (int k = 0; k < (*it_c)->cota_extra; k++)
//			{
//				if (zp_jk[(*it_c)->CodIdCC][k].get(GRB_DoubleAttr_Xn) > 0.999)
//				{
//					vector<item_modelo> aux;
//					for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
//					{
//						if (xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k].get(GRB_DoubleAttr_Xn) > 0.999)
//						{
////							printf("Item Extra %d\n", (*it)->CodIdCC);
//							int order = 0;
//							/*for (int i = 1; i < (*it_c)->ruta_proveedores.size(); i++)
//							{
//								if (strcmp((*it_c)->ruta_proveedores.at(i).c_str(), (*it)->sup_cod_dock) == 0)
//								{
//									order = i;
//									break;
//								}
//							}*/
//							aux.push_back(item_modelo(*it, ceil(xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k].get(GRB_DoubleAttr_Xn)),order));
//						}
//					}
////					printf("Camion %d", (*it_c)->CodIdCC);
//					(*it_c)->items_extras_modelo.push_back(aux);
//					Sub->num_camiones_extra_modelo++;
//					num_cam_extra_tipo++;
//				}
//				if (num_cam_extra_tipo > Sub->max_cam_extra_tipo_modelo)
//					Sub->max_cam_extra_tipo_modelo = num_cam_extra_tipo;
//			}
//			for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
//			{
//				if (x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC].get(GRB_DoubleAttr_Xn) > 0.999)
//				{
////					printf("Item  %d\n", (*it)->CodIdCC);
//					int order = 0;
//					/*for (int i = 1; i < (*it_c)->ruta_proveedores.size(); i++)
//					{
//						if (strcmp((*it_c)->ruta_proveedores.at(i).c_str(), (*it)->sup_cod_dock) == 0)
//						{
//							order = i;
//							break;
//						}
//					}*/
//					(*it_c)->items_modelo.push_back(item_modelo(*it, ceil(x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC].get(GRB_DoubleAttr_Xn)),order));
//				}
//			}
//			//
//			//
//			//				if (cplex.getValue(x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC], i) > 0.999)
//			//				{
//			////					cout << "Posición " << (*it)->CodIdCC << " " << (*it_c)->CodIdCC << " elegida de la solución " << i << "\n";
//			//				}
//			//
//			//
//			//					if (cplex.getValue(xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k], i) > 0.999)
//			//					{
//			////						cout << "Posición " << (*it)->CodIdCC << " " << (*it_c)->CodIdCC << " " << k << " elegida de la solución " << i << "\n";
//			//					}
//			//				}
//			//			}
//		}
//		cout << "camiones " << Sub->num_camiones_modelo << " Camiones extra " << Sub->num_camiones_extra_modelo << " Max Cam Extra Tipo " << Sub->max_cam_extra_tipo_modelo << endl;
//
//		string nombreArchivo1 = "Resultados_gurobi_items.txt";
//
//		ofstream archivo;
//		// Abrimos el archivo
//		archivo.open(nombreArchivo1.c_str(), ofstream::out | ofstream::app);
//		archivo << dir << ";" << Sub->lista_camiones.size() << ";" << Nsol << ";" << setprecision(8) << Sub->ObjMip << ";" << Sub->ObjBound << ";" << Sub->num_camiones_modelo << ";" << Sub->num_camiones_extra_modelo << ";" << Sub->max_cam_extra_tipo_modelo << endl;
//		archivo.close();
//	}
//
//	//Cerramos el entorno
////	cplex.end();
////	model.end(); env.end();
//	return 1;
//}