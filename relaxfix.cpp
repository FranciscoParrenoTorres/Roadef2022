#include "encabezados.h"
void Enteras(parameters param, Subproblema* Sub)
{
	IloEnv env;
	//Creamos el modelo
	IloModel model = IloModel(env);
	IloRangeArray restr(env);
	NumVarMatrix2F x_ij(env, Sub->lista_items.size());
	IloNumVarArray  z_j(env, Sub->lista_camiones.size());
	NumVarMatrix3F xp_ijk(env, Sub->lista_items.size());
	NumVarMatrix2F zp_jk(env, Sub->lista_camiones.size());


	if (param.tqueda() < (Sub->TiempoModelo + 20))
		return;

	//Creo las restricciones y función objetivo
	restricciones_RyF(Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, restr);
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{

		model.add(IloConversion(env, z_j[(*it)->CodIdCC], ILOBOOL));
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			model.add(IloConversion(env, zp_jk[(*it)->CodIdCC][k], ILOBOOL));
		}
	}
	IloCplex cplex(model);
	cplex.setParam(IloCplex::Threads, 8); //numero de hilos
	//	cplex.setParam(IloCplex::MIPEmphasis, 1);//Feasibility

	if (!Sub->Pruebas)
		cplex.setOut(env.getNullStream());
	//	else
	//		if (Sub->Pruebas)cplex.exportModel("pruebaryf.lp");
	//cplex.exportModel("pruebaryf.lp");
	double tiempo = max((double)2 * Sub->TiempoModelo, (double)3);


	cplex.setParam(IloCplex::TiLim, tiempo); //tiempo


	//	int alea = get_random((1 * (gap_max - 1)) / 3, (gap_max - 1));
	cplex.setParam(IloCplex::EpGap, MAX_GAP*2 );
	//	IloNumArray solucion(env);
	//	IloNumVarArray vars_anterior(env);
	//	if (Sub->ventana_temporal==Sub->total_dias_cambios)

	//	cplex.addMIPStart(vars_anterior, solucion);
	IloBool result = cplex.solve();

	if (param.tqueda() < (Sub->TiempoModelo + 20))
		return;
	double gap = 50;
	double objbound, sol = 0;
	//	double GAP = (double)Sub->gap_max / double(1000);
	if (result)
	{
		gap = cplex.getMIPRelativeGap();
		sol = cplex.getObjValue();
		if (cplex.getBestObjValue() > Sub->ObjBound_Original )
			Sub->ObjBound_Original = cplex.getBestObjValue();

	}
	//	int cont = 1;
	if (gap > 0.1 && ((cplex.getBestObjValue() > 50000 || cplex.getBestObjValue() < 1) || Sub->Iter_Resuelve_Subproblema > 0))
		result = cplex.solve();
	if (result)
	{
		gap = cplex.getMIPRelativeGap();
		sol = cplex.getObjValue();
		Sub->ObjBound_Original = cplex.getBestObjValue();
	}


	if (gap > 0.3 && ((cplex.getBestObjValue() > 50000 || cplex.getBestObjValue() < 1) || Sub->Iter_Resuelve_Subproblema > 0))
		result = cplex.solve();
	if (result)
	{
		gap = cplex.getMIPRelativeGap();
		sol = cplex.getObjValue();
		Sub->ObjBound_Original = cplex.getBestObjValue();
	}
	if (gap > 0.3 && ((cplex.getBestObjValue() > 50000 || cplex.getBestObjValue() < 1) || Sub->Iter_Resuelve_Subproblema > 0))
		result = cplex.solve();


	if (result)
	{
		gap = cplex.getMIPRelativeGap();
		sol = cplex.getObjValue();
		Sub->ObjBound_Original = cplex.getBestObjValue();
	}
	//	double sol_ant = 0;
	//	double dif = 100;

	if (cplex.getObjValue() > 50000)
	Sub->TiempoExtra = gap * 100;
	IloCplex::Status estado = cplex.getCplexStatus();
	if (Sub->Pruebas)
		cout << "Modelo:" << endl;
	if (result)
	{
		for (auto& it_c : Sub->lista_camiones)
		{

			bool cambiado = false;
			for (int k = ((it_c)->cota_extra - 1); k >= 0; k--)
			{
				//				cout << k << " Ce " << (it_c)->cota_extra << endl;
				if (cplex.isExtracted((zp_jk)[(it_c)->CodIdCC][k]) && cplex.getValue((zp_jk)[(it_c)->CodIdCC][k], 0) > 0.999)
				{
					(it_c)->cota_extra = 2 * (k + 1);
					cambiado = true;
					break;
				}
			}
			if (!cambiado)
			{
				if (cplex.isExtracted((z_j)[(it_c)->CodIdCC]) && cplex.getValue((z_j)[(it_c)->CodIdCC], 0) > 0.999)
					(it_c)->cota_extra = 2;
				else
					(it_c)->cota_extra = 1;
			}
		}
	}
	if (Sub->Pruebas)
		cout << "Terminado Modelo:" << endl;
	cplex.end();
	model.end();
	env.end();
	if (Sub->Pruebas)
		cout << "Terminado Modelo2:" << endl;


}
//función que devuelve la fecha de llagada del camión que llega después de un número de días.
long long int CalcularFechasDiasDespues(Subproblema* Sub, list<truck*>::iterator& ultimo_camion, int dias)
{
	int cambios = 0;
	for (auto itc = ultimo_camion; itc != Sub->lista_camiones.end(); itc++)
	{
		auto itc2 = itc;
		itc2++;
		if (itc2 != Sub->lista_camiones.end())
		{

			int dif_day = (*itc)->DiferenciaDiasCamCam((*itc2));
			if (dif_day > 0)
			{

				cambios++;
				if (cambios == dias)
				{
					//						it_ini_binaria = itc2;
					return (*itc2)->arrival_time;
				}
			}
		}
		else
		{
			return (*itc)->arrival_time;
		}
	}
}
double VolumenTotal(Subproblema* Sub)
{
	double total = 0;
	for (auto it : Sub->lista_camiones)
	{
		total += it->volumen_heuristico;
	}
	return total;
}

int relaxandfix(string dir, parameters param, Subproblema* Sub)
{
	Sub->contador_modelo = 0;
	Sub->gap_max = max((1000 * MAX_GAP), (double)4);

	bool primera = true;
	bool quedan_x_colocar = true;
	int caso = 0;
	//Calculo la ventana temporal
	list<truck*>::iterator iti, itf, itaux;
	iti = Sub->lista_camiones.begin();
	itf = Sub->lista_camiones.end();
	itf--;

	//int total_dias = Days_0001((*iti)->arrival_time_tm_ptr.tm_year, (*iti)->arrival_time_tm_ptr.tm_mon + 1, (*iti)->arrival_time_tm_ptr.tm_mday)
	//	- Days_0001((*itf)->arrival_time_tm_ptr.tm_year, (*itf)->arrival_time_tm_ptr.tm_mon + 1, (*itf)->arrival_time_tm_ptr.tm_mday);
	int total_dias_cambios = 0;
	for (auto itc = Sub->lista_camiones.begin(); itc != Sub->lista_camiones.end(); itc++)
	{
		auto itc2 = itc;
		itc2++;
		if (itc2 != Sub->lista_camiones.end())
		{

			int dif_day = Days_0001((*itc)->arrival_time_tm_ptr.tm_year, (*itc)->arrival_time_tm_ptr.tm_mon + 1, (*itc)->arrival_time_tm_ptr.tm_mday)
				- Days_0001((*itc2)->arrival_time_tm_ptr.tm_year, (*itc2)->arrival_time_tm_ptr.tm_mon + 1, (*itc2)->arrival_time_tm_ptr.tm_mday);
			if (dif_day > 0)
				total_dias_cambios++;
		}

	}
	total_dias_cambios++;
	Sub->MejoraCotas = false;
	Sub->total_dias_cambios = total_dias_cambios;
	//int ventana_temporal = max(1,(int)ceil((double)total_dias_cambios/(10)));
	int ventana_temporal = max(3, (int)ceil((double)total_dias_cambios / (7)));
	//Si es pequeño sumamos algo a la ventana
	if ((Sub->Total_Posibles_Asignaciones / total_dias_cambios) < 10000)
		ventana_temporal++;
	if ((Sub->Total_Posibles_Asignaciones / total_dias_cambios) < 20000)
		ventana_temporal++;

	if (ventana_temporal > total_dias_cambios)
		ventana_temporal = total_dias_cambios;
	ventana_temporal += Sub->Iter_Resuelve_Subproblema;

	//	int MasZsBinarias = max(2, ventana_temporal / 2);
	//	int dias_no_packing = max(0, ventana_temporal / 4);
	int MasZsBinarias = max(2, ventana_temporal / 3);
	int dias_no_packing = max(0, ventana_temporal / 5);
	if (ventana_temporal > 3)
		dias_no_packing = get_random(1, ventana_temporal / 3);
	MasZsBinarias += dias_no_packing;
	int iter_real = Sub->Iter_Resuelve_Subproblema % 100;
/*	if (Sub->ObjBound_Original < 50000 && Sub->Iter_Resuelve_Subproblema == 0)
	{
		ventana_temporal = total_dias_cambios;
		MasZsBinarias = 0;
		dias_no_packing = 0;
	}*/
	/*
	if (Sub->Total_Posibles_Asignaciones < 3000)
	{
		ventana_temporal = total_dias_cambios + 1;
		if (Sub->contador_infeasibles==0)
		Sub->TiempoModelo = Sub->TiempoModelo * 2;
	}
	else
	{
		if (Sub->Total_Posibles_Asignaciones < 6000)
		{
			ventana_temporal = total_dias_cambios / 2;
			if (Sub->contador_infeasibles == 0)
			Sub->TiempoModelo = Sub->TiempoModelo * 3;
			MasZsBinarias = max(3, ventana_temporal);
			dias_no_packing = max(0, ventana_temporal - 3);
			if (ventana_temporal > 3)
				dias_no_packing = get_random(1, ventana_temporal / 2);
			MasZsBinarias += dias_no_packing;
		}
	}
	*/
/*	if (Sub->CodS < 10 || (3 * param.tqueda()>param.time_limit))
	{
		ventana_temporal = min(1, ventana_temporal) + Sub->Iter_Resuelve_Subproblema;
		dias_no_packing = max((ventana_temporal/2) - 1, 0);
	}
	//Cambiado
	else
	{
		ventana_temporal = ceil((double)(total_dias_cambios * 50000) / (double)Sub->Total_Posibles_Asignaciones);
		ventana_temporal = max(1, ventana_temporal) + Sub->Iter_Resuelve_Subproblema;
		dias_no_packing = max(ventana_temporal / 2, 0);
	}*/
//	ventana_temporal = min(1, ventana_temporal) + Sub->Iter_Resuelve_Subproblema;
//	dias_no_packing = max((ventana_temporal / 2) - 1, 0);
//	MasZsBinarias = 1;
	MasZsBinarias = 1+ Sub->Iter_Resuelve_Subproblema/10;
	
	ventana_temporal = max(total_dias_cambios/30,1);
	dias_no_packing = 0;
	
//	ventana_temporal = 0;
	if (Sub->Iter_Todos_Camiones>0)
	{
		ventana_temporal = max(total_dias_cambios / 30, 1) + Sub->Iter_Todos_Camiones ;
		if (ventana_temporal>1)
		dias_no_packing = (ventana_temporal-1)-get_random(0,ventana_temporal-1);
	
		/*	if (Sub->CodS > 20 && Sub->Iter_Resuelve_Subproblema == 1)
			{
				ventana_temporal = total_dias_cambios;
			}*/
			//
		if (ventana_temporal == total_dias_cambios)
		{
			dias_no_packing = 0;
			Sub->Best_Sol_Modelo = DBL_MAX;
		}
		if (ventana_temporal > total_dias_cambios)
		{
			ventana_temporal = total_dias_cambios;
			dias_no_packing = 0;
		}
		if (ventana_temporal > total_dias_cambios)
		{

			ventana_temporal = max(3 + (iter_real / total_dias_cambios), (int)ceil((double)total_dias_cambios / (20)));
			ventana_temporal += ((iter_real % total_dias_cambios));
			dias_no_packing = (iter_real / total_dias_cambios) + 1;

		}
		//
		if (Sub->Iter_Todos_Camiones > 3)
			Sub->Iter_Todos_Camiones = -1;
	}

	if (Sub->Last_Ramon)
		Sub->Todos_Relax = true;
	if (Sub->Todos_Relax == true )
	{
		ventana_temporal = total_dias_cambios;

	}
	

	if (Sub->Pruebas)
	{
		if (Sub->Iter_Resuelve_Subproblema==0 || Sub->Iter_Todos_Camiones<0)
			cout << "Asignaciones Aproximadas" << (Sub->Total_Posibles_Asignaciones) << "Ventana temporal x: 0 "  << "Ventana temporal z: " << ventana_temporal + MasZsBinarias << " Total dias " << total_dias_cambios << " NO Packing " << dias_no_packing << endl;
		else
			cout << "Asignaciones Aproximadas" << (Sub->Total_Posibles_Asignaciones) << "Ventana temporal x: " << ventana_temporal << "Ventana temporal z: " << ventana_temporal + MasZsBinarias << " Total dias " << total_dias_cambios << " NO Packing " << dias_no_packing << endl;
		cout << "Iteracion: " << Sub->Iter_Resuelve_Subproblema << " Iter_Real " << iter_real << endl;
	}




	itaux = iti;
	long long int ini = (*itaux)->arrival_time;


	int inc = ventana_temporal;
	//Calcular el último
	int cambios = 0;
	auto itf_packing = itaux;



	//Creo las variables del modelo
	//Establecemos el entorno
	IloEnv env;
	//Creamos el modelo
	IloModel model = IloModel(env);
	IloRangeArray restr(env);
	NumVarMatrix2F x_ij(env, Sub->lista_items.size());
	IloNumVarArray  z_j(env, Sub->lista_camiones.size());
	NumVarMatrix3F xp_ijk(env, Sub->lista_items.size());
	NumVarMatrix2F zp_jk(env, Sub->lista_camiones.size());


	if (param.tqueda() < (Sub->TiempoModelo + 20))
		return 2;

	//Creo las restricciones y función objetivo
	restricciones_RyF(Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, restr);

	IloCplex cplex(model);


	int veces = 0;
	bool final = false;
	list<truck*> lista_cam_ventana;
	itaux = Sub->lista_camiones.begin();
	bool es_ultimo = false;
	long long int fecha_fin = 0;
	long long int fecha_fin2 = 0;
	long long int fecha_packing = 0;
	bool ultimo_puesto = false;
	long long int fecha_ini_binaria = 0;
	long long int fecha_ini_binaria2 = 0;
	Sub->ini_anterior = (*itaux)->arrival_time;
	int veces_ven = 0;
	double vol_antes =0;
	Sub->Baja = 0;
	int baja = 0;
	while (itaux != Sub->lista_camiones.end() && !final)
	{
		veces_ven++;
		double vol1 = VolumenTotal(Sub);
//		cout << vol1 << " " << vol_antes;
		if ((vol1 < vol_antes) || (vol_antes <= 1))
		{
			int Algo_pasa = 9;
//			baja=0;
		}
		else
		{
			cout << "No baja el volumen " << Sub->Baja << endl;
			Sub->Baja++;
		}

		vol_antes = vol1;
		if (Sub->Last_Ramon && Sub->Baja >6)
		{
			cout<< "Ahora TEngo que meter la restriccion"<<endl;
			//Revisa las restricciones  y pone el nuevo valor
			for (IloInt i = 0; i < restr.getSize(); i++)
			{
				IloRange r = restr[i];
				if (r.getName() != nullptr && strncmp(r.getName(), "TOD", 3) == 0)
				{
					
				//	r.setLB(0);
					cout << "mayor o igual a " << Sub->NumCamiones + 1 << endl;
					r.setLB(Sub->NumCamiones + 1);
				}
			}
		}
		//Si ha cambiado algo del modelo lo vuelve a escribir
		//Con esto también podríamos solucionar lo del problema imposible
		if (Sub->MejoraCotas)
		{
			//Revisa las restricciones  y pone el nuevo valor
			for (IloInt i = 0; i < restr.getSize(); i++)
			{
				IloRange r = restr[i];
				IloExpr expr = r.getExpr();
				//				cout << r << endl;
				if (r.getName() != nullptr && strncmp(r.getName(), "Vol", 3) == 0)
				{
					int id = -1;
					int id2 = -1;
					string s = r.getName();
					// Encuentra la primera y segunda posición de '_'
					size_t first_pos = s.find('_');
					size_t second_pos = s.find('_', first_pos + 1);

					// Si hay un segundo guion bajo, extrae id e id2
					if (second_pos != std::string::npos) {
						std::string id_str = s.substr(first_pos + 1, second_pos - first_pos - 1);
						std::string id2_str = s.substr(second_pos + 1);

						id = std::stoi(id_str);
						id2 = std::stoi(id2_str);

						//						std::cout << "id: " << id << std::endl;
						//						std::cout << "id2: " << id2 << std::endl;
					}
					// Si no hay un segundo guion bajo, solo extrae id
					else {
						std::string id_str = s.substr(first_pos + 1);
						id = std::stoi(id_str);

						//						std::cout << "id: " << id << std::endl;
					}

					for (auto it : Sub->lista_camiones)
					{
						if (it->CodIdCC == id)
						{

							if (id2 < 0)
								r.setLinearCoef(z_j[it->CodIdCC], (-1) * it->volumen_heuristico);
							else
								r.setLinearCoef(zp_jk[it->CodIdCC][id2], (-1) * it->volumen_heuristico);
							break;
						}
					}

				}

				//				cout << r << endl;
			}

			// Recupera el número de restricciones



					  // Usar la función para obtener el coeficiente de y en la restricción r



		  //			if (Sub->Pruebas)cplex.exportModel("pruebaryf.lp");
		  //			for (int i=0;i<model.)
			  /*		IloModel model2(env);
					  //Creo las restricciones y función objetivo
					  restricciones_RyF(Sub, param, model2, env, x_ij, z_j, xp_ijk, zp_jk,restr);
					  model = model2;
					  cplex.clearModel();
					  cplex.extract(model);*/
		}
		Sub->MejoraCotas = false;
		cambios = 0;
		ini = (*itaux)->arrival_time;
		fecha_ini_binaria = ini;

		itf = itaux;
		inc = ventana_temporal;
		lista_cam_ventana.clear();


		auto it_ini_binaria = itaux;
		itf_packing = itaux;
		for (auto itc = itaux; itc != Sub->lista_camiones.end(); itc++)
		{
			auto itc2 = itc;
			itc2++;
			if (itc2 != Sub->lista_camiones.end())
			{

				int dif_day = (*itc)->DiferenciaDiasCamCam((*itc2));
				if (dif_day > 0)
				{

					cambios++;
					if (cambios == (dias_no_packing))
					{
						it_ini_binaria = itc2;
						fecha_ini_binaria = (*itc2)->arrival_time;
					}
					if (cambios == (ventana_temporal - dias_no_packing))
					{
						itf_packing = itc2;
					}
				}

				if (cambios == (ventana_temporal))
				{
					itf = itc2;

					if ((*itf)->CodIdCC == Sub->lista_camiones.back()->CodIdCC)
						es_ultimo = true;
					break;
				}
			}
			else
			{
				itf = itc;
				es_ultimo = true;
			}

		}

		if (ventana_temporal == 1)
		{
			itf_packing = itf;
		}
		//el resto de veces la ventana temporal es uno menos.
//		if (veces == 0 && ventana_temporal > 2)
//			ventana_temporal--;

		//if (itaux == itf  && quedan_x_colocar==false)
		if ((*itf)->CodIdCC == Sub->lista_camiones.back()->CodIdCC && quedan_x_colocar == false)
			final = true;


		if (!final)
		{

			//Poner las binarias desde fecha ini hasta fecha fin
			if (primera)
			{
				ini = ini + 10000;
				fecha_ini_binaria = ini;
				fecha_ini_binaria2 = ini;
				primera = false;
			}
			else
			{
				fecha_ini_binaria2 = CalcularFechasDiasDespues(Sub, it_ini_binaria, MasZsBinarias);
			}

			if (es_ultimo)
			{
				fecha_fin = (*itf)->arrival_time - 10000;
				fecha_fin2 = (*itf)->arrival_time - 10000;

			}
			else
			{
				fecha_fin = (*itf)->arrival_time;
				fecha_fin2 = CalcularFechasDiasDespues(Sub, itf, MasZsBinarias);
			}

			if (param.tqueda() < (Sub->TiempoModelo + 20))
				return 2;
			//Pongo binarias
			if (Sub->Pruebas) cout << "Veces " << veces_ven << " Ventana temporal x: " << ventana_temporal << "Ventana temporal z: " << ventana_temporal + MasZsBinarias << " Total dias " << total_dias_cambios << " NO Packing " << dias_no_packing << endl;

			if (Sub->Pruebas)cout << "Fecha Inicio " << ini << "Fecha Fin " << fecha_fin << endl;
			if (!ultimo_puesto)
				Poner_binarias_RyF(Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, fecha_ini_binaria, fecha_fin, fecha_ini_binaria2, fecha_fin2, ventana_temporal);
			//				Poner_binarias_RyF(cplex, Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, ini, fecha_fin,ventana_temporal);
						//Este es el anterior
			//			Poner_binarias_RyF(Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, fecha_ini_binaria, fecha_fin, ventana_temporal);
			//			Poner_binarias_RyF(Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, fecha_ini_binaria, fecha_fin,fecha_fin2, ventana_temporal);

						//			void Poner_binarias_RyF(Subproblema * Sub, parameters param, IloEnv & env, NumVarMatrix2F & x_ij, IloNumVarArray & z_j, NumVarMatrix3F & xp_ijk, NumVarMatrix2F & zp_jk, long long int ini, long long int ffin, int ventana)

			if (es_ultimo && fecha_fin < Sub->lista_camiones.back()->arrival_time)
				ultimo_puesto = true;

			//Resuelvo el modelo		
			if (es_ultimo)
			{
				fecha_packing = fecha_fin;
			}
			else
			{
				fecha_packing = (*itf_packing)->arrival_time;
			}

			if (param.tqueda() < (Sub->TiempoModelo + 20))
				return 2;

			caso = resolver_modelo_RyF(cplex, Sub, param, model, env, x_ij, z_j, xp_ijk, zp_jk, ini, fecha_packing, lista_cam_ventana);
			if (caso == 1 ) 
				return 1;
			if (param.tqueda() < (Sub->TiempoModelo + 20))
				return 2;

			if (caso == 3)
				return caso;
			//
			if (Sub->Pruebas)
				cout << "Tama ventana" << lista_cam_ventana.size()<< endl ;
			if (lista_cam_ventana.size() == 0)
			{
				//Si el modelo no coloca nada y estoy en el último camion finalizo.
				if (es_ultimo && (*itf)->CodIdCC == (*Sub->lista_camiones.back()).CodIdCC)
				{
					quedan_x_colocar = false;
					itaux = itf;
				}
				else
				{
					//avanzo en los dias
					/*if(ventana_temporal>1)*/
					itaux = itf_packing;
					/*else
						itaux = itf;*/
				}
			}
			else
			{
				//if (Sub->Pruebas)cout << "Voy al packing" << endl;
				//Packing de los camiones
				//PackingRF(dir, param, Sub, lista_cam_ventana);
				PackingRF2(dir, param, Sub, lista_cam_ventana);
				Sub->NumCamiones = lista_cam_ventana.size();
				//Si no queda nada por colocar terminar
				if (Sub->Last_Ramon)
				{
					final = true;
					for (auto it : lista_cam_ventana)
					{
						if (final == false) break;
						for (auto it2 : (*it).items)
						{
							if (it2->num_items > 0)
							{
								final = false;
								break;
							}

						}
					
					}

				}
				if (Sub->Last_Ramon && final == true)
					break;
				if (param.tqueda() < (Sub->TiempoModelo + 20))
					return 2;

				//avanzo en los dias
				/*if (ventana_temporal > 1)*/
				itaux = itf_packing;
				/*else
					itaux = itf;*/

					//fijo las variables empaquetadas
				fijar_variables_RyF(lista_cam_ventana, param, env, x_ij, z_j, xp_ijk, zp_jk, Sub, quedan_x_colocar);
				
				if (param.tqueda() < (Sub->TiempoModelo + 20))
					return 2;
			}
			veces++;
		}
		else
		{
			final = true;
		}
	}
	//Cerramos el entorno
	cplex.end();
	model.end(); env.end();

	if (param.tqueda() < (Sub->TiempoModelo + 20))
		return 2;
	//Aqui se puede poner la función de búsqueda local
	//	if (Sub->Pruebas) cout << "Aqui" << endl;
	//LA función lo que hace es mirar si puede mejorar la solución obtenida
	//Escribo el coste de inventario antes
	if (Sub->Pruebas)
		printf("Coste inventario Antes %d \n", Sub->coste_inventario);
	
	BusquedaLocal(Sub);
	if (Sub->Pruebas)
		printf("Coste inventario Después %d \n",  Sub->coste_inventario);

	return caso;
}
void Poner_binarias_RyF(Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, long long int ini, long long int ffin, int ventana)
{

	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{

		if ((*it)->arrival_time <= ffin)
		{
			break;
		}

		if ((*it)->arrival_time <= ini && (*it)->arrival_time > ffin)
		{
			model.add(IloConversion(env, z_j[(*it)->CodIdCC], ILOINT));
			for (int k = 0; k < (*it)->cota_extra; k++)
			{
				model.add(IloConversion(env, zp_jk[(*it)->CodIdCC][k], ILOINT));
			}

			//if ((*it)->no_puede_colocar_nada == true) continue;
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				model.add(IloConversion(env, x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC], ILOINT));
				for (int k = 0; k < (*it)->cota_extra; k++)
				{
					model.add(IloConversion(env, xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k], ILOINT));
				}
			}
		}

	}
}
void Poner_binarias_RyF(Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, long long int ini, long long int ffin, long long int ini2, long long int ffin2, int ventana)
{
//	if (Sub->Pruebas) cout << "Aqui" << endl;
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{

		if ((*it)->arrival_time <= ffin2)
		{
			break;
		}

		if ((*it)->arrival_time <= ini && (*it)->arrival_time > ffin && (Sub->Todas_Enteras==true || (Sub->Iter_Resuelve_Subproblema > 0 || Sub->Iter_Todos_Camiones > 0)))
		{
			
			//if ((*it)->no_puede_colocar_nada == true) continue;
			for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
			{
				model.add(IloConversion(env, x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC], ILOINT));
				for (int k = 0; k < (*it)->cota_extra; k++)
				{
					model.add(IloConversion(env, xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k], ILOINT));
				}
			}
		}
		if ((*it)->arrival_time <= ini2 && (*it)->arrival_time > ffin2)
		{

			model.add(IloConversion(env, z_j[(*it)->CodIdCC], ILOINT));
			for (int k = 0; k < (*it)->cota_extra; k++)
			{
				model.add(IloConversion(env, zp_jk[(*it)->CodIdCC][k], ILOINT));
			}




		}

	}
//	if (Sub->Pruebas) cout << "Aqui 22" << endl;
}
void UnificarListaItemsSol(list<item_modelo>& lista)
{
	for (auto it = lista.begin(); it != lista.end(); ++it)
	{
		bool duplicadoEncontrado = false;
		auto it2 = std::next(it); 
		while(it2 != lista.end() )
		{
			if (it2->id_item->CodIdCC == it->id_item->CodIdCC)
			{
				it->colocado += it2->colocado;
				it2 = lista.erase(it2);  // Borrar el duplicado y avanzar el iterador
			}
			else
			{
				++it2;  // Avanzar el iterador si no es un duplicado
			}

		}

	}
}
void fijar_variables_RyF(list<truck*> lista, parameters param, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, Subproblema* Sub, bool& quedan)
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
		UnificarListaItemsSol((*ic)->items_sol);
		if (Sub->Last_Ramon && (Sub->Fijar == true) && (*ic)->necesito && Sub->Baja>3)
		{
			int nextra= 0;
			if ((*ic)->kextra < 0)
				nextra = 0;
			else
				nextra= (*ic)->kextra+1;
			if (Sub->Pruebas) cout<< "Fijo "<< (*ic)->CodIdCC << " " << nextra << endl;
			zp_jk[(*ic)->CodIdCC][nextra].setBounds(1, 1);
		}
		//Si es original
		if ((*ic)->id_truck[0] == 'P')
		{
			//			if (Sub->Pruebas) cout << "Es plani" << endl;
			if ((*ic)->items_sol.size() > 0)
			{
				if (Sub->Pruebas) cout << (*ic)->CodIdCC << endl;
				if (Sub->Fijar==true)
				z_j[(*ic)->CodIdCC].setBounds(1, 1);
				//model.add(z_j[(*ic)->CodIdCC] == 1);
//				if (Sub->Pruebas) cout << "Z_" << (*ic)->CodIdCC << endl;
				(*ic)->fijado = true;
				//(*ic)->lleno = true;
				if (Sub->Last_Ramon)
				{
					for (auto it : (*ic)->prohibidos)
					{
						x_ij[it][(*ic)->CodIdCC].setBounds(0, 0);
					}
					continue;
				}
				for (auto it = (*ic)->items_sol.begin(); it != (*ic)->items_sol.end(); it++)
				{
					string numeroStr = to_string((*it).id_item->CodIdCC);
					string variable = "X_" + numeroStr;
					//					if (Sub->Pruebas)cout << "X_" << (*it).id_item->CodIdCC << "_" << (*ic)->CodIdCC << "=" << (*it).colocado << endl;
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
						
						/*if ((*ic)->items_colocados[variable] > 0)
							continue;
						else*/
						if((*ic)->items_colocados.count(variable)==0 ||((*ic)->items_colocados.count(variable) > 0 && (*ic)->items_colocados[variable] == 0))
						{
							//							if (Sub->Pruebas)cout << variable << "_" << (*ic)->CodIdCC << "=0" << endl;
							x_ij[(*it)->CodIdCC][(*ic)->CodIdCC].setBounds(0, 0);
							//model.add(x_ij[(*it)->CodIdCC][(*ic)->CodIdCC] == 0);
						}
					}
				}
				//en otro caso cambiar las cotas anteriores, si las hubiera
				else
				{
					for (auto it = (*ic)->items.begin(); it != (*ic)->items.end(); it++)
					{
						string numeroStr = to_string((*it)->CodIdCC);
						string variable = "X_" + numeroStr;
						/*if ((*ic)->items_colocados[variable] > 0)
							continue;
						else*/
						if ((*ic)->items_colocados.count(variable) == 0 || ((*ic)->items_colocados.count(variable) > 0 && (*ic)->items_colocados[variable] == 0))
						{
							x_ij[(*it)->CodIdCC][(*ic)->CodIdCC].setBounds(0, (*it)->num_items_original);
						}
					}

				}
			}
		}
		//Si es una copia
		else
		{

			//			if (Sub->Pruebas) cout << "Es extra" << endl;
			int kextra = (*ic)->kextra;
			//				if (Sub->Pruebas) cout << "Es extra" <<kextra << " cota:"<< (*ic)->cota_extra<<endl;
			if ((*ic)->items_sol.size() > 0)
			{
				//					if (Sub->Pruebas) cout << "Tiene items_sol" << kextra << endl;
				//					if (Sub->Pruebas) cout << "ZP_" << (*ic)->CodIdCC << "_" << kextra << ": "<<(*ic)->id_truck<<endl;
//				if (!Sub->Last_Ramon)
				if (Sub->Fijar == true)
				zp_jk[(*ic)->CodIdCC][kextra].setBounds(1, 1);
				//model.add(zp_jk[(*ic)->CodIdCC][kextra] == 1);
//					if (Sub->Pruebas)cout << "ZP_" << (*ic)->CodIdCC <<"_"<<kextra<< endl;
				(*ic)->fijado = true;
				//(*ic)->lleno = true;
				if (Sub->Last_Ramon)
				{
				//	(*ic)->items_sol.clear();
					for (auto it : (*ic)->prohibidos)
					{
						xp_ijk[it][(*ic)->CodIdCC][kextra].setBounds(0, 0);
					}
					continue;
				}
				for (auto it = (*ic)->items_sol.begin(); it != (*ic)->items_sol.end(); it++)
				{
					string numeroStr = to_string((*it).id_item->CodIdCC);
					string variable = "XP_" + numeroStr;
					//						if (Sub->Pruebas)cout << "XP_" << (*it).id_item->CodIdCC << "_" << (*ic)->CodIdCC << "_"<< kextra << " = " << (*it).colocado << endl;
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
						/*if ((*ic)->items_colocados[variable] > 0)
							continue;
						else*/
						if ((*ic)->items_colocados.count(variable) == 0 || ((*ic)->items_colocados.count(variable) > 0 && (*ic)->items_colocados[variable] == 0))
						{
							//								if (Sub->Pruebas)cout << variable << "_" << (*ic)->CodIdCC << "=0" << endl;
							xp_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][kextra].setBounds(0, 0);
							//model.add(xp_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][kextra] == 0);
						}
					}
				}
				//en otro caso cambiar las cotas anteriores, si las hubiera
				else
				{
					for (auto it = (*ic)->items.begin(); it != (*ic)->items.end(); it++)
					{
						string numeroStr = to_string((*it)->CodIdCC);
						string variable = "XP_" + numeroStr;
						/*if ((*ic)->items_colocados[variable] > 0)
							continue;
						else*/
						if ((*ic)->items_colocados.count(variable) == 0 || ((*ic)->items_colocados.count(variable) > 0 && (*ic)->items_colocados[variable] == 0))
						{
							xp_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][kextra].setBounds(0, (*it)->num_items_original);
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
					if (Sub->Pruebas)cout << "quedan" << endl;
					break;
				}
			}
			if (quedan == false && Sub->Pruebas)
				cout << "No quedan" << endl;
		}
		else quedan = true;
	}
	//cplex.addMIPStart(vars, vals, cplex.MIPStartRepair);
}
int resolver_modelo_RyF(IloCplex& cplex, Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, long long int fini, long long int ffin, list<truck*>& listacam)
{

//		if(Sub->Pruebas)cplex.exportModel("pruebaryf1.lp");
		// //TODO Quitar esto


		//Optimizamos     
	//Para inicializar una solución



	IloNumVarArray vars(env);
	IloNumArray vals(env);
	if (Sub->Iter_Resuelve_Subproblema > 0 && Sub->Todos_Los_Camiones && Sub->Iter_Todos_Camiones>0)
	{
		for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
		{
			for (auto ic = (*it)->camiones.begin(); ic != (*it)->camiones.end(); ic++)
			{

				if (Sub->X_ij[(*it)->CodIdCC][(*ic)->CodIdCC] > 0)
				{
					vars.add(x_ij[(*it)->CodIdCC][(*ic)->CodIdCC]);
					vals.add(Sub->X_ij[(*it)->CodIdCC][(*ic)->CodIdCC]);
				}
				for (int k = 0; k < (*ic)->cota_extra; k++)
				{
					if (Sub->XP_ijk[(*it)->CodIdCC][(*ic)->CodIdCC].size()>k && Sub->XP_ijk[(*it)->CodIdCC][(*ic)->CodIdCC][k] > 0)
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
	/*	if (Sub->Pruebas && Sub->Iteracion_Modelo == 0)
		{
			cplex.exportModel("pruebaryf.lp");
			exit(2);
		}*/
//		string archivete = "pruebaryf" + to_string(Sub->Iteracion_Modelo) + ".lp";
//		cplex.exportModel(archivete.c_str());
	double tiempo = Sub->TiempoModelo + (Sub->Total_Posibles_Asignaciones / 1000000);
	tiempo = tiempo - Sub->Iteracion_Modelo;
	if (tiempo < 1) tiempo = 1;
	//Parámetros de optimización
	if (Sub->Algoritmo_Paralelo == false)
		cplex.setParam(IloCplex::Threads, 8); //numero de hilos
	else
		cplex.setParam(IloCplex::Threads, 1); //numero de hilos
	cplex.setParam(IloCplex::MIPEmphasis, 1);//Feasibility
	if (!Sub->Pruebas)
		cplex.setOut(env.getNullStream());

	if (Sub->cplex_trozo == false)
		cplex.setParam(IloCplex::TiLim, tiempo); //tiempo
	else
		cplex.setParam(IloCplex::TiLim, max((double)tiempo / (double)5, (double)1)); //tiempo

	double gap = MAX_GAP;

	if (Sub->contador_modelo >= 1)
	{

		Sub->gap_max += 5;
	}
	else
	{
		Sub->gap_max -= 5;
		if (Sub->gap_max < (1000 * MAX_GAP))
			Sub->gap_max = 1000 * MAX_GAP;
	}
	Sub->contador_modelo = 0;



	//	int alea = get_random((1 * (gap_max - 1)) / 3, (gap_max - 1));
	int quito = 0;
	if (Sub->Iter_Resuelve_Subproblema > 2)//CAmbiado
		quito = Sub->Iter_Resuelve_Subproblema - 10;
	double gap_inicial = (double)max(Sub->gap_max - quito, 0) / (double)1000;
	cplex.setParam(IloCplex::EpGap, gap_inicial);
	//	IloNumArray solucion(env);
	//	IloNumVarArray vars_anterior(env);
	//	if (Sub->ventana_temporal==Sub->total_dias_cambios)

	//	cplex.addMIPStart(vars_anterior, solucion);
	IloBool result = cplex.solve();

	if (param.tqueda() < (Sub->TiempoModelo + 20))
		return 2;

	double GAP = (double)Sub->gap_max / double(1000);
	if (result)
		gap = cplex.getMIPRelativeGap();
	int cont = 1;
	double sol = 0;
	if (result)
		sol = cplex.getObjValue();
	if (result && Sub->Iteracion_Modelo== 0)
	{
		if (cplex.getBestObjValue() > Sub->ObjBound_Original)
			Sub->ObjBound_Original = cplex.getBestObjValue();
	}

	double sol_ant = 0;
	double dif = 100;
	tiempo = max(tiempo / (double)2, (double)2);


	IloCplex::Status estado = cplex.getCplexStatus();
	if (estado == IloCplex::Infeasible)
	{
		
		Sub->contador_infeasibles++;
		if (Sub->Pruebas)cout << "ANInf " << Sub->contador_infeasibles << " Infeasible:" << fini << " - " << ffin << endl;
		if (Sub->contador_infeasibles > 1 && Sub->Iter_Resuelve_Subproblema > 0)
			return 1;
		//A todos los camiones de la componente anterior se les pone cota extra
		for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
		{
			//if ((*it)->arrival_time <= Sub->ini_anterior && (*it)->arrival_time >=ffin)
			if ((*it)->arrival_time >= ffin)
			{
				if ((*it)->cota_extra != 0)
					//				(*it)->cota_extra = max((double)((*it)->cota_extra) * 1.5, (double)((*it)->cota_extra)+1);
					(*it)->cota_extra++;
				//				if (Sub->Pruebas) cout << (*it)->arrival_time << " Z_"<<(*it)->CodIdCC<<": "<<(*it)->cota_extra<<endl;
			}
		}
		//Cerramos el entorno
		if (Sub->Pruebas)
		{
			//			cplex.exportModel("pruebaryf.lp");

		}
		cplex.end();
		model.end(); env.end();
		int tiempokk = Sub->TiempoModelo;
		Sub->reset();
		Sub->TiempoModelo = tiempokk;

		return 3;

	}

	if (result == 0) tiempo = 3 * tiempo;
	while ((gap > GAP && cont < MAX_VECES_MODELO && dif > MIN_DIF_SOL_MODELO) || result == 0)
	{
		//		if (result!=0)
		//		{
		Sub->contador_modelo++;
		cplex.setParam(IloCplex::TiLim, tiempo);
		sol_ant = sol;
		cplex.setParam(IloCplex::MIPEmphasis, 1);//Feasibility
		if (Sub->Pruebas)
			printf("\nGap MAyor de %.3f %d\n", GAP, cont);
		cplex.setParam(IloCplex::EpGap, GAP - 0.001);
		result = cplex.solve();

		if (param.tqueda() < (Sub->TiempoModelo + 20))
			return 2;

		if (result)
		{
			gap = cplex.getMIPRelativeGap();
			tiempo = max(tiempo / (double)2, (double)2);
			sol = cplex.getObjValue();
			if (Sub->Iteracion_Modelo == 0)
			{
				if (cplex.getBestObjValue() > Sub->ObjBound_Original)
					Sub->ObjBound_Original = cplex.getBestObjValue();
			}
			if (sol_ant > 1)
				dif = (sol_ant - sol) / sol_ant;
		}
		else
		{
			gap = 2 * GAP;
			tiempo = 2 * tiempo;
			estado = cplex.getCplexStatus();
			if (estado == IloCplex::Infeasible)
			{
				Sub->contador_infeasibles++;
				if (Sub->Pruebas)cout << "NInf "<< Sub->contador_infeasibles << " Infeasible:" << fini << " - " << ffin << endl;
				if (Sub->contador_infeasibles > 1 && Sub->Iter_Resuelve_Subproblema>0)
					return 1;
				//A todos los camiones de la componente anterior se les pone cota extra
				for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
				{
					//if ((*it)->arrival_time <= Sub->ini_anterior && (*it)->arrival_time >= ffin)
					if ((*it)->arrival_time >= ffin)
					{
						(*it)->cota_extra++;
						if (Sub->Pruebas)cout << (*it)->arrival_time << " Z_" << (*it)->CodIdCC << ": " << (*it)->cota_extra << endl;
					}
				}
				//Cerramos el entorno
				cplex.end();
				model.end(); env.end();
				Sub->reset();
				return 3;
			}
		}
		cont++;
	}

	//Si quisieramos generar soluciones
	// IloBool result = cplex.populate();
	//Estado del algoritmo  al terminar
	estado = cplex.getCplexStatus();

	if (Sub->Pruebas) cout << result << " " << estado << endl;
	if (result == 0)
	{
		if (Sub->Pruebas)
		{
			for (int i = 0; i < 100; i++)
				cout << "NO consigue solucion";

			string kk = param.param_file + "_Error.lp";
			cplex.exportModel(kk.c_str());
		}
	}
	Sub->ini_anterior = fini;
	//número de soluciones obtenidas
	int Nsol = cplex.getSolnPoolNsolns();
	if (Nsol > 0)
	{
		//		if (Sub->ventana_temporal == Sub->total_dias_cambios)
		//		cplex.getValues(solucion, vars_anterior);
		Sub->TieneSol = true;
	}
	else
		Sub->TieneSol = false;
	//VAlor del mip
//	Sub->ObjBound_Original = cplex.getBestObjValue();
	Sub->ObjMip = cplex.getObjValue();
	//Valor de la cota
	double kk = cplex.getBestObjValue();
	if (cplex.getBestObjValue() > Sub->best_coste_total)
		return 1;
	if (Sub->Iteracion_Modelo == 0 && Sub->Todos_Los_Camiones)
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

	if (param.tqueda() < (Sub->TiempoModelo + 20))
		return 2;
		//EscribirSolucion(Sub, &cplex, &x_ij, &z_j, &xp_ijk, &zp_jk, ffin, listacam);
	ConvertirSolucionModeloACamiones2(Sub, &cplex, &x_ij, &z_j, &xp_ijk, &zp_jk, ffin, listacam);

	//Quiero comprobar que están bien los costes de inventario

	//REcorro los camiones
	int coste_inventario_temp = 0;
	for (auto itcam : listacam)
	{
		//recorro los item sol de camiones
		for( auto ititem:itcam->items_modelo)
		{
			int total_dias = Days_0001(ititem.id_item->latest_arrival_tm_ptr.tm_year, ititem.id_item->latest_arrival_tm_ptr.tm_mon + 1, ititem.id_item->latest_arrival_tm_ptr.tm_mday + 1)
				- Days_0001(itcam->arrival_time_tm_ptr.tm_year, itcam->arrival_time_tm_ptr.tm_mon + 1, itcam->arrival_time_tm_ptr.tm_mday + 1);
			coste_inventario_temp += total_dias * ititem.id_item->inventory_cost*ititem.num_items;
		}
	}

	if (Sub->Pruebas) cout << " Terminado Modelo Temporal" << endl;
	return 0;

}
void restricciones_RyF(Subproblema* Sub, parameters param, IloModel& model, IloEnv& env, NumVarMatrix2F& x_ij, IloNumVarArray& z_j, NumVarMatrix3F& xp_ijk, NumVarMatrix2F& zp_jk, IloRangeArray& restr)
{
	Sub->inventory_cost = param.inventory_cost;
	Sub->tranportation_cost = param.transportation_cost;
	//Porcentge de volumen y peso.
	//para la aleatorización
	double por_p = (double)Sub->porcentage_peso / 100;
	double por_v = (double)Sub->porcentage_vol / 100;
	double por_vheur = (double)1 + (double)(1 - por_v);
	por_p = 1;
	por_v = 1;
	por_vheur = 1;
	

	Sub->PonerAlgunosItemsABinarios();
	Sub->CrearMatrizPosibles();

	char nombre[20];

	for (auto it_i = Sub->lista_items.begin(); it_i != Sub->lista_items.end(); it_i++) //Recorrer la lista de tipo items
	{

		/*if ((*it_i)->num_items <= 0)
			continue;*/

		x_ij[(*it_i)->CodIdCC] = IloNumVarArray(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
		xp_ijk[(*it_i)->CodIdCC] = NumVarMatrix2F(env, Sub->lista_camiones.size()); //Recorrer la lista de camiones
		//Nuevas variables

	}

	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
//		printf("%d %d\n",(*it)->CodId,(*it)->CodIdCC);
		//if ((*it)->no_puede_colocar_nada == true) continue;
		for (auto it_i = (*it)->items.begin(); it_i != (*it)->items.end(); it_i++)
		{
			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] = IloNumVar(env, 0, (*it_i)->num_items);

			sprintf(nombre, "X_%d_%d", (*it_i)->CodIdCC, (*it)->CodIdCC);
			x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC].setName(nombre);
			//printf("Variable Definida %d %d \n", (*it_i)->CodIdCC, (*it)->CodIdCC);
			//Pongo todas a reales				
			model.add(IloConversion(env, x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC], ILOFLOAT));


			
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
		z_j[(*it)->CodIdCC] = IloNumVar(env, 0, 1);
		z_j[(*it)->CodIdCC].setName(nombre);


		/*}*/
		zp_jk[(*it)->CodIdCC] = IloNumVarArray(env, max((*it)->cota_extra, 1));
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			zp_jk[(*it)->CodIdCC][k] = IloNumVar(env, 0, 1);

			sprintf(nombre, "ZP_%d_%d", (*it)->CodIdCC, k);
			zp_jk[(*it)->CodIdCC][k].setName(nombre);
		}
	}
	//Restricciones



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
						//		printf("Variable %d %d %d %d %s\n", (*it)->CodIdCC, (*it_c)->CodIdCC, k,(*it)->CodId, (*it)->Id);
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

		double por = 1;
		//		if (Sub->Iter_Resuelve_Subproblema == 0)
			//		por = (double) get_random(100,103)/(double) 100;

		//		(*it)->vol_maximo = por * ((*it)->volumen_heuristico + 0.01);
		/*		if (0.9 * (*it)->vol_maximo> (*it)->volumen_heuristico)
					(*it)->vol_maximo = (double)(get_random(90,95) * ((*it)->volumen_heuristico + 0.01))/(double)(100);
				else*/
		
		double quito = (*it)->volumen_heuristico / 60;
		

		(*it)->vol_maximo = por * ((*it)->volumen_heuristico + 0.01);
		if (Sub->Tipo == 19 && Sub->Iter_Todos_Camiones == 0)
			(*it)->vol_maximo -= min(((*it)->ndestinos - 1) * quito, 5 * quito);
		//		if (por * ((*it)->peso_heuristico + 1) <= (*it)->max_loading_weight)
		//		(*it)->max_loading_weight = por * ((*it)->peso_heuristico + 1);


				/*		if (Sub->Iter_Resuelve_Subproblema > 3)
				{
					(*it)->vol_maximo = ((double)get_random(96, 100) / (double)100) * (*it)->vol_maximo;
					(*it)->max_loading_weight = ((double)get_random(89, 98) / (double)100) * (*it)->max_loading_weight;
				}*/
				//		if ((double)((*it)->peso_heuristico + 1) < (double)(*it)->max_loading_weight)
				//			(*it)->max_loading_weight = (*it)->peso_heuristico + 1;
		if (!(*it)->pocos_items)
		{
			//			(*it)->vol_maximo = ((double)get_random(96, 100) / (double)100) * (*it)->vol_maximo;
			//			(*it)->max_loading_weight = ((double)get_random(89, 98) / (double)100) * (*it)->max_loading_weight;
		}
		//printf("Aqui401 %.2f %.2f \n", (*it)->volumen_heuristico, (*it)->peso_heuristico);
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

				double vol = (*it_i)->volumen;
				double vol_sin = (*it_i)->vol_con_nest;
				if (vol_sin < vol)
					int kk = 9;
				expr2 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->vol_con_nest * cuantos;
				expr3 += x_ij[(*it_i)->CodIdCC][(*it)->CodIdCC] * (*it_i)->weight * cuantos;
				hay_items = true;

			}

			if (hay_items)
			{


				sprintf(nombre, "Vol_%d", (*it)->CodIdCC);

				expr2 += (-1) * z_j[(*it)->CodIdCC] * (*it)->vol_maximo*por;


				expr3 += (-1) * z_j[(*it)->CodIdCC] * (*it)->max_loading_weight;


				restr.add(IloRange(env, -IloInfinity, expr2, 0, nombre));
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
				expr2p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->vol_con_nest * cuantos;
				expr3p += xp_ijk[(*it_i)->CodIdCC][(*it)->CodIdCC][k] * (*it_i)->weight * cuantos;
				hay_items = true;
			}

			if (hay_items)
			{
				sprintf(nombre, "Vol_%d_%d", (*it)->CodIdCC, k);

				expr2p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->vol_maximo*por;


				expr3p += (-1) * zp_jk[(*it)->CodIdCC][k] * (*it)->max_loading_weight;
				//(*it)->max_loading_weight = (*it)->peso_inicial ;
				restr.add(IloRange(env, -IloInfinity, expr2p, 0, nombre));
				restr.add(IloRange(env, -IloInfinity, expr3p, 0));
			}


			expr2p.end();
			expr3p.end();
		}
	}

	//Orden de los camiones
	/*
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
	}*/

	//ORDEN DE LOS EXTRAS, PARA QUE COJA PRIMERO EL 0 LUEGO EL 1...
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{

		if ((*it)->cota_extra > 0)
		{
			IloExpr expr2p(env);
			expr2p += z_j[(*it)->CodIdCC] - zp_jk[(*it)->CodIdCC][0];
			restr.add(IloRange(env, 0, expr2p, IloInfinity));
			expr2p.end();

		}

		for (int k = 0; k < (*it)->cota_extra - 1; k++)
		{

			
			IloExpr expr2p(env);
			expr2p += zp_jk[(*it)->CodIdCC][k] - zp_jk[(*it)->CodIdCC][k + 1];
			restr.add(IloRange(env, 0, expr2p, IloInfinity));
			expr2p.end();


		}
	}
	if (Sub->Last_Ramon)
	{
		IloExpr expr2p(env);
		for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
		{

			expr2p += z_j[(*it)->CodIdCC];

			for (int k = 0; k < (*it)->cota_extra ; k++)
			{
				expr2p += zp_jk[(*it)->CodIdCC][k];
			}
		}
		sprintf(nombre, "TOD");

		
//		restr.add(IloRange(env, -IloInfinity, expr2p, 0, nombre));
		restr.add(IloRange(env, 0, expr2p, IloInfinity,nombre));
		expr2p.end();
	}
	//Camiones ordenados por volumen

	//Si eleije un cmaión que al menos vaya un item


	//AÑADIMOS LAS RESTRICCIONES AL MODELO
	model.add(restr);

	//Función objetivo
	IloExpr obj(env);
	unsigned int obj_int = 0;
	float obj_float = 0;
	int cuantos_cam = 0;
	//Parte de camiones
	int mas_opciones = 0;
	for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
	{
		//
/*		if ((*it)->volumen_total_items>(*it)->volumen_heuristico)
		{
			mas_opciones= (1-(((*it)->volumen_total_items/ (*it)->volumen_heuristico)))* ((*it)->cost);
		}*/
		if ((*it)->no_puede_colocar_nada == true) continue;
		if (!(*it)->lleno)
		{

			obj += z_j[(*it)->CodIdCC] * ((*it)->cost +mas_opciones) * param.transportation_cost;

		}
		for (int k = 0; k < (*it)->cota_extra; k++)
		{
			int coste_extra = (*it)->cost * (1 + param.extra_truck_cost);
			obj += zp_jk[(*it)->CodIdCC][k] * coste_extra * param.transportation_cost;
			
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
				cuantos = 1;
				//				tm temp_i_kk = (*it_i)->latest_arrival_tm_ptr;
				//				tm temp_c_kk = (*it_c)->arrival_time_tm_ptr;

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
								//				cout << (*it_c)->arrival_time_tm_ptr.tm_hour << ":" << (*it_c)->arrival_time_tm_ptr.tm_min << " día_" << (*it_c)->arrival_time_tm_ptr.tm_mday << "_" << (*it_c)->arrival_time_tm_ptr.tm_mon << "_" << (*it_c)->arrival_time_tm_ptr.tm_year << "   ";
								//				cout << (*it_i)->latest_arrival_tm_ptr.tm_hour << ":" << (*it_i)->latest_arrival_tm_ptr.tm_min << " día_" << (*it_i)->latest_arrival_tm_ptr.tm_mday << "_" << (*it_i)->latest_arrival_tm_ptr.tm_mon << "_" << (*it_i)->latest_arrival_tm_ptr.tm_year << " Dif  Days " << days<<endl;
				if (days == 0)
				{
					tm temp_i = (*it_i)->latest_arrival_tm_ptr;
					tm temp_c = (*it_c)->arrival_time_tm_ptr;
					int kk = 9;

				}
				else
				{
					obj += cuantos * (days)*x_ij[(*it_i)->CodIdCC][(*it_c)->CodIdCC] * (*it_i)->inventory_cost * param.inventory_cost;
					obj_float += cuantos * (days) * (*it_i)->inventory_cost * param.inventory_cost;
				}


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
					{
						obj += cuantos * days * xp_ijk[(*it_i)->CodIdCC][(*it_c)->CodIdCC][k] * (*it_i)->inventory_cost * param.inventory_cost;
						obj_float += cuantos * (days) * (*it_i)->inventory_cost * param.inventory_cost;
					}
				}
			}
		}
	}
	//Añadimos la función objetivo al modelo

	IloObjective fobj = IloMinimize(env, obj);
	model.add(fobj);
	obj.end();

}
void DeleteCamionSolucion(Subproblema* Sub, truck* camion_orig,bool hacia_delante)
{
	if (hacia_delante)
	{
		for (auto it = Sub->sol.listado_camiones.begin(); it != Sub->sol.listado_camiones.end(); it++)
		{
			if (strcmp(it->id_truc.c_str(), camion_orig->id_truck) == 0)
			{
				Sub->sol.listado_camiones.erase(it);
				break;
			}
		}
	}
	else
	{
		//Quiero recorrerlo al reves y quitar el elemento que cumpla la condicion
        for (auto it = Sub->sol.listado_camiones.rbegin(); it != Sub->sol.listado_camiones.rend(); ++it)
        {
            if (strcmp(it->id_truc.c_str(), camion_orig->id_truck) == 0)
            {
                Sub->sol.listado_camiones.erase(std::next(it).base());
                break;
            }
        }
	}
}
void DeleteCamionSolucion(Subproblema* Sub, truck *camion_orig)
{
	if (Sub->camiones_solu.count(camion_orig->id_truck) > 0)
	{
//		if (camion_orig->pcamiones_extra.size() == 0)
//		{
			int coste = camion_orig->es_extra ? camion_orig->coste_extra : camion_orig->cost;
			Sub->coste_transporte -= coste;
			for (auto it = Sub->sol.listado_camiones.begin(); it != Sub->sol.listado_camiones.end(); it++)
			{
				if (strcmp(it->id_truc.c_str(), camion_orig->id_truck) == 0)
				{
					Sub->sol.listado_camiones.erase(it);
					break;
				}
			}

			//		Sub->sol.listado_camiones.erase(Sub->sol.listado_camiones.begin() + Sub->camiones_solu.at(camion_orig->id_truck));
			Sub->camiones_solu.erase(camion_orig->id_truck);
//		}
//		else
/* {
			//Tengo que eliminar el último
			//antes lo copio

			for (auto itex : camion_orig->pcamiones_extra)
			{
				if (itex->pilas_solucion.size() > 0)
				{

					Sub->coste_transporte -= itex->coste_extra;
					for (auto it = Sub->sol.listado_camiones.begin(); it != Sub->sol.listado_camiones.end(); it++)
					{
						if (strcmp(it->id_truc.c_str(), itex->id_truck) == 0)
						{
							Sub->sol.listado_camiones.erase(it);
							break;
						}
					}

					//		Sub->sol.listado_camiones.erase(Sub->sol.listado_camiones.begin() + Sub->camiones_solu.at(camion_orig->id_truck));
					
				}
				Sub->camiones_solu.erase(itex->id_truck);
			}
			camion_orig->pcamiones_extra.clear();
			int coste = camion_orig->es_extra ? camion_orig->coste_extra : camion_orig->cost;
			Sub->coste_transporte -= coste;
			for (auto it = Sub->sol.listado_camiones.begin(); it != Sub->sol.listado_camiones.end(); it++)
			{
				if (strcmp(it->id_truc.c_str(), camion_orig->id_truck) == 0)
				{
					Sub->sol.listado_camiones.erase(it);
					break;
				}
			}

			//		Sub->sol.listado_camiones.erase(Sub->sol.listado_camiones.begin() + Sub->camiones_solu.at(camion_orig->id_truck));
			Sub->camiones_solu.erase(camion_orig->id_truck);


			//cambio
		}
		*/

	}

}
//No vale para nada
void CambiarCamionSolucion(Subproblema* Sub, truck* camion_orig)
{
	for (auto auxt = Sub->sol.listado_camiones.begin(); auxt != Sub->sol.listado_camiones.end(); auxt++)
	{
		if (strcmp(camion_orig->id_truck, (*auxt).id_truc.c_str()) == 0)
		{
			Sub->sol.listado_camiones.erase(auxt);
			break;
		}
	}
	truck_sol camion(camion_orig->id_truck);
	camion.peso_max = camion_orig->max_loading_weight;
	camion.vol_loaded = camion_orig->volumen_ocupado;
	camion.peso_max = camion_orig->peso_cargado;
	camion.tmt = camion_orig->peso_cargado;
	camion.emm = camion_orig->emm_colocado;
	camion.emr = camion_orig->emr_colocado;
	camion.coste = camion_orig->es_extra ? camion_orig->coste_extra : camion_orig->cost;
	
	Sub->sol.listado_camiones.push_back(camion);
	Sub->camiones_solu.at(camion_orig->id_truck) = Sub->sol.listado_camiones.size() - 1;

}
void AddCamionSolucion(Subproblema* Sub, truck *camion_orig)
{
	if (Sub->camiones_solu.count(camion_orig->id_truck) == 0)
	{
		truck_sol camion(camion_orig->id_truck);
		camion.peso_max = camion_orig->max_loading_weight;
		camion.volumen_max = camion_orig->volumen;
		camion.coste = camion_orig->es_extra ? camion_orig->coste_extra : camion_orig->cost;
		Sub->sol.listado_camiones.push_back(camion);
		(Sub->sol.listado_camiones.back()).coste = camion_orig->cost;
		Sub->coste_transporte += camion_orig->cost;
		Sub->camiones_solu.insert(pair<string, int>(camion_orig->id_truck, Sub->sol.listado_camiones.size() - 1));
	}
	else // Si el camión ya está en la solución, se actualiza.
	{
		for (auto auxt = Sub->sol.listado_camiones.begin(); auxt != Sub->sol.listado_camiones.end(); auxt++)
		{
			if (strcmp(camion_orig->id_truck, (*auxt).id_truc.c_str()) == 0)
			{
				Sub->sol.listado_camiones.erase(auxt);
				break;
			}
		}
		truck_sol camion(camion_orig->id_truck);
		camion.peso_max = camion_orig->max_loading_weight;
		camion.volumen_max = camion_orig->volumen;
		camion.coste = camion_orig->es_extra ? camion_orig->coste_extra : camion_orig->cost;
		Sub->sol.listado_camiones.push_back(camion);
		Sub->camiones_solu.at(camion_orig->id_truck) = Sub->sol.listado_camiones.size() - 1;
	}

	// Determina si el camión es considerado "pesado" basado en su capacidad de peso y volumen.
	double peso = (camion_orig->max_loading_weight / camion_orig->peso_total_items_modelo);
	double vol = (camion_orig->volumen / camion_orig->volumen_total_items_modelo);
	if (peso < vol && peso < 2)
	{
		camion_orig->Modelo_pesado = true;
		camion_orig->peso_total_items_modelo_asignados = camion_orig->peso_total_items_modelo;
		camion_orig->volumen_total_items_modelo_asignados = camion_orig->volumen_total_items_modelo;
	}
	else
	{
		camion_orig->Modelo_pesado = false;
	}
	camion_orig->peso_a_cargar = camion_orig->peso_total_items_modelo;
	camion_orig->volumen_a_cargar = camion_orig->volumen_total_items_modelo;

}
// Función que realiza el proceso de empaquetado de camiones.
// Parámetros:
// - dir: dirección (no se utiliza en el código mostrado).
// - param: parámetros relacionados con el empaquetado.
// - Sub: subproblema que contiene información sobre el empaquetado.
// - lista_camiones_ventana: lista de camiones disponibles para el empaquetado.
bool PackingRF(string dir, parameters param, Subproblema* Sub, list<truck*> lista_camiones_ventana)
{
	// Indica si es necesario recargar algún camión.
	bool hay_recarga = false;
	// Contador de camiones que necesitan ser recargados.
	int cuantos_recargar = 0;
	// Calcula el número de camiones que se deben cargar basado en un porcentaje.
	int num_camiones_a_cargar = (int)ceil(param.porcentaje * Sub->num_camiones_modelo);
	// Puntero al siguiente camión a ser procesado.
	truck* sig_cam = NULL;
	// Indica si es necesario realizar una segunda pasada por la lista de camiones.
	bool segunda_vuelta = false;

	// Iterador para recorrer la lista de camiones.
	auto itc = lista_camiones_ventana.begin();
	while (itc != lista_camiones_ventana.end())
	{
		// Actualiza el número total de iteraciones de empaquetado para el camión actual.
		(*itc)->Total_Iter_Packing = Sub->Total_Iter_Packing;

		// Verifica si el camión actual necesita ser cargado.
		if (!(*itc)->lleno && (*itc)->items_modelo.size() != 0 && (*itc)->num_items_modelo > 0 || (segunda_vuelta && (*itc)->recargar))
		{
			// Busca el siguiente camión en la lista.
			buscar_sig_cam(lista_camiones_ventana, itc, sig_cam);

			// Si el camión actual no está en la solución, se crea y se añade.
			AddCamionSolucion(Sub,(*itc));


			// Carga los items en el camión actual.
			Sub->CargoCamion((*itc), (*itc)->items_modelo, (*itc)->id_truck, (*itc)->num_items_modelo, 0);

			// Si el camión necesita ser recargado, se actualiza el contador.
			if ((*itc)->recargar)
			{
				(*itc)->recargar = false;
				cuantos_recargar--;
			}
			num_camiones_a_cargar--;


			// Si aún quedan items por cargar y hay un siguiente camión, se intenta colocar los items en el siguiente camión.
			//Quitado 13/02/24

			if ((*itc)->num_items_modelo > 0 && sig_cam != NULL)
			{
				int difdays = (*itc)->DiferenciaDiasCamCam(sig_cam);
				if (difdays == 0)
				{
					if (strcmp((*itc)->id_truck, sig_cam->id_truck) != 0)
						colocar_en_sig_cam(Sub, itc, sig_cam, hay_recarga, cuantos_recargar, false, (*itc));
				}
			}
		}

		// Avanza al siguiente camión en la lista.
		itc++;
		// Si se ha recorrido toda la lista y aún hay camiones por recargar, se reinicia el iterador para una segunda pasada.
		if (itc == lista_camiones_ventana.end() && cuantos_recargar > 0)
		{
			itc = lista_camiones_ventana.begin();
			segunda_vuelta = true;
		}
	}
	return true;
}
bool PuedeIrItemCam(item* It_choose, truck* cam)
{
	for (auto it = cam->items.begin(); it != cam->items.end(); it++)
	{
		if ((*it)->CodIdCC == It_choose->CodIdCC)
		{
			return true;
		}
	}
	return false;
}
bool PuedeIrCamItem(item* Item, truck* cam_choose)
{
	for (auto it = Item->camiones.begin(); it != Item->camiones.end(); it++)
	{
		if ((*it)->CodIdCC == cam_choose->CodIdCC)
		{
			return true;
		}
	}
	return false;
}
bool puede_ir_camion(item_modelo itM ,truck* cam)
{
	if (itM.num_items == 0) return false;

	bool puede_ir = false;
	for (auto it = cam->items.begin(); it != cam->items.end(); it++) 
	{
		if ((*it)->CodIdCC == itM.id_item->CodIdCC) 
		{
			puede_ir = true;
			break;
		}
	}
	if (puede_ir)
	{
		//Si por fecha puede ir.

		int dias_earli = Days_0001(cam->arrival_time_tm_ptr.tm_year, cam->arrival_time_tm_ptr.tm_mon + 1, cam->arrival_time_tm_ptr.tm_mday)
			- Days_0001(itM.id_item->earliest_arrival_tm_ptr.tm_year, itM.id_item->earliest_arrival_tm_ptr.tm_mon + 1, itM.id_item->earliest_arrival_tm_ptr.tm_mday);
		int dias_last = Days_0001(cam->arrival_time_tm_ptr.tm_year, cam->arrival_time_tm_ptr.tm_mon + 1, cam->arrival_time_tm_ptr.tm_mday)
			- Days_0001(itM.id_item->latest_arrival_tm_ptr.tm_year, itM.id_item->latest_arrival_tm_ptr.tm_mon + 1, itM.id_item->latest_arrival_tm_ptr.tm_mday);
		if (dias_earli < 0 || dias_last >0)
			return false;
		bool enc = false;
		for (auto itb = cam->items_modelo.begin(); itb != cam->items_modelo.end() && !enc; itb++)
		{
			if ((*itb).id_item->CodId == itM.id_item->CodId)
			{
				(*itb).num_items += itM.num_items;
				(*itb).dia = itM.dia;
				if (cam->CodIdCC == (itM.id_item->camiones.back()->CodIdCC))
					(*itb).obligado = 1;
				else
					(*itb).obligado = 0;

/*				int total_dias = cam->DiferenciaDiasLastCamItem(itM.id_item);
				if (total_dias == 0)
					int kk = 9;
				if (cam->CodIdCC == (itM.id_item->camiones.back()->CodIdCC) || total_dias == 0)
					(*itb).obligado = 1;*/
				enc = true;
				break;
			}
		}
		if (!enc)
		{
			int obli = 0;
/*			int total_dias = cam->DiferenciaDiasLastCamItem(itM.id_item);
			if (total_dias == 0)
				int kk = 9;
			if (cam->CodIdCC == (itM.id_item->camiones.back()->CodIdCC) || total_dias == 0)
				obli = 1;*/
			if (cam->CodIdCC == (itM.id_item->camiones.back()->CodIdCC))
				obli = 1;
			else
				int kk = 8;
			//							if (total_dias == 0)
//			if (Pruebas) cout << " Item " << itM.id_item->CodIdCC << " ultimo es  " << itM.id_item->camiones.back()->CodIdCC << " quedan "<< itM.id_item->num_items << endl;
//			if (cam->CodIdCC == (itM.id_item->camiones.back()->CodIdCC) || total_dias==0)
//			item kk = *itM.id_item;

			cam->items_modelo.push_back(item_modelo(itM.id_item, itM.num_items, cam->orden_ruta[itM.id_item->sup_cod_dock], obli,itM.dia,itM.Cod_Camion,itM.Cod_Camion_Last,itM.Cod_Extra_Last));
		}

		cam->num_items_modelo += itM.num_items;
		cam->total_items_modelos += itM.num_items;
		cam->peso_total_items_modelo += itM.num_items * itM.id_item->weight;
		cam->volumen_total_items_modelo += itM.num_items * itM.id_item->vol_con_nest;
		if (cam->dim_minima > itM.id_item->length) cam->dim_minima = itM.id_item->length;
		if (cam->dim_minima > itM.id_item->width) cam->dim_minima = itM.id_item->width;
		return true;
	}
	else return false;
}

void Actualizar_lista(Subproblema* sub, truck* cam)
{
	for (auto its : cam->items_modelo)
	{
		//buscar ese item ene la lista
		for (auto its2=sub->lista_items_ventana.begin(); its2!=sub->lista_items_ventana.end(); its2++)
		{
			if (its.id_item->CodId == (*its2).id_item->CodId)
			{
				(*its2).num_items = its.num_items;
			}
		}
		
	}
}
bool PackingRF2(string dir, parameters param, Subproblema* Sub, list<truck*> lista_camiones_ventana)
{
	if (Sub->Pruebas) cout << endl << "Entra en packing " << endl;
	Sub->lista_items_ventana.clear();

	// Indica si es necesario recargar algún camión.
	bool hay_recarga = false;
	// Contador de camiones que necesitan ser recargados.
	int cuantos_recargar = 0;
	// Calcula el número de camiones que se deben cargar basado en un porcentaje.
	int num_camiones_a_cargar = (int)ceil(param.porcentaje * Sub->num_camiones_modelo);
	// Puntero al siguiente camión a ser procesado.
	truck* sig_cam = NULL;
	// Indica si es necesario realizar una segunda pasada por la lista de camiones.
	bool segunda_vuelta = false;
	//VAmos a cargar en una única lista todos los items que devuelve el modelo de todos los camieons
	bool enc = false;
	bool enc2 = false;
	int obli = 0;
	int contdia = 0;
	int dia_ant = 0;
	bool por_camion = Sub->Por_Camion;
	set<int> destinos;
	for (auto itc : lista_camiones_ventana)
	{

		//Así es 91
	/*
//		if (get_random(0,1) == 0)
//		{
			if (dia_ant == 0)
			{
				dia_ant = itc->arrival_time_tm_ptr.tm_mday;
			}
			else
			{
				if (dia_ant != itc->arrival_time_tm_ptr.tm_mday)
				{
					contdia++;
					dia_ant = itc->arrival_time_tm_ptr.tm_mday;
				}
			}
//		}
	*/
		//Si es la iteracion 1 puede ser que tenga un número mayor 
		// 
		//
		for (auto itb = itc->items_modelo.begin(); itb != itc->items_modelo.end(); itb++)
		{
			if ((*itb).id_item->num_items < (*itb).num_items)
				(*itb).num_items = (*itb).id_item->num_items;
		}
	//de cada camión vamos a cargar los items que devuelve el modelo en una sola lista
		for (auto itb = itc->items_modelo.begin();  itb != itc->items_modelo.end(); itb++)
		{
			//cout << "ItemModelo " << (*itb).id_item->CodIdCC << " " << (*itb).num_items << endl;
			destinos.insert((*itb).orden_ruta);
			enc2 = false;
			obli = 0;

			for (auto itsi = Sub->lista_items_ventana.begin(); itsi != Sub->lista_items_ventana.end() && !enc2; itsi++)
			{
				//Si el item está en la lista de items del subproblema, ñado la cantidad
				if ((*itb).id_item->CodId == (*itsi).id_item->CodId)
				{
					(*itsi).num_items += (*itb).num_items;
					(*itsi).obligado = 0;
					(*itsi).Cod_Camion_Last = (*itc).CodIdCC;
					(*itsi).Cod_Extra_Last = (*itc).kextra;
					enc2 = true;
					break;
				}
			}
			if (!enc2) //si no está en la lista de items del subproblema, añado el item
			{
				//si es en el último camión que puede ir, lo pongo como obligado.
//				if (itc->CodIdCC == ((*itb).id_item->camiones.back()->CodIdCC))
//					obli = 1;
//				int total_dias = (itc)->DiferenciaDiasCamItem((*itb).id_item);
				//int diac = (*it_c)->arrival_time;
				//long long int diai = (*it)->earliest_arrival;
				//Si no es el último lo pongo 
//				if (obli == 0 && total_dias == 0)
//					obli = 1;
				obli = 0;
				item_modelo itm((*itb).id_item, (*itb).num_items, itc->orden_ruta[(*itb).id_item->sup_cod_dock], obli, contdia, (*itc).CodId);
				itm.Cod_Camion_Last = (*itc).CodIdCC;
				itm.Cod_Extra_Last = (*itc).kextra;
//				itm.Cod_Camion = (*itc).CodId;
				Sub->lista_items_ventana.push_back(itm);
				obli = 0;
			}
		}
	}
	for (auto itc : lista_camiones_ventana)
	{
		for (auto itb = itc->items.begin(); itb != itc->items.end(); itb++)
		{
			for (auto itsi = Sub->lista_items_ventana.begin(); itsi != Sub->lista_items_ventana.end() && !enc2; itsi++)
			{

				if ((*itb)->CodId == (*itsi).id_item->CodId)
				{
					(*itsi).Cod_Camion_Last = (*itc).CodIdCC;
					(*itsi).Cod_Extra_Last = (*itc).kextra;
					break;
				}
			}
		}
	}


	for (auto itsi = Sub->lista_items_ventana.begin(); itsi != Sub->lista_items_ventana.end() ; itsi++)
	{
		if ((*itsi).id_item->num_items < (*itsi).num_items)
			(*itsi).num_items = (*itsi).id_item->num_items;
	}
	if (Sub->Pruebas)
	{
		int cuenta = 0;
		if (por_camion)
		{
			for (auto itc : lista_camiones_ventana)
			{
				for (auto itb = itc->items_modelo.begin(); itb != itc->items_modelo.end(); itb++)
				{
					if ((*itb).id_item->num_items < (*itb).num_items)
					{
						printf("cambiado");
						(*itb).num_items = (*itb).id_item->num_items;
					}
					cuenta += (*itb).num_items;

				}
			}
		}
		else
		{
			for (auto itcv : Sub->lista_items_ventana)
			{
				//cout << "ItemVentana " << itcv.id_item->CodIdCC << " " << itcv.num_items << endl;
				cuenta += itcv.num_items;
			}
		}
		//
		cout << "Quedan " << cuenta << endl;
	}
	bool first_camion = true;
	//Ahora hay que colocar todos los items de la lista_ventana en cada uno de los camiones que pueda ir el item.
	int cuantos = 0;
	int camion = 0;
	for (auto itc : lista_camiones_ventana)
	{
		camion++;
		itc->items_colocados.clear();
		itc->items_modelo.clear();
		itc->volumen_total_items_modelo = 0;
		itc->num_items_modelo = 0;
//		if (lista_camiones_ventana.size() > 5)
//			int kk = 9;
		//metemos items en el camión
		//
		bool puede = false;
		for (auto itsi = Sub->lista_items_ventana.begin(); itsi != Sub->lista_items_ventana.end(); itsi++)
		{
			if (!por_camion || ((3*cuantos>lista_camiones_ventana.size()) || (*itsi).Cod_Camion == (*itc).CodId))
							//Si el item puede ir en el camion
			puede = puede_ir_camion((*itsi), itc);
			//Voy a ponerlo como obligado
			if (puede && Sub->Last_Ramon)
//			if (puede )
			{
				//Poner obligados 
				
				PonerObligados(itc, (*itsi),lista_camiones_ventana);
			}
		}
		cuantos++;
		//esta función de la lista total de items mete en el camión los que pueden ir



		if (itc->items_modelo.size() != 0)
		{
			//Empaqueto camion
		// Si el camión actual no está en la solución, se crea y se añade.

			AddCamionSolucion(Sub, itc);

			// Carga los items en el camión actual.
			Sub->CargoCamion(itc, itc->items_modelo, itc->id_truck, itc->num_items_modelo, 0);
			if (Sub->Last_Ramon)
			{
				int ntobligados = 0;
				for (auto it3 : itc->items_modelo)
				{
					if (it3.obligado && it3.num_items > 0)
					{
						ntobligados+= it3.num_items;
//						if (Sub->Pruebas) cout<< "Obligado " << it3.id_item->CodIdCC << " " << it3.num_items << " Last "<< it3.Cod_Camion_Last << endl;
					}

				}
				if (ntobligados > 0 )
				{
					if (itc != lista_camiones_ventana.back())
					{
						bool siguiente = false;

						for (auto itc2 = lista_camiones_ventana.begin(); itc2 != lista_camiones_ventana.end(); itc2++)
						{

							if ((*itc2)->CodIdCC == itc->CodIdCC)
							{
								itc2++;
								if ((*itc2)->CodId != itc->CodId)
								{
									itc->necesito = true;
									printf("NO METE todos los obligado en este camion %s  NObligados %d Titems %d\n", itc->id_truck, ntobligados, itc->items_modelo.size());

								}
								break;
							}

						}
					}
					else
					{
						itc->necesito = true;
						printf("NO METE todos los obligado en este camion %s  NObligados %d Titems %d\n", itc->id_truck, ntobligados, itc->items_modelo.size());

					}

				}
			}
			if (itc->pilas_solucion.size() <= 0)
			{
				int kk = 0;
			}
			// Si el camión necesita ser recargado, se actualiza el contador.
			if (itc->recargar)
			{
				itc->recargar = false;
				cuantos_recargar--;
			}
			num_camiones_a_cargar--;
			Actualizar_lista(Sub, itc);

			int cuenta = 0;
			if (Sub->Pruebas)
			{
				if (por_camion)
				{
					for (auto itc : lista_camiones_ventana)
					{
						for (auto itb = itc->items_modelo.begin(); itb != itc->items_modelo.end(); itb++)
						{
							cuenta += (*itb).num_items;

						}
					}

				}
				else
				{
					for (auto itcv : Sub->lista_items_ventana)
					{
						cuenta += itcv.num_items;
					}
				}
				//
				cout << "Quedan Ahora " << cuenta << " " << itc->id_truck << " Vol Heur " << itc->volumen_heuristico << " Vol Total " << itc->volumen_total_items_modelo << " Vol Cargado " << itc->volumen_ocupado << endl;
				if (2 * (itc->volumen_ocupado) < itc->volumen_total_items_modelo)
					int kk = 9;

/*				if (Sub->Iter_Todos_Camiones > 0 && 10 * (itc->volumen_total_items_modelo) > 8 * itc->volumen_heuristico
					&& itc->volumen_ocupado < itc->volumen_heuristico)
					itc->volumen_heuristico = itc->volumen_ocupado;*/

			}
		}
		else
		{
			cout << itc->CodIdCC << " No pone nada" << endl;
		}
//		first_camion = false;
	}


	// Borrar los camiones que no tengan pilas
	for (auto camit = lista_camiones_ventana.begin(); camit != lista_camiones_ventana.end(); )
	{
		if ((*camit)->pilas_solucion.size() == 0)
		{
			
			DeleteCamionSolucion(Sub, *camit);
			camit = lista_camiones_ventana.erase(camit); // Eliminar el elemento y avanzar el iterador de manera segura
		}
		else
		{
			++camit; // Avanzar el iterador solo si no se eliminó el elemento
		}
	}
	
	return true;
}
// esta función mira cuál es el último camión que puede ir un item y lo pone como obligado
//si es ese camión el que lo lleva
std::pair<int,int> UltimoCamion(item_modelo Item, std::list<truck*> lista_cam)
{
	for (auto itc = Item.id_item->camiones.rbegin(); itc != Item.id_item->camiones.rend(); itc++)
	{
		for (auto itb = lista_cam.rbegin(); itb != lista_cam.rend(); itb++)
		{
			if ((*itb)->CodIdCC==(*itc)->CodIdCC)
				return { (*itb)->CodIdCC,(*itc)->kextra };

		}
	}
	return {-1,-1};
}
void PonerObligados(truck *Camion,item_modelo Item, std::list<truck*> lista_cam)
{
	bool obligado = true;
	//Ver si está oblgado en el camión
	for (auto itb = Camion->items_modelo.begin(); itb != Camion->items_modelo.end(); itb++)
	{
		if ((*itb).id_item->CodId == Item.id_item->CodId)
		{
			obligado = (*itb).obligado;
			break;
		}
	}
	if (obligado)
		return;
	//Ver si está obligado en los camiones que ya han sido cargados
//	std::pair<int, int> camion_ultimo = UltimoCamion(Item, lista_cam);
	if (Item.Cod_Camion_Last == Camion->CodIdCC && Item.Cod_Extra_Last == Camion->kextra)
	{
		//Lo pongo obligado en la lista
		for (auto itb = Camion->items_modelo.begin(); itb != Camion->items_modelo.end(); itb++)
		{
			if ((*itb).id_item->CodId == Item.id_item->CodId)
			{
				(*itb).obligado = true;
				break;
			}
		}
	}

}
bool PackingRF2N(string dir, parameters param, Subproblema* Sub, list<truck*> lista_camiones_ventana)
{
	Sub->lista_items_ventana.clear();

	// Indica si es necesario recargar algún camión.
	bool hay_recarga = false;
	// Contador de camiones que necesitan ser recargados.
	int cuantos_recargar = 0;
	// Calcula el número de camiones que se deben cargar basado en un porcentaje.
	int num_camiones_a_cargar = (int)ceil(param.porcentaje * Sub->num_camiones_modelo);
	// Puntero al siguiente camión a ser procesado.
	truck* sig_cam = NULL;
	// Indica si es necesario realizar una segunda pasada por la lista de camiones.
	bool segunda_vuelta = false;
	//VAmos a cargar en una única lista todos los items que devuelve el modelo de todos los camieons
	bool enc = false;
	bool enc2 = false;
	int obli = 0;
	int contdia = 0;
	int dia_ant = 0;
	set<int> destinos;
	for (auto itc:lista_camiones_ventana)
	{
		
		//Así es 91
	/*
//		if (get_random(0,1) == 0)
//		{
			if (dia_ant == 0)
			{
				dia_ant = itc->arrival_time_tm_ptr.tm_mday;
			}
			else
			{
				if (dia_ant != itc->arrival_time_tm_ptr.tm_mday)
				{
					contdia++;
					dia_ant = itc->arrival_time_tm_ptr.tm_mday;
				}
			}
//		}
	*/
		//de cada camión vamos a cargar los items que devuelve el modelo en una sola lista
		for (auto itb = itc->items_modelo.begin(); itb != itc->items_modelo.end(); itb++)
		{
			//cout << "ItemModelo " << (*itb).id_item->CodIdCC << " " << (*itb).num_items << endl;
			destinos.insert((*itb).orden_ruta);
			enc2 = false;
			obli = 0;
			
			for (auto itsi = Sub->lista_items_ventana.begin(); itsi != Sub->lista_items_ventana.end() && !enc2; itsi++)
			{
				//Si el item está en la lista de items del subproblema, ñado la cantidad
				if ((*itb).id_item->CodId == (*itsi).id_item->CodId)
				{
					(*itsi).num_items += (*itb).num_items;
					enc2 = true;
					break;
				}
			}
			if (!enc2) //si no está en la lista de items del subproblema, añado el item
			{
				//si es en el último camión que puede ir, lo pongo como obligado.
				if (itc->CodIdCC == ((*itb).id_item->camiones.back()->CodIdCC))
					obli = 1;
				int total_dias = (itc)->DiferenciaDiasLastCamItem((*itb).id_item);
				//int diac = (*it_c)->arrival_time;
				//long long int diai = (*it)->earliest_arrival;
				//Si no es el último lo pongo 
				if (obli==0 && total_dias == 0 )
					obli = 1;
				item_modelo itm((*itb).id_item, (*itb).num_items, itc->orden_ruta[(*itb).id_item->sup_cod_dock], obli,contdia);
				Sub->lista_items_ventana.push_back(itm);
				obli = 0;
			}	
		}
	}
	if (Sub->Pruebas)
	{
		int cuenta = 0;
		for (auto itcv : Sub->lista_items_ventana)
		{
			//cout << "ItemVentana " << itcv.id_item->CodIdCC << " " << itcv.num_items << endl;
			cuenta += itcv.num_items;
		}
		//
		cout << "Quedan " << cuenta << endl;
	}
	//Ahora hay que colocar todos los items de la lista_ventana en cada uno de los camiones que pueda ir el item.
	for (auto itc : lista_camiones_ventana)
	{
		itc->items_modelo.clear();
		itc->volumen_total_items_modelo = 0;
		itc->num_items_modelo = 0;
		itc->items_colocados.clear();
		//metemos items en el camión
		//
		for (auto itsi = Sub->lista_items_ventana.begin(); itsi != Sub->lista_items_ventana.end(); itsi++)
		{

	
			
			if (destinos.size()>1 && (*itsi).obligado == 0)
			{
				if (itc->CodIdCC == ((*itsi).id_item->camiones.back()->CodIdCC))
					obli = 1;
				int total_dias = (itc)->DiferenciaDiasLastCamItem((*itsi).id_item);
				//int diac = (*it_c)->arrival_time;
				//long long int diai = (*it)->earliest_arrival;
				//Si no es el último lo pongo 
				if (obli == 0 && total_dias == 0)
					obli = 1;
				if (obli == 0)
					continue;
				else
					(*itsi).obligado = 1;
			}
				
			//Si el item puede ir en el camion
			bool puede = puede_ir_camion((*itsi), itc);
			//Cambiado 15/06/2024
			if (destinos.size() > 1 && (itc->volumen_heuristico < itc->volumen_total_items_modelo))
				break;
		}
		//esta función de la lista total de items mete en el camión los que pueden ir

		for (auto itsi = Sub->lista_items_ventana.begin();  itsi != Sub->lista_items_ventana.end() && (itc->volumen_heuristico >= itc->volumen_total_items_modelo && destinos.size()>1); itsi++)
		{
			if ((*itsi).obligado == 1)
				continue;
			//Si el item puede ir en el camion
			bool puede=puede_ir_camion((*itsi), itc);
			//Cambiado 15/06/2024
			if (destinos.size()>1 && (itc->volumen_heuristico < itc->volumen_total_items_modelo))
				break;
		}
		
		if (itc->items_modelo.size() != 0)
		{
			//Empaqueto camion
		// Si el camión actual no está en la solución, se crea y se añade.

			AddCamionSolucion(Sub, itc);

			// Carga los items en el camión actual.
			Sub->CargoCamion(itc, itc->items_modelo, itc->id_truck, itc->num_items_modelo, 0);

			if (itc->pilas_solucion.size() <= 0)
			{
				int kk = 0;
			}
			// Si el camión necesita ser recargado, se actualiza el contador.
			if (itc->recargar)
			{
				itc->recargar = false;
				cuantos_recargar--;
			}
			num_camiones_a_cargar--;
			Actualizar_lista(Sub, itc);

			int cuenta = 0;
			if (Sub->Pruebas)
			{
				for (auto itcv : Sub->lista_items_ventana)
				{
					cuenta += itcv.num_items;
				}
				//
				cout << "Quedan Ahora " << cuenta << itc->id_truck << " Vol Heur "  << itc->volumen_heuristico <<" Vol Total " << itc->volumen_total_items_modelo << " Vol Cargado " << itc->volumen_ocupado<< endl;
				if (2 * (itc->volumen_ocupado) < itc->volumen_total_items_modelo)
					int kk = 9;
				//98 es con esto y 99 comentado
				if (Sub->Iter_Todos_Camiones>0 && 10 * (itc->volumen_total_items_modelo) >8 * itc->volumen_heuristico
					&& itc->volumen_ocupado< itc->volumen_heuristico)
					itc->volumen_heuristico=itc->volumen_ocupado;
			}
		}
		else
		{
			cout << itc->CodIdCC << " No pone nada" << endl;
		}
		
	}


	// Iterador para recorrer la lista de camiones.
	
	return true;
}
// Función que realiza el proceso de empaquetado de camiones.
// Parámetros:
// - dir: dirección (no se utiliza en el código mostrado).
// - param: parámetros relacionados con el empaquetado.
// - Sub: subproblema que contiene información sobre el empaquetado.
// - lista_camiones_ventana: lista de camiones disponibles para el empaquetado.
bool PackingRF_New(string dir, parameters param, Subproblema* Sub, list<truck*> lista_camiones_ventana)
{
	// Indica si es necesario recargar algún camión.
	bool hay_recarga = false;
	// Contador de camiones que necesitan ser recargados.
	int cuantos_recargar = 0;
	// Calcula el número de camiones que se deben cargar basado en un porcentaje.
	int num_camiones_a_cargar = (int)ceil(param.porcentaje * Sub->num_camiones_modelo);
	// Puntero al siguiente camión a ser procesado.
	truck* sig_cam = NULL;
	// Indica si es necesario realizar una segunda pasada por la lista de camiones.
	bool segunda_vuelta = false;

	// Iterador para recorrer la lista de camiones.
	auto itc = lista_camiones_ventana.begin();
	while (itc != lista_camiones_ventana.end())
	{
		// Actualiza el número total de iteraciones de empaquetado para el camión actual.
		(*itc)->Total_Iter_Packing = Sub->Total_Iter_Packing;

		// Verifica si el camión actual necesita ser cargado.
		if (!(*itc)->lleno && (*itc)->items_modelo.size() != 0 && (*itc)->num_items_modelo > 0 || (segunda_vuelta && (*itc)->recargar))
		{
			// Busca el siguiente camión en la lista.
			buscar_sig_cam(lista_camiones_ventana, itc, sig_cam);

			// Si el camión actual no está en la solución, se crea y se añade.
			if (Sub->camiones_solu.count((*itc)->id_truck) == 0)
			{
				truck_sol camion((*itc)->id_truck);
				camion.peso_max = (*itc)->max_loading_weight;
				camion.volumen_max = (*itc)->volumen;
				camion.coste = (*itc)->es_extra ? (*itc)->coste_extra : (*itc)->cost;
				Sub->sol.listado_camiones.push_back(camion);
				(Sub->sol.listado_camiones.back()).coste = (*itc)->cost;
				Sub->coste_transporte += (*itc)->cost;
				Sub->camiones_solu.insert(pair<string, int>((*itc)->id_truck, Sub->sol.listado_camiones.size() - 1));
			}
			else // Si el camión ya está en la solución, se actualiza.
			{
				for (auto auxt = Sub->sol.listado_camiones.begin(); auxt != Sub->sol.listado_camiones.end(); auxt++)
				{

					if (strcmp((*itc)->id_truck, (*auxt).id_truc.c_str()) == 0)
					{
						Sub->sol.listado_camiones.erase(auxt);
						break;
					}
				}
				truck_sol camion((*itc)->id_truck);
				camion.peso_max = (*itc)->max_loading_weight;
				camion.volumen_max = (*itc)->volumen;
				camion.coste = (*itc)->es_extra ? (*itc)->coste_extra : (*itc)->cost;
				Sub->sol.listado_camiones.push_back(camion);
				Sub->camiones_solu.at((*itc)->id_truck) = Sub->sol.listado_camiones.size() - 1;
			}

			// Determina si el camión es considerado "pesado" basado en su capacidad de peso y volumen.
			double peso = ((*itc)->max_loading_weight / (*itc)->peso_total_items_modelo);
			double vol = ((*itc)->volumen / (*itc)->volumen_total_items_modelo);
			if (peso < vol && peso < 2)
			{
				(*itc)->Modelo_pesado = true;
				(*itc)->peso_total_items_modelo_asignados = (*itc)->peso_total_items_modelo;
				(*itc)->volumen_total_items_modelo_asignados = (*itc)->volumen_total_items_modelo;
			}
			else
			{
				(*itc)->Modelo_pesado = false;
			}
			(*itc)->peso_a_cargar = (*itc)->peso_total_items_modelo;
			(*itc)->volumen_a_cargar = (*itc)->volumen_total_items_modelo;

			// Carga los items en el camión actual.
			Sub->CargoCamion((*itc), (*itc)->items_modelo, (*itc)->id_truck, (*itc)->num_items_modelo, 0);

			// Si el camión necesita ser recargado, se actualiza el contador.
			if ((*itc)->recargar)
			{
				(*itc)->recargar = false;
				cuantos_recargar--;
			}
			num_camiones_a_cargar--;

			// Si el camión actual no tiene items asignados, se copian desde el modelo.
			if ((*itc)->items_sol.empty())
			{
				for (auto it : (*itc)->items_modelo)
					(*itc)->items_sol.push_back((it));
			}

			// Si aún quedan items por cargar y hay un siguiente camión, se intenta colocar los items en el siguiente camión.
			//Quitado 13/02/24

			if ((*itc)->num_items_modelo > 0 && sig_cam != NULL)
			{
				int difdays = (*itc)->DiferenciaDiasCamCam(sig_cam);
				difdays == 0; //51
				if (difdays == 0)
				{
					if ((*itc)->CodIdCC == sig_cam->CodIdCC)
						colocar_en_sig_cam(Sub, itc, sig_cam, hay_recarga, cuantos_recargar, false, (*itc));
				}
			}
		}

		// Avanza al siguiente camión en la lista.
		itc++;
		// Si se ha recorrido toda la lista y aún hay camiones por recargar, se reinicia el iterador para una segunda pasada.
		if (itc == lista_camiones_ventana.end() && cuantos_recargar > 0)
		{
			itc = lista_camiones_ventana.begin();
			segunda_vuelta = true;
		}
	}
	return true;
}
//

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
		for (itexcam = (*itc)->pcamiones_extra.begin(); itexcam != itex && !enc; itexcam++)
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

					enc = false;
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
					enc = false;
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
						for (ini = lista_camiones_ventana.begin(); ini != itc && !enc; ini++)
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
			ini = lista_camiones_ventana.begin();

			enc = false;
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
						int total_dias = sig_cam->DiferenciaDiasLastCamItem((*im).id_item);
						int obli = 0;
						//							if (total_dias == 0)
						if (sig_cam->CodIdCC == ((*im).id_item->camiones.back()->CodIdCC))
							obli = 1;
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
				sig_cam->reanudar();
				sig_cam->recargar = true;
				hay_recarga = true;
				cuantos_recargar++;

			}
		}

	}
}
int Modeloryf1(string dir, parameters param)
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
	//		for (auto ic = (*it)->camiones.begin(); ic != (*it)->camiones.end(); ic++)
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
void EscribirSolucion(Subproblema* Sub, IloCplex* cplex, NumVarMatrix2F* x_ij, IloNumVarArray* z_j, NumVarMatrix3F* xp_ijk, NumVarMatrix2F* zp_jk, long long int fin, list<truck*>& listacam)
{

	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{
		if (cplex->isExtracted((*z_j)[(*it_c)->CodIdCC]) && cplex->getValue((*z_j)[(*it_c)->CodIdCC], 0) > 0.999)
		{
			double vol_cargar = 0;
			double peso_cargar = 0;
			//for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
			for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
			{
				//			for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
				//			{
				int kk = (*it)->CodIdCC;
				if (cplex->isExtracted((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]))
				{
					if (cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) > 0.999)
					{


						if (Sub->Pruebas) cout << "X_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << " cplex:" << cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) << endl;
						int valor = (int)ceil(cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) - 0.001);
						peso_cargar += valor * (*it)->weight;
						vol_cargar += valor * (*it)->vol_con_nest;
					}
				}

			}
			if (Sub->Pruebas) cout << "Z_" << (*it_c)->CodIdCC << " Vol heur " << (*it_c)->volumen_heuristico << " Peso heur " << (*it_c)->peso_heuristico << " Vol Cargar " << vol_cargar << "Peso Cargar " << peso_cargar << endl;


		}
		for (int k = 0; k < (*it_c)->cota_extra; k++)
		{
			double vol_cargar = 0;
			double peso_cargar = 0;
			if (cplex->isExtracted((*zp_jk)[(*it_c)->CodIdCC][k]) && cplex->getValue((*zp_jk)[(*it_c)->CodIdCC][k], 0) > 0.999)
			{

				for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
				{
					//				for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
					//				{
					if (cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > 0.999)
					{

						if (Sub->Pruebas) cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) << endl;
						int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.001);

						vol_cargar += valor * (*it)->vol_con_nest;
						peso_cargar += valor * (*it)->weight;

					}
				}
				if (k >= 1)
					int kk = 9;
				if (Sub->Pruebas) cout << "Z_" << (*it_c)->CodIdCC << "_" << k << " Vol heur " << (*it_c)->volumen_heuristico << " Vol Cargar " << vol_cargar << "Peso Cargar " << peso_cargar << endl;

			}

		}
	}
}
void ConvertirSolucionModeloACamiones3(Subproblema* Sub, IloCplex* cplex, NumVarMatrix2F* x_ij, IloNumVarArray* z_j, NumVarMatrix3F* xp_ijk, NumVarMatrix2F* zp_jk, long long int fin, list<truck*>& listacam)
{
	if (Sub->Best_Sol_Modelo > (Sub->ObjMip + DBL_EPSILON))
	{
		Sub->Best_Sol_Modelo = Sub->ObjMip;
		Sub->Z_j.assign(Sub->lista_camiones.size(), false);
		Sub->X_ij.assign(Sub->lista_items.size(), std::vector<int>(Sub->lista_camiones.size(), 0));
		Sub->XP_ijk.assign(Sub->lista_items.size(), std::vector<vector<int>>(Sub->lista_camiones.size()));
		for (auto& item : Sub->lista_items)
			for (auto& camion : item->camiones)
				Sub->XP_ijk[item->CodIdCC][camion->CodIdCC].assign(camion->cota_extra, 0);
		Sub->ZP_jk.resize(Sub->lista_camiones.size());
		for (auto& camion : Sub->lista_camiones)
			Sub->ZP_jk[camion->CodIdCC].assign(camion->cota_extra, 0);


	}
	int i = 0;

	//Extraer los valores de X
	if (Sub->Pruebas)cout << "Modelo:" << endl;
	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{
		//TODO cambiar el del primer día igual break
		if ((*it_c)->arrival_time <= fin)
			break;
		if ((*it_c)->no_puede_colocar_nada == true) continue;
		(*it_c)->reset2();
		(*it_c)->Modelo_pesado = false;
		(*it_c)->es_extra = false;
		(*it_c)->kextra = -1;
		(*it_c)->peso_cargado = 0;
		(*it_c)->volumen_ocupado = 0;


		if (!(*it_c)->lleno)
		{
			if (cplex->isExtracted((*z_j)[(*it_c)->CodIdCC]) && cplex->getValue((*z_j)[(*it_c)->CodIdCC], 0) > 0.999)
			{

				bool hay_algo = false;
				Sub->Z_j[(*it_c)->CodIdCC] = true;
				//					if (Sub->Pruebas) cout << "Z_" <<  (*it_c)->CodIdCC << " " << (*it_c)->arrival_time << endl;
				Sub->num_camiones_modelo++;
				(*it_c)->reset2();

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
								//								cout << "X_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:"<< cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC])<<
								//									" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
							}

							int cuantos = 1;
							if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
								cuantos = (*it)->num_items;
							//ver si la fecha limite del item con la del camión. si coinciden el item tiene que ir en ese
							int total_dias = (*it_c)->DiferenciaDiasLastCamItem((*it));
							//int diac = (*it_c)->arrival_time;
							//long long int diai = (*it)->earliest_arrival;
							int obli = 0;
							if (total_dias == 0 || strcmp((*(*it)->camiones.back()).id_truck, (*it_c)->id_truck) == 0)
								obli = 1;
							(*it_c)->AddItem(*it, cuantos * valor, order, obli);

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

			}
			(*it_c)->total_items_modelos += (*it_c)->num_items_modelo;
			//cout << "Titems:" << (*it_c)->num_items_modelo << endl;
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
				if (cplex->isExtracted((*zp_jk)[(*it_c)->CodIdCC][k]) && cplex->getValue((*zp_jk)[(*it_c)->CodIdCC][k], 0) > 0.999)
				{
					bool hay_algo = false;
					Sub->ZP_jk[(*it_c)->CodIdCC][k] = true;
					//crear el camion extra
//						if (Sub->Pruebas)
//							cout << "ZP_" << (*it_c)->CodIdCC << "_"<< k << " " << (*it_c)->arrival_time <<endl;

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
										//							cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
									//									" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
									}

									int cuantos = 1;
									if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
										cuantos = (*it)->num_items;
									//Si el item solo puede ir en ese camon por fecha
									int total_dias = (*it_c)->DiferenciaDiasLastCamItem((*it));
									int obli = 0;
									if (total_dias == 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
										obli = 1;
									camion_extra->AddItem(*it, cuantos * valor, order, obli);
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
							(*it_c)->reset2();
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
											//								cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
											//									" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
										}

										int cuantos = 1;
										if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
											cuantos = (*it)->num_items;
										//Si el item solo puede ir en ese camon por fecha
										int total_dias = (*it_c)->DiferenciaDiasLastCamItem((*it));
										int obli = 0;
										if (total_dias == 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
											obli = 1;
										(*it_c)->AddItem(*it, cuantos * valor, order, obli);
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
							textra->reset2();
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
										//							cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
										//								" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
									}

									if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
										cuantos = (*it)->num_items;
									//Si el item solo puede ir en ese camon por fecha
									int total_dias = (*it_c)->DiferenciaDiasLastCamItem((*it));
									int obli = 0;
									//Si el item solo puede ir en ese camon por fecha o es el último camión en el que puede ir
									if (total_dias == 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
										obli = 1;
									aux.push_back(item_modelo(*it, cuantos * valor, order, obli));
									textra->AddItem(*it, cuantos * valor, order, obli);
									//textra->coste_invetario_items_por_colocar += (*it)->inventory_cost * valor;
									cant_items += cuantos * valor;
									peso_total_items_modelo += cuantos * valor * (*it)->weight;
									volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
									if ((*it_c)->dim_minima > (*it)->length)(*it_c)->dim_minima = (*it)->length;
									if ((*it_c)->dim_minima > (*it)->width)(*it_c)->dim_minima = (*it)->width;
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


				}
			}

			if (num_cam_extra_tipo > Sub->max_cam_extra_tipo_modelo)
				Sub->max_cam_extra_tipo_modelo = num_cam_extra_tipo;
		}

	}


}
void ConvertirSolucionModeloACamiones2(Subproblema* Sub, IloCplex* cplex, NumVarMatrix2F* x_ij, IloNumVarArray* z_j, NumVarMatrix3F* xp_ijk, NumVarMatrix2F* zp_jk, long long int fin, list<truck*>& listacam)
{

	double val_minimo = 0.999;
	if (!(Sub->Iter_Resuelve_Subproblema > 0 || Sub->Iter_Todos_Camiones > 0))
		val_minimo = 0.02;
	if (Sub->Last_Ramon == true)
	{
		if (Sub->Iter_Resuelve_Subproblema<=1)
		val_minimo = -0.01;
		else
			val_minimo = 0.0+((double)Sub->Iter_Resuelve_Subproblema /(double)100);
	}
	if (Sub->Pruebas) cout << "Val Min " << val_minimo;
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
			{
				Sub->XP_ijk[(*it)->CodIdCC][(*ic)->CodIdCC].resize((*ic)->cota_extra, 0);
			}

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
			//TODO cambiar el del primer día igual break
			if ((*it_c)->arrival_time <= fin)
				break;
			if ((*it_c)->no_puede_colocar_nada == true) continue;
			(*it_c)->necesito = false;
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
			//Cambiado 30/01/2025
//			(*it_c)->peso_cargado = 0;
//			(*it_c)->volumen_ocupado = 0;


			if (!(*it_c)->lleno)
			{
				if (cplex->isExtracted((*z_j)[(*it_c)->CodIdCC]) && cplex->getValue((*z_j)[(*it_c)->CodIdCC], i) > 0.999)
				{

					bool hay_algo = false;
					Sub->Z_j[(*it_c)->CodIdCC] = true;
					if (Sub->Pruebas) cout << endl << "Z_" << (*it_c)->CodIdCC << " " << (*it_c)->arrival_time << " "<< (*it_c)->id_truck;
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

						if ((*it)->num_items == 0 && Sub->Last_Ramon==false) continue;
						if (cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) >  val_minimo )
						{


							int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
							int valor = (int)ceil(cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) - 0.0001);
							string variable = "X_" + to_string((*it)->CodIdCC);
							if ((*it_c)->items_colocados.count(variable) == 0)
							{
								hay_algo = true;
								if (hay_algo == true && Sub->Pruebas)
								{
//																	cout << "X_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:"<< cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC])<<
//																		" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
								}
								//if (hay_algo == false && Sub->Pruebas)
								//{
								//	if ((*it)->num_items < valor)
								//		int kk = 0;
								//}
								//Sub->X_ij[(*it)->CodIdCC][(*it_c)->CodIdCC] = valor;
								int cuantos = 1;
								if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
									cuantos = (*it)->num_items;
								//ver si la fecha limite del item con la del camión. si coinciden el item tiene que ir en ese
//								int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday+1)
//									- Days_0001((*it)->earliest_arrival_tm_ptr.tm_year, (*it)->earliest_arrival_tm_ptr.tm_mon + 1, (*it)->earliest_arrival_tm_ptr.tm_mday + 1);
								int total_dias = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1)
									- Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday + 1);

								//int diac = (*it_c)->arrival_time;
								//long long int diai = (*it)->earliest_arrival;
								int obli = 0;
								//								if (total_dias <= 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
								if ((*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
									obli = 1;
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
					if (hay_algo || Sub->Last_Ramon)
//					if (hay_algo)
					{
						listacam.push_back((*it_c));
						if (Sub->Pruebas) cout << " Packing ";
						//Si el camion es de los que ya tenia algo empaquetado, reiniciar el camion, para que los empaquete todos.
						if (!(*it_c)->items_sol.empty())
							(*it_c)->reanudar_solucion();
					}
					else
					{
						int kk = 9;
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
					if (Sub->Pruebas) cout << endl << "Z_" << (*it_c)->CodIdCC << " " << (*it_c)->arrival_time << " " << (*it_c)->id_truck << "Lleno";

					if ((*it_c)->arrival_time < fin)
						break;
					for (auto it = (*it_c)->items.begin(); it != (*it_c)->items.end(); it++)
					{

						if ((*it)->num_items == 0 && Sub->Last_Ramon == false) continue;
						//cout << "X_" << (*it)->CodIdCC <<"_"<< (*it_c)->CodIdCC << "=" << (*it)->num_items << endl;
						if (cplex->isExtracted((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) && cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) > val_minimo)
						{
							int valor = (int)ceil(cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC]) - 0.0001);
							string numero = to_string((*it)->CodIdCC);
							string variable = "X_" + numero;
							if (Sub->Pruebas && (*it_c)->items_colocados.count(variable) > 0 && (*it_c)->items_colocados[variable] != valor)
							{
								cout << endl << "Z_" << (*it_c)->CodIdCC;
															cout << "X_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " colocados="<< (*it_c)->items_colocados[variable] << endl;
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
				if ((*it_c)->pcamiones_extra.size() == 0)
					esta_lleno = false;
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
//							cout << endl << "ZP_" << (*it_c)->CodIdCC << "_" << k << " " << (*it_c)->arrival_time;
							cout << endl << "ZP_" << (*it_c)->CodIdCC << "_" << k << " " << (*it_c)->id_truck;

						//if (!(*it_c)->lleno && (*it_c)->items_modelo.size() == 0)
						if (camion_extra != nullptr)
						{
//							cout << "Ya utilizado";
							if (camion_extra->lleno)
							{
								if (Sub->Pruebas)	cout << " LLeno";
							}
							else
							{
								if (Sub->Pruebas)
									cout << " A medias";
							}
							for (auto it = Sub->mapa_camiones[(*it_c)->id_truck].begin(); it != Sub->mapa_camiones[(*it_c)->id_truck].end(); it++)
							{
								if (Sub->matriz_variables[(*it)->CodIdCC][(*it_c)->CodIdCC] == false) continue;
								if ((*it)->num_items == 0)continue;

								if (cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > val_minimo)
								{

									int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
									int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.0001);
									string variable = "XP_" + to_string((*it)->CodIdCC);
									int kk = camion_extra->items_colocados[variable];

									if (camion_extra->items_colocados[variable] < valor)
									{
										if (camion_extra->items_colocados.count(variable) != 0)
											cout << "mete más";
										hay_algo = true;

										//Sub->XP_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k] = valor;
										if (hay_algo == true && Sub->Pruebas)
										{
//																		cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
//																			" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
										}

										int cuantos = 1;
										if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
											cuantos = (*it)->num_items;
										if (((*it)->num_items - valor) < 0)
											cout << "cuidado" << endl;
										int total_dias = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1)
											- Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday + 1);

										//Si el item solo puede ir en ese camon por fecha
//										int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday+1)
//											- Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1);
										int obli = 0;
										if (total_dias == 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
											//										if ( (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
											obli = 1;
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
							if ((hay_algo || Sub->Last_Ramon) && !camion_extra->items_sol.empty())
//							if (hay_algo && !camion_extra->items_sol.empty())
								camion_extra->reanudar_solucion();
//							if (hay_algo)
							if (hay_algo || Sub->Last_Ramon)
							{
								listacam.push_back(camion_extra);
								if (Sub->Pruebas) cout << " Empaquetar ";
							}
						}
						else
						{
							if (Sub->Pruebas) cout << "NO utilizado";
							//Si en el fijo no había puesto
							if ((*it_c)->items_sol.size() == 0 && (*it_c)->items_modelo.size() == 0 && (*it_c)->pcamiones_extra.size() == 0)
							{
//								cout << "ITEMSOL0";
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

									if (cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > val_minimo)
									{

										int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
										int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.0001);
										string variable = "XP_" + to_string((*it)->CodIdCC);
										//if ((*it_c)->items_colocados.count(variable) == 0)
										//{
										if ((*it_c)->items_colocados[variable] < valor)
										{
											if ((*it_c)->items_colocados.count(variable) != 0)
												cout << "mete más";
											hay_algo = true;

											//Sub->XP_ijk[(*it)->CodIdCC][(*it_c)->CodIdCC][k] = valor;
											if (hay_algo == true && Sub->Pruebas)
											{
//																				cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
//																					" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
											}

											int cuantos = 1;
											if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
												cuantos = (*it)->num_items;
											//Si el item solo puede ir en ese camon por fecha
											int total_dias = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1)
												- Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday + 1);

											//											int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday+1)
											//												- Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1);
											int obli = 0;
											//											if (total_dias <= 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
											if ((total_dias == 0) || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
												obli = 1;
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
								if ((hay_algo || Sub->Last_Ramon) && !(*it_c)->items_sol.empty())
//								if (hay_algo && !(*it_c)->items_sol.empty())
									(*it_c)->reanudar_solucion();
//								if (hay_algo)
								if (hay_algo || Sub->Last_Ramon)
								{
									if (Sub->Pruebas) cout << " Empaquetar ";
									listacam.push_back((*it_c));
								}

							}
							else
							{
								if (Sub->Pruebas) cout << "NEWEXTRA";
								if (!(*it_c)->lleno)
								{
									if (Sub->Pruebas)
									cout << "  Juntos";
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

									if (cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) > val_minimo)
									{
										hay_algo = true;
										int order = (*it_c)->orden_ruta[(*it)->sup_cod_dock];
										int valor = (int)ceil(cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) - 0.001);
										int cuantos = 1;
										if (hay_algo == false && Sub->Pruebas)
										{
//																		cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
//																			" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
										}
										if (((*it)->num_items - valor) < 0)
										{
											cout << "cuidado NI " << (*it)->num_items << " Valor " << valor << endl;
										}
										if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
											cuantos = (*it)->num_items;
										//Si el item solo puede ir en ese camon por fecha
										tm kk = (*it)->latest_arrival_tm_ptr;
										int total_dias = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1)
											- Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday + 1);

										int obli = 0;
										if (total_dias < 0)
											int kk2 = 9;
										//Si el item solo puede ir en ese camon por fecha o es el último camión en el que puede ir
//										if (total_dias <= 0 || strcmp((*(*it)->camiones.back()).id_truck, (*it_c)->id_truck) == 0)
										if (total_dias == 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
											obli = 1;
										aux.push_back(item_modelo(*it, cuantos * valor, order, obli));
										//Si el anterior es del mismo tipo lo pongo en el anterior
										if (listacam.size() == 0 || !(listacam.back()->CodIdCC == textra->CodIdCC))
										{

											textra->items_modelo.push_back(item_modelo(*it, cuantos * valor, order, obli));
											textra->num_items_modelo += cuantos * valor;
											textra->peso_total_items_modelo += cuantos * valor * (*it)->weight;
											textra->volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
											textra->total_items_modelos += cuantos * valor;
											if (textra->dim_minima > (*it)->length)textra->dim_minima = (*it)->length;
											if (textra->dim_minima > (*it)->width)textra->dim_minima = (*it)->width;

										}
										else
										{

											listacam.back()->items_modelo.push_back(item_modelo(*it, cuantos * valor, order, obli));
											listacam.back()->num_items_modelo += cuantos * valor;
											listacam.back()->peso_total_items_modelo += cuantos * valor * (*it)->weight;
											listacam.back()->volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
											listacam.back()->total_items_modelos += cuantos * valor;
											if (listacam.back()->dim_minima > (*it)->length) listacam.back()->dim_minima = (*it)->length;
											if (listacam.back()->dim_minima > (*it)->width) listacam.back()->dim_minima = (*it)->width;


										}
										//textra->coste_invetario_items_por_colocar += (*it)->inventory_cost * valor;
										cant_items += cuantos * valor;
										peso_total_items_modelo += cuantos * valor * (*it)->weight;
										volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
									}
								}
								//Si estaba empaquetado y hay que poner más.
								if ((hay_algo || Sub->Last_Ramon) && !textra->items_sol.empty())
//								if (hay_algo && !textra->items_sol.empty())
									textra->reanudar_solucion();
								if (hay_algo || Sub->Last_Ramon)
//								if (hay_algo )
								{
									if (Sub->Pruebas) cout << " Empaquetar ";
									listacam.push_back(textra);
								}

								(*it_c)->peso_total_items_modelo_extra.push_back(peso_total_items_modelo);
								(*it_c)->volumen_total_items_modelo_items_extra.push_back(volumen_total_items_modelo);
								(*it_c)->items_extras_modelo.push_back(aux);
								(*it_c)->num_items_extra.push_back(cant_items);
								(*it_c)->total_items_modelos += cant_items;
								//cout << "Titems:" << (*it_c)->num_items_modelo << endl;
								num_cam_extra_tipo++;
							}
						}


					}
				}
				else
				{
					if (Sub->Pruebas)
						cout << endl << "ZP_" << (*it_c)->CodIdCC << "_" << k << " " << (*it_c)->arrival_time << " " << (*it_c)->id_truck << " Lleno";


				}

				if (num_cam_extra_tipo > Sub->max_cam_extra_tipo_modelo)
					Sub->max_cam_extra_tipo_modelo = num_cam_extra_tipo;
			}

		}

	}
}
void ConvertirSolucionModeloACamiones4(Subproblema* Sub, IloCplex* cplex, NumVarMatrix2F* x_ij, IloNumVarArray* z_j, NumVarMatrix3F* xp_ijk, NumVarMatrix2F* zp_jk, long long int fin, list<truck*>& listacam)
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
	int i = 0;

	//Extraer los valores de X
	if (Sub->Pruebas)cout << "Modelo:" << endl;
	for (auto it_c = Sub->lista_camiones.begin(); it_c != Sub->lista_camiones.end(); it_c++)
	{
		//TODO cambiar el del primer día igual break
		if ((*it_c)->arrival_time <= fin)
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

		//Si camión no esta lleno
		if (!(*it_c)->lleno)
		{
			if (cplex->isExtracted((*z_j)[(*it_c)->CodIdCC]) && cplex->getValue((*z_j)[(*it_c)->CodIdCC], i) > 0.999)
			{

				bool hay_algo = false;
				Sub->Z_j[(*it_c)->CodIdCC] = true;
				//					if (Sub->Pruebas) cout << "Z_" <<  (*it_c)->CodIdCC << " " << (*it_c)->arrival_time << endl;
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

							int cuantos = 1;
							if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
								cuantos = (*it)->num_items;
							//ver si la fecha limite del item con la del camión. si coinciden el item tiene que ir en ese
							int obli = 0;
							//								if (total_dias <= 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
							if ((*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
								obli = 1;
							(*it_c)->items_modelo.push_back(item_modelo(*it, cuantos * valor, order, obli));
							(*it_c)->num_items_modelo += cuantos * valor;
							(*it_c)->peso_total_items_modelo += cuantos * valor * (*it)->weight;
							(*it_c)->volumen_total_items_modelo += cuantos * valor * (*it)->volumen;
							if ((*it_c)->dim_minima > (*it)->length)(*it_c)->dim_minima = (*it)->length;
							if ((*it_c)->dim_minima > (*it)->width)(*it_c)->dim_minima = (*it)->width;

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


		int num_cam_extra_tipo = 0;
		bool esta_lleno = false;

		for (int k = 0; k < (*it_c)->cota_extra; k++)
		{
			truck* camion_extra = nullptr;
			//Compruebo que el camion no se ha llenado
			esta_lleno = false;
			if ((*it_c)->pcamiones_extra.size() == 0) esta_lleno = false;
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

									int cuantos = 1;
									if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
										cuantos = (*it)->num_items;
									int obli = 0;
									if ((*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
										obli = 1;
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
						//Si en el fijo no hay nada puesto pero en el extra si, entonces hay que ponerlo en el fijo
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

										int cuantos = 1;
										if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
											cuantos = (*it)->num_items;
										int obli = 0;
										if ((*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
											obli = 1;
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
									if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
										cuantos = (*it)->num_items;
									//Si el item solo puede ir en ese camon por fecha

									int obli = 0;

									//Si el item solo puede ir en ese camon por fecha o es el último camión en el que puede ir
//										if (total_dias <= 0 || strcmp((*(*it)->camiones.back()).id_truck, (*it_c)->id_truck) == 0)
									if ((*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
										obli = 1;
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


				}
			}

			if (num_cam_extra_tipo > Sub->max_cam_extra_tipo_modelo)
				Sub->max_cam_extra_tipo_modelo = num_cam_extra_tipo;
		}

	}


}
void ConvertirSolucionModeloACamionesTodoCamion1(Subproblema* Sub, IloCplex* cplex, NumVarMatrix2F* x_ij, IloNumVarArray* z_j, NumVarMatrix3F* xp_ijk, NumVarMatrix2F* zp_jk, long long int fin, list<truck*>& listacam)
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
			//TODO cambiar el del primer día igual break
			if ((*it_c)->arrival_time <= fin)
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
					//					if (Sub->Pruebas) cout << "Z_" <<  (*it_c)->CodIdCC << " " << (*it_c)->arrival_time << endl;
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
									//								cout << "X_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:"<< cplex->getValue((*x_ij)[(*it)->CodIdCC][(*it_c)->CodIdCC])<<
									//									" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
								}
								//if (hay_algo == false && Sub->Pruebas)
								//{
								//	if ((*it)->num_items < valor)
								//		int kk = 0;
								//}
								//Sub->X_ij[(*it)->CodIdCC][(*it_c)->CodIdCC] = valor;
								int cuantos = 1;
								if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
									cuantos = (*it)->num_items;
								//ver si la fecha limite del item con la del camión. si coinciden el item tiene que ir en ese
//								int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday+1)
//									- Days_0001((*it)->earliest_arrival_tm_ptr.tm_year, (*it)->earliest_arrival_tm_ptr.tm_mon + 1, (*it)->earliest_arrival_tm_ptr.tm_mday + 1);
								int total_dias = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1)
									- Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday + 1);

								//int diac = (*it_c)->arrival_time;
								//long long int diai = (*it)->earliest_arrival;
								int obli = 0;
								if (total_dias == 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
									obli = 1;
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
							if (Sub->Pruebas && (*it_c)->items_colocados.count(variable) > 0 && (*it_c)->items_colocados[variable] != valor)
							{
								//							cout << "Z_" << (*it_c)->CodIdCC << endl;
								//							cout << "X_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " colocados="<< (*it_c)->items_colocados[variable] << endl;
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
//						if (Sub->Pruebas)
//							cout << "ZP_" << (*it_c)->CodIdCC << "_"<< k << " " << (*it_c)->arrival_time <<endl;

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
											//							cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
										//									" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
										}

										int cuantos = 1;
										if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
											cuantos = (*it)->num_items;
										//Si el item solo puede ir en ese camon por fecha
//										int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday+1)
//											- Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1);
										int total_dias = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1)
											- Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday + 1);

										int obli = 0;
										if (total_dias == 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
											obli = 1;
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
												//								cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
												//									" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
											}

											int cuantos = 1;
											if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
												cuantos = (*it)->num_items;
											//Si el item solo puede ir en ese camon por fecha
											int total_dias = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1)
												- Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday + 1);

											//											int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday+1)
											//												- Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1);
											int obli = 0;
											if (total_dias == 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
												obli = 1;
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
											//							cout << "XP_" << (*it)->CodIdCC << "_" << (*it_c)->CodIdCC << "=" << valor << " cplex:" << cplex->getValue((*xp_ijk)[(*it)->CodIdCC][(*it_c)->CodIdCC][k]) <<
											//								" - hay:" << (*it)->num_items << "E:" << (*it)->earliest_arrival << " L:" << (*it)->latest_arrival << endl;
										}

										if (Sub->items_muchas_unidades[(*it)->CodIdCC] == false)
											cuantos = (*it)->num_items;
										//Si el item solo puede ir en ese camon por fecha
//										int total_dias = Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday+1)
//											- Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1);
										int total_dias = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday + 1)
											- Days_0001((*it_c)->arrival_time_tm_ptr.tm_year, (*it_c)->arrival_time_tm_ptr.tm_mon + 1, (*it_c)->arrival_time_tm_ptr.tm_mday + 1);

										int obli = 0;
										//Si el item solo puede ir en ese camon por fecha o es el último camión en el que puede ir
										if (total_dias == 0 || (*(*it)->camiones.back()).CodIdCC == (*it_c)->CodIdCC)
											obli = 1;
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


					}
				}

				if (num_cam_extra_tipo > Sub->max_cam_extra_tipo_modelo)
					Sub->max_cam_extra_tipo_modelo = num_cam_extra_tipo;
			}

		}

	}
}