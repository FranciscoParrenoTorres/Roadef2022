#include "truck.h"


void truck::InicializarPesoEjes(int a)
{
	emm_colocado = 0;
	emr_colocado = 0;
	emm_colocado_anterior = 0;
	emr_colocado_anterior = 0;
	Total_Iter_Packing = a;
}
void truck::SetMultipleCliente(vector < item_modelo> &vec_items)
{
	if (vec_items.front().orden_ruta != vec_items.back().orden_ruta)
	{
		multiple_cliente = true;
	}
	else multiple_cliente = false;
}
bool truck::ComprobarPesos()
{
	double tpeso = 0,tpeso_x_medio=0;
	double teje = 0, tejr = 0, temh = 0, temr = 0, temm = 0;
	for (auto it : pilas_solucion)
	{
		tpeso += it.id_pila.peso;
		tpeso_x_medio = tpeso_x_medio + ((static_cast<double>(it.id_pila.x_ini)+(static_cast<double>(it.id_pila.x_fin- it.id_pila.x_ini)/2)) * (it.id_pila.peso));
		teje = tpeso_x_medio / tpeso;
		tejr = distance_star_trailer_harness + distance_harness_rear_axle - teje;
		temh = ((tpeso * tejr) + (weight_empty_truck * distance_cog_tractor_rear_axle)) / distance_harness_rear_axle;
		temr = tpeso + weight_empty_truck - temh;
		//TO DO en la VA itc->distance_front_midle_axle es 0
		temm = 0;
		if (distance_front_midle_axle > 0)
			temm = ((weight_tractor * distance_front_axle_cog_tractor) + (temh * distnace_front_axle_harness_tractor)) / distance_front_midle_axle;

	}
	//quiero saber si el peso de la carga es el mismo que el que calculo
	


	if (temm != emm_colocado)
	{
		PintarProblema(45);
		return false;
	}
	if (temr != emr_colocado)
	{
		PintarProblema(46);
		return false;
	}
	return true;

}
void truck::reanudar()
{
	Empezando_Camion = false;
	//items_modelo.clear();
	//items_extras_modelo.clear();
	peso_total_items_modelo = 0;
	volumen_total_items_modelo = 0;
	//num_items_modelo = 0;
	//num_items_extra.clear();
	//peso_total_items_modelo_extra.clear();
	//volumen_total_items_modelo_items_extra.clear();
	dim_minima = 0;
	coste_inventario_camion = 0;
	coste_inventario_item = 0;
	pila_multi_dock = false;
	Modelo_pesado = false;
	peso_total_items_modelo_asignados = 0;
	volumen_total_items_modelo_asignados = 0;
	num_extras_creados = 0;
	total_items_modelos = 0;
	index_minimo = 0;
	codigo_cierre = 0;
	Abajo = true;
	pilas_solucion.clear();
	orden_por_el_que_voy = 0;
	volumen_ocupado = 0;
	Intensidad_Pallets_Medios = 1;
	for (auto it = lesp.begin(); it != lesp.end(); it++)
	{
		(*it).clear();
	}
	//pcamiones_extra.clear();
	lleno = false;
	volumen_a_cargar = 0; //Volumen que voy a querer cargar en el camion
	peso_a_cargar = 0; //Peso que voy a querer cargar en el camion
	Empezando_Camion = true;
	emm_colocado = 0;
	emr_colocado = 0;
	emm_colocado_anterior = 0;
	emr_colocado_anterior = 0;
	peso_cargado = 0;
	no_puede_colocar_nada = false;
	vol_antes = 0;
	peso_antes = 0;
	recargar = false;
	//Cargar los items de colocados

	for (auto its = items_sol.begin(); its != items_sol.end(); its++)
	{
		vector<item_modelo>::iterator itm = items_modelo.begin();
		for (; itm != items_modelo.end(); itm++)
		{
			if ((*its).id_item->CodIdCC == (*itm).id_item->CodIdCC)
			{
				if ((*its).colocado > 0)
				{
					(*itm).num_items+= (*its).colocado;
					(*itm).id_item->num_items += (*its).colocado;
					if ((*its).obligado) (*itm).obligado = 0;
					num_items_modelo += (*its).colocado;
					peso_total_items_modelo += (*its).colocado * (*itm).id_item->weight;
					volumen_total_items_modelo += (*its).colocado * (*itm).id_item->volumen;
					if (dim_minima > (*itm).id_item->length)dim_minima = (*itm).id_item->length;
					if (dim_minima > (*itm).id_item->width)dim_minima = (*itm).id_item->width;
					(*itm).colocado = 0;
				}
				else
				{
					num_items_modelo += (*itm).num_items;
					peso_total_items_modelo += (*itm).num_items * (*itm).id_item->weight;
					volumen_total_items_modelo += (*itm).num_items * (*itm).id_item->volumen;
					if (dim_minima > (*itm).id_item->length)dim_minima = (*itm).id_item->length;
					if (dim_minima > (*itm).id_item->width)dim_minima = (*itm).id_item->width;
				}
				break;
			}
		}
		if (itm == items_modelo.end())
		{
			(*its).num_items = (*its).colocado;
			(*its).id_item->num_items += (*its).colocado;			
			num_items_modelo += (*its).colocado;
			peso_total_items_modelo += (*its).colocado * (*itm).id_item->weight;
			volumen_total_items_modelo += (*its).colocado * (*itm).id_item->volumen;
			if (dim_minima > (*itm).id_item->length)dim_minima = (*itm).id_item->length;
			if (dim_minima > (*itm).id_item->width)dim_minima = (*itm).id_item->width;
			(*its).colocado = 0;
			items_modelo.push_back((*its));
		}
	}
	items_sol.clear();
	total_items_modelos = num_items_modelo;

	
	
	/*for (auto itm = items_modelo.begin(); itm != items_modelo.end(); itm++)
	{
		cout<< "X_"<<(*itm).id_item->CodIdCC << " = " << (*itm).num_items << "hay"<< (*itm).id_item->num_items<<endl;
	}*/
}
void truck::reset2()
{

	index_minimo = -1;
	num_items_modelo = 0;
	total_items_modelos = 0;
	peso_total_items_modelo = 0;
	volumen_total_items_modelo = 0;
	dim_minima = length;
	coste_invetario_items_por_colocar = 0;



}
void truck::AddItem(item* iT, int cuantos, int orden, int obli)
{
	items_modelo.push_back(item_modelo(iT, cuantos , orden, obli));
	num_items_modelo += cuantos;
	peso_total_items_modelo += cuantos * iT->weight;
	volumen_total_items_modelo += cuantos * iT->volumen;
	if (dim_minima > iT->length) dim_minima = iT->length;
	if (dim_minima > iT->width) dim_minima = iT->width;

}
int truck::DiferenciaDiasCamCam(truck* iT)
{
	return Days_0001(arrival_time_tm_ptr.tm_year, arrival_time_tm_ptr.tm_mon + 1, arrival_time_tm_ptr.tm_mday)
		- Days_0001(iT->arrival_time_tm_ptr.tm_year, iT->arrival_time_tm_ptr.tm_mon + 1, iT->arrival_time_tm_ptr.tm_mday);
}
int truck::DiferenciaDiasCamItem(item* iT)
{
	return Days_0001(arrival_time_tm_ptr.tm_year, arrival_time_tm_ptr.tm_mon + 1, arrival_time_tm_ptr.tm_mday)
		- Days_0001(iT->earliest_arrival_tm_ptr.tm_year, iT->earliest_arrival_tm_ptr.tm_mon + 1, iT->earliest_arrival_tm_ptr.tm_mday + 1);

}
int truck::DiferenciaDiasLastCamItem(item* iT)
{
	return Days_0001(arrival_time_tm_ptr.tm_year, arrival_time_tm_ptr.tm_mon + 1, arrival_time_tm_ptr.tm_mday)
		- Days_0001(iT->camiones.back()->arrival_time_tm_ptr.tm_year, iT->camiones.back()->arrival_time_tm_ptr.tm_mon + 1, iT->camiones.back()->arrival_time_tm_ptr.tm_mday + 1);

}
void truck::reanudar_solucion()
{
	Empezando_Camion = false;
	//items_modelo.clear();
	//items_extras_modelo.clear();
	peso_total_items_modelo = 0;
	volumen_total_items_modelo = 0;
	//num_items_modelo = 0;
	//num_items_extra.clear();
	//peso_total_items_modelo_extra.clear();
	//volumen_total_items_modelo_items_extra.clear();
	dim_minima = 0;
	coste_inventario_item = 0;
	coste_inventario_camion = 0;
	pila_multi_dock = false;
	Modelo_pesado = false;
	peso_total_items_modelo_asignados = 0;
	volumen_total_items_modelo_asignados = 0;
	num_extras_creados = 0;
	total_items_modelos = 0;
	index_minimo = 0;
	codigo_cierre = 0;
	Abajo = true;
	pilas_solucion.clear();
	orden_por_el_que_voy = 0;
	volumen_ocupado = 0;
	Intensidad_Pallets_Medios = 1;
	for (auto it = lesp.begin(); it != lesp.end(); it++)
	{
		(*it).clear();
	}
	//pcamiones_extra.clear();
	lleno = false;
	volumen_a_cargar = 0; //Volumen que voy a querer cargar en el camion
	peso_a_cargar = 0; //Peso que voy a querer cargar en el camion
	Empezando_Camion = true;
	emm_colocado = 0;
	emr_colocado = 0;
	emm_colocado_anterior = 0;
	emr_colocado_anterior = 0;
	peso_cargado = 0;
	no_puede_colocar_nada = false;
	vol_antes = 0;
	peso_antes = 0;
	recargar = false;
	//Cargar los items de colocados

	for (auto its = items_sol.begin(); its != items_sol.end(); its++)
	{
		vector<item_modelo>::iterator itm = items_modelo.begin();
		for (; itm != items_modelo.end(); itm++)
		{
			if ((*its).id_item->CodIdCC == (*itm).id_item->CodIdCC)
			{
				if ((*its).colocado > 0)
				{
					
						(*itm).num_items += (*its).colocado;
						(*itm).id_item->num_items += (*its).colocado;
						if ((*its).obligado) (*itm).obligado = 0;
						num_items_modelo += (*its).colocado;
						peso_total_items_modelo += (*its).colocado * (*itm).id_item->weight;
						volumen_total_items_modelo += (*its).colocado * (*itm).id_item->volumen;
						if (dim_minima > (*itm).id_item->length)dim_minima = (*itm).id_item->length;
						if (dim_minima > (*itm).id_item->width)dim_minima = (*itm).id_item->width;
						(*itm).colocado = 0;
					
				}
				else
				{
					num_items_modelo += (*itm).num_items;
					peso_total_items_modelo += (*itm).num_items * (*itm).id_item->weight;
					volumen_total_items_modelo += (*itm).num_items * (*itm).id_item->volumen;
					if (dim_minima > (*itm).id_item->length)dim_minima = (*itm).id_item->length;
					if (dim_minima > (*itm).id_item->width)dim_minima = (*itm).id_item->width;
				}
				break;
			}
		}
		if (itm == items_modelo.end())
		{
			(*its).num_items = (*its).colocado;
			(*its).id_item->num_items += (*its).colocado;
			num_items_modelo += (*its).colocado;
			peso_total_items_modelo += (*its).colocado * (*itm).id_item->weight;
			volumen_total_items_modelo += (*its).colocado * (*itm).id_item->volumen;
			if (dim_minima > (*itm).id_item->length)dim_minima = (*itm).id_item->length;
			if (dim_minima > (*itm).id_item->width)dim_minima = (*itm).id_item->width;
			(*its).colocado = 0;
			items_modelo.push_back((*its));
		}
	}
	items_sol.clear();
	total_items_modelos = num_items_modelo;


	
		/*for (auto itm = items_modelo.begin(); itm != items_modelo.end(); itm++)
		{
			cout << "X_" << (*itm).id_item->CodIdCC << " = " << (*itm).num_items << "hay" << (*itm).id_item->num_items << endl;
		}*/
	
}