#include "encabezados.h"


void leer_datos( parameters& params, list<item*>& lista_items, list<truck*>& lista_cam, map <string, truck*>& m_camiones, map<string, int>& proveedores,map <string, list<item*>>& proveedor_items)
{
	string archivo_parametros =params.param_file;
	string archivo_items = params.input_items;
	string archivo_trucks = params.input_trucks;

	string linea;
	char delimitador = ';';
	
	truck* cam_m_vol=NULL;
	truck* cam_m_peso=NULL;
	double peso_tot=0;
	double vol_tot=0;
	int num_proveedor=0;
	int num_supplier_code = 0;
	int num_supplier_dock=0;
	int num_plant_code = 0;
	int num_plant_dock = 0;
	int num_product_code=0;
	int num_stackability_code=0;
	int num_sup_cod_dock = 0;

	//lista auxiliar, para saber que productos van en la instancia
	map<string, list<item*>> prodprov_item;  //por cada codigo producto, lista de items de ese producto. 
	
	map<string, int> prod_peso_bottom;
	map<string, int> map_supplier_code;
	map<string, int> map_supplier_dock;
	map<string, int> map_plant_code;
	map<string, int> map_plant_dock;
	map<string, int> map_product_code;
	map<string, int> map_stackability_code;
	map<string, int> map_sup_cod_dock;
	

	/*********** LEO LOS PARÁMETROS **************************/

	ifstream archivo(archivo_parametros);
	if (!archivo)
		cout << "No se encuentra el archivo de parametros " << archivo_parametros << endl;

	getline(archivo, linea);// Leemos la primer línea para descartarla, pues es el encabezado
	// Leemos todas las líneas
	while (getline(archivo, linea))
	{
		stringstream stream(linea); // Convertir la cadena a un stream
		string ic, tc, et, rt;
		// Extraer todos los valores de esa fila
		getline(stream, ic, delimitador);
		replace(ic.begin(), ic.end(), ',', '.');  //para convertir el separador decimal de , a .
		getline(stream, tc, delimitador);
		replace(tc.begin(), tc.end(), ',', '.');
		getline(stream, et, delimitador);
		replace(et.begin(), et.end(), ',', '.');
		getline(stream, rt, delimitador);
		replace(rt.begin(), rt.end(), ',', '.');
		//añado los valores a la clase parameters
		params.inventory_cost = stof(ic.c_str());
		params.transportation_cost = stof(tc.c_str());
		params.extra_truck_cost = stof(et.c_str());
		params.time_limit = atoi(rt.c_str());
	}
	
	/*********** LEO LOS ITEMS **************************/
	ifstream archivo2(archivo_items);
	if (!archivo2)
		cout << "No se encuentra el archivo de items" << archivo_items << endl;

//		cout << "Ha leido hasta aqui dddll" << endl;
	int idi = 0; //id del item del 0 al N
	getline(archivo2, linea);// Leemos la primer línea para descartarla, pues es el encabezado
	// Leemos todas las líneas
	while (getline(archivo2, linea))
	{
		item* it = new item();
		stringstream stream(linea); // Convertir la cadena a un stream
		string id, sc, sd, pc, pd, prc, pacc, ni, l, w, h, we, ne, stc, fo, eat, lat, ic, ms;		
		
		// Extraer todos los valores de esa fila
		getline(stream, id, delimitador);
		getline(stream, sc, delimitador);
		getline(stream, sd, delimitador);
		getline(stream, pc, delimitador);
		getline(stream, pd, delimitador);
		getline(stream, prc, delimitador);
		getline(stream, pacc, delimitador);
		getline(stream, ni, delimitador);
		getline(stream, l, delimitador);
		getline(stream, w, delimitador);
		getline(stream, h, delimitador);
		getline(stream, we, delimitador);
		replace(we.begin(), we.end(), ',', '.');
		getline(stream, ne, delimitador);
		getline(stream, stc, delimitador);
		getline(stream, fo, delimitador);
		getline(stream, eat, delimitador);
		getline(stream, lat, delimitador);
		getline(stream, ic, delimitador);
		getline(stream, ms, delimitador);

		//añado los valores a la clase item
		strcpy(it->Id, id.c_str());
		
		strcpy(it->suplier_code, sc.c_str());
		if ((map_supplier_code.empty()) || (map_supplier_code.count(sc) == 0))
		{
			map_supplier_code.insert(pair<string, int>(sc, num_supplier_code));
			it->suplier_code_int = num_supplier_code;
			num_supplier_code++;
		}
		else
			it->suplier_code_int = map_supplier_code.at(sc);
		
		//cout << it->suplier_code << " ; " << it->suplier_code_int << endl;
		strcpy(it->suplier_dock, sd.c_str());
	
		if ((map_supplier_dock.empty()) || (map_supplier_dock.count(sd) == 0))
		{
			map_supplier_dock.insert(pair<string, int>(sd, num_supplier_dock));
			it->suplier_dock_int = num_supplier_dock;
			num_supplier_dock++;
		}
		else
			it->suplier_dock_int = map_supplier_dock.at(sd);

		//cout << it->suplier_dock << " ; " << it->suplier_dock_int << endl;
		
		strcpy(it->sup_cod_dock, sc.c_str());
		strcat(it->sup_cod_dock, "|");
		strcat(it->sup_cod_dock,  sd.c_str());
		strcat(it->sup_cod_dock, "|");
		strcat(it->sup_cod_dock, pd.c_str());
		if ((map_sup_cod_dock.empty()) || (map_sup_cod_dock.count(it->sup_cod_dock) == 0))
		{
			map_sup_cod_dock.insert(pair<string, int>(it->sup_cod_dock, num_sup_cod_dock));
			it->sup_cod_dock_int = num_sup_cod_dock;
			num_sup_cod_dock++;
		}
		else
			it->sup_cod_dock_int = map_sup_cod_dock.at(it->sup_cod_dock);
		//cout << it->sup_cod_dock << " ; " << it->sup_cod_dock_int << endl;
		
		strcpy(it->plant_code, pc.c_str());
		if ((map_plant_code.empty()) || (map_plant_code.count(pc) == 0))
		{
			map_plant_code.insert(pair<string, int>(pc, num_plant_code));
			it->plant_code_int = num_plant_code;
			num_plant_code++;
		}
		else
			it->plant_code_int = map_plant_code.at(pc);

		//cout << it->plant_code << " ; " << it->plant_code_int << endl;
		strcpy(it->plant_dock, pd.c_str());
		if ((map_plant_dock.empty()) || (map_plant_dock.count(pd) == 0))
		{
			map_plant_dock.insert(pair<string, int>(pd, num_plant_dock));
			it->plant_dock_int = num_plant_dock;
			num_plant_dock++;
		}
		else
			it->plant_dock_int = map_plant_dock.at(pd);

		//cout << it->plant_dock << " ; " << it->plant_dock_int << endl;
		
		
		
		strcpy(it->product_code, prc.c_str());
		if ((map_product_code.empty()) || (map_product_code.count(prc) == 0))
		{
			map_product_code.insert(pair<string, int>(prc, num_product_code));
			it->product_code_int = num_product_code;
			num_product_code++;
		}
		else
			it->product_code_int = map_product_code.at(prc);
		
		strcpy(it->package_code, pacc.c_str());
		it->num_items = atoi(ni.c_str());
		
		it->length = atoi(l.c_str());
		it->width = atoi(w.c_str());
		it->height = atoi(h.c_str());
		//se divide las dimensiones entre 1000 pra evitar desbordamiento
		vol_tot += (((double)it->length/1000) * ((double)it->width/1000) * ((double)it->height/1000)) * it->num_items;
		it->weight = stof(we.c_str());

		//se divide el peso entre 100 para evitar desbordamiento al calcular el peso total de la instancia
		peso_tot += ((double)it->weight/100) * it->num_items;	
		it->volumen = (((double)it->length / 1000) * ((double)it->width / 1000) * ((double)it->height / 1000));
		it->nesting_height = atoi(ne.c_str());

		it->vol_con_nest = (((double)it->length / 1000) * ((double)it->width / 1000) * ((double)(it->height - it->nesting_height) / 1000));
		/*cout << it->height << " " << it->nesting_height << " " << it->height - it->nesting_height << endl;
		cout << it->volumen << " ; " << it->vol_con_nest << endl;
		if (it->nesting_height > 0)
			int kk = 0;*/

		strcpy(it->stackability_code, stc.c_str());
		if ((map_stackability_code.empty()) || (map_stackability_code.count(stc) == 0))
		{
			map_stackability_code.insert(pair<string, int>(stc, num_stackability_code));
			it->stackability_code_int = num_stackability_code;
			num_stackability_code++;
		}
		else
			it->stackability_code_int = map_stackability_code.at(stc);
		//cout << it->stackability_code << " ; " << it->stackability_code_int << endl;
		
		strcpy(it->forced_orientation, fo.c_str());
		if (strcmp(it->forced_orientation, "none") == 0)
			it->forced_orientation_int = 0;
		else
		{
			if (strcmp(it->forced_orientation, "widthwise") == 0)it->forced_orientation_int = 2;
			else it->forced_orientation_int = 1;
		}
		it->densidad = it->weight / it->volumen;
//		cout << it->weight << " " << it->volumen << " " << it->densidad << endl;
		//Tiempo de llegada del item, se guarda en formato original y en formato fecha
		it->earliest_arrival = stoll(eat.c_str(), NULL, 10);
		it->latest_arrival = stoll(lat.c_str(), NULL, 10);
		tm et{};
		et.tm_sec = 0;
		et.tm_min = stoi(eat.substr(10, 2));
		et.tm_hour = stoi(eat.substr(8, 2));
		et.tm_mday = stoi(eat.substr(6, 2));
		et.tm_mon = stoi(eat.substr(4, 2)) - 1;
		et.tm_year = stoi(eat.substr(0, 4)) - 1900;
		et.tm_isdst = -1; //FALTABA ESTO
//		
		it->earliest_arrival_tm_ptr = et;
		it->earliest_arrival_tm = std::mktime(&et);
		
		tm lt{};
		lt.tm_sec = 0;
		lt.tm_min = stoi(lat.substr(10, 2));
		lt.tm_hour = stoi(lat.substr(8, 2));
		lt.tm_mday = stoi(lat.substr(6, 2));
		lt.tm_mon = stoi(lat.substr(4, 2))-1;
		lt.tm_year = stoi(lat.substr(0, 4))-1900;
		lt.tm_isdst = -1; //FALTABA ESTO
//		
		it->latest_arrival_tm_ptr = lt;
		it->latest_arrival_tm = std::mktime(&lt);
		
		it->inventory_cost = atoi(ic.c_str());
		it->max_stackability = atoi(ms.c_str());
		it->colocado = false;
		it->num_items_original = it->num_items;
		it->CodId = idi;		
		idi++;
		

		
		//Añado a la lista de proveedores, su id y el entero que lo identifica.
		//Esto es para las componente conexas que se le pasan como enteros.
		string clave_p = sc+"|"+sd+"|"+pd;
		if ((proveedores.empty()) || (proveedores.count(clave_p) == 0))
		{
			proveedores.insert(pair<string,int> (clave_p,num_proveedor));
			num_proveedor++;
		}
		
		
		//lista auxiliar (producto,proveedor) ->lista_de items de la instancia
		string clave = prc + "|" + sc;		
		if ((!prodprov_item.empty()) && (prodprov_item.count(clave) > 0))
		{
			prodprov_item[clave].push_back(it); //si ya existe la clave, añado el item a la lista
		}
		else
		{
			list<item*> li_au;
			li_au.push_back(it);
			prodprov_item.insert(pair<string, list<item*>>(clave, li_au));
		}
		
		// listado provedor-items para las componentes
		if ((!proveedor_items.empty()) && (proveedor_items.count(clave_p) > 0))
		{
			proveedor_items[clave_p].push_back(it); //si ya existe la clave, añado el item a la lista
		}
		else
		{
			list<item*> li_au;
			li_au.push_back(it);
			proveedor_items.insert(pair<string, list<item*>>(clave_p, li_au));
		}

		
		//añado el item a la lista de items de la instancia
		lista_items.push_back(it);	

			
	}
	

//		cout << "Ha leido hasta aquí 1\n" << endl;

	/*********** LEO LOS camiones **************************/
	ifstream archivo3(archivo_trucks);
	if (!archivo3)
		cout << "No se encuentra el archivo de camiones" << archivo_trucks << endl;
	getline(archivo3, linea);// Leemos la primer línea para descartarla, pues es el encabezado
	// Leemos todas las líneas
	while (getline(archivo3, linea))
	{
		truck* camion=new truck();		
		
		stringstream stream(linea); // Convertir la cadena a un stream
		string cos, slo, sd, sdlo, pc, pd, pdlo, prc, at, id, l, w, h, mlw, smd, msd, mwbi, c, maw, raw, wt, dfma, dfacogt, dfht, wet, dhrat, dcogra, dsh;
		// Extraer todos los valores de esa fila
		getline(stream, cos, delimitador);
		getline(stream, slo, delimitador);
		getline(stream, sd, delimitador);
		getline(stream, sdlo, delimitador);
		getline(stream, pc, delimitador);
		getline(stream, pd, delimitador);
		getline(stream, pdlo, delimitador);
		getline(stream, prc, delimitador);
		getline(stream, at, delimitador);
		getline(stream, id, delimitador);
		getline(stream, l, delimitador);
		getline(stream, w, delimitador);
		getline(stream, h, delimitador);
		getline(stream, mlw, delimitador);
		getline(stream, smd, delimitador);
		getline(stream, msd, delimitador);
		getline(stream, mwbi, delimitador);
		getline(stream, c, delimitador);
		getline(stream, maw, delimitador);
		getline(stream, raw, delimitador);
		getline(stream, wt, delimitador);
		replace(wt.begin(), wt.end(), ',', '.');
		getline(stream, dfma, delimitador);
		getline(stream, dfacogt, delimitador);
		getline(stream, dfht, delimitador);
		getline(stream, wet, delimitador);
		replace(wet.begin(), wet.end(), ',', '.');
		getline(stream, dhrat, delimitador);
		getline(stream, dcogra, delimitador);
		getline(stream, dsh, delimitador);

			
		//Si el proveedor está en la instancia y el producto está en la instancia, genero el camión, sino NO.
		//Porque hay camiones que pueden llevar productos que no están en la instancia.
		string clave = prc + "|" + cos;
		if (prodprov_item.count(clave)>0)
		{
			//Compruebo si el camión existe
			if ((!m_camiones.empty()) && (m_camiones.count(id.c_str()) > 0))
			{
				//Añado producto a la lista de productos que puede llevar el camión
				m_camiones[id.c_str()]->code_product.push_back(prc.c_str());
				
				//Añado proveedor, si no está genero el codigo proveedor|muelle.
				string codigo_proveedor = cos + "|" + sd;
				int sup_or = atoi(slo.c_str()); //lo coloco en su sitio, orden.
				int pd_order = atoi(pdlo.c_str());
				int sup_dock = atoi(sdlo.c_str());				
				
				//añado el peso por producto
				m_camiones[id.c_str()]->pesos_bottom_prod.insert(pair<string, int>(prc, atoi(mwbi.c_str())));
				
				string id_ruta = cos + "|" + sd + "|" + pd;
				int prio = sup_or * 100 + sup_dock * 10 + pd_order;
				m_camiones[id.c_str()]->orden_ruta.insert(pair<string,int>(id_ruta,prio));
				
			}
			else //si el camión no existe
			{
				
				camion->code_product.push_back(prc.c_str());
				
				//tiempo de llegada del camión en formato original y en formato fecha hora
				camion->arrival_time = stoll(at.c_str(), NULL, 10);				
				//tm lt = { 0 };
				tm lt{ };
				lt.tm_sec = 0;
				lt.tm_min = atoi(at.substr(10, 2).c_str());
				lt.tm_hour = atoi(at.substr(8, 2).c_str()) ;
				lt.tm_mday = atoi(at.substr(6, 2).c_str());
				lt.tm_mon = atoi(at.substr(4, 2).c_str()) - 1;
				lt.tm_year = atoi(at.substr(0, 4).c_str())-1900;
				lt.tm_isdst = 1; //faltaba esto

//				if (strcmp(id.c_str(),"P080436601")==0)
//				int kk = 9;
				camion->arrival_time_tm_ptr = lt;
				camion->arrival_time_tm = mktime(&lt);

				
				strcpy(camion->id_truck, id.c_str());
				camion->num_extras_creados = 0;
				camion->length = atoi(l.c_str());
				camion->width = atoi(w.c_str());
				camion->height = atoi(h.c_str());
				camion->volumen = ((double)camion->length/1000) * ((double)camion->width/1000) ;
				camion->volumen=camion->volumen* ((double)camion->height/1000);
				camion->max_loading_weight = atoi(mlw.c_str());
				camion->peso_heuristico = camion->max_loading_weight;
				camion->max_loading_weight_orig = camion->max_loading_weight;
				camion->volumen_heuristico = camion->volumen;
				camion->stack_with_multiple_docks = atoi(smd.c_str());
				camion->max_stack_density = atoi(msd.c_str());
				camion->max_weight_bottom_item_stack = atoi(mwbi.c_str());
				camion->cost = atoi(c.c_str());
				camion->coste_extra = camion->cost + (params.extra_truck_cost*camion->cost);
				camion->max_weight_middle_axle = atoi(maw.c_str());
				camion->max_weight_rear_axle = atoi(raw.c_str());
				camion->weight_tractor = stof(wt.c_str());
				camion->distance_front_midle_axle = atoi(dfma.c_str());
				camion->distance_front_axle_cog_tractor = atoi(dfacogt.c_str());
				camion->distnace_front_axle_harness_tractor = atoi(dfht.c_str());
				camion->weight_empty_truck = stof(wet.c_str());
				camion->distance_harness_rear_axle = atoi(dhrat.c_str());
				camion->distance_cog_tractor_rear_axle = atoi(dcogra.c_str());
				camion->distance_star_trailer_harness = atoi(dsh.c_str());
				camion->pila_multi_dock = false;
				camion->lleno = false;
				camion->codigo_cierre = 0;
				camion->volumen_ocupado = 0;
				camion->peso_inicial = 0;
				camion->volumen_inicial = 0;
				camion->peso_cargado = 0;
				camion->orden_por_el_que_voy = 0;
				camion->no_puede_colocar_nada = false;
				camion->Empezando_Camion = true;
				camion->emm_colocado = 0;
				camion->emr_colocado = 0;
				camion->emm_colocado_anterior = 0;
				camion->emr_colocado_anterior = 0;
				camion->vol_antes = 0;
				camion->peso_antes = 0;
				if (cam_m_vol != NULL)
				{
					if((camion->volumen) > (cam_m_vol->volumen))
						cam_m_vol=camion;
				}
				else cam_m_vol = camion;
				if (cam_m_peso != NULL)
				{
					if ((camion->max_loading_weight) > (cam_m_peso->max_loading_weight))
						cam_m_peso = camion;
				}
				else cam_m_peso = camion;
				
				//añado el producto peso de abajo
				camion->pesos_bottom_prod.insert(pair<string, int>(prc, atoi(mwbi.c_str())));
				
				m_camiones.insert(pair<string, truck*>(camion->id_truck, camion));
				string id_ruta = cos + "|" + sd + "|" + pd;
				int dp = atoi(sdlo.c_str());
				int pd_order = atoi(pdlo.c_str());
				int sup_or = atoi(slo.c_str());
				int prio = sup_or * 100 + dp * 10 + pd_order;
				camion->orden_ruta.insert(pair<string,int>(id_ruta,prio));
				lista_cam.push_back(camion);
			}
			
			
			//Añado los items de ese producto-proveedor, que pueden ir en el camión
			for (list<item*>::iterator iti = prodprov_item[clave].begin(); iti != prodprov_item[clave].end(); iti++)
			{
				//por ventana temporal, dock, plant, plant dock
				if (((*iti)->earliest_arrival <= m_camiones[id.c_str()]->arrival_time && (*iti)->latest_arrival >= m_camiones[id.c_str()]->arrival_time)
					&& (strcmp((*iti)->suplier_dock, sd.c_str()) == 0)
					&& (strcmp((*iti)->plant_code, pc.c_str()) == 0)
					&& (strcmp((*iti)->plant_dock, pd.c_str()) == 0)
					)
				{
					m_camiones[id.c_str()]->items.push_back((*iti)); //añado el item al camión.
					(*iti)->lista_camiones.push_back(id.c_str()); //añado el camión a el item.
					m_camiones[id.c_str()]->dimensiones_posibles_base.insert((*iti)->length);			
					m_camiones[id.c_str()]->dimensiones_posibles_base.insert((*iti)->width);					
					m_camiones[id.c_str()]->dimensiones_posibles_height.insert((*iti)->height);

				}
			}
		}
	}
	archivo3.close();
	
	//Calculo la cota de camiones por peso y volumen de la instancia.
	params.cam_x_peso = peso_tot / (cam_m_peso->max_loading_weight / 100.0);
	params.cam_x_vol = vol_tot / cam_m_vol->volumen;


//		cout << "Ha leido hasta aquí 2\n" << endl;
	
	//Incluir puntero a camion en cada item
	auto itc_ant = lista_cam.begin();
	int idc = 0; //id del camion 0 a Ncamiones

	for (auto it_c = lista_cam.begin(); it_c != lista_cam.end(); it_c++)
	{
		double pes_it = 0;
		double vol_it = 0;
	
//			cout << "Ha leido hasta " << (*it_c)->CodId << endl;
		// Eliminar los camiones que no tienen items que pueden cargar.
		if ((*it_c)->items.size() == 0)
		{
		
//				cout << "Nada " << endl;
//			free((*it_c));
			lista_cam.erase(it_c--);
//			it_c = itc_ant;

//			cout << "Nada 2" << endl;
		}
		else
		{
			(*it_c)->CodId = idc;  //añadimos el id del camión de 0 a Ncam
			idc++;
			//Calculamos el máximo width y length de los camiones
			(*it_c)->length_kp = knapsack_bouknap((*it_c)->dimensiones_posibles_base, (*it_c)->length);
			(*it_c)->width_kp = knapsack_bouknap((*it_c)->dimensiones_posibles_base, (*it_c)->width);
			//(*it_c)->height_kp = knapsack_bouknap((*it_c)->dimensiones_posibles_height, (*it_c)->height);
			(*it_c)->volumen_kp = (((double)(*it_c)->length_kp / 1000) * ((double)(*it_c)->width_kp / 1000) * ((double)(*it_c)->height / 1000));

			//CAMBIAR
			(*it_c)->volumen_heuristico = (((double)(*it_c)->length / 1000) * ((double)(*it_c)->width/ 1000) * ((double)(*it_c)->height / 1000));

//			cout << "Algo " << endl;
			//Para todos los items que puede llevar el camión, calculamos su vol y peso
			for (auto it_i = (*it_c)->items.begin(); it_i != (*it_c)->items.end(); it_i++)
			{
				(*it_i)->camiones.push_back(*it_c); //Añadirmos puntero al camión en el item
				pes_it += (*it_i)->num_items * (*it_i)->weight;
				vol_it += (*it_i)->num_items * ((double)(*it_i)->length / 1000) * ((double)(*it_i)->width / 1000) * ((double)(*it_i)->height / 1000);

			}
			//Añadimos el peso y vol total de los items que pùede llevar el camión
			(*it_c)->peso_total_items = pes_it;
			(*it_c)->volumen_total_items = vol_it;

			
			//Calculamos la cota del número de camiones extra por peso y volumen, necesarios para trasportar todos los items.
//			double dp = pes_it / ((*it_c)->max_loading_weight / 100);
			int cot_1_pes = (int)ceil(pes_it / ((*it_c)->max_loading_weight ));
			int cot_1_vol = (int)ceil(vol_it / (*it_c)->volumen_kp);
			int cot_1 = std::max(cot_1_pes, cot_1_vol);

			(*it_c)->cota_extra = (double)(1.2*(cot_1+1));

			//if (cot_1 > 1)
			//	int kk = 9;
			espacios esp;
			for (int i = 0; i <= cot_1; i++)
			{
				(*it_c)->lesp.push_back(esp);
			}

			
		}
//		cout << "Nada 3" << endl;
//		itc_ant = it_c;
//		cout << "Nada 4" << endl;

	}
}
//Escribe las componentes

//Escribe las componentes
void escribe_info_componentes(Problema& Prob, string instancia, int tot_cam, int tot_item, int prove)
{
	string nombreArchivo1 = "porcompo.csv";
	ofstream archivo;
	// Abrimos el archivo
	archivo.open(nombreArchivo1.c_str(), ofstream::out | ofstream::app);

	archivo << instancia <<  ";" << tot_cam <<";"<< tot_item << ";" << Prob.subproblemas.size() <<";"<< prove << endl;
	
	/*for (int its = 0; its < Prob.subproblemas.size(); its++)
	{
		archivo << " ; ; ; ;C"<< its <<";" << Prob.subproblemas.at(its)->lista_camiones.size() << "; " << Prob.subproblemas.at(its)->lista_items.size() << endl;
	}*/
	
	archivo.close();

	//int total_cam = 0;
	//int total_item = 0;
	//int total_extra = 0;
	//int total_inventory = 0;
	//int max_ex = 0;
	//int max_inv = 0;
	//float med_inv=0;
	////Calcular estadísticas componentes
	//for (auto its = Prob.subproblemas.begin(); its != Prob.subproblemas.end(); its++)
	//{
	//	total_cam += (*its)->lista_camiones.size();
	//	total_item += (*its)->total_items;
	//	total_extra += (*its)->total_extra;
	//	med_inv += (*its)->media_inventory_item;

	//	
	//	if (max_ex < (*its)->max_cam_extra) max_ex = (*its)->max_cam_extra;
	//	if (max_inv < (*its)->max_invetory_item) max_inv = (*its)->max_invetory_item;

	//}
	//ofstream f("./estacompo.txt", ios_base::app);
	//f << file_to_read<<";"<< Prob.subproblemas.size() << ";" << total_cam << ";" << total_item << ";" << total_extra / total_cam << ";" << max_ex << ";" << med_inv / Prob.subproblemas.size() << ";" << max_inv << ";" << endl;
	//f.close();
}


void Problema::escribeG(parameters& params, mejor_sol mej)
{
	total_volumen = 0;
	total_volumen_camiones = 0;
	total_peso = 0;
	total_peso_camiones = 0;
	ofstream ft(params.output_trucks, ios_base::trunc);
	ft << "Id truck; Loaded length; Weight of loaded items; Volumevof loaded items; emm; emr" << endl;

	ofstream fs(params.output_stack, ios_base::trunc);
	fs << "Id truck; Id stack; Stack code; X origin; Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;

	ofstream fi(params.output_items, ios_base::trunc);
	fi << "Item ident;Id truck;Id stack;Item code;X origin;Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;
	int compo = 0;

	for (auto itc = mej.solucion_compo.begin(); itc != mej.solucion_compo.end(); itc++)
	{
		//cout << "Compo:" <<compo<<" cam:" <<(*itc).listado_camiones.size() <<" pilas:"<< (*itc).listado_pilas.size() << "items: "<< (*itc).listado_items.size()<<endl;

		for (auto it = (*itc).listado_items.begin(); it != (*itc).listado_items.end(); it++)
		{
			(*it).escribe(fi);

		}


		for (auto it = (*itc).listado_camiones.rbegin(); it != (*itc).listado_camiones.rend(); it++)
		{
			//			if (camiones_en_sol.count((*it).id_truc) == 0)
			//			{
			//				camiones_en_sol.insert(pair<string, bool>((*it).id_truc, true));
			total_volumen += (*it).volumen_temp_ocupado;
			total_peso += (*it).tmt;
			total_volumen_camiones += (*it).volumen_max;
			total_peso_camiones += (*it).peso_max;
			(*it).escribe(ft);
			//			}
			//			else
			//				int kk = 9;

		}


		for (auto it = (*itc).listado_pilas.begin(); it != (*itc).listado_pilas.end(); it++)
			(*it).escribe(fs);
	

		compo++;

	}
	
	
	ft.close();
	fs.close();
	fi.close();

}
void escribeG(parameters& params, mejor_sol mej)
{
//	map<string, bool> camiones_en_sol;
	ofstream ft(params.output_trucks, ios_base::trunc);
	ft << "Id truck; Loaded length; Weight of loaded items; Volumevof loaded items; emm; emr" << endl;
	
	ofstream fs(params.output_stack, ios_base::trunc);
	fs << "Id truck; Id stack; Stack code; X origin; Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;
	
	ofstream fi(params.output_items,ios_base::trunc);
	fi << "Item ident;Id truck;Id stack;Item code;X origin;Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;
	ofstream ft_c("output_trucks.csv", ios_base::trunc);

	int compo = 0;

	for (auto itc = mej.solucion_compo.begin(); itc != mej.solucion_compo.end(); itc++)
	{
		//cout << "Compo:" <<compo<<" cam:" <<(*itc).listado_camiones.size() <<" pilas:"<< (*itc).listado_pilas.size() << "items: "<< (*itc).listado_items.size()<<endl;

		for (auto it = (*itc).listado_items.begin(); it != (*itc).listado_items.end(); it++)
		{
			(*it).escribe(fi);

		}


		for (auto it = (*itc).listado_camiones.rbegin(); it != (*itc).listado_camiones.rend(); it++)
		{
//			if (camiones_en_sol.count((*it).id_truc) == 0)
//			{
//				camiones_en_sol.insert(pair<string, bool>((*it).id_truc, true));
				(*it).escribe(ft);
//			}
//			else
//				int kk = 9;
			
		}


		for (auto it = (*itc).listado_pilas.begin(); it != (*itc).listado_pilas.end(); it++)
			(*it).escribe(fs);
	
		compo++;

	}
	ft.close();
	fs.close();
	fi.close();
}
int PintarProblema(int a) {
	printf("Problema %d \n", a);
	//	 return 1;
	for (int i = 0; i < 1000; i++)
	{
		printf("Problema %d ", a);
	}
	exit(1);
	//return 1;
}
int Days_0001(int y, int m, int d) { /* Rata Die day one is 0001-01-01 */
	if (m < 3)
		y--, m += 12;
	return 365 * y + y / 4 - y / 100 + y / 400 + (153 * m - 457) / 5 + d - 306;
}


	//Leer archivo de productos
void escribe_item(string dir,  map <string, truck*> mapa_camiones,parameters para,list<item*> lista_item, list<truck*>& lista_cam)
{
	string nombreArchivo1 = "cam_x_item.csv";
	string nombreArchivo2 = dir + "compruebo.csv";
	string nombreArchivo3 = dir + "input_parametersM.csv";
	
	ofstream archivo;
	// Abrimos el archivo
	archivo.open(nombreArchivo1.c_str(), ofstream::out | ofstream::app);
	
	int min = (int)lista_cam.size(),min2= (int)lista_item.size();
	int max=0, med=0,min_width=10000,max2=0,med2=0,min3=min,med3=0,max3=0, tot_cam=0;
	int cam_a_cero = 0;
	int cont_item = 0, cont_cam=0;
	//recorremos la lita de items
	for (list<truck*>::iterator itt = lista_cam.begin(); itt != lista_cam.end(); itt++)
	{
		//Camiones con 0 items
		if ((*itt)->items.size()==0)
		{
			cam_a_cero++;
			continue;
		}
		else
		{
			//camiones por item, min, max, media

			if ((*itt)->items.size() < min)
				min = (int)(*itt)->items.size();
			if ((*itt)->items.size() > max)
				max = (int)(*itt)->items.size();
			med += (int)(*itt)->items.size();

			double pes_it = 0;
			double vol_it = 0;
			for (list<item*>::iterator iti = (*itt)->items.begin(); iti != (*itt)->items.end(); iti++)
			{
				pes_it +=(*iti)->num_items* (*iti)->weight / 100;
				vol_it += (*iti)->num_items* ((double)(*iti)->length / 1000) * ((double)(*iti)->width / 1000) * ((double)(*iti)->height / 1000);
			}

			(* itt)->peso_total_items = pes_it;
			(* itt)->volumen_total_items = vol_it;
			int cot_1_pes = (int)ceil(pes_it / ((*itt)->max_loading_weight / 100));
			int cot_1_vol = (int)ceil(vol_it / (*itt)->volumen_kp);
//			cout << "CotaV " << cot_1_vol << " Vol " << vol_it << "VolC " << (*itt)->volumen <<endl;
//			cout << "CotaP " << cot_1_pes << " Pes " << pes_it << "PesC " << ((*itt)->max_loading_weight / 100) << endl;

			int cot_1 = std::max(cot_1_pes, cot_1_vol);
			tot_cam += cot_1;
			//TODO poner en el leer y revisar si están bien cantidades
			(*itt)->cota_extra = cot_1-1;

			if (cot_1 < min3)min3 = cot_1;
			if (cot_1 > max3) max3 = cot_1;
			med3 += cot_1;
			
		}		
	}
	for(list<item*>::iterator it2=lista_item.begin();it2!=lista_item.end(); it2++)
	{
		cont_item += (*it2)->num_items;
		cont_cam += (int)(*it2)->lista_camiones.size();
		if (min2 > (*it2)->lista_camiones.size()) min2 = (int)(*it2)->lista_camiones.size();
		if (max2 < (*it2)->lista_camiones.size()) max2 = (int)(*it2)->lista_camiones.size();
		med2 += (int)(*it2)->lista_camiones.size();
		
		//minima with teniendo en cuenta que pueden ir orientados
		if (strcmp((*it2)->forced_orientation, "widthwise") == 0)
		{
			if (min_width > (*it2)->length) min_width = (*it2)->length;
		}
		else
		{
			if (min_width > (*it2)->width) min_width = (*it2)->width;
		}
		
	}
	
	archivo << dir << ";" << lista_cam.size() << ";" << cam_a_cero << ";" << min << ";" << med / lista_cam.size() << ";" << max << ";" << cont_item << ";" << min2 << ";" << med2 / lista_item.size() << ";" << max2 << ";"
		<< min_width << ";" << para.cam_x_peso << ";" << para.cam_x_vol << ";"<< min3 << ";" << med3/(lista_cam.size()-cam_a_cero) << ";" << max3 << endl;
	archivo.close();
};