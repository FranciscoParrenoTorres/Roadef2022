#include "Problema.h"

Problema::Problema()
{
	ObjBound = 0;
	ObjMip = 0;
	TiempoModelo = 0;
	num_camiones_modelo = 0;
	num_camiones_extra_modelo = 0;
	max_cam_extra_tipo_modelo = 0;
	total_items_cuadrado = 0;
	coste_transporte = 0;
	coste_inventario = 0;
	coste_total = 0;
	for (int i = 0; i < MAX_ITER_MODELO; i++)
	{
		pair<int, int> p(0, 0);
		veces_cargocamion.push_back(p);
	}
	for (int i = 0; i < MAX_ITER_MODELO; i++)
	{
		vector<double> p(5);
		cargocamion_char.push_back(p);
	}
}

Problema::~Problema()
{
	subproblemas.clear();
}
void Problema::PrintEstadisticas()
{
	for (int i = 1; i < MAX_ITER_MODELO && veces_cargocamion[i].first > 0; i++)
	{
		 
		cout << ";" << i << ";";
		cout << veces_cargocamion[i].first << ";";
		cout <<  veces_cargocamion[i].second << ";";
		cout << setprecision(8) << cargocamion_char[i][0] << ";";
		cout << setprecision(8) << cargocamion_char[i][1] << ";";
		cout << setprecision(8) << cargocamion_char[i][2] << ";";
		cout << setprecision(8) << cargocamion_char[i][3] << ";";
	}
}
void Problema::ActualizarEstadisticas(Subproblema* sub)
{
	
	for (int i = 0; i < MAX_ITER_MODELO ; i++)
	{
		veces_cargocamion[i].first += sub->veces_cargocamion[i].first;
		veces_cargocamion[i].second += sub->veces_cargocamion[i].second;
		if (veces_cargocamion[i].second > 1)
		{
			cargocamion_char[i][0] = (((double)(veces_cargocamion[i].second - sub->veces_cargocamion[i].second) * cargocamion_char[i][0]) + ((double)(sub->veces_cargocamion[i].second)*sub->cargocamion_char[i][0])) / ((double)veces_cargocamion[i].second);
			cargocamion_char[i][1] = (((double)(veces_cargocamion[i].second - sub->veces_cargocamion[i].second) * cargocamion_char[i][1]) + ((double)(sub->veces_cargocamion[i].second) * sub->cargocamion_char[i][1])) / ((double)veces_cargocamion[i].second);
			cargocamion_char[i][2] = (((double)(veces_cargocamion[i].second - sub->veces_cargocamion[i].second) * cargocamion_char[i][2]) + ((double)(sub->veces_cargocamion[i].second) * sub->cargocamion_char[i][2])) / ((double)veces_cargocamion[i].second);
			cargocamion_char[i][3] = (((double)(veces_cargocamion[i].second - sub->veces_cargocamion[i].second) * cargocamion_char[i][3]) + ((double)(sub->veces_cargocamion[i].second) * sub->cargocamion_char[i][3])) / ((double)veces_cargocamion[i].second);

		}
		else
		{
			cargocamion_char[i][0] =sub->cargocamion_char[i][0];
			cargocamion_char[i][1] = sub->cargocamion_char[i][1];
			cargocamion_char[i][2] = sub->cargocamion_char[i][2];
			cargocamion_char[i][3] = sub->cargocamion_char[i][3];
		}
		
	}
}
void Problema::reset()
{
	ObjBound = 0;
	ObjMip = 0;
	num_camiones_modelo = 0;
	num_camiones_extra_modelo = 0;
	max_cam_extra_tipo_modelo = 0;
	coste_transporte = 0;
	coste_inventario = 0;
	coste_total = 0;
	
}
void Problema::Insertar_camiones_ordenado_por_fechas(Subproblema *sub, truck *itcam)
{
	if (sub->lista_camiones.empty())
	{
		sub->lista_camiones.push_back(itcam);
	}
	else
	{
		bool insertado = false;
		if(sub->lista_camiones.back()->arrival_time>(*itcam).arrival_time)
			sub->lista_camiones.push_back(itcam);
		else
		{
			for (auto t = sub->lista_camiones.begin(); t != sub->lista_camiones.end() && !insertado; t++)
			{
				if ((*t)->arrival_time < (*itcam).arrival_time)
				{
					if (t == sub->lista_camiones.begin())
					{
						sub->lista_camiones.insert(sub->lista_camiones.begin(), itcam);
						insertado = true;
					}
					else
					{
						//t--;
						sub->lista_camiones.insert(t, itcam);
						insertado = true;
					}
					break;
				}
			}
			if (!insertado)
				sub->lista_camiones.push_back(itcam);
		}
	}
}
Problema::Problema(timeb tini, int  tmax, bool prueba)
{
	ObjBound = 0;
	ObjMip = 0;
	TiempoModelo = 0;
	num_camiones_modelo = 0;
	num_camiones_extra_modelo = 0;
	max_cam_extra_tipo_modelo = 0;
	TiempoInicio = tini;
	TiempoTotal = tmax*2;
	Pruebas = prueba;

	TiempoModelo = (tmax-subproblemas.size()) / 12;
	Total_Iter_Packing = 50;


	//inicializo la mejor solucion
	mejor_solucion.coste_total = 0;
	mejor_solucion.coste_inventario = 0;
	mejor_solucion.coste_transporte = 0;
	coste_inventario = 0;
	coste_total = 0;
	coste_transporte = 0;
	veces_pila_cambia = 0;
	veces_pila = 0;

}
void Problema::Add_componente(map<int, list<string>> componentes, map <string, truck*> mapa_camiones, map<string, list<item*>> cam_item)
{

	map<int, list<string>>::iterator itc = componentes.begin();
	int contc = -1;
	
	//Para cada componente conexa creo un subproblema
	for (itc; itc != componentes.end(); itc++)
	{
		
		map<string, int> items_metidos;
	//	contc++;
		contc = (*itc).first;
		Subproblema* sub = new Subproblema();
		sub->Todas_Enteras = this->Todos_Enteras;
		sub->Pruebas = this->Pruebas;
		sub->Last_Ramon = this->Last_Ramon;
		sub->Fijar = this->Fijar;
		sub->Todos_Relax= this->Todos_Relax;
		sub->Por_Camion	= this->Por_Camion;
		sub->CodS = contc;
		list<string>::iterator itp = (*itc).second.begin();

		int cont_item = 0;
		int cont_cam = 0;

		for (itp; itp != (*itc).second.end(); itp++)
		{
			//truck* t = mapa_camiones[*itp];
						
			//Creo la lista de items

			for (auto iti= cam_item[*itp].begin(); iti != cam_item[*itp].end(); iti++)
			{
				
				if (items_metidos.count((*iti)->Id) == 0)
				{
					items_metidos.insert(pair<string, int>((*iti)->Id,(*iti)->CodId));
				}
				else
				{
					continue;
				}
				(*iti)->CodIdCC = cont_item;
				cont_item++;
				//Los voy a incluir en orden de detras a delante
				//inserto items ordenados por fecha
				auto it_i = sub->lista_items.begin();
				for (; it_i != sub->lista_items.end() && (*it_i)->latest_arrival > (*iti)->latest_arrival; it_i++)
				{

				}
				sub->lista_items.insert(it_i, *iti);
				//				sub->lista_items.push_back(*iti);
								//Calculo el minimo de ancho y de largo de una pieza en este subproblema
								//Si la orientación 0, none, 1, lengthwise, 2, withwise
				if ((*iti)->forced_orientation_int == 0)
				{
					if (min((*iti)->length, (*iti)->width) < sub->min_dim_length)
						sub->min_dim_length = min((*iti)->length, (*iti)->width);
					if (min((*iti)->length, (*iti)->width) < sub->min_dim_width)
						sub->min_dim_width = min((*iti)->length, (*iti)->width);
				}
				else
				{
					if ((*iti)->forced_orientation_int == 1)
					{
						if ((*iti)->length < sub->min_dim_length)
							sub->min_dim_length = (*iti)->length;
						if ((*iti)->width < sub->min_dim_width)
							sub->min_dim_width = (*iti)->width;
					}
					else
					{
						if ((*iti)->width < sub->min_dim_length)
							sub->min_dim_length = (*iti)->width;
						if ((*iti)->length < sub->min_dim_width)
							sub->min_dim_width = (*iti)->length;
					}
				}
				total_volumen += (*iti)->num_items * (*iti)->vol_con_nest;
				total_peso+= (*iti)->num_items * (*iti)->weight;

				sub->total_items += (*iti)->num_items;

				//Creo la lista de camiones				
				for (list<truck*>::iterator itcam = (*iti)->camiones.begin(); itcam != (*iti)->camiones.end(); itcam++)
				{

					if (sub->lista_camiones.empty() || sub->mapa_camiones.count((*itcam)->id_truck) == 0)
					{
						(*itcam)->CodIdCC = cont_cam; 
						(*itcam)->lcompo.push_back(contc);
						cont_cam++;
						//inserto ordenado por fechas
						//Insertar_camiones_ordenado_por_fechas(sub,*itcam);
						//Los voy a incluir en orden de detras a delante
						auto it_c = sub->lista_camiones.begin();
						for (; it_c != sub->lista_camiones.end() && (*it_c)->arrival_time > (*itcam)->arrival_time; it_c++)
						{

						}
						sub->lista_camiones.insert(it_c, *itcam);
						//					sub->lista_camiones.push_back(*itcam);
						list<item*> aux;
						aux.push_back((*iti));
						sub->mapa_camiones.insert(pair<string, list<item*>>((*itcam)->id_truck, aux));
					}
					//inserto el item que va en ese camion.
					else
					{
						sub->mapa_camiones[(*itcam)->id_truck].push_back(*iti);
					}

				}
			}
		}
		sub->Total_Posibles_Asignaciones = 0;
		for (auto ic = sub->lista_camiones.begin(); ic != sub->lista_camiones.end(); ic++)
		{
			(*ic)->items_originales.clear();
			(*ic)->items.sort(item::comparador);
			std::copy((*ic)->items.begin(), (*ic)->items.end(), std::back_inserter((*ic)->items_originales));
			//			(*ic)->items_originales.insert( (*ic)->items.begin(), (*ic)->items.end());


		}
		for (auto it = sub->lista_items.begin(); it != sub->lista_items.end(); it++)
		{

			for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
			{
//				sub->Total_Posibles_Asignaciones += ((*it)->num_items) * ((*it_c)->cota_extra);
				sub->Total_Posibles_Asignaciones +=  ((*it_c)->cota_extra);
			}
			
			(*it)->camiones.sort(truck::comparador);
		

		}


		cont_cam = 0;
		for (auto itc2 = sub->lista_camiones.begin(); itc2 != sub->lista_camiones.end(); itc2++, cont_cam++)
		{
			(*itc2)->CodIdCC = cont_cam;

		}
		cont_item = 0;
		int tot_items = 0;
		for (auto itc2 = sub->lista_items.begin(); itc2 != sub->lista_items.end(); itc2++, cont_item++)
		{
			(*itc2)->CodIdCC = cont_item;
			tot_items += (*itc2)->num_items;
		}
		//if (sub->total_items != tot_items)
		//{
		//	int kk = 0;
		//}
		if (this->subproblemas.size() == 190)
			int kk = 9;
		total_items_cuadrado += (int)pow(sub->total_items, 2) / 100;
		total_items_camiones += sub->Total_Posibles_Asignaciones;
		//Añado el subproblema al vector de problema.
		this->subproblemas.push_back(sub);
		if (Pruebas)
		{
			sub->max_med_trucks();
			sub->max_med_items();
		}

	}
	//Calculamos cunato puedo llenar cada camion
	for (auto itS = subproblemas.begin(); itS != subproblemas.end(); itS++)
	{
		for (auto itC = (*itS)->lista_camiones.begin(); itC != (*itS)->lista_camiones.end(); itC++)
		{

			for (auto iti = (*itC)->items.begin(); iti != (*itC)->items.end(); iti++)
			{
				(*itC)->items_modelo.push_back(item_modelo(*iti, (*iti)->num_items, 1,false));
				(*itC)->num_items_modelo += (*iti)->num_items;
				(*itC)->total_items_modelos += (*iti)->num_items;
				(*itC)->peso_total_items_modelo += (*iti)->num_items * (*iti)->weight;
				(*itC)->volumen_total_items_modelo += (*iti)->num_items * (*iti)->volumen;
				if ((*itC)->dim_minima > (*iti)->length)(*itC)->dim_minima = (*iti)->length;
				if ((*itC)->dim_minima > (*iti)->width)(*itC)->dim_minima = (*iti)->width;
			}
			vector < item_modelo> best_cantidad_items = (*itC)->items_modelo;
			//Creo el camion y lo añado a la solución
			truck_sol camion((*itC)->id_truck);
			camion.volumen_max = (*itC)->volumen;
			camion.peso_max = (*itC)->max_loading_weight;
			(*itS)->sol.listado_camiones.push_back(camion);
			(*itS)->Total_Iter_Packing = 20;
			//TODO
//			(*itS)->CargoCamion((*itC),(*itC)->items_modelo,(*itC)->id_truck,(*itC)->num_items_modelo,0,1);
//			(*itC)->volumen_inicial = (*itC)->volumen_ocupado;
			(*itC)->volumen_inicial = (*itC)->volumen_kp;
			auto it2 = (*itC)->items.begin();
			for (auto it1 = best_cantidad_items.begin(); it1 != best_cantidad_items.end(); it1++, it2++)
			{
				(*it2)->num_items = it1->num_items;
			}

			(*itC)->reset();

			for (auto iti = (*itC)->items.begin(); iti != (*itC)->items.end(); iti++)
			{
				(*itC)->items_modelo.push_back(item_modelo(*iti, (*iti)->num_items, 1,false));
				(*itC)->num_items_modelo += (*iti)->num_items;
				(*itC)->total_items_modelos += (*iti)->num_items;
				(*itC)->peso_total_items_modelo += (*iti)->num_items * (*iti)->weight;
				(*itC)->volumen_total_items_modelo += (*iti)->num_items * (*iti)->volumen;
				if ((*itC)->dim_minima > (*iti)->length)(*itC)->dim_minima = (*iti)->length;
				if ((*itC)->dim_minima > (*iti)->width)(*itC)->dim_minima = (*iti)->width;
			}
			//Creo el camion y lo añado a la solución
			//truck_sol camion((*itC)->id_truck);
			(*itS)->sol.listado_camiones.push_back(camion);
			(*itS)->Total_Iter_Packing = 20;
			//TODO
//			(*itS)->CargoCamion((*itC), (*itC)->items_modelo, (*itC)->id_truck, (*itC)->num_items_modelo, 0, 2);
//			(*itC)->peso_inicial = (*itC)->peso_cargado;
			(*itC)->peso_inicial = (*itC)->max_loading_weight;

			it2 = (*itC)->items.begin();
			for (auto it1 = best_cantidad_items.begin(); it1 != best_cantidad_items.end(); it1++, it2++)
			{
				(*it2)->num_items = it1->num_items;
			}

			(*itC)->reset();
		}
		(*itS)->reset();
	}



	//Calcular cota heurística

	for (int i = 0; i < MAX_ITER_MODELO; i++)
	{
		pair<int, int> p(0, 0);
		veces_cargocamion.push_back(p);
	}
	for (int i = 0; i < MAX_ITER_MODELO; i++)
	{
		vector<double> p(5);
		cargocamion_char.push_back(p);
	}
	mejor_solucion.solucion_compo.clear();
	for (auto it = subproblemas.begin(); it != subproblemas.end(); it++)
	{
		//Minimo de ancho de los camioens
		for (auto itc2 = (*it)->lista_camiones.begin(); itc2 != (*it)->lista_camiones.end(); itc2++)
		{
			for (auto it_it = (*itc2)->items.begin(); it_it != (*itc2)->items.end(); it_it++)
			{
				if ((*it_it)->forced_orientation_int == 0)
				{
					if (min((*it_it)->length, (*it_it)->width) < (*itc2)->min_dim_length)
						(*itc2)->min_dim_length = min((*it_it)->length, (*it_it)->width);
					if (min((*it_it)->length, (*it_it)->width) < (*itc2)->min_dim_width)
						(*itc2)->min_dim_width = min((*it_it)->length, (*it_it)->width);
				}
				else
				{
					if ((*it_it)->forced_orientation_int == 1)
					{
						if ((*it_it)->length < (*itc2)->min_dim_length)
							(*itc2)->min_dim_length = (*it_it)->length;
						if ((*it_it)->width < (*it)->min_dim_width)
							(*itc2)->min_dim_width = (*it_it)->width;
					}
					else
					{
						if ((*it_it)->width < (*it)->min_dim_length)
							(*itc2)->min_dim_length = (*it_it)->width;
						if ((*it_it)->length < (*it)->min_dim_width)
							(*itc2)->min_dim_width = (*it_it)->length;
					}
				}
			}
			//int kk = (*itc2)->min_dim_length;
			//int jj = (*itc2)->min_dim_width;
			//int ki = 9;


		}
		solucion sol;
		(*it)->Pruebas = Pruebas;
		mejor_solucion.solucion_compo.push_back(sol);
	}
}
void Problema::Add_componente2(map<int, list<string>> componentes, map <string, truck*> mapa_camiones, map<string, list<item*>> cam_item)
{

	map<int, list<string>>::iterator itc = componentes.begin();
	int contc = -1;

	//Para cada componente conexa creo un subproblema
	for (itc; itc != componentes.end(); itc++)
	{

		map<string, int> items_metidos;
		//	contc++;
		contc = (*itc).first;
		Subproblema* sub = new Subproblema();
		sub->CodS = contc;
		list<string>::iterator itp = (*itc).second.begin();

		int cont_item = 0;
		int cont_cam = 0;

		for (itp; itp != (*itc).second.end(); itp++)
		{
			//truck* t = mapa_camiones[*itp];

			//Creo la lista de items

			for (auto iti = cam_item[*itp].begin(); iti != cam_item[*itp].end(); iti++)
			{

				if (items_metidos.count((*iti)->Id) == 0)
				{
					items_metidos.insert(pair<string, int>((*iti)->Id, (*iti)->CodId));
				}
				else
				{
					continue;
				}
				(*iti)->CodIdCC = cont_item;
				cont_item++;
				//Los voy a incluir en orden de detras a delante
				//inserto items ordenados por fecha
				auto it_i = sub->lista_items.begin();
				for (; it_i != sub->lista_items.end() && (*it_i)->latest_arrival > (*iti)->latest_arrival; it_i++)
				{

				}
				sub->lista_items.insert(it_i, *iti);
				//				sub->lista_items.push_back(*iti);
								//Calculo el minimo de ancho y de largo de una pieza en este subproblema
								//Si la orientación 0, none, 1, lengthwise, 2, withwise
				if ((*iti)->forced_orientation_int == 0)
				{
					if (min((*iti)->length, (*iti)->width) < sub->min_dim_length)
						sub->min_dim_length = min((*iti)->length, (*iti)->width);
					if (min((*iti)->length, (*iti)->width) < sub->min_dim_width)
						sub->min_dim_width = min((*iti)->length, (*iti)->width);
				}
				else
				{
					if ((*iti)->forced_orientation_int == 1)
					{
						if ((*iti)->length < sub->min_dim_length)
							sub->min_dim_length = (*iti)->length;
						if ((*iti)->width < sub->min_dim_width)
							sub->min_dim_width = (*iti)->width;
					}
					else
					{
						if ((*iti)->width < sub->min_dim_length)
							sub->min_dim_length = (*iti)->width;
						if ((*iti)->length < sub->min_dim_width)
							sub->min_dim_width = (*iti)->length;
					}
				}
				total_volumen += (*iti)->num_items * (*iti)->vol_con_nest;
				total_peso += (*iti)->num_items * (*iti)->weight;

				sub->total_items += (*iti)->num_items;

				//Creo la lista de camiones				
				for (list<truck*>::iterator itcam = (*iti)->camiones.begin(); itcam != (*iti)->camiones.end(); itcam++)
				{

					if (sub->lista_camiones.empty() || sub->mapa_camiones.count((*itcam)->id_truck) == 0)
					{
						(*itcam)->CodIdCC = cont_cam;
						(*itcam)->lcompo.push_back(contc);
						cont_cam++;
						//inserto ordenado por fechas
						//Insertar_camiones_ordenado_por_fechas(sub,*itcam);
						//Los voy a incluir en orden de detras a delante
						auto it_c = sub->lista_camiones.begin();
						for (; it_c != sub->lista_camiones.end() && (*it_c)->arrival_time > (*itcam)->arrival_time; it_c++)
						{

						}
						sub->lista_camiones.insert(it_c, *itcam);
						//					sub->lista_camiones.push_back(*itcam);
						list<item*> aux;
						aux.push_back((*iti));
						sub->mapa_camiones.insert(pair<string, list<item*>>((*itcam)->id_truck, aux));
					}
					//inserto el item que va en ese camion.
					else
					{
						sub->mapa_camiones[(*itcam)->id_truck].push_back(*iti);
					}

				}
			}
		}
		sub->Total_Posibles_Asignaciones = 0;
		for (auto ic = sub->lista_camiones.begin(); ic != sub->lista_camiones.end(); ic++)
		{
			(*ic)->items_originales.clear();
			(*ic)->items.sort(item::comparador);
			std::copy((*ic)->items.begin(), (*ic)->items.end(), std::back_inserter((*ic)->items_originales));
			//			(*ic)->items_originales.insert( (*ic)->items.begin(), (*ic)->items.end());


		}
		for (auto it = sub->lista_items.begin(); it != sub->lista_items.end(); it++)
		{

			for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
			{
				//				sub->Total_Posibles_Asignaciones += ((*it)->num_items) * ((*it_c)->cota_extra);
				sub->Total_Posibles_Asignaciones += ((*it_c)->cota_extra);
			}

			(*it)->camiones.sort(truck::comparador);


		}


		cont_cam = 0;
		for (auto itc2 = sub->lista_camiones.begin(); itc2 != sub->lista_camiones.end(); itc2++, cont_cam++)
		{
			(*itc2)->CodIdCC = cont_cam;

		}
		cont_item = 0;
		int tot_items = 0;
		for (auto itc2 = sub->lista_items.begin(); itc2 != sub->lista_items.end(); itc2++, cont_item++)
		{
			(*itc2)->CodIdCC = cont_item;
			tot_items += (*itc2)->num_items;
		}
		//if (sub->total_items != tot_items)
		//{
		//	int kk = 0;
		//}
		if (this->subproblemas.size() == 190)
			int kk = 9;
		total_items_cuadrado += (int)pow(sub->total_items, 2) / 100;
		total_items_camiones += sub->Total_Posibles_Asignaciones;
		//Añado el subproblema al vector de problema.
		this->subproblemas.push_back(sub);
		if (Pruebas)
		{
			sub->max_med_trucks();
			sub->max_med_items();
		}

	}
	//Calculamos cunato puedo llenar cada camion
//	for (auto itS = subproblemas.begin(); itS != subproblemas.end(); itS++)
//	{
//		for (auto itC = (*itS)->lista_camiones.begin(); itC != (*itS)->lista_camiones.end(); itC++)
//		{
//
//			for (auto iti = (*itC)->items.begin(); iti != (*itC)->items.end(); iti++)
//			{
//				(*itC)->items_modelo.push_back(item_modelo(*iti, (*iti)->num_items, 1, false));
//				(*itC)->num_items_modelo += (*iti)->num_items;
//				(*itC)->total_items_modelos += (*iti)->num_items;
//				(*itC)->peso_total_items_modelo += (*iti)->num_items * (*iti)->weight;
//				(*itC)->volumen_total_items_modelo += (*iti)->num_items * (*iti)->volumen;
//				if ((*itC)->dim_minima > (*iti)->length)(*itC)->dim_minima = (*iti)->length;
//				if ((*itC)->dim_minima > (*iti)->width)(*itC)->dim_minima = (*iti)->width;
//			}
//			vector < item_modelo> best_cantidad_items = (*itC)->items_modelo;
//			//Creo el camion y lo añado a la solución
//			truck_sol camion((*itC)->id_truck);
//			camion.volumen_max = (*itC)->volumen;
//			camion.peso_max = (*itC)->max_loading_weight;
//			(*itS)->sol.listado_camiones.push_back(camion);
//			(*itS)->Total_Iter_Packing = 20;
//			//TODO
////			(*itS)->CargoCamion((*itC),(*itC)->items_modelo,(*itC)->id_truck,(*itC)->num_items_modelo,0,1);
////			(*itC)->volumen_inicial = (*itC)->volumen_ocupado;
//			(*itC)->volumen_inicial = (*itC)->volumen_kp;
//			auto it2 = (*itC)->items.begin();
//			for (auto it1 = best_cantidad_items.begin(); it1 != best_cantidad_items.end(); it1++, it2++)
//			{
//				(*it2)->num_items = it1->num_items;
//			}
//
//			(*itC)->reset();
//
//			for (auto iti = (*itC)->items.begin(); iti != (*itC)->items.end(); iti++)
//			{
//				(*itC)->items_modelo.push_back(item_modelo(*iti, (*iti)->num_items, 1, false));
//				(*itC)->num_items_modelo += (*iti)->num_items;
//				(*itC)->total_items_modelos += (*iti)->num_items;
//				(*itC)->peso_total_items_modelo += (*iti)->num_items * (*iti)->weight;
//				(*itC)->volumen_total_items_modelo += (*iti)->num_items * (*iti)->volumen;
//				if ((*itC)->dim_minima > (*iti)->length)(*itC)->dim_minima = (*iti)->length;
//				if ((*itC)->dim_minima > (*iti)->width)(*itC)->dim_minima = (*iti)->width;
//			}
//			//Creo el camion y lo añado a la solución
//			//truck_sol camion((*itC)->id_truck);
//			(*itS)->sol.listado_camiones.push_back(camion);
//			(*itS)->Total_Iter_Packing = 20;
//			//TODO
////			(*itS)->CargoCamion((*itC), (*itC)->items_modelo, (*itC)->id_truck, (*itC)->num_items_modelo, 0, 2);
////			(*itC)->peso_inicial = (*itC)->peso_cargado;
//			(*itC)->peso_inicial = (*itC)->max_loading_weight;
//
//			it2 = (*itC)->items.begin();
//			for (auto it1 = best_cantidad_items.begin(); it1 != best_cantidad_items.end(); it1++, it2++)
//			{
//				(*it2)->num_items = it1->num_items;
//			}
//
//			(*itC)->reset();
//		}
//		(*itS)->reset();
//	}



	//Calcular cota heurística

	//for (int i = 0; i < MAX_ITER_MODELO; i++)
	//{
	//	pair<int, int> p(0, 0);
	//	veces_cargocamion.push_back(p);
	//}
	//for (int i = 0; i < MAX_ITER_MODELO; i++)
	//{
	//	vector<double> p(5);
	//	cargocamion_char.push_back(p);
	//}
	//mejor_solucion.solucion_compo.clear();
	//for (auto it = subproblemas.begin(); it != subproblemas.end(); it++)
	//{
	//	//Minimo de ancho de los camioens
	//	for (auto itc2 = (*it)->lista_camiones.begin(); itc2 != (*it)->lista_camiones.end(); itc2++)
	//	{
	//		for (auto it_it = (*itc2)->items.begin(); it_it != (*itc2)->items.end(); it_it++)
	//		{
	//			if ((*it_it)->forced_orientation_int == 0)
	//			{
	//				if (min((*it_it)->length, (*it_it)->width) < (*itc2)->min_dim_length)
	//					(*itc2)->min_dim_length = min((*it_it)->length, (*it_it)->width);
	//				if (min((*it_it)->length, (*it_it)->width) < (*itc2)->min_dim_width)
	//					(*itc2)->min_dim_width = min((*it_it)->length, (*it_it)->width);
	//			}
	//			else
	//			{
	//				if ((*it_it)->forced_orientation_int == 1)
	//				{
	//					if ((*it_it)->length < (*itc2)->min_dim_length)
	//						(*itc2)->min_dim_length = (*it_it)->length;
	//					if ((*it_it)->width < (*it)->min_dim_width)
	//						(*itc2)->min_dim_width = (*it_it)->width;
	//				}
	//				else
	//				{
	//					if ((*it_it)->width < (*it)->min_dim_length)
	//						(*itc2)->min_dim_length = (*it_it)->width;
	//					if ((*it_it)->length < (*it)->min_dim_width)
	//						(*itc2)->min_dim_width = (*it_it)->length;
	//				}
	//			}
	//		}
	//		//int kk = (*itc2)->min_dim_length;
	//		//int jj = (*itc2)->min_dim_width;
	//		//int ki = 9;


	//	}
	//	solucion sol;
	//	(*it)->Pruebas = Pruebas;
	//	mejor_solucion.solucion_compo.push_back(sol);
	//}
}
Problema::Problema(map<int, list<string>> componentes, map <string, list<item*>>& proveedor_items, timeb tini,int  tmax,bool prueba)
{
	ObjBound = 0;
	ObjMip = 0;
	TiempoModelo = 0;
	num_camiones_modelo = 0;
	num_camiones_extra_modelo = 0;
	max_cam_extra_tipo_modelo = 0;
	TiempoInicio = tini;
	TiempoTotal = tmax;
	Pruebas = prueba;
	
	Total_Iter_Packing = 20;
	

	//inicializo la mejor solucion
	mejor_solucion.coste_total = 0;
	mejor_solucion.coste_inventario = 0;
	mejor_solucion.coste_transporte = 0;
	coste_inventario = 0;
	coste_total = 0;
	coste_transporte = 0;
	veces_pila_cambia = 0;
	veces_pila = 0;

	map<int, list<string>>::iterator itc = componentes.begin();
	int contc = 0;
	//Para cada componente conexa creo un subproblema
	for (itc; itc != componentes.end(); itc++)
	{
		contc++;
		

		Subproblema* sub = new Subproblema();
		list<string>::iterator itp = (*itc).second.begin();
		
		int cont_item = 0;
		int cont_cam = 0;

		for (itp; itp != (*itc).second.end(); itp++)
		{
			
			list<item*>::iterator iti = proveedor_items[*itp].begin();
			//Creo la lista de items

			for (iti; iti != proveedor_items[*itp].end(); iti++)
			{
				(*iti)->CodIdCC = cont_item;
				cont_item++;
				//Los voy a incluir en orden de detras a delante
				//inserto items ordenados por fecha
				auto it_i = sub->lista_items.begin();
				for ( ;it_i != sub->lista_items.end() && (*it_i)->latest_arrival > (*iti)->latest_arrival; it_i++)
				{

				}
				sub->lista_items.insert(it_i, *iti);
//				sub->lista_items.push_back(*iti);
				//Calculo el minimo de ancho y de largo de una pieza en este subproblema
				//Si la orientación 0, none, 1, lengthwise, 2, withwise
				if ((*iti)->forced_orientation_int == 0)
				{
					if (min((*iti)->length, (*iti)->width) < sub->min_dim_length)
						sub->min_dim_length = min((*iti)->length, (*iti)->width);
					if (min((*iti)->length, (*iti)->width) < sub->min_dim_width)
						sub->min_dim_width = min((*iti)->length, (*iti)->width);
				}
				else
				{
					if ((*iti)->forced_orientation_int == 1)
					{
						if ((*iti)->length < sub->min_dim_length)
							sub->min_dim_length = (*iti)->length;
						if ((*iti)->width < sub->min_dim_width)
							sub->min_dim_width = (*iti)->width;
					}
					else
					{
						if ((*iti)->width < sub->min_dim_length)
							sub->min_dim_length = (*iti)->width;
						if ((*iti)->length < sub->min_dim_width)
							sub->min_dim_width = (*iti)->length;
					}
				}

				sub->total_items += (*iti)->num_items;

				//Creo la lista de camiones				
				for (list<truck*>::iterator itcam = (*iti)->camiones.begin(); itcam != (*iti)->camiones.end(); itcam++)
				{

					if (sub->lista_camiones.empty() || sub->mapa_camiones.count((*itcam)->id_truck) == 0)
					{
						(*itcam)->CodIdCC = cont_cam;
						(*itcam)->lcompo.push_back(contc);
						cont_cam++;
						//inserto ordenado por fechas
						//Insertar_camiones_ordenado_por_fechas(sub,*itcam);
						//Los voy a incluir en orden de detras a delante
						auto it_c = sub->lista_camiones.begin();
						for (; it_c != sub->lista_camiones.end() && (*it_c)->arrival_time > (*itcam)->arrival_time; it_c++)
						{

						}
						sub->lista_camiones.insert(it_c, *itcam);
	//					sub->lista_camiones.push_back(*itcam);
						list<item*> aux;
						aux.push_back((*iti));
						sub->mapa_camiones.insert(pair<string, list<item*>>((*itcam)->id_truck, aux));
					}
					//inserto el item que va en ese camion.
					else
					{
						sub->mapa_camiones[(*itcam)->id_truck].push_back(*iti);
					}

				}
			}
		}	
		sub->Total_Posibles_Asignaciones = 0;
		for (auto ic = sub->lista_camiones.begin(); ic != sub->lista_camiones.end(); ic++)
		{
			(*ic)->items.sort(item::comparador);
			std::copy((*ic)->items.begin(), (*ic)->items.end(), std::back_inserter((*ic)->items_originales));
//			(*ic)->items_originales.insert( (*ic)->items.begin(), (*ic)->items.end());


		}
		for (auto it = sub->lista_items.begin(); it != sub->lista_items.end(); it++)
		{
			sub->Total_Posibles_Asignaciones += ((*it)->camiones.size());
/*			for (auto it_c = (*it)->camiones.begin(); it_c != (*it)->camiones.end(); it_c++)
			{
//				sub->Total_Posibles_Asignaciones += ((*it)->num_items) * ((*it_c)->cota_extra);
				sub->Total_Posibles_Asignaciones += ((*it_c)->cota_extra);
			}
			*/
			(*it)->camiones.sort(truck::comparador);
			std::copy((*it)->camiones.begin(), (*it)->camiones.end(), std::back_inserter((*it)->camiones_originales));

//			(*it)->camiones_originales.insert((*it)->camiones.begin(), (*it)->camiones.end());

	//		sort((*it)->camiones.begin(), (*it)->camiones.end(),truck::comparador);
		}
	

		cont_cam = 0;
		for (auto itc2 = sub->lista_camiones.begin(); itc2 != sub->lista_camiones.end(); itc2++, cont_cam++)
		{
			(*itc2)->CodIdCC = cont_cam;

		}
		cont_item = 0;
		for (auto itc2 = sub->lista_items.begin(); itc2 != sub->lista_items.end(); itc2++, cont_item++)
		{
			(*itc2)->CodIdCC = cont_item;

		}
		total_items_cuadrado += (int)pow(sub->total_items, 2) / 100;
		total_items_camiones += sub->Total_Posibles_Asignaciones;
		//Añado el subproblema al vector de problema.
		this->subproblemas.push_back(sub);
		if (Pruebas)
		{
			sub->max_med_trucks();
			sub->max_med_items();
		}

	}
	//Calculamos cunato puedo llenar cada camion
	for (auto itS = subproblemas.begin(); itS != subproblemas.end(); itS++)
	{
		for (auto itC = (*itS)->lista_camiones.begin(); itC != (*itS)->lista_camiones.end(); itC++)
		{
			
			for (auto iti = (*itC)->items.begin(); iti != (*itC)->items.end(); iti++)
			{
				(*itC)->items_modelo.push_back(item_modelo(*iti, (*iti)->num_items, 1,false));
				(*itC)->num_items_modelo += (*iti)->num_items;
				(*itC)->total_items_modelos+= (*iti)->num_items;
				(*itC)->peso_total_items_modelo += (*iti)->num_items * (*iti)->weight;
				(*itC)->volumen_total_items_modelo += (*iti)->num_items * (*iti)->volumen;
				if ((*itC)->dim_minima > (*iti)->length)(*itC)->dim_minima = (*iti)->length;
				if ((*itC)->dim_minima > (*iti)->width)(*itC)->dim_minima = (*iti)->width;
			}
			vector < item_modelo> best_cantidad_items = (*itC)->items_modelo;
			//Creo el camion y lo añado a la solución
			truck_sol camion((*itC)->id_truck);
			camion.volumen_max = (*itC)->volumen;
			camion.peso_max = (*itC)->max_loading_weight;
			(*itS)->sol.listado_camiones.push_back(camion);
			(*itS)->Total_Iter_Packing = 20;
			//TODO
//			(*itS)->CargoCamion((*itC),(*itC)->items_modelo,(*itC)->id_truck,(*itC)->num_items_modelo,0,1);
//			(*itC)->volumen_inicial = (*itC)->volumen_ocupado;
			(*itC)->volumen_inicial = (*itC)->volumen_kp;
			auto it2 = (*itC)->items.begin();
			for (auto it1 = best_cantidad_items.begin(); it1 != best_cantidad_items.end(); it1++,it2++)
			{
				(*it2)->num_items = it1->num_items;
			}

			(*itC)->reset();
			
			for (auto iti = (*itC)->items.begin(); iti != (*itC)->items.end(); iti++)
			{
				(*itC)->items_modelo.push_back(item_modelo(*iti, (*iti)->num_items, 1,false));
				(*itC)->num_items_modelo += (*iti)->num_items;
				(*itC)->total_items_modelos += (*iti)->num_items;
				(*itC)->peso_total_items_modelo += (*iti)->num_items * (*iti)->weight;
				(*itC)->volumen_total_items_modelo += (*iti)->num_items * (*iti)->volumen;
				if ((*itC)->dim_minima > (*iti)->length)(*itC)->dim_minima = (*iti)->length;
				if ((*itC)->dim_minima > (*iti)->width)(*itC)->dim_minima = (*iti)->width;
			}
			//Creo el camion y lo añado a la solución
			//truck_sol camion((*itC)->id_truck);
			(*itS)->sol.listado_camiones.push_back(camion);
			(*itS)->Total_Iter_Packing = 20;
			//TODO
//			(*itS)->CargoCamion((*itC), (*itC)->items_modelo, (*itC)->id_truck, (*itC)->num_items_modelo, 0, 2);
//			(*itC)->peso_inicial = (*itC)->peso_cargado;
			(*itC)->peso_inicial = (*itC)->max_loading_weight;
			
			it2 = (*itC)->items.begin();
			for (auto it1 = best_cantidad_items.begin(); it1 != best_cantidad_items.end(); it1++, it2++)
			{
				(*it2)->num_items = it1->num_items;
			}

			(*itC)->reset();
		}
		(*itS)->reset();
	}
	


	//Calcular cota heurística
	for (auto itS = subproblemas.begin(); itS != subproblemas.end() ; itS++)
	{
		for (auto itC = (*itS)->lista_camiones.begin(); itC != (*itS)->lista_camiones.end(); itC++)
		{
	//		CargoCamion((*itC), (*itC)->items_modelo, (*itC)->id_truck, (*itC)->num_items_modelo, 0);
	//		if ((*itC)->volumen_ocupado>0)
	//			(*itC)->volumen_heuristico = (*itC)->volumen_ocupado;


		}
	}
	for (int i = 0; i < MAX_ITER_MODELO; i++)
	{
		pair<int, int> p(0, 0);
		veces_cargocamion.push_back(p);
	}
	for (int i = 0; i < MAX_ITER_MODELO; i++)
	{
		vector<double> p(5);
		cargocamion_char.push_back(p);
	}
	
	for (auto it = subproblemas.begin(); it != subproblemas.end(); it++)
	{
		//Minimo de ancho de los camioens
		for (auto itc2 =(*it)->lista_camiones.begin(); itc2 != (*it)->lista_camiones.end(); itc2++)
		{
			for (auto it_it = (*itc2)->items.begin(); it_it != (*itc2)->items.end(); it_it++)
			{
				if ((*it_it)->forced_orientation_int == 0)
				{
					if (min((*it_it)->length, (*it_it)->width) < (*itc2)->min_dim_length)
						(*itc2)->min_dim_length = min((*it_it)->length, (*it_it)->width);
					if (min((*it_it)->length, (*it_it)->width) < (*itc2)->min_dim_width)
						(*itc2)->min_dim_width = min((*it_it)->length, (*it_it)->width);
				}
				else
				{
					if ((*it_it)->forced_orientation_int == 1)
					{
						if ((*it_it)->length < (*itc2)->min_dim_length)
							(*itc2)->min_dim_length = (*it_it)->length;
						if ((*it_it)->width < (*it)->min_dim_width)
							(*itc2)->min_dim_width = (*it_it)->width;
					}
					else
					{
						if ((*it_it)->width < (*it)->min_dim_length)
							(*itc2)->min_dim_length = (*it_it)->width;
						if ((*it_it)->length < (*it)->min_dim_width)
							(*itc2)->min_dim_width = (*it_it)->length;
					}
				}
			}
			//int kk = (*itc2)->min_dim_length;
			//int jj = (*itc2)->min_dim_width;
			//int ki = 9;


		}
		solucion sol;
		(*it)->Pruebas = Pruebas;
		mejor_solucion.solucion_compo.push_back(sol);
	}


	
 }
 int Problema::Resolver_Paralelo(int& iter, string file_to_read, parameters param)
 {


	 coste_transporte = 0;
	 coste_inventario = 0;
	 coste_total = 0;



//	 omp_set_num_threads(8);
	 bool packing = false;
	 int caso = 0;
	 int NSubproblemas = subproblemas.size();
#pragma omp parallel for
	 for (int nsub_it=0;nsub_it<NSubproblemas;nsub_it++)
	 {

		 int cont = 0;
		 auto it = subproblemas.begin();
		 for (cont = 0; it != subproblemas.end() && cont < nsub_it; ++it, cont++);
			 


			 if (param.tllevo() <= (TiempoTotal - 20))
			 {
				 
				 (*it)->Algoritmo_Paralelo = Algoritmo_Paralelo;
				 //Inicializo el contador de veces modelo para reactivo GAP
				 (*it)->contador_modelo = 0;

				 if (Pruebas) cout << "COMPONENTE:" << cont << " Iteracion:" << iter << endl;


				 if (Pruebas) cout << "Aqui 112 " << (*it)->coste_total << " " << (*it)->coste_inventario << " " << (*it)->coste_transporte << endl;



				 if (Pruebas) cout << "Aqui 112 " << (*it)->coste_total << " " << (*it)->coste_inventario << " " << (*it)->coste_transporte << (*it)->optimo << endl;


				 (*it)->Total_Iter_Packing = Total_Iter_Packing;
				 if ((*it)->optimo == true)  //si está resuelta optimamente no la vuelvo a resolver 
				 {
					 continue;
				 }
				 else
				 {
					 (*it)->reset();
				 }


				 //Calculo el tiempo que va a estar el modelo.
				 (*it)->TiempoModelo = (int)ceil((double)(TiempoModelo * ((*it)->Total_Posibles_Asignaciones)) / (double)total_items_camiones);
				 //(*it)->TiempoModelo = (int)ceil((double) 600 * (pow(((*it)->total_items), 2) / 100) / (double)total_items_cuadrado);
				 (*it)->TiempoModelo += (*it)->TiempoExtra;



				 //		printf("Aqui1 %d\n", iter);
				 (*it)->Iter_Resuelve_Subproblema = iter;
				 //		printf("Aqui2 %d\n", iter);
				 //		(*it)->FiltroItems(iter + 3);
//				 printf("Aqui3 %d\n", iter);

				 (*it)->FiltroItems(iter + 2);




//					 (*it)->ReducirExtras();



//				 		printf("Aqui4 %d\n", iter);
				 if ((*it)->TiempoModelo < 5) (*it)->TiempoModelo = 3;



				 (*it)->porcentage_peso = 100;
				 (*it)->porcentage_vol = 100;




				 if (param.tqueda() > ((*it)->TiempoModelo + 20))
				 {
					 caso = 0;
					 do
					 {
						 caso = relaxandfix(file_to_read, param, (*it));
						 if (param.tqueda() < (*it)->TiempoModelo + 20)
							 caso = 2;

					 } while (caso == 3 && caso != 2);

				 }
				 else continue;

				 if (caso != 2)
				 {

					 //Si el modelo no ha dado solución le doy 5 extra y paso a la sig componente
					 if ((*it)->TieneSol == false)
					 {
						 (*it)->TiempoExtra += 5;
						 continue;
					 }
					 //Hacemos el packing
					 //(*it)->PackingModelo(1, file_to_read, param);




					 //Si el packing no ha dado solución le doy 5 extra y paso a la sig componente
					 if ((*it)->TieneSol == false)
					 {
						 (*it)->TiempoExtra += 5;
						 continue;
					 }
					 else packing = true;


					 //AQUI PASO LAS PILAS CARGADAS DE CADA CAMION A LA SOLUCION
					 //Compruebo si hay solución

					 pasar_solucion((*it), param);


					 //Cambiado 28/01/2023
					 //Y no ha realizado filtro
					 if ((*it)->Iter_Resuelve_Subproblema > 3 && abs(((*it)->coste_total) - (*it)->ObjBound) < 0.05)
					 {
						 (*it)->optimo = true;
					 }


					 //si la comonente tiene mejor coste me quedo con esa.
					 if ((*it)->best_iteracion < 0 || (*it)->coste_total < mejor_solucion.solucion_compo.at(nsub_it).coste_total)
					 {
						 (*it)->best_iteracion = iter;
						 actualizo_mejor_solucion_paralelo((*it), nsub_it);
					 }

					 if (Pruebas) cout << endl << setprecision(8) << "Tiempo " << param.tqueda() << " Componente " << nsub_it << endl;
				 }

			 }

	 }
	 
	 if (packing != false)
		 MejorSolucion(param);
	 return caso;

 }
 
int Problema::Resolver(int &iter, string file_to_read, parameters param)
{
	//Inicializo los valoresPosilbles_Asignaciones
	coste_transporte = 0;
	coste_inventario = 0;
	coste_total = 0;

	int cont = -1;
	int compo = -1;

	int Temp_Iter = iter;
	bool packing = false;

	for (auto& it : subproblemas )  //Recorro los subproblemas
	{
		if (cont == 0 && iter > 3 && it->Tipo == 18)
		{
			(*it).Last_Ramon = true;
		}

		it->contador_infeasibles=0;
		it->coste_transporte = 0;
		it->coste_inventario = 0;
		if ((param.tllevo() > (TiempoTotal - 20)) ||  (param.tqueda() < (30)))
			return 2;
		if (Tipo == 17 && ((2 * param.tllevo() > (TiempoTotal) || iter > 5)))
		{
			

			if (iter > 10)
			{
				(*it).Last_Ramon = false;
				(*it).Por_Camion = true;
				if (Pruebas) cout << "Por Camion" << endl;
			}
			else
			{
				if ((*it).Last_Ramon == false)
					Temp_Iter = 1;
				if (Pruebas) cout<<"Last Ramon"<<endl;
				(*it).Last_Ramon = true;
			}
		}
		if ( Pruebas && compo==(-1))
		{
			cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
			cout << "ITERACION " << iter << " CT " << mejor_solucion.coste_transporte << " CI " << mejor_solucion.coste_inventario << " CT " << mejor_solucion.coste_total << endl;
			
		}
		if (iter==0)
			it->Total_Iter_Packing = Total_Iter_Packing;
		it->Algoritmo_Paralelo = Algoritmo_Paralelo;
		if (Temp_Iter>10 )
			it->Total_Iter_Packing = 10+ it->Total_Iter_Packing;
//		if (iter==0)
//			(*it)->MejorarNumExtras();
		cont++;
		compo++;
		it->CodS = compo;
		it->escribe_componente = false;
//		if (cont !=43)
//		continue;
		/*F
		//Para elegir componente
		if (cont != 198)
		{
			it->escribe_componente = false;
			continue;
		}
		else
		{
			it->escribe_componente = true;

		}*/
		
		
// 
		
//Inicializo el contador de veces modelo para reactivo GAP
		it->contador_modelo = 0;

		if (Pruebas) cout << "COMPONENTE:" << cont << " Iteracion:" << iter << endl;
		if (Pruebas && iter==0)
		{
			set<string> supliers;
			set<string> supliers_dock;
			set<string> plant_dock;
			for (auto itp : it->lista_items)
			{
				string str(itp->suplier_code);
				supliers.insert(str);
				string str1(itp->sup_cod_dock);
				supliers_dock.insert(str1);
				string str2(itp->plant_dock);
				plant_dock.insert(str2);

			}
			cout << "Supliers " << endl;
			for (auto its : supliers)
			{
				cout << its << endl;
			}
			cout << "Supliers dock" << endl;
			for (auto its : supliers_dock)
			{
				cout << its << endl;
			}
			cout << "Plant " << endl;
			for (auto its : plant_dock)
			{
				cout << its << endl;
			}

		}
		 
//		if (Pruebas) cout << "Aqui 112 " << it->coste_total << " " << it->coste_inventario << " " << it->coste_transporte << endl;



//		if (Pruebas) cout << "Aqui 112 " << it->coste_total << " " << it->coste_inventario << " " << it->coste_transporte << " "<< it->optimo << endl;


		
		if (it->optimo == true)  //si está resuelta optimamente no la vuelvo a resolver 
		{
			if (Pruebas) cout <<"optimo"<< endl;
			num_camiones_modelo += it->num_camiones_modelo;
			num_camiones_extra_modelo += it->num_camiones_extra_modelo;
			max_cam_extra_tipo_modelo = max(it->max_cam_extra_tipo_modelo, max_cam_extra_tipo_modelo);
			coste_transporte += it->coste_transporte;
			coste_inventario += it->coste_inventario;
			coste_total += it->coste_total;
			//Calcular nuevos tiempos
			total_items_cuadrado = 0;
			total_items_camiones = 0;
			for (auto ittemp = subproblemas.begin(); ittemp != subproblemas.end(); ittemp++)
			{
				if ((*ittemp)->optimo != true)
					//				total_items_cuadrado += (int)pow((*ittemp)->total_items, 2) / 100;
					//				total_items_camiones += (int)((*ittemp)->total_items * (it->num_camiones_modelo + it->num_camiones_extra_modelo)) / 100;
					total_items_camiones += (*ittemp)->Total_Posibles_Asignaciones;

			}
			continue;
		}
		else
			it->reset();


		if (iter == 0)
		{
			//Calculo el tiempo que va a estar el modelo.
			it->TiempoModelo = TiempoModelo * ((double)( (it->Total_Posibles_Asignaciones)) / (double)total_items_camiones);
			//it->TiempoModelo = (int)ceil((double) 600 * (pow((it->total_items), 2) / 100) / (double)total_items_cuadrado);
			it->TiempoModelo = it->TiempoModelo / 3;
			if (Pruebas) cout << "Tiempo Modelo" << it->TiempoModelo << " " << it->Total_Posibles_Asignaciones << " " << total_items_camiones << endl;

		}
		else
		{
			//Calculo el tiempo que va a estar el modelo.
			it->TiempoModelo = (int)ceil((double)(TiempoModelo * (it->best_coste_total)) / (double)(mejor_solucion.coste_total));

		}
		it->TiempoModelo += it->TiempoExtra;
//		it->TiempoModelo = 2;
//		it->TiempoModelo = min(it->TiempoModelo, 25);
//		continue;
		//if (cont != 31) continue;
	/*if (cont != 3)
		{
			continue;
		}
		else it->TiempoModelo=5;*/
//		printf("Aqui1 %d\n", iter);
		it->Iter_Resuelve_Subproblema = Temp_Iter;
//		printf("Aqui2 %d\n", iter);
//		it->FiltroItems(iter + 3);
//		printf("Aqui3 %d\n", iter);
//		if (cont < 123) continue;
		it->FiltroItems(Temp_Iter +1);
		it->ReducirExtras();
//		it->ReducirExtras(param);
// 		   //En 78 estaba asi
//		if (!( (it->Iter_Resuelve_Subproblema > 0) && (it->Todos_Los_Camiones)) )
//			Enteras(param, it);
		//En 79 lo pongo asi
		if (it->Iter_Resuelve_Subproblema == 0 || (it->Iter_Todos_Camiones ==0) || (iter==10))
			Enteras(param, it);
//		else
//			it->TiempoExtra = 0;
//		if (!((it->Iter_Resuelve_Subproblema > 0) ))
//			Enteras( param, it);
//		printf("Aqui3 %d\n", iter);


		it->TiempoModelo += it->TiempoExtra;

//		if (it->Pruebas) printf("Aqui4 %d\n", iter);
		
		if (it->TiempoModelo < 1) it->TiempoModelo = 1;


//		if (it->TiempoModelo < 5) it->TiempoModelo = 1;
		//OJO con el mínimo que se pone


		//Llamamos al modelo si queda tiempo
	//		int tt=param.tqueda();

//		it->FiltroItems(iter + 1);
		//		it->ReducirExtras();

		if (iter == 14 || iter==16)
			int kk = 9;
		
		if (param.tqueda() > (it->TiempoModelo + 20))
		{
			int caso = 0;
			do
			{
				caso = relaxandfix(file_to_read, param, it);
				if (caso == 2 || param.tqueda() < it->TiempoModelo + 20)
					return 2;
			} while (caso == 3);
			if (caso == 1)
			{
				if (Pruebas) cout << "No voy a conseguir nada mejor" << endl;
				packing = true;
				goto fin2;
			}
			
		}
		else
		{
			continue;
		}

//		if (it->Pruebas)printf("Aqui5 %d\n", iter);
		//Si el modelo no ha dado solución le doy 5 extra y paso a la sig componente
		if (it->TieneSol == false)
		{
			it->TiempoExtra += 5;
			continue;
		}
		//Hacemos el packing
		//it->PackingModelo(1, file_to_read, param);


//		if (it->Pruebas) printf("Aqui6 %d\n", iter);
		veces_pila_cambia += it->veces_pila_cambia;
		veces_pila += it->veces_pila;
		//Si el packing no ha dado solución le doy 5 extra y paso a la sig componente
		if (it->TieneSol == false)
		{
			it->TiempoExtra += 5;
			continue;
		}
		else packing = true;

		if (it->Pruebas)printf("Aqui7 %d\n", iter);
		//AQUI PASO LAS PILAS CARGADAS DE CADA CAMION A LA SOLUCION
		//Compruebo si hay solución

		pasar_solucion(it, param);
		
/*		bool check = comprobar_solucion();
		if (check == false)
		{
			PintarProblema(555);
		}
		*/

		//Y no ha realizado filtro
		if (it->coste_total < it->best_coste_total)
			it->best_coste_total = it->coste_total;
/*		if (it->Todos_Los_Camiones && abs((it->best_coste_total) - it->ObjBound) < 1)
		{
			it->optimo = true;
		}*/

		//Last Ramon pone las x como lineales
		if (it->Todos_Los_Camiones  && !it->Last_Ramon && (it->best_coste_total - it->ObjBound_Original) < 1)
		{
			it->optimo = true;
		}
		ObjBound += it->ObjBound;
		ObjMip += it->ObjMip;
		num_camiones_modelo += it->num_camiones_modelo;
		num_camiones_extra_modelo += it->num_camiones_extra_modelo;
		max_cam_extra_tipo_modelo = max(it->max_cam_extra_tipo_modelo, max_cam_extra_tipo_modelo);
		coste_transporte += it->coste_transporte;
		coste_inventario += it->coste_inventario;
		coste_total += it->coste_total;
		//Se aplican porcentajes a los costes.
		coste_total = param.transportation_cost * coste_transporte + param.inventory_cost * coste_inventario;
		total_infeasibles += it->contador_infeasibles;
		if (it->Pruebas) printf("Aqui8 %d\n", iter);

		
		//si la comonente tiene mejor coste me quedo con esa.
		if (it->Pruebas) printf("Aqui8 Compo %d %d\n", compo, mejor_solucion.solucion_compo.at(compo).coste_total);
		if (mejor_solucion.solucion_compo.at(compo).coste_total == 0 || it->coste_total < mejor_solucion.solucion_compo.at(compo).coste_total)
		{
			it->ComprobarItemsColocados();
			for (auto ts : it->sol.listado_camiones)
			{
				it->ComprobarPesosCamion(ts);
			}
			it->best_iteracion = iter;
			best_iteracion = iter;
			actualizo_mejor_solucion(it, compo);
			//bool check = comprobar_solucion();
			//if (check == false)
			//{
			//	PintarProblema(555);
			//}
		}
fin2:
		if (Pruebas) cout << "COMPONENTE:" << cont << " Iteracion:" << iter << "Mejor Iter" << it->best_iteracion << endl;
		if (Pruebas) cout << "GAP "<<  (double)(it->coste_total- it->ObjBound_Original)/ (double)(it->coste_total) << " ObjBound " << it->ObjBound_Original << "Coste " << it->coste_total << " " << it->coste_inventario << " " << it->coste_transporte << " Op " << it->optimo << " CT " << param.transportation_cost << " CI " << param.inventory_cost << " BS_T " << mejor_solucion.coste_total << " BS_I " << mejor_solucion.coste_inventario << " BS_T " << mejor_solucion.coste_transporte << endl;
		
		if (Pruebas) cout << endl << setprecision(8) << "Tiempo " << param.tqueda() << " Componente " << compo << endl;
		if (it->Todos_Los_Camiones == false)
			it->ObjBound_Original = 0;
		if (param.tllevo() > (TiempoTotal - 20))
			return 2;
	}

	if (packing == false)
		iter = MAX_ITER_GLOBAL;
	return 0;
	
}

void Problema::escribe_cam(Subproblema* su, int cont)
{
	/*ofstream f("./output_trucks.csv",);
	f << "Id truck; Loaded length; Weight of loaded items; Volumevof loaded items; emm; emr" << endl;
	f.close();
	ofstream f2("./output_stacks.csv");
	f2 << "Id truck; Id stack; Stack code; X origin; Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;
	f2.close();
	ofstream f3( "./output_items.csv");
	f3 << "Item ident;Id truck;Id stack;Item code;X origin;Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;
	f3.close();*/

	
	for (auto it = su->sol.listado_items.begin(); it != su->sol.listado_items.end(); it++)
		(*it).escribe("./");
	for (auto it = su->sol.listado_camiones.begin(); it != su->sol.listado_camiones.end(); it++)
		(*it).escribe("./");
	for (auto it = su->sol.listado_pilas.begin(); it != su->sol.listado_pilas.end(); it++)
		(*it).escribe("./");
}
void Problema::pasar_solucion(Subproblema* it,parameters param)
{
	double coste_tras = 0;
	double inventory = 0;
	int planificados = 0;
	int extras=0;
	
	for (auto itc = it->lista_camiones.begin(); itc != it->lista_camiones.end(); itc++)
	{
//		(*itc)->ComprobarPesos();
		if ((*itc)->pilas_solucion.size() <= 0 && (*itc)->pcamiones_extra.size() > 0)
		{
			for (auto itex : (*itc)->pcamiones_extra)
			{
				if (itex->pilas_solucion.size() > 0)
				{
					if (Pruebas) cout << "camion" << (*itc)->id_truck << "Z_" << (*itc)->CodIdCC << "; Extras ;" << (*itc)->cota_extra;
					if (Pruebas) cout << "Posible problema, tiene el extra pero no el planificado pero no tiene pilas" << endl;
//					PintarProblema(110);
				}
			}
		}

		if ((*itc)->pilas_solucion.size() <= 0 && (*itc)->pcamiones_extra.size() <= 0)
		{
			//cout << "Vacio:" << (*itc)->id_truck << endl;
			continue;
		}
		
		if ((*itc)->pilas_solucion.size() > 0)
		{
			coste_tras += (*itc)->cost;
			planificados++;
			
		}
		for (auto itp = (*itc)->pilas_solucion.begin(); itp != (*itc)->pilas_solucion.end(); itp++)
		{
			
			it->sol.listado_pilas.push_back(((*itp).id_pila));
			for (auto iti = (*itp).items.begin(); iti != (*itp).items.end(); iti++)
			{
				
				it->sol.listado_items.push_back(((*iti)));
				inventory += (*iti).inventory_cost;
			}

		}
		for (auto ite = (*itc)->pcamiones_extra.begin(); ite != (*itc)->pcamiones_extra.end(); ite++)
		{

			if ((*ite)->pilas_solucion.size() <= 0)
			{
				//cout << "Vacio:" << (*ite)->id_truck << endl;
				continue;
			}
			
			if ((*ite)->pilas_solucion.size() > 0)
			{
				coste_tras += (*ite)->coste_extra;
				extras++;
				
			}
			for (auto itp = (*ite)->pilas_solucion.begin(); itp != (*ite)->pilas_solucion.end(); itp++)
			{
				it->sol.listado_pilas.push_back(((*itp).id_pila));
				for (auto iti = (*itp).items.begin(); iti != (*itp).items.end(); iti++)
				{
					it->sol.listado_items.push_back(((*iti)));
					inventory += (*iti).inventory_cost;
				}
			}
		}
	}
	if (Pruebas) cout << "CT " << it->coste_total << " CI  " << it->coste_inventario << " CT " << it->coste_transporte << endl;

//	if (Pruebas) cout << "Aqui 112 " << coste_tras << " " << inventory << " " << endl;
	// 
	//Actualizo costes del subproblema sin aplicar porcentajes, se hace al final
	it->coste_transporte = coste_tras;
	it->sol.coste_transporte = coste_tras;
	it->coste_inventario = inventory;
	it->sol.coste_inventario = inventory;
	it->planificados = planificados;
	it->extras= extras;

	it->coste_total = param.transportation_cost * coste_tras + param.inventory_cost * inventory;
	it->sol.coste_total = it->coste_total;
//	if (Pruebas) cout << "Aqui 112 " << it->coste_total << " " << it->coste_inventario << " " << it->coste_transporte << endl;

}
void Problema::MejorSolucion(parameters param)
{
	int cont = 0;
	ObjBound = 0;
	ObjMip = 0;
	coste_transporte = 0;
	coste_inventario = 0;
	coste_total = 0;
	for (auto it = subproblemas.begin(); it != subproblemas.end() ; it++,cont++)
	{
		//Actualizo el coste, nuevo coste - el coste que tenia antes.

		ObjBound += (*it)->ObjBound;
		ObjMip += (*it)->ObjMip;
		if (best_iteracion<(*it)->best_iteracion)
			best_iteracion = (*it)->best_iteracion;
		num_camiones_modelo += (*it)->num_camiones_modelo;
		num_camiones_extra_modelo += (*it)->num_camiones_extra_modelo;
		max_cam_extra_tipo_modelo = max((*it)->max_cam_extra_tipo_modelo, max_cam_extra_tipo_modelo);
		coste_transporte += mejor_solucion.solucion_compo.at(cont).coste_transporte;
		coste_inventario += mejor_solucion.solucion_compo.at(cont).coste_inventario;
//		coste_total += (*it)->coste_total;
		//Se aplican porcentajes a los costes.
		total_infeasibles += (*it)->contador_infeasibles;
	}
	
	coste_total = param.transportation_cost * coste_transporte + param.inventory_cost * coste_inventario;
	//Actualizo el coste, nuevo coste - el coste que tenia antes.
	mejor_solucion.coste_transporte = coste_transporte;
	mejor_solucion.coste_inventario = coste_inventario;
	if (coste_total < mejor_solucion.coste_total)
	{
		mejor_solucion.coste_total = coste_total;
		
	}
	

}
void Problema::actualizo_mejor_solucion_paralelo(Subproblema* it, int compo)
{
	mejor_solucion.solucion_compo.at(compo).reset();
	mejor_solucion.solucion_compo.at(compo).coste_total = it->coste_total;
	mejor_solucion.solucion_compo.at(compo).coste_transporte = it->coste_transporte;
	mejor_solucion.solucion_compo.at(compo).coste_inventario = it->coste_inventario;

	for (auto it2 = it->sol.listado_camiones.begin(); it2 != it->sol.listado_camiones.end(); it2++)
	{
		mejor_solucion.solucion_compo.at(compo).listado_camiones.push_back((*it2));
	}
	for (auto it2 = it->sol.listado_items.begin(); it2 != it->sol.listado_items.end(); it2++)
		mejor_solucion.solucion_compo.at(compo).listado_items.push_back((*it2));
	for (auto it2 = it->sol.listado_pilas.begin(); it2 != it->sol.listado_pilas.end(); it2++)
		mejor_solucion.solucion_compo.at(compo).listado_pilas.push_back((*it2));
	if (Pruebas) cout << it->coste_total << " ; " << mejor_solucion.solucion_compo.at(compo).coste_total << " ; " << it->coste_total - mejor_solucion.solucion_compo.at(compo).coste_total << endl;

}
void Problema::actualizo_mejor_solucion(Subproblema* it, int compo)
{
	
//	printf("Aqui 23 %d %d",it->coste_transporte,compo);
	//Actualizo el coste, nuevo coste - el coste que tenia antes.
	mejor_solucion.coste_transporte += it->coste_transporte - mejor_solucion.solucion_compo.at(compo).coste_transporte;
	mejor_solucion.coste_inventario += it->coste_inventario - mejor_solucion.solucion_compo.at(compo).coste_inventario;
	mejor_solucion.coste_total+= it->coste_total - mejor_solucion.solucion_compo.at(compo).coste_total;


	mejor_solucion.solucion_compo.at(compo).reset();
	mejor_solucion.solucion_compo.at(compo).coste_total = it->coste_total;
	mejor_solucion.solucion_compo.at(compo).coste_transporte = it->coste_transporte;
	mejor_solucion.solucion_compo.at(compo).coste_inventario = it->coste_inventario;

	for (auto it2 = it->sol.listado_camiones.begin(); it2 != it->sol.listado_camiones.end(); it2++)
	{

//		printf("Aqui 222 %d %d Size %d ", (*it2).coste, compo, it->sol.listado_camiones.size());
		mejor_solucion.solucion_compo.at(compo).listado_camiones.push_back((*it2));
		
	}
	
	for (auto it2 = it->sol.listado_items.begin(); it2 != it->sol.listado_items.end(); it2++)
	{
		mejor_solucion.solucion_compo.at(compo).listado_items.push_back((*it2));
		
	}
	for (auto it2 = it->sol.listado_pilas.begin(); it2 != it->sol.listado_pilas.end(); it2++)
		mejor_solucion.solucion_compo.at(compo).listado_pilas.push_back((*it2));
	if (Pruebas) cout << it->coste_total << " ; " << mejor_solucion.solucion_compo.at(compo).coste_total << " ; " << it->coste_total - mejor_solucion.solucion_compo.at(compo).coste_total << endl;
	
	
}
bool Problema::comprobar_solucion()
{
	//Recorro la mejor solucion y veo si coinciden los costes.
	int ccam = 0;
	int invetcost = 0;
	
	for (int i = 0; i < mejor_solucion.solucion_compo.size(); i++)
	{
		solucion sol=mejor_solucion.solucion_compo.at(i);

			for (auto cam = sol.listado_camiones.begin(); cam != sol.listado_camiones.end(); cam++)
			{
//				printf("Aqui 233 coste %d", (*cam).coste);
				ccam+=(*cam).coste;
			}
			
			for(auto itemc=sol.listado_items.begin();itemc!=sol.listado_items.end();itemc++)
			{
				invetcost += (*itemc).inventory_cost;
			}
			

	}
//	printf("Aqui coste %d %d", ccam, mejor_solucion.coste_transporte);
	if (ccam != mejor_solucion.coste_transporte)
	{
		cout << "Coste transporte no coincide" << endl;
		return false;
	}
	if (invetcost != mejor_solucion.coste_inventario)
	{
		cout << "Coste inventario no coincide" << endl;
		return false;
	}
	return true;
}
//Borro de la lista de items del camion, el item
bool Problema::BorrarItemDelCamion(truck* camion, item* item)
{
	bool borrado = false;
	auto itic = camion->items.begin();
	while (itic != camion->items.end() && !borrado)
	{
		//Borramos el que tiene un camion anterior
		if ((*itic)->CodId == item->CodId)
		{
//			if (Pruebas)
//				cout << "Borro del camion " << (*itic)->CodId << " " << (*itic)->earliest_arrival << endl;
			itic = camion->items.erase(itic);
			borrado = true;
			if (camion->items.empty())
			{
//				if (Pruebas)
//					cout << "Este camion no tien item" << (*itic)->CodId << endl;
			}
		}
		else
			itic++;
	}
	return borrado;
}
//Borro de la lista de camiones del item, el camion
bool Problema::BorrarCamionDelItem(truck* camion, item* item)
{
	bool borrado = false;
	auto itic = item->camiones.begin();
	while (itic != item->camiones.end() && !borrado)
	{
		//Borramos el que tiene un camion anterior
		if ((*itic)->CodId == camion->CodId)
		{
//			if (Pruebas)
//				cout << "Borro del camion " << (*itic)->CodId << " " << (*itic)->arrival_time << endl;
			itic = item->camiones.erase(itic);
			borrado = true;
			if (item->camiones.empty())
			{
//				if (Pruebas)
//					cout << "Este item no tiene camion" << (*itic)->CodId << endl;
			}
		}
		else
			itic++;
	}
	return borrado;
}
bool Problema::divide_subproblema(Subproblema* Sub)
{
//	cout << "Aqui 0" <<endl ;
	int total_cambios = 0;

	int contcam = 0;
	int contcam2 = 0;
	int max_dif_day = 1;
	int cam_elegido = 0;
	truck* camion_divisor = NULL;

	//1) buscar en toda la serie temporal del subproblema un corte de días en el centro.
	for (auto itc = Sub->lista_camiones.begin(); itc != Sub->lista_camiones.end(); itc++)
	{
		contcam++;
		if (4 * contcam > Sub->lista_camiones.size() && (4 * contcam) < (Sub->lista_camiones.size() * 3))
		{
			auto itc2 = itc;
			itc2++;
			contcam2 = contcam ;
			if (itc2 != Sub->lista_camiones.end())
			{

				contcam2++;
				if (4 * contcam2 > Sub->lista_camiones.size() && (4 * contcam2) < (Sub->lista_camiones.size() * 3))
				{
					int dif_day = Days_0001((*itc)->arrival_time_tm_ptr.tm_year, (*itc)->arrival_time_tm_ptr.tm_mon + 1, (*itc)->arrival_time_tm_ptr.tm_mday)
						- Days_0001((*itc2)->arrival_time_tm_ptr.tm_year, (*itc2)->arrival_time_tm_ptr.tm_mon + 1, (*itc2)->arrival_time_tm_ptr.tm_mday);

					if (dif_day >= max_dif_day)
					{
						//De los iguales el que más cerca esté del medio
						int antes = abs((double)cam_elegido - (double)(Sub->lista_camiones.size() / 2));
						int ahora = abs((double)contcam2 - (double)(Sub->lista_camiones.size() / 2));
						if (dif_day == max_dif_day && (antes>ahora))
						{
							max_dif_day = dif_day;
								cam_elegido = contcam2;
								camion_divisor = (*itc);
						}
						if (dif_day > max_dif_day)
						{
							max_dif_day = dif_day;
							cam_elegido = contcam2;
							camion_divisor = (*itc);
						}

					}
				}
			}
		}
	}
//	cout << "Aqui 01" <<endl;
	//Si no ha encontrado un camión con una diferencia de un día en ese tramo cojo el del medio
	//No puedo coger porque tendría que coger el primero que cambie de día
	if (camion_divisor == NULL)
	{
		cout << "No se puede dividir la componente";
		Sub->NoSePuedeDividir = true;
		return false;

	}
	

	
	//2) divido los camiones en dos maps para pasarlos a la función addcomponentes
	map<string, truck*>mapa_cam;
	list<string> lista_cam1, lista_cam2;
	bool compo1 = true;
	list<item*> listit;
	map<string, list<item*>> cam_item;
	map<string, int> id_codicc1,id_codicc2;
	map<int,int> item_compo1;
//	cout << "Aqui 1"<< endl;
	for (auto itc = Sub->lista_camiones.begin(); itc != Sub->lista_camiones.end(); itc++)
	{
		

		listit.clear();

		//Compo 1 es la del última
		if (compo1) 
		{
			//Primera componente es la que hay que borrar de los items que vayan antes del camión intermedio 
			//de esos camiones
			auto iti = (*itc)->items.begin();
			while (iti!=(*itc)->items.end())
			{

				
				if ((*iti)->CodId == 27687)
					int este = 9;
//				if (strcmp((*iti)->Id, "0090038200_14062023009397") == 0)
//					int este = 9;

				//long long fit = (*iti)->latest_arrival;
				//De los items que van en la primera componente
				int dif_day = Days_0001((*iti)->earliest_arrival_tm_ptr.tm_year, (*iti)->earliest_arrival_tm_ptr.tm_mon + 1, (*iti)->earliest_arrival_tm_ptr.tm_mday)
					- Days_0001(camion_divisor->arrival_time_tm_ptr.tm_year, camion_divisor->arrival_time_tm_ptr.tm_mon + 1, camion_divisor->arrival_time_tm_ptr.tm_mday);
				//Si puede llegar antes del camion divisor
				if (dif_day < 0)
				{
					
					//Comprobar si el item puede ir en un camión en una fecha anterior del camión divisor
					//Si es asi lo borro de los camiones posteriores en los que puede ir
					int alguno_borrado = false;

					for (auto itci = (*iti)->camiones.begin();  itci != (*iti)->camiones.end(); )
					{
						if ((*itc)->CodId == (*itci)->CodId)
							int kk = 9;
						int dif_day2 = Days_0001((*itci)->arrival_time_tm_ptr.tm_year, (*itci)->arrival_time_tm_ptr.tm_mon + 1, (*itci)->arrival_time_tm_ptr.tm_mday)
							- Days_0001(camion_divisor->arrival_time_tm_ptr.tm_year, camion_divisor->arrival_time_tm_ptr.tm_mon + 1, camion_divisor->arrival_time_tm_ptr.tm_mday);
						//Si ese item está en un camión anterior a camión divisor
						//Tengo que borrarlo de ese camion
						if (dif_day2 < 0)
						{
							//quito el camion del item

//							cout << "Aqui 011" << endl;
							//Borro de la lista de items del camion, el item
							BorrarItemDelCamion((*itci), *iti);
							//Borro de la lista de camiones del item, el camion
							itci = (*iti)->camiones.erase(itci);
							//BorrarCamionDelItem((*itci), *iti);
//							cout << "Aqui 012" << endl;



						}
						else
							itci++;


					}
//					cout << "Aqui 11" << endl;
					listit.push_back((*iti));
					item_compo1.insert(pair<int, int>((*iti)->CodId, (*iti)->CodIdCC));
					iti++;
//					cout << "Aqui 12" << endl;

					
				}
				else 
				{
					listit.push_back((*iti));
					item_compo1.insert(pair<int, int>((*iti)->CodId,(*iti)->CodIdCC));
					iti++;
				}
					
			}
			//Meto el camion y ese item
			if (!listit.empty())
			{
				cam_item.insert(pair<string, list<item*>>((*itc)->id_truck, listit));
				lista_cam1.push_back((*itc)->id_truck);
				mapa_cam.insert(pair<string, truck*>((*itc)->id_truck, (*itc)));
				id_codicc1.insert(pair<string,int>((*itc)->id_truck,(*itc)->CodIdCC));
			}
			else
			{
				int esvacia = true;
			}

			
		}
		else
		{
			lista_cam2.push_back((*itc)->id_truck);
			mapa_cam.insert(pair<string, truck*>((*itc)->id_truck, (*itc)));
			
			for (auto iti:(*itc)->items)
			{
				if (item_compo1[iti->CodId] != 0)
				{
					//esta en la compo1
					printf("%d %lld %lld\n",iti->CodId,iti->earliest_arrival,iti->latest_arrival);
				}
				listit.push_back(iti);
			}
			cam_item.insert(pair<string, list<item*>>((*itc)->id_truck, listit));
			id_codicc2.insert(pair<string, int>((*itc)->id_truck, (*itc)->CodIdCC));
		}
		if ((*itc)->CodId == camion_divisor->CodId)
		{
			compo1 = false;
		}
	}
	//Comprueba que todo ok
//	cout << "Aqui 2" << endl;

	for (auto itc : Sub->lista_camiones)
	{
		bool encontrado = false;
		for (auto itc1 : lista_cam1)
		{
			if (strcmp((*itc).id_truck, itc1.c_str()) == 0)
				encontrado = true;
		}
		for (auto itc1 : lista_cam2)
		{
			if (strcmp((*itc).id_truck, itc1.c_str()) == 0)
				encontrado = true;
		}
		if (encontrado == false)
		{
			//
			if (Pruebas) cout << "Este camión "<< (*itc).CodId<<" " << (*itc).id_truck << "no está en ninguna de las dos listas\n";
		}

	}
//	cout << "Aqui 3" << endl;
	//3) Borra el subproblema y generar los dos nuevos

	map<int, list<string>> compo_new;
	int ncompo = this->subproblemas.size();
	int codigo = Sub->CodS;
	Sub->CodS = 99999;

	compo_new.insert(pair<int, list<string>>(codigo, lista_cam1));
	compo_new.insert(pair<int, list<string>>(ncompo,lista_cam2));
//	cout << "Aqui 4" << endl;
	this->Add_componente(compo_new, mapa_cam, cam_item);
//	cout << "Aqui 5" << endl;
	return true;
	//Revisa que todos los items en una component
//	for (auto itS = subproblemas.begin(); itS != subproblemas.end(); itS++)
//	{
//		if ((*its)->CodS==codigo)

}