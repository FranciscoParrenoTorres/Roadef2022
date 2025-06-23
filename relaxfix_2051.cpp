#include "encabezados.h"



void relaxandfix(string dir, parameters param, Subproblema* Sub)
{
	bool primera = true;
	bool quedan_x_colocar = true;
	//Calculo la ventana temporal
	list<truck*>::iterator iti, itf, itaux;
	iti = Sub->lista_camiones.begin();
	itf = Sub->lista_camiones.end();
	itf--;

	int total_dias = Days_0001((*iti)->arrival_time_tm_ptr.tm_year, (*iti)->arrival_time_tm_ptr.tm_mon + 1, (*iti)->arrival_time_tm_ptr.tm_mday)
		- Days_0001((*itf)->arrival_time_tm_ptr.tm_year, (*itf)->arrival_time_tm_ptr.tm_mon + 1, (*itf)->arrival_time_tm_ptr.tm_mday);

	int ventana_temporal = max(1, (int)ceil((double)total_dias / (10)));
	ventana_temporal += Sub->Iter_Resuelve_Subproblema;
	if (Sub->Pruebas)
	{
		cout << "Ventana temporal: " << ventana_temporal << " Total dias " << total_dias << endl;
		cout << "Iteracion: " << Sub->Iter_Resuelve_Subproblema << endl;
	}


	////Si la componente es muy grande, la ventana temporal es 1
	//if (Sub->total_items > 1500)
	//{
	//	ventana_temporal = 1;
	//}
	itaux = iti;
	long long int ini = (*itaux)->arrival_time;
	long long int fin;

	int inc = ventana_temporal;
	//Mientras la diferencia de dias sea menos a inc 
	for (itf = itaux; itf != Sub->lista_camiones.end(); itf++)
	{
		int dif_day = Days_0001((*iti)->arrival_time_tm_ptr.tm_year, (*iti)->arrival_time_tm_ptr.tm_mon + 1, (*iti)->arrival_time_tm_ptr.tm_mday)
			- Days_0001((*itf)->arrival_time_tm_ptr.tm_year, (*itf)->arrival_time_tm_ptr.tm_mon + 1, (*itf)->arrival_time_tm_ptr.tm_mday);
		if (dif_day > ventana_temporal)
			break;
		//TODO los días no pueden ser así
	}
	if (itf == Sub->lista_camiones.end())
	{
		itf--;
	}


	//Creo las variables del modelo
	//Establecemos el entorno
	IloEnv env;
	//Creamos el modelo
	IloModel model = IloModel(env);

	NumVarMatrix2F x_ij(env, Sub->lista_items.size());
	IloIntVarArray z_j(env, Sub->lista_camiones.size(), 0, 1);
	NumVarMatrix3F xp_ijk(env, Sub->lista_items.size());
	NumVarMatrix2 zp_jk(env, Sub->lista_camiones.size());

	//Creo las restricciones y función objetivo
	restricciones_RyF(Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, (*itf)->arrival_time);

	IloCplex cplex(model);

	int veces = 0;
	bool final = false;
	list<truck*> lista_cam_ventana;

	while (itaux != Sub->lista_camiones.end() && !final)
	{
		long long int ini = (*itaux)->arrival_time;
		long long int fin;
		itf = itaux;
		int inc = ventana_temporal;
		lista_cam_ventana.clear();
		//lista_cam_ventana.push_back((*itaux));

		//Mientras la diferencia de dias sea menos a inc 
		for (itf = itaux; itf != Sub->lista_camiones.end(); itf++)
		{
			int dif_day = Days_0001((*itaux)->arrival_time_tm_ptr.tm_year, (*itaux)->arrival_time_tm_ptr.tm_mon + 1, (*itaux)->arrival_time_tm_ptr.tm_mday)
				- Days_0001((*itf)->arrival_time_tm_ptr.tm_year, (*itf)->arrival_time_tm_ptr.tm_mon + 1, (*itf)->arrival_time_tm_ptr.tm_mday);
			if (dif_day >= ventana_temporal)
				break;

		}

		if (itf == Sub->lista_camiones.end())
			itf--;
		if (itaux == itf && quedan_x_colocar == false)
			final = true;


		if (!final)
		{

			//Poner las binarias desde fecha ini hasta fecha fin
			if (primera)
			{
				ini = ini + 10000;
				primera = false;
			}
			Poner_binarias_RyF(cplex, Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, ini, (*itf)->arrival_time);

			//Resuelvo el modelo		
			resolver_modelo_RyF(cplex, Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, (*itf)->arrival_time, lista_cam_ventana);
			if (lista_cam_ventana.size() == 0)
			{
				//Si el modelo no coloca nada y estoy en el último camion finalizo.
				if ((*itaux)->CodIdCC == (*Sub->lista_camiones.back()).CodIdCC)
				{
					quedan_x_colocar = false;
				}
				//avanzo en los dias
				itaux = itf;
			}
			else
			{
				//if (Sub->Pruebas)cout << "Voy al packing" << endl;
				//Packing de los camiones
				PackingRF(dir, param, Sub, (*itf)->arrival_time, lista_cam_ventana);
				//avanzo en los dias
				itaux = itf;

				//fijo las variables empaquetadas
				fijar_variables_RyF(cplex, lista_cam_ventana, param, model, env, x_ij, z_j, xp_ijk, zp_jk, Sub, quedan_x_colocar);

				veces++;
			}
		}
		else
		{
			final = true;
		}
	}
	//Cerramos el entorno
	cplex.end();
	model.end(); env.end();

	//ha quedado algo por poner
	if (Sub->Pruebas)
	{
		for (auto iti = Sub->lista_items.begin(); iti != Sub->lista_items.end(); iti++)
		{
			if ((*iti)->num_items > 0)
			{
				cout << "X_" << (*iti)->CodIdCC << "=" << (*iti)->num_items << endl;
				for (auto itc = (*iti)->camiones.begin(); itc != (*iti)->camiones.end(); itc++)
				{
					cout << "    c: Z_" << (*itc)->CodIdCC << endl;
				}

				int kk = 0;
			}
		}
	}
}
void Poner_binarias_RyF(IloCplex& cplex, Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloIntVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2& zp_jk, long long int ini, long long int ffin)
{
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{

		if ((*it)->arrival_time < ffin)
		{
			break;
		}
		if ((*it)->arrival_time < ini && (*it)->arrival_time >= ffin)
		{
			//if ((*it)->no_puede_colocar_nada == true) continue;
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{


				model.add(IloConversion(env, x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC], ILOINT));
				/*}*/
				for (int k = 0; k < (*it)->cota_extra; k++)
				{

					//todas a reales
					//cout << "XP_" << (*it_i)->CodIdCC << "_" << (*it)->CodIdCC << endl;
					model.add(IloConversion(env, xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k], ILOINT));

				}
			}
		}
	}
}
void fijar_variables_RyF(IloCplex& cplex, list<truck*> lista, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloIntVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2& zp_jk, Subproblema* Sub, bool& quedan)
{
	//Para inicializar una solución
	IloNumVarArray vars(env);
	IloNumArray vals(env);
	bool es_ultimo = false;


	if (Sub->Pruebas) cout << "Fijado:" << endl;

	for (auto ic = lista.begin(); ic != lista.end(); ic++)
	{

		if ((*ic)->CodIdCC == (*Sub->lista_camiones.back()).CodIdCC)
		{
			es_ultimo = true;
			if (Sub->Pruebas) cout << "Es el ultimo" << endl;
		}
		if ((*ic)->id_truck[0] == 'P')
		{
			if (Sub->Pruebas) cout << "Es plani" << endl;
			if ((*ic)->items_sol.size() > 0)
			{
				if (Sub->Pruebas)cout << (*ic)->CodIdCC << endl;
				z_j[(*ic)->CodIdCC].setBounds(1, 1);
				//model.add(z_j[(*ic)->CodIdCC] == 1);
				if (Sub->Pruebas) cout << "Z_" << (*ic)->CodIdCC << endl;
				(*ic)->fijado = true;
				//(*ic)->lleno = true;
				for (auto it = (*ic)->items_sol.begin(); it != (*ic)->items_sol.end(); it++)
				{
					string numeroStr = to_string((*it).id_item->CodIdCC);
					string variable = "X_" + numeroStr;
					if (Sub->Pruebas)cout << "X_" << (*it).id_item->CodIdCC << "_" << (*ic)->CodIdCC << "=" << (*it).colocado << endl;
					x_ij[(*it).id_item->CodIdCC][(*ic)->CodIdCC].setBounds((*it).colocado, (*it).colocado);
					//model.add(x_ij[(*it).id_item->CodIdCC][(*ic)->CodIdCC] == (*it).colocado);
					if ((*ic)->items_colocados.count(variable) == 0)
						(*ic)->items_colocados.insert(pair<string, int>(variable, (*it).colocado));
					else
						(*ic)->items_colocados[variable] = (*it).colocado;
				}
				//si esta lleno poner el resto de variables a 0

				if ((*ic)->lleno)
				{
					for (auto it = (*ic)->items.begin(); it != (*ic)->items.end(); it++)
					{
						string numeroStr = to_string((*it)->CodIdCC);
						string variable = "X_" + numeroStr;
						if ((*ic)->items_colocados[variable] > 0)
							continue;
						else
						{
							if (Sub->Pruebas)cout << variable << "_" << (*ic)->CodIdCC << "=0" << endl;
							x_ij[(*it)->CodIdCC][(*ic)->CodIdCC].setBounds(0, 0);
							//model.add(x_ij[(*it)->CodIdCC][(*ic)->CodIdCC] == 0);
						}
					}
				}
			}
		}
		else
		{

			if (Sub->Pruebas) cout << "Es extra" << endl;
			int kextra = (*ic)->kextra;
			if (Sub->Pruebas) cout << "Es extra" << kextra << " cota:" << (*ic)->cota_extra << endl;
			if ((*ic)->items_sol.size() > 0)
			{
				if (Sub->Pruebas) cout << "Tiene items_sol" << kextra << endl;
				if (Sub->Pruebas) cout << "ZP_" << (*ic)->CodIdCC << "_" << kextra << ": " << (*ic)->id_truck << endl;

				zp_jk[(*ic)->CodIdCC][kextra].setBounds(1, 1);
				//model.add(zp_jk[(*ic)->CodIdCC][kextra] == 1);
				if (Sub->Pruebas)cout << "ZP_" << (*ic)->CodIdCC << "_" << kextra << endl;
				(*ic)->fijado = true;
				//(*ic)->lleno = true;
				for (auto it = (*ic)->items_sol.begin(); it != (*ic)->items_sol.end(); it++)
				{
					string numeroStr = to_string((*it).id_item->CodIdCC);
					string variable = "XP_" + numeroStr;
					if (Sub->Pruebas)cout << "XP_" << (*it).id_item->CodIdCC << "_" << (*ic)->CodIdCC << "_" << kextra << " = " << (*it).colocado << endl;
					xp_ijk[(*it).id_item->CodIdCC][(*ic)->CodIdCC][kextra].setBounds((*it).colocado, (*it).colocado);
					//model.add(xp_ijk[(*it).id_item->CodIdCC][(*ic)->CodIdCC][kextra] == (*it).colocado);	
					if ((*ic)->items_colocados.count(variable) == 0)
						(*ic)->items_colocados.insert(pair<string, int>(variable, (*it).colocado));
					else
						(*ic)->items_colocados[variable] = (*it).colocado;

				}
				//si esta lleno poner el resto de variables a 0
				if ((*ic)->lleno)
				{
					for (auto it = (*ic)->items.begin(); it != (*ic)->items.end(); it++)
					{
						string numeroStr = to_string((*it)->CodIdCC);
						string variable = "XP_" + numeroStr;
						if ((*ic)->items_colocados[variable] > 0)
							continue;
						else
						{
							if (Sub->Pruebas)cout << variable << "_" << (*ic)->CodIdCC << "=0" << endl;
							xp_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][kextra].setBounds(0, 0);
							//model.add(xp_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][kextra] == 0);
						}
					}
				}
			}


		}
		if (es_ultimo)
		{
			//Compruebo que están todos colocados
			quedan = false;
			for (auto iti = Sub->lista_items.begin(); iti != Sub->lista_items.end(); iti++)
			{
				if ((*iti)->num_items > 0)
				{
					quedan = true;
					cout << "quedan" << endl;
					break;
				}
			}
			if (quedan == false) cout << "No quedan" << endl;
		}
		else quedan = true;
	}
	//cplex.addMIPStart(vars, vals, cplex.MIPStartRepair);
}
void resolver_modelo_RyF(IloCplex& cplex, Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloIntVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2& zp_jk, long long int ffin, list<truck*>& listacam)
{

	//if(Sub->Pruebas)cplex.exportModel("pruebaryf.lp");
	// //TODO Quitar esto
//	if (!Sub->Pruebas)
//		cplex.setOut(env.getNullStream());

	//Parámetros de optimización
	cplex.setParam(IloCplex::Threads, 8); //numero de hilos
	cplex.setParam(IloCplex::MIPEmphasis, 1);//Feasibility

	if (Sub->cplex_trozo == false)
		cplex.setParam(IloCplex::TiLim, Sub->TiempoModelo); //tiempo
	else
		cplex.setParam(IloCplex::TiLim, max((double)Sub->TiempoModelo / (double)5, (double)1)); //tiempo

	//Optimizamos	
//Para inicializar una solución
	IloNumVarArray vars(env);
	IloNumArray vals(env);
	if (Sub->Iter_Resuelve_Subproblema > 0)
	{
		for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
		{
			for (auto ic = (*it)->camiones.begin(); ic != (*it)->camiones.begin(); ic++)
			{

				if (Sub->X_ij[(*it)->CodIdCC][(*ic)->CodIdCC] > 0)
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
			if (Sub->Z_j[(*ic)->CodIdCC] == true)
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
	int gap_max = max((1000 * MAX_GAP) - (4 * Sub->Iter_Resuelve_Subproblema), (double)4);

	int alea = get_random((1 * (gap_max - 1)) / 3, (gap_max - 1));
	double gap_inicial = (double)gap_max / (double)1000;
	cplex.setParam(IloCplex::EpGap, gap_inicial);

	IloBool result = cplex.solve();
	double GAP = (double)alea / double(1000);
	if (result)
		gap = cplex.getMIPRelativeGap();
	int cont = 1;
	double sol = 0;
	double sol_ant = 0;
	double dif = 100;
	tiempo = max(tiempo / (double)2, (double)3);
	while (gap > GAP && cont < MAX_VECES_MODELO && dif > MIN_DIF_SOL_MODELO)
	{
		cplex.setParam(IloCplex::TiLim, tiempo);
		sol_ant = sol;
		cplex.setParam(IloCplex::MIPEmphasis, 0);//Feasibility
		if (Sub->Pruebas)
			printf("\nGap MAyor de %.3f %d\n", GAP, cont);
		cplex.setParam(IloCplex::EpGap, GAP - 0.001);
		result = cplex.solve();
		if (result)
		{
			gap = cplex.getMIPRelativeGap();
			tiempo = max(tiempo / (double)2, (double)3);
			sol = cplex.getObjValue();

			if (sol_ant > 1)
				dif = (sol_ant - sol) / sol_ant;
		}
		else
		{
			gap = 2 * GAP;
			tiempo = 2 * tiempo;

		}
		cont++;
	}

	//Si quisieramos generar soluciones
	// IloBool result = cplex.populate(); 
	//Estado del algoritmo	al terminar
	IloCplex::Status estado = cplex.getCplexStatus();

	if (Sub->Pruebas) cout << result << " " << estado << endl;
	if (result == 0)
	{
		if (Sub->Pruebas)
		{
			for (int i = 0; i < 100; i++)
				cout << "NO consigue solucion";
		}
		string kk = param.param_file + "_Error.lp";
		cplex.exportModel(kk.c_str());
	}
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
	if (Sub->Iteracion_Modelo == 0 && Sub->Iter_Resuelve_Subproblema>=3)
		Sub->ObjBound = cplex.getBestObjValue();
	Sub->Iteracion_Modelo++;

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
	ConvertirSolucionModeloACamiones2(Sub, &cplex, &x_ij, &z_j, &xp_ijk, &zp_jk, ffin, listacam);

	////Quitar de la lista los camiones que no tinenen items asignados
	//listacam.remove_if([](truck* t) 
	//{
	//	bool emp = false;
	//	if (t->lleno)
	//	{
	//		for (auto it = t->pcamiones_extra.begin(); it != t->pcamiones_extra.end(); it++)
	//		{
	//			if (!(*it)->lleno)
	//			{
	//				if ((*it)->items_modelo.size() == 0)
	//					emp = true;
	//				else emp = false;
	//				break;
	//			}
	//		}
	//	}
	//	else emp= t->items_modelo.empty(); 
	//	return emp;
	//});

	if (Sub->Pruebas) cout << "Terminado RoadefExtra" << endl;


}
void restricciones_RyF(Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloIntVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2& zp_jk, long long int ffin)
{
	//Porcentge de volumen y peso.
	//para la aleatorización
	double por_p = (double)Sub->porcentage_peso / 100;
	double por_v = (double)Sub->porcentage_vol / 100;
	double por_vheur = (double)1 + (double)(1 - por_v);


	Sub->PonerAlgunosItemsABinarios();
	Sub->CrearMatrizPosibles();

	char nombre[20];

	for (auto it_i = Sub->lista_items.begin(); it_i != Sub->lista_items.end(); it_i++) //Recorrer la lista de tipo items
	{

		/*if ((*it_i)->num_items <= 0)
			continue;*/

		x_ij[(*it_i)->CodIdCC] = IloNumVarArray(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
		xp_ijk[(*it_i)->CodIdCC] = NumVarMatrix2F(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones

	}

	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		//if ((*it)->no_puede_colocar_nada == true) continue;
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{

			/*if (Sub->cuantos_item[(*it_i)->CodIdCC] > MAX_CAMIONES_POR_ITEM)
				continue;

			if ((*it_i)->num_items <= 0)continue;
			if (!(*it)->lleno)
			{*/

			/*if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == true)*/
			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloNumVar(env, 0, (*it_i)->num_items);
			/*else
				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloNumVar(env, 0, 1);*/
				//				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);

			sprintf(nombre, "X_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].setName(nombre);
			//printf("Variable Definida %d %d \n", (*it_i)->CodIdCC, (*it)->CodIdCC);
			//Pongo todas a reales
			//IloConversion(env, x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC], ILOFLOAT);
			model.add(IloConversion(env, x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC], ILOFLOAT));


			/*}*/
			xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloNumVarArray(env, (*it)->cota_extra);
			for (int k = 0; k < (*it)->cota_extra; k++)
			{
				/*if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == true)*/
				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloNumVar(env, 0, (*it_i)->num_items);
				/*else
					xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloNumVar(env, 0, 1);*/
				sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k].setName(nombre);
				//printf("Variable Definida %d %d %d\n", (*it_i)->CodIdCC, (*it)->CodIdCC, k);

				//todas a reales
				model.add(IloConversion(env, xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k], ILOFLOAT));

			}
		}
	}


	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		/*if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{*/
		sprintf(nombre, "Z_%d", (*it)->CodIdCC);
		z_j[(*it)->CodIdCC].setName(nombre);
		/*}*/
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
				/*double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
				double dif_vol = volumen_ampliado - (*it_i)->volumen;
				if ((*it)->volumen_heuristico < 1)
					expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * volumen_ampliado * cuantos;
				else*/
				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->vol_con_nest * cuantos;
				expr3 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->weight * cuantos;
				hay_items = true;

			}

			if (hay_items)
			{


				//				expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen * por_vheur;
				// 				   				(*it)->vol_maximo = (*it)->volumen;
								//CambiadoFran
				expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_heuristico * por_vheur;
				(*it)->vol_maximo = ((*it)->volumen_heuristico * por_vheur) + 1;
				//				expr3 += (-1) * z_j[(*it)->CodIdCC] * (*it)->peso_inicial * por_p;
				//				(*it)->max_loading_weight = (*it)->peso_inicial * por_p;
								//CambiadoFran
				expr3 += (-1) * z_j[(*it)->CodIdCC] * (*it)->peso_heuristico * por_p;
				(*it)->max_loading_weight = ((*it)->peso_heuristico * por_p) + 1;


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
				/*double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
				if ((*it)->volumen_heuristico < 1)
					expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * volumen_ampliado * por_vheur * cuantos;
				else*/
				expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->vol_con_nest * cuantos;
				expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->weight * cuantos;
				hay_items = true;
			}

			if (hay_items)
			{
				//		expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_kp * por_v;
				/*if ((*it)->volumen_heuristico < 1)
				{*/
				expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen;
				(*it)->vol_maximo = (*it)->volumen;
				/*}
				else
				{
					expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_heuristico ;
					(*it)->vol_maximo = (*it)->volumen_heuristico ;
				}*/
				expr3p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->max_loading_weight;
				//(*it)->max_loading_weight = (*it)->peso_inicial ;
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

	//ORDEN DE LOS EXTRAS, PARA QUE COJA PRIMERO EL 0 LUEGO EL 1...
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			IloExpr expr2p(env);
			expr2p += z_j[(*it)->CodIdCC] - zp_jk[(*it)->CodIdCC][k];
			restr.add(IloRange(env, 0, expr2p, IloInfinity));
			expr2p.end();
		}
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			/*IloExpr expr2p(env);
			expr2p += zp_jk[(*it)->CodIdCC][k] - zp_jk[(*it)->CodIdCC][k+1];
			restr.add(IloRange(env, 0, expr2p, IloInfinity));
			expr2p.end();*/
			for (int kk = 0; kk < k; kk++)
			{
				IloExpr expr2p(env);
				expr2p += zp_jk[(*it)->CodIdCC][kk] - zp_jk[(*it)->CodIdCC][k];
				restr.add(IloRange(env, 0, expr2p, IloInfinity));
				expr2p.end();
			}

		}
	}

	//Si eleije un cmaión que al menos vaya un item

	/*for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		if ((*it)->no_puede_colocar_nada == true) continue;
		IloExpr expr2p(env);
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false)
				continue;
			if ((*it_i)->num_items <= 0) continue;

			expr2p += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC];
		}
		expr2p += (-1) * z_j[(*it)->CodIdCC];
		restr.add(IloRange(env, 0, expr2p, IloInfinity));
		expr2p.end();
		for (int k = 0; k < (*it)->cota_extra; k++)
		{

			IloExpr expr3p(env);
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false)
					continue;
				if ((*it_i)->num_items <= 0) continue;

				expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k];
			}
			expr3p += (-1) * zp_jk[(*it)->CodIdCC][k];
			restr.add(IloRange(env, 0, expr3p, IloInfinity));
			expr3p.end();
		}
	}*/
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
			int cuantos = Sub->mapa_camiones[(*it_c)->id_truck].size();
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
					time_t temp_i2 = (*it_i)->latest_arrival_tm;
					time_t temp_c2 = (*it_c)->arrival_time_tm;
					long long int temp_i3 = (*it_i)->latest_arrival;
					long long int temp_c3 = (*it_c)->arrival_time;
					if ((temp_i.tm_min > temp_c.tm_min))
					{
						int kk = 9;
					}
					int kk = 9;
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
						*/
					if (days > 0)
						obj += cuantos * days * xp_ijk[(*it_i)->CodIdCC][(*it_c)->CodIdCC][k] * (*it_i)->inventory_cost * param.inventory_cost;
				}
			}
		}
	}
	//Añadimos la función objetivo al modelo

	IloObjective fobj = IloMinimize(env, obj);
	model.add(fobj);
	obj.end();

}
bool PackingRF(string dir, parameters param, Subproblema* Sub, long long int fin, list<truck*> lista_camiones_ventana)
{
	bool hay_recarga = false;
	int cuantos_recargar = 0;
	bool alguno_queda = false;
	int cuantos_quedan = 0;
	int num_camiones_a_cargar = (int)ceil(param.porcentaje * Sub->num_camiones_modelo);
	truck* sig_cam = NULL;
	truck* plani = NULL;
	bool segunda_vuelta = false;

	//		if (Sub->Pruebas)cout << "entro en el packing" << endl;
	auto itc = lista_camiones_ventana.begin();
	while (itc != lista_camiones_ventana.end())
	{
		if (strcmp((*itc)->id_truck, "Q315392701") == 0)
		{
			int kk = 0;
		}
		if (Sub->Pruebas)
		{
			if ((*itc)->es_extra)
				cout << "ZP_" << (*itc)->CodIdCC << "_" << (*itc)->kextra << ":" << (*itc)->lleno << ";" << (*itc)->items_modelo.size() << ";" << (*itc)->num_items_modelo << ";" << segunda_vuelta << ";" << (*itc)->recargar << endl;
			else
				cout << "Z_" << (*itc)->CodIdCC << ":" << (*itc)->lleno << ";" << (*itc)->items_modelo.size() << ";" << (*itc)->num_items_modelo << ";" << segunda_vuelta << ";" << (*itc)->recargar << endl;
		}
		(*itc)->Total_Iter_Packing = Sub->Total_Iter_Packing;

		if (!(*itc)->lleno && (*itc)->items_modelo.size() != 0 && (*itc)->num_items_modelo > 0 || (segunda_vuelta && (*itc)->recargar))
		{
			if ((*itc)->CodIdCC == 34)
			{
				int kk = 0;
			}
			//Busco el siguiente camión
			buscar_sig_cam(lista_camiones_ventana, itc, sig_cam);
			//if (Sub->Pruebas)cout << "Siguiente encontrado"  << endl;

			//Creo el camion y lo añado a la solución
			if (Sub->camiones_solu.count((*itc)->id_truck) == 0)
			{
				truck_sol camion((*itc)->id_truck);
				Sub->sol.listado_camiones.push_back(camion);

				Sub->coste_transporte += (*itc)->cost;
				Sub->camiones_solu.insert(pair<string, int>((*itc)->id_truck, Sub->sol.listado_camiones.size() - 1));
			}
			else
			{
				//Buscar el camion, eliminarlo de la solución y volver a añadirlo
				for (auto auxt = Sub->sol.listado_camiones.begin(); auxt != Sub->sol.listado_camiones.end(); auxt++)
				{
					if (strcmp((*itc)->id_truck, (*auxt).id_truc.c_str()) == 0)
					{
						Sub->sol.listado_camiones.erase(auxt);
						break;
					}
				}
				truck_sol camion((*itc)->id_truck);
				Sub->sol.listado_camiones.push_back(camion);
				Sub->camiones_solu.at((*itc)->id_truck) = Sub->sol.listado_camiones.size() - 1;
			}


			//num_camiones_modelo++;



			//Veo si es pesado o voluminoso
			double peso = ((*itc)->max_loading_weight / (*itc)->peso_total_items_modelo);
			double vol = ((*itc)->volumen / (*itc)->volumen_total_items_modelo);
			if (peso < vol && peso < 2)
			{
				(*itc)->Modelo_pesado = true;
				(*itc)->peso_total_items_modelo_asignados = (*itc)->peso_total_items_modelo;
				(*itc)->volumen_total_items_modelo_asignados = (*itc)->volumen_total_items_modelo;
			}
			else
				(*itc)->Modelo_pesado = false;
			(*itc)->peso_a_cargar = (*itc)->peso_total_items_modelo;
			(*itc)->volumen_a_cargar = (*itc)->volumen_total_items_modelo;
			//int antes = (*itc)->total_items_modelos;
			int antes = (*itc)->num_items_modelo;


			Sub->CargoCamion((*itc), (*itc)->items_modelo, (*itc)->id_truck, (*itc)->num_items_modelo, 0);
			if (Sub->Pruebas)
			{
				if ((*itc)->es_extra)
				{
					cout << "ZP_" << (*itc)->CodIdCC << "_" << (*itc)->kextra << " llenado:" << (*itc)->lleno << "queda:" << (*itc)->num_items_modelo << endl;
					cout << "Volumen_cam:" << (*itc)->volumen << "Vol_cargado:" << (*itc)->volumen_ocupado << endl;
				}
				else
				{
					cout << "Z_" << (*itc)->CodIdCC << " llenado:" << (*itc)->lleno << "queda:" << (*itc)->num_items_modelo << endl;
					cout << "Volumen_cam:" << (*itc)->volumen << "Vol_cargado:" << (*itc)->volumen_ocupado << endl;
				}
			}

			if ((*itc)->recargar)
			{
				(*itc)->recargar = false;
				cuantos_recargar--;
			}
			num_camiones_a_cargar--;
			if ((*itc)->items_sol.empty())
			{
				for (auto it = (*itc)->items_modelo.begin(); it != (*itc)->items_modelo.end(); it++)
					(*itc)->items_sol.push_back((*it));
			}
			////Imprime le camion P08035001
			//if (Sub->Pruebas && strcmp((*itc)->id_truck, "P080350001") == 0)
			//{
			//	cout << "P8035001" << endl;
			//	cout << "itmes sol" << endl;
			//	for (auto its = (*itc)->items_sol.begin(); its != (*itc)->items_sol.end(); its++)
			//	{
			//		cout << "X_"<<(*its).id_item->CodIdCC << " " << (*its).colocado << endl;
			//	}
			//	cout<<"pilas sol"<<endl;
			//	for (auto itp = (*itc)->pilas_solucion.begin(); itp != (*itc)->pilas_solucion.end(); itp++)
			//	{
			//		for (auto itii = (*itp).items.begin(); itii != (*itp).items.end(); itii++)
			//		{
			//			cout << "X_" << (*itii)->id_item<< " 1"<< endl;
			//		}
			//	}
			//}
			//if (Sub->Pruebas && strcmp((*itc)->id_truck, "P093350001") == 0)
			//{
			//	cout << "P8035001" << endl;
			//	cout << "itmes sol" << endl;
			//	for (auto its = (*itc)->items_sol.begin(); its != (*itc)->items_sol.end(); its++)
			//	{
			//		cout << "X_" << (*its).id_item->CodIdCC << " " << (*its).colocado << endl;
			//	}
			//	cout << "pilas sol" << endl;
			//	for (auto itp = (*itc)->pilas_solucion.begin(); itp != (*itc)->pilas_solucion.end(); itp++)
			//	{
			//		for (auto itii = (*itp).items.begin(); itii != (*itp).items.end(); itii++)
			//		{
			//			cout << "X_" << (*itii)->id_item << " 1" << endl;
			//		}
			//	}
			//}
			if ((*itc)->num_items_modelo > 0 && sig_cam != NULL)
			{
				if (strcmp((*itc)->id_truck, sig_cam->id_truck) != 0)
					colocar_en_sig_cam(Sub, itc, sig_cam, hay_recarga, cuantos_recargar, false, (*itc));
			}


			if (Sub->Pruebas)
			{
				if ((*itc)->peso_cargado > (*itc)->max_loading_weight)
				{
					cout << "El camion:" << (*itc)->id_truck << " tiene cargado " << (*itc)->peso_cargado << " y el máximo es " << (*itc)->max_loading_weight << " la diferencia: " << (*itc)->peso_cargado - (*itc)->max_loading_weight << endl;
					cout << "El camion:" << (*itc)->id_truck << " tiene vol cargado " << (*itc)->volumen_ocupado << " y el vol máximo es " << (*itc)->volumen_kp << "la diferencia: " << (*itc)->volumen_ocupado - (*itc)->volumen_kp << endl;

					PintarProblema(211);
				}
				if ((*itc)->vol_antes > (*itc)->volumen_ocupado)
					cout << "Vol:" << (*itc)->id_truck << ":" << (*itc)->vol_antes << "-" << (*itc)->volumen_ocupado << endl;
				if ((*itc)->peso_antes > (*itc)->peso_cargado)
					cout << "Peso:" << (*itc)->id_truck << ":" << (*itc)->peso_antes << "-" << (*itc)->peso_cargado << endl;
			}
			if ((*itc)->num_items_modelo < 0)
				PintarProblema(111);
			if (antes == (*itc)->num_items_modelo)
				PintarProblema(6);

		}

		itc++;
		if (itc == lista_camiones_ventana.end() && cuantos_recargar > 0)
		{
			cout << "SEGUNDA VUELTA" << endl;
			itc = lista_camiones_ventana.begin();
			segunda_vuelta = true;
		}
	}
	return true;
}
void buscar_sig_cam_del_extra(list<truck*> lista_camiones_ventana, list<truck*>::iterator itc, list<truck*>::iterator itex, truck*& sig_cam, truck*& plani)
{
	list<truck* >::iterator sigc;
	plani = NULL;
	bool enc = false;
	auto itexcam = itex;
	itexcam++;
	for (; itexcam != (*itc)->pcamiones_extra.end() && !enc; itexcam++)
	{
		if ((*itexcam)->lleno)
			continue;
		sig_cam = (*itexcam);
		enc = true;
		plani = (*itc);
	}
	if (!enc)
	{
		//Lo busco entre sus planificados
		for (auto itexcam = (*itc)->pcamiones_extra.begin(); itexcam != itex && !enc; itexcam++)
		{
			if ((*itexcam)->lleno)
				continue;
			sig_cam = (*itexcam);
			enc = true;
			plani = (*itc);
		}
		if (!enc)
		{
			if (lista_camiones_ventana.size() == 1) sig_cam = NULL;
			else
			{
				//Si es el último camion de la lista, buscar desde el principio.
				if ((*itc)->CodId == (*lista_camiones_ventana.back()).CodId)
				{
					//buscar desde el principio de la lista uno que no esté lleno
					list<truck* >::iterator ini = lista_camiones_ventana.begin();

					bool enc = false;
					for (; ini != itc && !enc; ini++)
					{
						if (!(*ini)->lleno)
						{
							sig_cam = (*ini);
							enc = true;
						}
						else
						{
							//si tiene extras busco entre los extras
							for (auto exini = (*ini)->pcamiones_extra.begin(); exini != (*ini)->pcamiones_extra.end() && !enc; exini++)
							{
								if (!(*exini)->lleno)
								{
									sig_cam = (*exini);
									enc = true;
									plani = (*ini);
								}
							}
						}
					}
					if (!enc) sig_cam = NULL;
				}
				else
				{
					bool enc = false;
					//busco itc en la lista
					list<truck* >::iterator ini = lista_camiones_ventana.begin();
					for (; ini != lista_camiones_ventana.end(); ini++)
					{
						if ((*ini)->CodIdCC == (*itc)->CodIdCC)
							break;
					}
					ini++;
					for (; ini != lista_camiones_ventana.end() && !enc; ini++)
					{
						if (!(*ini)->lleno)
						{
							sig_cam = (*ini);
							enc = true;
						}
						else
						{
							//si tiene extras busco entre los extras
							for (auto exini = (*ini)->pcamiones_extra.begin(); exini != (*ini)->pcamiones_extra.end() && !enc; exini++)
							{
								if (!(*exini)->lleno)
								{
									sig_cam = (*exini);
									enc = true;
									plani = (*ini);
								}
							}
						}
					}
					//sino lo buscamos desde el iniico hasta itc
					if (!enc)
					{
						for (auto ini = lista_camiones_ventana.begin(); ini != itc && !enc; ini++)
						{
							if (!(*ini)->lleno)
							{
								sig_cam = (*ini);
								enc = true;
							}
							else
							{
								//si tiene extras busco entre los extras
								for (auto exini = (*ini)->pcamiones_extra.begin(); exini != (*ini)->pcamiones_extra.end() && !enc; exini++)
								{
									if (!(*exini)->lleno)
									{
										sig_cam = (*exini);
										enc = true;
										plani = (*ini);
									}
								}
							}
						}
					}
					if (!enc) sig_cam = NULL;
				}
			}
		}
	}
	if (sig_cam != NULL)
	{
		if (strcmp(sig_cam->id_truck, (*itex)->id_truck) == 0)
		{
			sig_cam = NULL;
		}
	}
}
void buscar_sig_cam(list<truck*> lista_camiones_ventana, list<truck*>::iterator itc, truck*& sig_cam)
{
	list<truck* >::iterator sigc;
	bool hay_sig = false;
	//si tiene extras no llenos será uno de ellos
	bool enc = false;



	/*if ((*itc)->pcamiones_extra.size() > 0)
	{
		for (auto exini = (*itc)->pcamiones_extra.begin(); exini != (*itc)->pcamiones_extra.end() && !enc; exini++)
		{
			if (!(*exini)->lleno)
			{
				sig_cam = (*exini);
				enc = true;
				hay_sig = true;
			}
		}

	}*/
	////sino será el siguiente en la lista
	//if(hay_sig==false)
	//{
	if (lista_camiones_ventana.size() == 1) sig_cam = NULL;
	else
	{
		auto ini = lista_camiones_ventana.begin();
		for (; ini != lista_camiones_ventana.end(); ini++)
		{
			if ((*ini)->CodIdCC == (*itc)->CodIdCC)
			{
				break;
			}
		}
		for (; ini != lista_camiones_ventana.end() && !enc; ini++)
		{
			if (strcmp((*ini)->id_truck, (*itc)->id_truck) == 0)
				continue;
			if (!(*ini)->lleno)
			{
				sig_cam = (*ini);
				enc = true;
			}
		}
		if (!enc)
		{

			//buscar desde el principio de la lista uno que no esté lleno
			list<truck* >::iterator ini = lista_camiones_ventana.begin();

			bool enc = false;
			for (; (*ini)->CodIdCC != (*itc)->CodIdCC && !enc; ini++)
			{
				if (!(*ini)->lleno)
				{
					sig_cam = (*ini);
					enc = true;
				}
			}
			if (!enc) sig_cam = NULL;
		}
		//else
		//{
		//	bool enc = false;
		//	//buscar desde itc hasta el final

		//	//busco itc en la lista
		//	list<truck* >::iterator ini = lista_camiones_ventana.begin();
		//	for (; ini != lista_camiones_ventana.end(); ini++)
		//	{
		//		if ((*ini)->CodIdCC == (*itc)->CodIdCC)
		//			break;
		//	}
		//	ini++;
		//	for (; ini != lista_camiones_ventana.end() && !enc; ini++)
		//	{
		//		if (!(*ini)->lleno)
		//		{
		//			sig_cam = (*ini);
		//			enc = true;
		//		}
		//		//else
		//		//{
		//		//	//si tiene extras busco entre los extras
		//		//	for (auto exini = (*ini)->pcamiones_extra.begin(); exini != (*ini)->pcamiones_extra.end() && !enc; exini++)
		//		//	{
		//		//		if (!(*exini)->lleno)
		//		//		{
		//		//			sig_cam = (*exini);
		//		//			enc = true;
		//		//		}
		//		//	}
		//		//}
		//	}
		//	//sino lo buscamos desde el iniico hasta itc
		//	if (!enc)
		//	{
		//		for (auto ini = lista_camiones_ventana.begin(); (*ini)->CodIdCC != (*itc)->CodIdCC && !enc; ini++)
		//		{
		//			if (!(*ini)->lleno)
		//			{
		//				sig_cam = (*ini);
		//				enc = true;
		//			}
		//			//else
		//			//{
		//			//	//si tiene extras busco entre los extras
		//			//	for (auto exini = (*ini)->pcamiones_extra.begin(); exini != (*ini)->pcamiones_extra.end() && !enc; exini++)
		//			//	{
		//			//		if (!(*exini)->lleno)
		//			//		{
		//			//			sig_cam = (*exini);
		//			//			enc = true;
		//			//		}
		//			//	}
		//			//}
		//		}
			//	//	}
			//		if (!enc) sig_cam = NULL;
			//	}
			//}
		/*}*/
	}
	if (sig_cam != NULL)
	{
		if (strcmp(sig_cam->id_truck, (*itc)->id_truck) == 0)
		{
			sig_cam = NULL;
		}
	}
}
void colocar_en_sig_cam(Subproblema* Sub, list<truck*>::iterator itc, truck*& sig_cam, bool& hay_recarga, int& cuantos_recargar, bool es_extra, truck*& plani)
{
	bool he_colocado_en_el_sig_cam = false;
	int cont_cam = 0;
	list<vector<item_modelo>>::iterator item;

	if (Sub->Pruebas)
	{
		if (es_extra) cout << "En ZP_" << (*itc)->CodIdCC << ": " << (*itc)->id_truck << " No he podido colocar y lo pongo en:" << sig_cam->id_truck << endl;
		else cout << "En Z_" << (*itc)->CodIdCC << ": " << (*itc)->id_truck << " No he podido colocar y lo pongo en:" << sig_cam->id_truck << endl;
	}
	//Si me han quedadado items del camión anterior los incorporo.
	if ((*itc)->total_items_modelos > 0 && sig_cam != NULL)
	{
		//si es extra averguoa cual es.
		if (sig_cam->id_truck[0] == 'Q')
		{

			for (auto itex = plani->pcamiones_extra.begin(); itex != plani->pcamiones_extra.end(); itex++)
			{
				if (strcmp((*itex)->id_truck, sig_cam->id_truck) == 0)
					break;
				else cont_cam++;
			}
			int este = cont_cam;
			if (plani->items_extras_modelo.size() == 1)
				item = plani->items_extras_modelo.begin();
			else
			{
				for (item = plani->items_extras_modelo.begin(); item != plani->items_extras_modelo.end(); item++)
				{
					este--;
					if (este <= 0)
						break;
				}
			}
		}

		for (auto im = (*itc)->items_sol.begin(); im != (*itc)->items_sol.end(); im++)
		{
			if ((*im).num_items > 0)
			{

				//Ver si ese item puede ir en ese camión
				bool puede_ir = false;
				for (auto it = sig_cam->items.begin(); it != sig_cam->items.end(); ++it) {
					if ((*it)->CodIdCC == (*im).id_item->CodIdCC) {
						puede_ir = true; // Devuelve un puntero al elemento encontrado
						break;
					}
				}

				if (puede_ir)
				{
					//OJO CON LOS que no pueden ir en el siguiente
					int dias_earli = Days_0001(sig_cam->arrival_time_tm_ptr.tm_year, sig_cam->arrival_time_tm_ptr.tm_mon + 1, sig_cam->arrival_time_tm_ptr.tm_mday)
						- Days_0001((*im).id_item->earliest_arrival_tm_ptr.tm_year, (*im).id_item->earliest_arrival_tm_ptr.tm_mon + 1, (*im).id_item->earliest_arrival_tm_ptr.tm_mday);
					int dias_last = Days_0001(sig_cam->arrival_time_tm_ptr.tm_year, sig_cam->arrival_time_tm_ptr.tm_mon + 1, sig_cam->arrival_time_tm_ptr.tm_mday)
						- Days_0001((*im).id_item->latest_arrival_tm_ptr.tm_year, (*im).id_item->latest_arrival_tm_ptr.tm_mon + 1, (*im).id_item->latest_arrival_tm_ptr.tm_mday);

					if (dias_earli < 0 || dias_last >0)
						continue;

					if (Sub->Pruebas)
					{
						if (sig_cam->id_truck[0] != 'P')
						{
							//cout << "Los restos van en ZP_" << sig_cam->CodIdCC << ":" << sig_cam->id_truck << endl;
							cout << "XP_" << (*im).id_item->CodIdCC << "_" << sig_cam->CodIdCC << "=" << (*im).num_items << endl;
						}
						else
						{
							//cout << "Los restos van en Z_" << sig_cam->CodIdCC << ":" << sig_cam->id_truck << endl;
							cout << "X_" << (*im).id_item->CodIdCC << "_" << sig_cam->CodIdCC << "=" << (*im).num_items << endl;
						}
					}


					bool enc = false;
					for (auto itb = sig_cam->items_modelo.begin(); itb != sig_cam->items_modelo.end() && !enc; itb++)
					{
						if ((*itb).id_item->CodId == (*im).id_item->CodId)
						{
							(*itb).num_items += (*im).num_items;
							enc = true;
							break;
						}
					}
					if (!enc)
					{
						int total_dias = Days_0001(sig_cam->arrival_time_tm_ptr.tm_year, sig_cam->arrival_time_tm_ptr.tm_mon + 1, sig_cam->arrival_time_tm_ptr.tm_mday)
							- Days_0001((*im).id_item->latest_arrival_tm_ptr.tm_year, (*im).id_item->latest_arrival_tm_ptr.tm_mon + 1, (*im).id_item->latest_arrival_tm_ptr.tm_mday + 1);
						int obli = 0;
						if (total_dias == 0) obli = 1;
						sig_cam->items_modelo.push_back(item_modelo((*im).id_item, (*im).num_items, sig_cam->orden_ruta[(*im).id_item->sup_cod_dock], obli));
					}
					he_colocado_en_el_sig_cam = true;
					sig_cam->num_items_modelo += (*im).num_items;
					sig_cam->total_items_modelos += (*im).num_items;
					/*if (sig_cam->id_truck[0] == 'Q')
						plani->total_items_modelos += (*im).num_items;*/
					sig_cam->peso_total_items_modelo += (*im).num_items * (*im).id_item->weight;
					sig_cam->volumen_total_items_modelo += (*im).num_items * (*im).id_item->volumen;
					if (sig_cam->dim_minima > (*im).id_item->length)sig_cam->dim_minima = (*im).id_item->length;
					if (sig_cam->dim_minima > (*im).id_item->width)sig_cam->dim_minima = (*im).id_item->width;
				}
			}
		}


		if (he_colocado_en_el_sig_cam)
		{
			if (!sig_cam->items_sol.empty())
			{
				//Reiniciar el camion con los items que tenia y los nuevos
				cout << "antes de reanudar" << endl;
				for (auto its = sig_cam->items_modelo.begin(); its != sig_cam->items_modelo.end(); its++)
				{
					cout << "X_" << (*its).id_item->CodIdCC << "_" << sig_cam->CodIdCC << "=" << (*its).num_items << ";" << (*its).id_item->num_items << endl;
				}
				cout << "Voy a reanudar" << endl;
				sig_cam->reanudar();
				sig_cam->recargar = true;
				hay_recarga = true;
				cuantos_recargar++;
				cout << "despues de reanudar" << endl;
				for (auto its = sig_cam->items_modelo.begin(); its != sig_cam->items_modelo.end(); its++)
				{
					cout << "X_" << (*its).id_item->CodIdCC << "_" << sig_cam->CodIdCC << "=" << (*its).num_items << ";" << (*its).id_item->num_items << endl;
				}
				cout << "fin reanudar" << endl;


			}
		}
		if (he_colocado_en_el_sig_cam)
		{
			if (Sub->Pruebas)
			{
				if (sig_cam->es_extra)
					if (Sub->Pruebas)cout << "En el siguiente camion ZP_" << sig_cam->CodIdCC << " " << sig_cam->id_truck << endl;
					else
						if (Sub->Pruebas)cout << "En el siguiente camion Z_" << sig_cam->CodIdCC << " " << sig_cam->id_truck << endl;
				for (auto its = sig_cam->items_modelo.begin(); its != sig_cam->items_modelo.end(); its++)
				{
					if (sig_cam->es_extra)
					{
						if (Sub->Pruebas)cout << "XP_" << (*its).id_item->CodIdCC << "_" << sig_cam->CodIdCC << "=" << (*its).num_items << ";" << (*its).id_item->num_items << endl;
					}
					else
						if (Sub->Pruebas)
						{
							cout << "X_" << (*its).id_item->CodIdCC << "_" << sig_cam->CodIdCC << "=" << (*its).num_items << ";" << (*its).id_item->num_items << endl;
							if ((*its).num_items > (*its).id_item->num_items)
							{
								int kk = 0;
							}
						}
				}
			}
		}
	}
}
int Modeloryf1(string dir, parameters param, Subproblema* Sub, long long int fini, long long int ffin, IloEnv& env, IloModel& model)
{


	////Porcentge de volumen y peso.
	////para la aleatorización
	//double por_p = (double)Sub->porcentage_peso / 100;
	//double por_v = (double)Sub->porcentage_vol / 100;
	//double por_vheur = (double)1 + (double)(1 - por_v);


	//Sub->PonerAlgunosItemsABinarios();
	//Sub->CrearMatrizPosibles();

	//
	////Creamos las variables

	//NumVarMatrix2 x_ij(env, Sub->lista_items.size());
	//IloIntVarArray z_j(env, Sub->lista_camiones.size(), 0, 1);
	//NumVarMatrix3 xp_ijk(env, Sub->lista_items.size());
	//NumVarMatrix2 zp_jk(env, Sub->lista_camiones.size());
	////Si queremos que sean continuas
	////IloNumVarArray X2(env, num_nodos, 0, 1, ILOFLOAT);
	////Ponemos los nombres
	//char nombre[20];

	//for (auto it_i = Sub->lista_items.begin(); it_i != Sub->lista_items.end(); it_i++) //Recorrer la lista de tipo items
	//{

	//	if ((*it_i)->num_items <= 0)
	//		continue;
	//	
	//		x_ij[(*it_i)->CodIdCC] = IloIntVarArray(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
	//		xp_ijk[(*it_i)->CodIdCC] = NumVarMatrix2(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
	//	
	//}

	//for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	//{
	//	if ((*it)->no_puede_colocar_nada == true) continue;
	//	for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
	//	{

	//		if (Sub->cuantos_item[(*it_i)->CodIdCC] > MAX_CAMIONES_POR_ITEM)
	//			continue;

	//		if ((*it_i)->num_items <= 0)continue;
	//		if (!(*it)->lleno)
	//		{

	//			if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == true)
	//				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);
	//			else
	//				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, 1);
	//			//				x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVar(env, 0, (*it_i)->num_items);

	//			sprintf(nombre, "X_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
	//			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].setName(nombre);

	//			//si el camion no está en la ventana temporal poner a real.
	//			if ((*it)->arrival_time < ffin)
	//			{
	//				IloConversion(env, x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC], ILOFLOAT);
	//			}

	//		}
	//		xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
	//		for (int k = 0; k < (*it)->cota_extra; k++)
	//		{
	//			if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == true)
	//				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, (*it_i)->num_items);
	//			else
	//				xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
	//			sprintf(nombre, "XP_%d_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
	//			xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k].setName(nombre);
	//			//				printf("Variable Definida %d %d %d\n", (*it_i)->CodIdCC, (*it)->CodIdCC, k);
	//			
	//			//si el camion no está en la ventana temporal poner a real.
	//			if ((*it)->arrival_time < ffin)
	//			{
	//				IloConversion(env, xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k], ILOFLOAT);
	//			}
	//		}
	//	}
	//}

	//
	//for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	//{
	//	if ((*it)->no_puede_colocar_nada == true) continue;
	//	if (!(*it)->lleno)
	//	{
	//		sprintf(nombre, "Z_%d", (*it)->CodIdCC);
	//		z_j[(*it)->CodIdCC].setName(nombre);
	//		//Para la segunda vez, ponemos la variable a cero
	//		/*if ((*it)->items_modelo.size() > 0)
	//			z_j[(*it)->CodIdCC] = IloIntVar(env, 0, 0);*/
	//	}
	//	zp_jk[(*it)->CodIdCC] = IloIntVarArray(env, (*it)->cota_extra);
	//	for (int k = 0; k < (*it)->cota_extra; k++)
	//	{
	//		zp_jk[(*it)->CodIdCC][k] = IloIntVar(env, 0, 1);
	//		sprintf(nombre, "ZP_%d_%d", (*it)->CodIdCC, k);
	//		zp_jk[(*it)->CodIdCC][k].setName(nombre);
	//	}
	//}


	////Restricciones
	//IloRangeArray restr(env);


	////Restricción suma de items igual a todos los items 
	////restricción 1

	//for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
	//{
	//	if ((*it)->num_items <= 0) continue;

	//	IloExpr expr1(env);
	//	bool algo = false;
	//	for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
	//	{
	//		if (Sub->matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false)
	//			continue;
	//		if ((*it_c)->no_puede_colocar_nada == true) continue;
	//		if (!(*it_c)->lleno)
	//		{
	//			//			printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC,k);
	//			if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
	//				expr1 += (*it)->num_items * x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
	//			else
	//				expr1 += x_ij[(*it)->CodIdCC][(*it_c)->CodIdCC];
	//			algo = true;
	//		}
	//		for (int k = 0; k < (*it_c)->cota_extra; k++)
	//		{
	//			//				printf("Variable %d %d %d\n", (*it)->CodIdCC, (*it_c)->CodIdCC, k);
	//			if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
	//				expr1 += (*it)->num_items * xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
	//			else
	//				expr1 += xp_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k];
	//			algo = true;
	//		}

	//	}
	//	if (algo)
	//	{
	//		restr.add(IloRange(env, (*it)->num_items, expr1, (*it)->num_items));
	//		expr1.end();
	//	}
	//}
	////Restricción 2 y 3
	//for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	//{

	//	if ((*it)->no_puede_colocar_nada == true) continue;
	//	if (!(*it)->lleno)
	//	{
	//		IloExpr expr2(env);
	//		IloExpr expr3(env);


	//		bool hay_items = false;
	//		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
	//		{
	//			if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false)
	//				continue;
	//			if ((*it_i)->num_items <= 0) continue;
	//			int cuantos = 1;
	//			if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
	//				cuantos = (*it_i)->num_items;
	//			double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
	//			double dif_vol = volumen_ampliado - (*it_i)->volumen;
	//			if ((*it)->volumen_heuristico < 1)
	//				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * volumen_ampliado * cuantos;
	//			else
	//				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->volumen * cuantos;
	//			expr3 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->weight * cuantos;
	//			hay_items = true;

	//		}

	//		if (hay_items)
	//		{


	//			if ((*it)->volumen_heuristico < 1)
	//				expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_inicial * por_v;
	//			else
	//				expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->volumen_heuristico * por_vheur;
	//			expr3 += (-1) * z_j[(*it)->CodIdCC] * (*it)->peso_inicial * por_p;
	//			restr.add(IloRange(env, -IloInfinity, expr2, 0));
	//			restr.add(IloRange(env, -IloInfinity, expr3, 0));
	//		}
	//		expr2.end();
	//		expr3.end();
	//	}

	//	for (int k = 0; k < (*it)->cota_extra; k++)
	//	{
	//		IloExpr expr2p(env);
	//		IloExpr expr3p(env);
	//		//Si es la primera vez que ejecuta el modelo y no ha creado ningun extra

	//		bool hay_items = false;
	//		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
	//		{
	//			if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it)->CodIdCC] == false)
	//				continue;
	//			if ((*it_i)->num_items <= 0) continue;
	//			int cuantos = 1;
	//			if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
	//				cuantos = (*it_i)->num_items;
	//			double volumen_ampliado = (*it_i)->Volumen((*it)->width, (*it)->min_dim_width);
	//			if ((*it)->volumen_heuristico < 1)
	//				expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * volumen_ampliado * por_vheur * cuantos;
	//			else
	//				expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->volumen * cuantos;
	//			expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->weight * cuantos;
	//			hay_items = true;
	//		}

	//		if (hay_items)
	//		{
	//			//		expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_kp * por_v;
	//			if ((*it)->volumen_heuristico < 1)
	//				expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_inicial * por_v;
	//			else
	//				expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->volumen_heuristico * por_vheur;
	//			expr3p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->peso_inicial * por_p;

	//			restr.add(IloRange(env, -IloInfinity, expr2p, 0));
	//			restr.add(IloRange(env, -IloInfinity, expr3p, 0));
	//		}


	//		expr2p.end();
	//		expr3p.end();
	//	}
	//}
	////Orden de los camiones
	///**/
	//for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	//{

	//	if ((*it)->no_puede_colocar_nada == true) continue;
	//	IloExpr expr2p(env);
	//	expr2p += (-1) * z_j[(*it)->CodIdCC] * (*it)->cota_extra;
	//	//			obj += z_j[(*it)->CodIdCC] * (*it)->cost * param.transportation_cost;
	//	for (int k = 0; k < (*it)->cota_extra; k++)
	//	{
	//		expr2p += zp_jk[(*it)->CodIdCC][k];

	//	}
	//	restr.add(IloRange(env, -IloInfinity, expr2p, 0));
	//	expr2p.end();
	//}
	////AÑADIMOS LAS RESTRICCIONES AL MODELO
	//model.add(restr);

	////Función objetivo
	//IloExpr obj(env);
	////Parte de camiones
	//for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	//{
	//	if ((*it)->no_puede_colocar_nada == true) continue;
	//	if (!(*it)->lleno)
	//	{

	//		obj += z_j[(*it)->CodIdCC] * (*it)->cost * param.transportation_cost;
	//	}
	//	for (int k = 0; k < (*it)->cota_extra; k++)
	//	{
	//		obj += zp_jk[(*it)->CodIdCC][k] * (*it)->cost * (1 + param.extra_truck_cost) * param.transportation_cost;

	//	}

	//}
	////Parte de items
	//for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	//{

	//	if (strcmp((*it_c)->id_truck, "P307541403") == 0)
	//	{
	//		int kk = 9;
	//	}
	//	if ((*it_c)->no_puede_colocar_nada == true) continue;
	//	if (!(*it_c)->lleno)
	//	{
	//		int cuantos = Sub->mapa_camiones[(*it_c)->id_truck].size();
	//		for (auto it_i = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it_i != Sub->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
	//		{
	//			if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it_c)->CodIdCC] == false)
	//				continue;
	//			if ((*it_i)->num_items <= 0) continue;
	//			//Dias de diferencia entre una fecha y otra
	//			int cuantos = 1;
	//			if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
	//				cuantos = (*it_i)->num_items;
	//			//				int days2 = difftime((*it_i)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
	//			int days = Days_0001((*it_i)->latest_arrival_tm_ptr.tm_year, (*it_i)->latest_arrival_tm_ptr.tm_mon + 1, (*it_i)->latest_arrival_tm_ptr.tm_mday) -
	//				Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);
	//			/*				if (days2 != days)
	//							{
	//								time_t tempt = (*it_i)->latest_arrival_tm;
	//								tm temp = (*it_i)->latest_arrival_tm_ptr;
	//								int kk = 9;
	//							}*/
	//			if (days == 0)
	//			{
	//				tm temp_i = (*it_i)->latest_arrival_tm_ptr;
	//				tm temp_c = (*it_c)->arrival_time_tm_ptr;
	//				time_t temp_i2 = (*it_i)->latest_arrival_tm;
	//				time_t temp_c2 = (*it_c)->arrival_time_tm;
	//				long long int temp_i3 = (*it_i)->latest_arrival;
	//				long long int temp_c3 = (*it_c)->arrival_time;
	//				if ((temp_i.tm_min > temp_c.tm_min))
	//				{
	//					int kk = 9;
	//				}
	//				int kk = 9;
	//			}
	//			else
	//				obj += cuantos * (days)*x_ij[(*it_i)->CodIdCC][(*it_c)->CodIdCC] * (*it_i)->inventory_cost * param.inventory_cost;


	//		}
	//	}
	//	for (int k = 0; k < (*it_c)->cota_extra; k++)
	//	{

	//		if (!(*it_c)->lleno)
	//		{
	//			for (auto it_i = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it_i != Sub->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
	//			{
	//				if (Sub->matriz_variables[(*it_i)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
	//				if ((*it_i)->num_items <= 0) continue;
	//				int cuantos = 1;
	//				if (Sub->items_muchas_unidades[(*it_i)->CodIdCC] == false)
	//					cuantos = (*it_i)->num_items;
	//				//					int days2 = difftime((*it_i)->latest_arrival_tm, (*it_c)->arrival_time_tm) / (60 * 60 * 24);
	//				int days = Days_0001((*it_i)->latest_arrival_tm_ptr.tm_year, (*it_i)->latest_arrival_tm_ptr.tm_mon + 1, (*it_i)->latest_arrival_tm_ptr.tm_mday) -
	//					Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday);
	//				/*					if (days2 != days)
	//									{
	//										time_t tempt = (*it_i)->latest_arrival_tm;
	//										tm temp = (*it_i)->latest_arrival_tm_ptr;
	//										int kk = 9;
	//									}
	//					*/					if (days > 0)
	//						obj += cuantos * days * xp_ijk[(*it_i)->CodIdCC][(*it_c)->CodIdCC][k] * (*it_i)->inventory_cost * param.inventory_cost;
	//			}
	//		}
	//	}
	//}
	////Añadimos la función objetivo al modelo

	//IloObjective fobj = IloMinimize(env, obj);
	//model.add(fobj);
	//obj.end();
	//IloCplex cplex(model);
	//	cplex.exportModel("pruebaryf.lp");
	//	//cplex.setOut(env.getNullStream());
	//if (!Sub->Pruebas)
	//	cplex.setOut(env.getNullStream());
	////Parámetros de optimización
	//cplex.setParam(IloCplex::Threads, 8); //numero de hilos
	//cplex.setParam(IloCplex::MIPEmphasis, 1);//Feasibility


	//if (Sub->cplex_trozo == false)
	//	cplex.setParam(IloCplex::TiLim, Sub->TiempoModelo); //tiempo
	//else
	//	cplex.setParam(IloCplex::TiLim, max((double)Sub->TiempoModelo / (double)5, (double)1)); //tiempo
	////	cplex.setParam(IloCplex::SolnPoolReplace, 2);
	////	cplex.setParam(IloCplex::SolnPoolCapacity, 100); //Capacidad de la pool
	//	//Optimizamos	
	////Para inicializar una solución
	//IloNumVarArray vars(env);
	//IloNumArray vals(env);
	//if (Sub->Iter_Resuelve_Subproblema > 0)
	//{
	//	for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
	//	{
	//		for (auto ic = (*it)->camiones.begin(); ic != (*it)->camiones.begin(); ic++)
	//		{

	//			if (Sub->X_ij[(*it)->CodIdCC][(*ic)->CodIdCC] > 0)
	//			{
	//				vars.add(x_ij[(*it)->CodIdCC][(*ic)->CodIdCC]);
	//				vals.add(Sub->X_ij[(*it)->CodIdCC][(*ic)->CodIdCC]);
	//			}
	//			for (int k = 0; k < (*ic)->cota_extra; k++)
	//			{
	//				if (Sub->XP_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][k] > 0)
	//				{
	//					vars.add(xp_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][k]);
	//					vals.add(Sub->XP_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][k]);
	//				}
	//			}

	//		}
	//	}
	//	for (auto ic = Sub->lista_camiones.begin(); ic != Sub->lista_camiones.end(); ic++)
	//	{
	//		if (Sub->Z_j[(*ic)->CodIdCC] == true)
	//		{
	//			vars.add(z_j[(*ic)->CodIdCC]);
	//			vals.add(1);
	//		}
	//		for (int k = 0; k < (*ic)->cota_extra; k++)
	//		{
	//			if (Sub->ZP_jk[(*ic)->CodIdCC][k] == true)
	//			{
	//				vars.add(zp_jk[(*ic)->CodIdCC][k]);
	//				vals.add(1);
	//			}
	//		}
	//	}
	//	cplex.addMIPStart(vars, vals, cplex.MIPStartRepair);

	//}

	//double tiempo = Sub->TiempoModelo;
	//double gap = MAX_GAP;
	//int gap_max = 1000 * MAX_GAP;
	//int alea = get_random((2 * gap_max) / 3, gap_max);
	//cplex.setParam(IloCplex::EpGap, gap);

	//IloBool result = cplex.solve();
	//double GAP = (double)alea / double(1000);
	//if (result)
	//	gap = cplex.getMIPRelativeGap();
	//int cont = 1;
	//while (gap > GAP && cont < MAX_VECES_MODELO)
	//{
	//	cplex.setParam(IloCplex::MIPEmphasis, 2);//Feasibility
	//	if (Sub->Pruebas)
	//		printf("\nGap MAyor de %.3f %d\n", GAP, cont);
	//	cplex.setParam(IloCplex::EpGap, GAP - 0.001);
	//	result = cplex.solve();
	//	if (result)
	//	{
	//		gap = cplex.getMIPRelativeGap();
	//	}
	//	else
	//	{
	//		gap = GAP + 0.01;
	//		tiempo = 2 * tiempo;
	//		cplex.setParam(IloCplex::TiLim, tiempo);
	//	}
	//	cont++;
	//}

	////Si quisieramos generar soluciones
	//// IloBool result = cplex.populate(); 
	////Estado del algoritmo	al terminar
	//IloCplex::Status estado = cplex.getCplexStatus();

	//if (Sub->Pruebas) cout << result << " " << estado << endl;
	////número de soluciones obtenidas
	//int Nsol = cplex.getSolnPoolNsolns();
	//if (Nsol > 0)
	//	Sub->TieneSol = true;
	//else
	//	Sub->TieneSol = false;
	////VAlor del mip
	//Sub->ObjBound_Original = cplex.getBestObjValue();
	//Sub->ObjMip = cplex.getObjValue();
	////Valor de la cota
	//if (Sub->porcentage_peso >= 99 && Sub->porcentage_vol >= 99)
	//	Sub->ObjBound = cplex.getBestObjValue();
	//if (Sub->Pruebas) cout << "Best Value " << Sub->ObjMip << " Cota " << Sub->ObjBound << endl;
	////Limpio todos los valores porque puede ser la segunda vez que lo llamo
	//Sub->num_camiones_modelo = 0;
	//Sub->num_camiones_extra_modelo = 0;
	//Sub->max_cam_extra_tipo_modelo = 0;
	//for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	//{
	//	(*it_c)->items_modelo.clear();
	//	(*it_c)->items_extras_modelo.clear();
	//	(*it_c)->num_items_extra.clear();
	//}
	//ConvertirSolucionModeloACamiones2(Sub, &cplex, &x_ij, &z_j, &xp_ijk, &zp_jk,ffin,listacam);
	//if (Sub->Pruebas) cout << "Terminado RoadefExtra" << endl;

	////Cerramos el entorno
	//cplex.end();
	//model.end(); env.end();
	//return 1;
}

void ConvertirSolucionModeloACamiones2(Subproblema* Sub, IloCplex* cplex, NumVarMatrix2F* x_ij, IloIntVarArray* z_j, NumVarMatrix3F* xp_ijk, NumVarMatrix2* zp_jk, long long int fin, list<truck*>& listacam)
{

	if (Sub->Best_Sol_Modelo > (Sub->ObjMip + 0.00001))
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
		if (Sub->Pruebas)cout << "Modelo:" << endl;
		for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
		{
			if ((*it_c)->arrival_time < fin)
				break;
			if ((*it_c)->no_puede_colocar_nada == true) continue;
			(*it_c)->Modelo_pesado = false;
			(*it_c)->index_minimo = -1;
			(*it_c)->num_items_modelo = 0;
			(*it_c)->total_items_modelos = 0;
			(*it_c)->peso_total_items_modelo = 0;
			(*it_c)->volumen_total_items_modelo = 0;
			(*it_c)->dim_minima = (*it_c)->length;
			(*it_c)->coste_invetario_items_por_colocar = 0;
			(*it_c)->es_extra = false;
			(*it_c)->kextra = -1;
			(*it_c)->peso_cargado = 0;
			(*it_c)->volumen_ocupado = 0;


			if (!(*it_c)->lleno)
			{
				if (cplex->isExtracted((*z_j)[(*it_c)->CodIdCC]) && cplex->getValue((*z_j)[(*it_c)->CodIdCC], i) > 0.999)
				{

					bool hay_algo = false;
					Sub->Z_j[(*it_c)->CodIdCC] = true;
					if (Sub->Pruebas) cout << "Z_" << (*it_c)->CodIdCC << " " << (*it_c)->arrival_time << endl;
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
							string variable = "X_" + to_string((*it)->CodIdCC);
							if ((*it_c)->items_colocados.count(variable) == 0)
							{
								hay_algo = true;
								if (hay_algo == true && Sub->Pruebas)
								{
									cout << "X_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) <<
										" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
								}
								if (hay_algo == false && Sub->Pruebas)
								{
									if ((*it)->num_items < valor)
										int kk = 0;
								}
								//Sub->X_ij[(*it)->CodIdCC][(*it_c)->CodIdCC] = valor;
								int cuantos = 1;
								if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
									cuantos = (*it)->num_items;
								//ver si la fecha limite del item con la del camión. si coinciden el item tiene que ir en ese
								int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday)
									- Days_0001((*it)->earliest_arrival_tm_ptr.tm_year, (*it)->earliest_arrival_tm_ptr.tm_mon + 1, (*it)->earliest_arrival_tm_ptr.tm_mday + 1);
								int diac = (*it_c)->arrival_time;
								long long int diai = (*it)->earliest_arrival;
								int obli = 1;
								if (total_dias <= 0 || strcmp((*(*it)->camiones.back()).id_truck, (*it_c)->id_truck) == 0)
									obli = 0;
								(*it_c)->items_modelo.push_back(item_modelo(*it, cuantos * valor, order, obli));
								(*it_c)->num_items_modelo += cuantos * valor;
								(*it_c)->peso_total_items_modelo += cuantos * valor * (*it)->weight;
								(*it_c)->volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
								if ((*it_c)->dim_minima > (*it)->length)(*it_c)->dim_minima = (*it)->length;
								if ((*it_c)->dim_minima > (*it)->width)(*it_c)->dim_minima = (*it)->width;

								//(*it_c)->coste_invetario_items_por_colocar += (*it)->inventory_cost * valor;
							}

						}
					}
					if (hay_algo)
					{
						listacam.push_back((*it_c));
						//Si el camion es de los que ya tenia algo empaquetado, reiniciar el camion, para que los empaquete todos.
						if (!(*it_c)->items_sol.empty())
							(*it_c)->reanudar_solucion();
					}
					else
					{
						//	cout << " no hay nada" << endl;
					}
				}
				(*it_c)->total_items_modelos += (*it_c)->num_items_modelo;
				//cout << "Titems:" << (*it_c)->num_items_modelo << endl;
			}
			else
			{
				//esta lleno
				if (cplex->isExtracted((*z_j)[(*it_c)->CodIdCC]) && cplex->getValue((*z_j)[(*it_c)->CodIdCC], i) > 0.999)
				{
					if ((*it_c)->arrival_time < fin)
						break;
					for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
					{

						if ((*it)->num_items == 0)continue;
						//cout << "X_" << (*it)->CodIdCC <<"_"<< (*it_c)->CodIdCC << "=" << (*it)->num_items << endl;
						if (cplex->isExtracted((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) && cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) > 0.999)
						{
							int valor = (int)ceil(cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) - 0.001);
							string numero = to_string((*it)->CodIdCC);
							string variable = "X_" + numero;
							if ((*it_c)->items_colocados.count(variable) > 0 && (*it_c)->items_colocados[variable] != valor)
							{
								cout << "Z_" << (*it_c)->CodIdCC << endl;
								cout << "X_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " colocados=" << (*it_c)->items_colocados[variable] << endl;
							}

						}
					}
				}
			}

			int num_cam_extra_tipo = 0;
			bool esta_lleno = false;

			for (int k = 0; k < (*it_c)->cota_extra; k++)
			{
				truck* camion_extra = nullptr;
				//Compruebo que el camion no se ha llenado
				esta_lleno = false;
				if ((*it_c)->pcamiones_extra.size() == 0)esta_lleno = false;
				else
				{
					//busco el extra k
					bool enc = false;

					for (auto itpe = (*it_c)->pcamiones_extra.begin(); itpe != (*it_c)->pcamiones_extra.end(); itpe++)
					{
						if ((*itpe)->kextra == k)
						{
							esta_lleno = (*itpe)->lleno;
							enc = true;
							camion_extra = (*itpe);
							break;
						}
					}
				}

				if (!esta_lleno)
				{
					if (cplex->isExtracted((*zp_jk)[(*it_c)->CodIdCC][k]) && cplex->getValue((*zp_jk)[(*it_c)->CodIdCC][k], i) > 0.999)
					{
						bool hay_algo = false;
						Sub->ZP_jk[(*it_c)->CodIdCC][k] = true;
						//crear el camion extra
						if (Sub->Pruebas)
							cout << "ZP_" << (*it_c)->CodIdCC << "_" << k << " " << (*it_c)->arrival_time << endl;

						//if (!(*it_c)->lleno && (*it_c)->items_modelo.size() == 0)
						if (camion_extra != nullptr)
						{

							for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
							{
								if (Sub->matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
								if ((*it)->num_items == 0)continue;

								if (cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > 0.999)
								{

									int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
									int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.001);
									string variable = "XP_" + to_string((*it)->CodIdCC);
									if (camion_extra->items_colocados.count(variable) == 0)
									{
										hay_algo = true;

										//Sub->XP_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k] = valor;
										if (hay_algo == true && Sub->Pruebas)
										{
											cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
												" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
										}

										int cuantos = 1;
										if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
											cuantos = (*it)->num_items;
										//Si el item solo puede ir en ese camon por fecha
										int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday)
											- Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1);
										int obli = 1;
										if (total_dias <= 0 || strcmp((*(*it)->camiones.back()).id_truck, (*it_c)->id_truck) == 0) obli = 0;
										camion_extra->items_modelo.push_back(item_modelo(*it, cuantos * valor, order, obli));
										camion_extra->num_items_modelo += cuantos * valor;
										camion_extra->peso_total_items_modelo += cuantos * valor * (*it)->weight;
										camion_extra->volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
										if (camion_extra->dim_minima > (*it)->length)camion_extra->dim_minima = (*it)->length;
										if (camion_extra->dim_minima > (*it)->width)camion_extra->dim_minima = (*it)->width;
									}

								}
							}
							(*it_c)->total_items_modelos += camion_extra->num_items_modelo;
							if (hay_algo && !camion_extra->items_sol.empty())
								camion_extra->reanudar_solucion();
							if (hay_algo)
								listacam.push_back(camion_extra);
						}
						else
						{
							if ((*it_c)->items_sol.size() == 0 && (*it_c)->items_modelo.size() == 0 && (*it_c)->pcamiones_extra.size() == 0)
							{
								Sub->num_camiones_modelo++;
								(*it_c)->num_items_modelo = 0;
								(*it_c)->total_items_modelos = 0;
								(*it_c)->peso_total_items_modelo = 0;
								(*it_c)->volumen_total_items_modelo = 0;
								(*it_c)->dim_minima = (*it_c)->length;
								(*it_c)->coste_invetario_items_por_colocar = 0;
								hay_algo = false;
								//Ha puesto en el extra y no en el planificado.
								for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
								{
									if (Sub->matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
									if ((*it)->num_items == 0)continue;

									if (cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > 0.999)
									{

										int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
										int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.001);
										string variable = "XP_" + to_string((*it)->CodIdCC);
										if ((*it_c)->items_colocados.count(variable) == 0)
										{
											hay_algo = true;

											//Sub->XP_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k] = valor;
											if (hay_algo == true && Sub->Pruebas)
											{
												cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
													" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
											}

											int cuantos = 1;
											if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
												cuantos = (*it)->num_items;
											//Si el item solo puede ir en ese camon por fecha
											int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday)
												- Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1);
											int obli = 1;
											if (total_dias <= 0 || strcmp((*(*it)->camiones.back()).id_truck, (*it_c)->id_truck) == 0) obli = 0;
											(*it_c)->items_modelo.push_back(item_modelo(*it, cuantos * valor, order, obli));
											(*it_c)->num_items_modelo += cuantos * valor;
											(*it_c)->peso_total_items_modelo += cuantos * valor * (*it)->weight;
											(*it_c)->volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
											if ((*it_c)->dim_minima > (*it)->length)(*it_c)->dim_minima = (*it)->length;
											if ((*it_c)->dim_minima > (*it)->width)(*it_c)->dim_minima = (*it)->width;
										}

									}
								}

								(*it_c)->total_items_modelos += (*it_c)->num_items_modelo;
								//cout << "Titems:"<<(*it_c)->num_items_modelo << endl;
								if (hay_algo && !(*it_c)->items_sol.empty())
									(*it_c)->reanudar_solucion();
								if (hay_algo)
									listacam.push_back((*it_c));

							}
							else
							{

								truck* textra = new truck(*(*it_c));
								(*it_c)->pcamiones_extra.push_back(textra);
								textra->num_items_modelo = 0;
								textra->total_items_modelos = 0;
								textra->peso_total_items_modelo = 0;
								textra->volumen_total_items_modelo = 0;
								textra->dim_minima = textra->length;
								textra->coste_invetario_items_por_colocar = 0;
								textra->id_truck[0] = 'Q';
								textra->fijado = false;
								textra->lleno = false;
								textra->recargar = false;
								textra->es_extra = true;
								textra->kextra = k;
								textra->peso_cargado = 0;
								textra->volumen_ocupado = 0;
								hay_algo = false;
								string  num = "_" + to_string((int)(*it_c)->pcamiones_extra.size());
								strcat(textra->id_truck, num.c_str());
								//textra->kextra = (int)(*it_c)->pcamiones_extra.size()-1;

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
										hay_algo = true;
										int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
										int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.001);
										int cuantos = 1;
										if (hay_algo == false && Sub->Pruebas)
										{
											cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
												" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
										}

										if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
											cuantos = (*it)->num_items;
										//Si el item solo puede ir en ese camon por fecha
										int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday)
											- Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1);
										int obli = 1;
										//Si el item solo puede ir en ese camon por fecha o es el último camión en el que puede ir
										if (total_dias <= 0 || strcmp((*(*it)->camiones.back()).id_truck, (*it_c)->id_truck) == 0)
											obli = 0;
										aux.push_back(item_modelo(*it, cuantos * valor, order, obli));
										textra->items_modelo.push_back(item_modelo(*it, cuantos * valor, order, obli));
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
								//Si estaba empaquetado y hay que poner más.
								if (hay_algo && !textra->items_sol.empty())
									textra->reanudar_solucion();
								if (hay_algo)
									listacam.push_back(textra);

								(*it_c)->peso_total_items_modelo_extra.push_back(peso_total_items_modelo);
								(*it_c)->volumen_total_items_modelo_items_extra.push_back(volumen_total_items_modelo);
								(*it_c)->items_extras_modelo.push_back(aux);
								(*it_c)->num_items_extra.push_back(cant_items);
								(*it_c)->total_items_modelos += cant_items;
								//cout << "Titems:" << (*it_c)->num_items_modelo << endl;
								num_cam_extra_tipo++;
							}
						}

						/*if (hay_algo)
						{
							if (listacam.size() == 0 || (listacam.back())->CodIdCC != (*it_c)->CodIdCC)
							{
								listacam.push_back((*it_c));
							}
						}
						else
						{
							cout << " no hay nada" << endl;
						}*/
					}
				}
				//else
				//{
				//	//Si no esta lleno
				//	if (cplex->isExtracted((*zp_jk)[(*it_c)->CodIdCC][k]) && cplex->getValue((*zp_jk)[(*it_c)->CodIdCC][k], i) > 0.999)
				//	{
				//		if ((*it_c)->arrival_time < fin)
				//			break;
				//		for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
				//		{
				//			if ((*it)->num_items == 0)continue;
				//			if (cplex->isExtracted((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) && cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > 0.999)
				//			{
				//				int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.001);
				//				string numero = to_string((*it)->CodIdCC);
				//				string variable = "XP_" + numero;
				//				if (Sub->Pruebas && (*it_c)->items_colocados.count(variable) > 0 && (*it_c)->items_colocados[variable] != valor)
				//				{
				//					cout << "ZP_" << (*it_c)->CodIdCC << "_" << k << endl;
				//					cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << endl;
				//				}
				//			}
				//		}
				//	}

				//}
				if (num_cam_extra_tipo > Sub->max_cam_extra_tipo_modelo)
					Sub->max_cam_extra_tipo_modelo = num_cam_extra_tipo;
			}

		}

	}
}