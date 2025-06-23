#include "subproblema.h"

Subproblema::Subproblema()
{
	//Estas variables son para tener la menor dimension a lo largo y ancho de la
	//componente
	min_dim_width = 99999;
	min_dim_length = 99999;
	//InicializarVeces
	Iteracion_Modelo = 0;
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
	cuantos_item.resize(lista_items.size());
	//Esta matriz me indica que items pueden ir en cada camión
	//Podemos hacer que solamente vayan a un número de camiones
	matriz_variables.resize(lista_items.size(), std::vector<bool>(lista_camiones.size(), false));
	items_muchas_unidades.resize(lista_items.size());
	ObjBound = 0;
	ObjMip = 0;
	ObjBound_Original = 0;
	TiempoExtra = 0;
	TiempoModelo = 0;
	num_camiones_modelo = 0;
	num_camiones_extra_modelo = 0;
	max_cam_extra_tipo_modelo = 0;
	total_items = 0;
	coste_transporte = 0;
	coste_inventario = 0;
	coste_total = 0;
	porcentage_peso = 0;
	porcentage_vol = 0;
	optimo = false;
	cplex_trozo = false;
	cam_llenos = 0;
	Iter_Resuelve_Subproblema = 0;
	Best_Sol_Modelo = DBL_MAX;
	veces_pila_cambia = 0;
	veces_pila = 0;
}
Subproblema::~Subproblema()
{
	lista_camiones.clear();
	lista_items.clear();
	mapa_camiones.clear();
	mapa_item_camion.clear();
}
//Miro los que tiene más de dos 
void Subproblema::FiltroItems(int periodos)
{
	if (Todos_Los_Camiones)
	{
		Iter_Todos_Camiones++;
		return;
	}
	Todos_Los_Camiones = true;
	Iter_Todos_Camiones = 0;

	//Filtramos los items que no caben en ningún camión
	for (auto it =lista_items.begin(); it!= lista_items.end(); it++)
	{
		
		//Si es la primera vez, guardamos la lista de camiones original
		if (Iter_Resuelve_Subproblema > 0)
		{
			(*it)->camiones.clear();
			std::copy((*it)->camiones_originales.begin(), (*it)->camiones_originales.end(), std::back_inserter((*it)->camiones));

//			(*it)->camiones.insert((*it)->camiones_originales.begin(), (*it)->camiones_originales.back());
		}
		
		//SI tiene pocos no voy a quitar ninguno
		//Esto los quito de la lista de items de los camiones
		int cont = 0;
//		if ((*it)->camiones.size() == (periodos + 1))
//			Todos_Los_Camiones = false;
		if ((*it)->camiones.size() <= (periodos+1))
			continue;
//		int itcam = (*it)->camiones.size();
//		int itinv = (*it)->inventory_cost;

		auto ic = (*it)->camiones.begin();
		int ncamiones = 0;
		int periodos_item = periodos;
		do
		{
			auto ic_prior = ic;
			if (cont <= (periodos_item))
			{
				if ((ncamiones > periodos_item * 4) && cont>0)
				{
					periodos_item = cont;
					if (cont == 0)
						int kk = 8;
				}
				ic++;
				ncamiones++;
				if (ic == (*it)->camiones.end())
					break;
				if (!((*ic_prior)->mismo_dia((*ic))))
				{
					cont++;

				}
			}
			else
			{
				int kksize = (*it)->camiones.size();
				if (ncamiones >= (periodos_item * 4) )
					int kk = 9;
				//elimino los camiones a partir de periodo
				ic = (*it)->camiones.erase(ic);
				
				Todos_Los_Camiones = false;
			}
//			if (cont==periodos)
//				Todos_Los_Camiones = false;

			
			
		} while (ic != (*it)->camiones.end());
		

	}
	//Una vez que he quitado los camiones de esos items, tengo que hacer lo complementario
	//quitar los items de los camiones
	for (auto ic = lista_camiones.begin(); ic != lista_camiones.end(); ic++)
	{
		(*ic)->items.clear();
	}
	//Añado los items a los camiones que se han quedado en la lista.
	for (auto it = lista_items.begin(); it != lista_items.end(); it++)
	{
		for (auto ic = (*it)->camiones.begin(); ic != (*it)->camiones.end(); ic++)
		{
			(*ic)->items.push_back((*it));
		}

	}
	for (auto ic = lista_camiones.begin(); ic != lista_camiones.end(); ic++)
	{
		set<int> destinos;
		for (auto it = (*ic)->items.begin(); it != (*ic)->items.end(); it++)
		{
			destinos.insert((*ic)->orden_ruta[(*it)->sup_cod_dock]);
		}
		(*ic)->ndestinos = destinos.size();
	}

	coste_inventario = 0;
}
void Subproblema::PonerMasExtras()
{
	for (auto ic = lista_camiones.begin(); ic != lista_camiones.end(); ic++)
	{
		double volumen = 0;
		double peso = 0;

		for (auto it = (*ic)->items.begin(); it != (*ic)->items.end(); it++)
		{
			//Solamente pongo el peso y volumen para los extras de los primeros que ap
			volumen += (*it)->volumen;
			peso += (*it)->weight;
		}
		int cot_1_pes = (int)ceil(peso / ((*ic)->max_loading_weight));
		int cot_1_vol = (int)ceil(volumen / (*ic)->volumen_kp);
		int cot_1 = std::max(cot_1_pes, cot_1_vol);
		(*ic)->cota_extra = (double)(2 * (cot_1)+2);
	}

}
void Subproblema::ReducirExtras()
{
	//	printf("Aqui411 %d\n", Total_Iter_Packing);

	//	return;
	if (Iter_Todos_Camiones>0) return;

	int veces = 3;
	if (Iter_Resuelve_Subproblema == 0)
		veces = 5;
	Best_Sol_Modelo = DBL_MAX;
	//Calculado el numero de extras con el heuristico
	Total_Iter_Packing = veces * Total_Iter_Packing;
	for (auto& itc : lista_camiones)
	{
		
		double volumen = 0;
		double peso = 0;
		itc->dim_minima = itc->length;
		itc->vol_maximo = itc->volumen;
		if (Iter_Resuelve_Subproblema == 0)
		{
			itc->volumen_heuristico = 0;
			itc->peso_heuristico = 0;
		}
		//		(*itc)->peso_heuristico = 0;
		for (auto& it : itc->items_originales)
		{
			//			if (it->camiones.front()->CodIdCC == itc->CodIdCC)
			//			{
						//No tienen ruta ni nada obligado
			itc->items_modelo.push_back(item_modelo(it, it->num_items_original, 0, 0));
			itc->num_items_modelo += it->num_items_original;
			volumen += it->volumen * it->num_items;
			peso += it->weight * it->num_items;
			if (it->forced_orientation_int == 2)
			{
				if (it->width < itc->dim_minima)
					itc->dim_minima = it->width;
			}
			else
			{
				if (it->forced_orientation_int == 1)
				{
					if (it->length < itc->dim_minima)
						itc->dim_minima = it->length;
				}
				else
				{
					if (it->width < itc->dim_minima)
						itc->dim_minima = it->width;
					if (it->length < itc->dim_minima)
						itc->dim_minima = it->length;
				}


			}
			//			}
		}
		//	printf("Aqui40 %.2f %.2f \n", peso, volumen);



	//		printf("Aqui41 %.3f %.3f %d\n", itc->peso_heuristico, itc->volumen_heuristico,Total_Iter_Packing);
			//Por volumen
	/*		if (strcmp(itc->id_truck, "P201654305") == 0)
			{
				int hh = 0;
			}*/
		if (itc->CodId == 1455)
			int kk = 9;
		CargoCamionIni(itc, itc->items_modelo, itc->id_truck, itc->num_items_modelo, 0);
		//		itc->peso_heuristico = itc->max_loading_weight_orig;
		//		itc->volumen_heuristico = itc->volumen_kp;
		//		printf("Aqui42 %.3f %.3f %d\n", itc->peso_heuristico, itc->volumen_heuristico, Total_Iter_Packing);
		//Si es mayor del 75% del volumen
		// No funciona mejor
//		if (4*itc->volumen_heuristico>3*itc->volumen_kp)
//			itc->volumen_heuristico = itc->volumen_heuristico - 2 * (itc->ndestinos);
		itc->items_modelo.clear();
		itc->num_items_modelo = 0;
		if (itc->peso_heuristico > itc->max_loading_weight)
			int kk = 9;
		//		itc->peso_heuristico = itc->max_loading_weight;
		//		itc->volumen_heuristico = itc->volumen_kp;
		if ((itc->peso_heuristico) > (peso - 1))
			itc->pocos_items = true;
		coste_inventario = 0;
	}
	Total_Iter_Packing = Total_Iter_Packing / veces;


	//	printf("Aqui413 \n");
	for (auto& ic : lista_camiones)
	{
		double volumen = 0;
		double peso = 0;
		double volumen_total = 0;
		double peso_total = 0;
		for (auto it : (ic)->items)
		{
			//Solamente pongo el peso y volumen para los extras de los primeros que aparece
			if (it->camiones.front()->CodIdCC == (ic)->CodIdCC)
			{
				volumen += it->volumen * it->num_items;
				peso += it->weight * it->num_items;
				volumen_total += it->volumen * it->num_items;
				peso_total += it->weight * it->num_items;
			}
			else
			{
				volumen_total += it->volumen * it->num_items;
				peso_total += it->weight * it->num_items;
			}


		}
		int cot_2_pes = (int)ceil(peso_total / ((ic)->peso_heuristico));
		int cot_2_vol = (int)ceil(volumen_total / (ic)->volumen_heuristico);
		int cot_1_pes = (int)ceil(peso / ((ic)->peso_heuristico));
		int cot_1_vol = (int)ceil(volumen / (ic)->volumen_heuristico);
		//		int cot_2 = std::max(cot_2_pes, cot_2_vol);
		int cot_1 = std::max(cot_1_pes, cot_1_vol);
		(ic)->cota_extra = (double)(2 * (cot_1 + 1));
		//		if (Pruebas) cout << "Cota extra " << (ic)->cota_extra << " Con ultimos " << cot_1 << " Con total "<< cot_2 << endl;

	}
}
std::string Subproblema::obtenerTextoEntreBarras(const std::string& str) {
	size_t ultimaBarra = str.rfind('/');
	if (ultimaBarra == std::string::npos) {
		return ""; // No se encontró ninguna barra
	}

	size_t penultimaBarra = str.rfind('/', ultimaBarra - 1);
	if (penultimaBarra == std::string::npos) {
		return ""; // Solo hay una barra
	}

	return str.substr(penultimaBarra + 1, ultimaBarra - penultimaBarra - 1);
}
void Subproblema::EscribeCamion(int contador,string id_truck)
{
	//	int idS = CodS;
	if (!escribe_componente) return;
#ifdef __WINDOWS__
#else
	return;
#endif
	string archivo_parametros = param_file;
	string archivo_items = input_items;
	string archivo_trucks = input_trucks;
	string carpeta_ ="./Camiones/"+ obtenerTextoEntreBarras(input_items) + "/";
#ifdef __WINDOWS__
	try {


		if (std::filesystem::create_directory(carpeta_)) {
			std::cout << "Carpeta creada con éxito." << std::endl;
		}
		else {
			std::cout << "La carpeta ya existe o no se pudo crear." << std::endl;
		}
	}
	catch (std::filesystem::filesystem_error& e) {
		std::cerr << e.what() << std::endl;
	}

	carpeta_ = "./Camiones/" + obtenerTextoEntreBarras(input_items) + "/Camiones" + to_string(contador) + "/";
	try {
		if (std::filesystem::create_directory(carpeta_)) {
			std::cout << "Carpeta creada con éxito." << std::endl;
		}
		else {
			std::cout << "La carpeta ya existe o no se pudo crear." << std::endl;
		}
	}
	catch (std::filesystem::filesystem_error& e) {
		std::cerr << e.what() << std::endl;
	}
#endif
	std::ofstream archivoDestinoI(carpeta_+"input_items.csv");
	std::ofstream archivoDestinoT(carpeta_ + "input_trucks.csv");
	std::ofstream archivoDestinoP(carpeta_ + "input_parameters.csv");
	string linea;
	char delimitador = ';';
	ifstream archivop(archivo_parametros);
	if (!archivop)
		cout << "No se encuentra el archivo de parametros" << archivo_parametros << endl;

	getline(archivop, linea);// Leemos la primer línea para descartarla, pues es el encabezado
	archivoDestinoP << linea << endl;
	// Leemos todas las líneas

	while (getline(archivop, linea))
	{
		archivoDestinoP << linea << endl;
	}
	ifstream archivo(archivo_items);
	if (!archivo)
		cout << "No se encuentra el archivo de items" << archivo_items << endl;

	getline(archivo, linea);// Leemos la primer línea para descartarla, pues es el encabezado
	archivoDestinoI << linea << endl;
	// Leemos todas las líneas

	while (getline(archivo, linea))
	{

		stringstream stream(linea); // Convertir la cadena a un stream
		// Extraer todos los valores de esa fila
		string id, sc, sd, pc, pd, prc, pacc, ni, l, w, h, we, ne, stc, fo, eat, lat, ic, ms;

		// Extraer todos los valores de esa fila

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
		getline(stream, ne, delimitador);
		getline(stream, stc, delimitador);
		getline(stream, fo, delimitador);
		getline(stream, eat, delimitador);
		getline(stream, lat, delimitador);
		getline(stream, ic, delimitador);
		getline(stream, ms, delimitador);

		int num = EstaItemCamion(id);
		if (num!=0)
		{
			if (atoi(ni.c_str())!=num)
			ni = to_string(num);
			string nuevaLinea = id + delimitador + sc + delimitador + sd + delimitador + pc + delimitador + pd + delimitador + prc + delimitador + pacc + delimitador + ni + delimitador + l + delimitador + w + delimitador + h + delimitador + we + delimitador + ne + delimitador + stc + delimitador + fo + delimitador + eat + delimitador + lat + delimitador + ic + delimitador + ms;

			archivoDestinoI << nuevaLinea << endl;
		}

	}
	ifstream archivo2(archivo_trucks);
	if (!archivo2)
		cout << "No se encuentra el archivo de trucks" << archivo_items << endl;

	getline(archivo2, linea);// Leemos la primer línea para descartarla, pues es el encabezado
	archivoDestinoT << linea << endl;
	// Leemos todas las líneas

	while (getline(archivo2, linea))
	{

		stringstream stream(linea); // Convertir la cadena a un stream
		// Extraer todos los valores de esa fila

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




		if (strcmp(id_truck.c_str(), id.c_str()) == 0)
		{
			archivoDestinoT << linea << endl;
		}

	}
	archivop.close();
	archivo.close();
	archivo2.close();
	archivoDestinoP.close();
	archivoDestinoT.close();
	archivoDestinoI.close();
}
void Subproblema::EscribeComponente(parameters param)
{
	//	int idS = CodS;
	string archivo_parametros = param.param_file;
	string archivo_items = param.input_items;
	string archivo_trucks = param.input_trucks;
	std::ofstream archivoDestinoI("./KK/input_items.csv");
	std::ofstream archivoDestinoT("./KK/input_trucks.csv");
	std::ofstream archivoDestinoP("./KK/input_parameters.csv");
	string linea;
	char delimitador = ';';
	ifstream archivop(archivo_parametros);
	if (!archivop)
		cout << "No se encuentra el archivo de parametros" << archivo_parametros << endl;

	getline(archivop, linea);// Leemos la primer línea para descartarla, pues es el encabezado
	archivoDestinoP << linea << endl;
	// Leemos todas las líneas

	while (getline(archivop, linea))
	{
		archivoDestinoP << linea << endl;
	}
	ifstream archivo(archivo_items);
	if (!archivo)
		cout << "No se encuentra el archivo de items" << archivo_items << endl;

	getline(archivo, linea);// Leemos la primer línea para descartarla, pues es el encabezado
	archivoDestinoI << linea << endl;
	// Leemos todas las líneas

	while (getline(archivo, linea))
	{

		stringstream stream(linea); // Convertir la cadena a un stream
		// Extraer todos los valores de esa fila
		string id, sc, sd, pc, pd, prc, pacc, ni, l, w, h, we, ne, stc, fo, eat, lat, ic, ms;

		// Extraer todos los valores de esa fila
		getline(stream, id, delimitador);


		if (EstaItem(id))
		{
			archivoDestinoI << linea << endl;
		}

	}
	ifstream archivo2(archivo_trucks);
	if (!archivo2)
		cout << "No se encuentra el archivo de trucks" << archivo_items << endl;

	getline(archivo2, linea);// Leemos la primer línea para descartarla, pues es el encabezado
	archivoDestinoT << linea << endl;
	// Leemos todas las líneas

	while (getline(archivo2, linea))
	{

		stringstream stream(linea); // Convertir la cadena a un stream
		// Extraer todos los valores de esa fila

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




		if (EstaTruck(id))
		{
			archivoDestinoT << linea << endl;
		}

	}
	archivop.close();
	archivo.close();
	archivo2.close();
	archivoDestinoP.close();
	archivoDestinoT.close();
	archivoDestinoI.close();
}
bool Subproblema::EstaTruck(string id_str)
{
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		if (strcmp((*it)->id_truck, id_str.c_str()) == 0)
			return true;
	}
	return false;
}
bool Subproblema::EstaTruck(int id)
{
	for (auto it = lista_camiones.begin(); it != lista_camiones.end(); it++)
	{
		if ((*it)->CodId == id)
			return true;
	}
	return false;
}
bool Subproblema::EstaItem(string id_str)
{
	for (auto it = lista_items.begin(); it != lista_items.end(); it++)
	{
		if (strcmp((*it)->Id, id_str.c_str()) == 0)
			return true;
	}
	return false;
}
bool Subproblema::EstaItem(int id)
{
	for (auto it = lista_items.begin(); it != lista_items.end(); it++)
	{
		if ((*it)->CodId == id)
			return true;
	}
	return false;
}
int Subproblema::EstaItemCamion(string id_str)
{
	for (auto it : vector_items_modelo_escribir)
	{
		if (strcmp(it.id_item->Id, id_str.c_str()) == 0)
			return it.num_items;
	}
	return 0;
}
bool Subproblema::EstaItemCamion(int id)
{
	for (auto it : vector_items_modelo_escribir)
	{
		if (it.id_item->CodIdCC== id)
			return true;
	}
	return false;
}
bool Subproblema::cabe_item_en_espacio(Tripleta<int> esp, item* it, int& orienta, truck* tr)
{
	//Si la orientación lengthwise
	if (it->forced_orientation_int==1)
	{
		if ((it->length <= (tr->length - esp.x))
			&& (it->width <= (esp.y_fin - esp.y_ini))
			&& (it->height <= tr->height))
		{
			orienta = 1;
			return true;
		}
		else return false;
	}
	//Si la orientación widthwise
	if (it->forced_orientation_int==2)
	{
		if ((it->length <= (esp.y_fin - esp.y_ini))
			&& (it->width <= (tr->length - esp.x))
			&& (it->height <= tr->height))
		{
			orienta = 2;
			return true;
		}
		else return false;
	}
	//Si la orientación none,
	if (it->forced_orientation_int == 0)
	{
//		int widthkk = it->width;
//		int lengthkk = it->length;
		//Si cabe a lo largo
		if ((it->length <= (tr->length - esp.x))
			&& (it->width <= (esp.y_fin - esp.y_ini))
			&& (it->height <= tr->height))
		{

			//Forzamos a vertical
			if ((esp.x < (tr->distance_front_midle_axle)) && (tr->Intensidad_Pallets_Medios >= 3 * INTENSITY_MIDDLES))
			{
				if (it->length> it->width)
				orienta = 1;
				else
					orienta = 2;
				return true;
			}

			//si también cabe a lo ancho
			if ((it->length <= (esp.y_fin - esp.y_ini))
				&& (it->width <= (tr->length - esp.x))
				&& (it->height <= tr->height))
			{
				//hago el cociente entre L y W item y el ancho espacio
				double  pel = 0, pew = 0;
//				int kkl = it->length;
//				int kkw = it->width;
				double pdl = modf(((double)(esp.y_fin - esp.y_ini) / (double)it->length), &pel);
				double pdw = modf(((double)(esp.y_fin - esp.y_ini) / (double)it->width), &pew);
//				cout << (esp.y_fin - esp.y_ini) << " " << it->length << " " << pdl << " " << pel << endl;
				//Esto es para cuando estamos al final del camion
			
				if ((tr->length - esp.x) < (esp.y_fin - esp.y_ini))
				{
					pdl = modf(((double)(tr->length - esp.x) / (double)it->width), &pel);
					pdw = modf(((double)(tr->length - esp.x) / (double)it->length), &pew);
				}
				//Si no caben
				if (pel == 0 && pew == 0) return false;
				//si solo cabe a lo ancho
				if (pel == 0 && pew > 0)
				{
					orienta = 1;
					return true;
				}
				else
				{
					//Si solo cabe a lo largo
					if (pew == 0 && pel > 0)
					{
						orienta = 2;
						return true;
					}
					else
					{

						//si la diferencia entre las partes fraccionarias es menor 5%, 
						// el que menor parte entera
						if (pel!=1 && pew!=1 && abs(pdl - pdw) <= ((double)get_random(1,5)/(double)100))
						{
							if (pel < pew)
							{
								//vertical
								orienta = 2;
								return true;
							}
							else
							{
								//horizontal
								orienta = 1;
								return true;
							}
						}
						else // el de menor parte fraccionaria
						{
							if (pdl > pdw)
							{
								//horizontal
								orienta = 1;
								return true;
							}
							else
							{
								orienta = 2;
								return true;
							}
						}
					}
				}
			}
			else //cabe a lo largo pero no alo ancho
			{
				orienta = 1;
				return true;
			}
		}
		else //no cabe a lo largo
		{
			//Probamos si cabe a lo ancho
			if ((it->length <= (esp.y_fin - esp.y_ini))
				&& (it->width <= (tr->length - esp.x))
				&& (it->height <= tr->height))
			{
				orienta = 2;
				return true;
			}
			else return false;
		}
	}
	return false;
}
int Subproblema::Cuantos_apilar(truck* itc, item* it, double alto, int max_apila,  truck_sol& ts, double densi_pila, Tripleta<int> esp, int orienta,double peso_pila,int max_num,double mwia, double volp)
{

	
	//1.- los que me caben por altura
	double aux_h = (alto - it->nesting_height) / (it->height - it->nesting_height);
	if (volp>0.01)
	aux_h = (alto) / (it->height - it->nesting_height);
	int por_altura = (int)floor(aux_h);

	//2.- el máximo que puede tener encima
	int max_stack = max_apila;
	

	//3.- los que puede soportar el item base
//		int pms = mwia;
		int soporta = max((int)floor(mwia / it->weight), 1);
	
	//el mínimo de los tres
	int num_items = min(min(por_altura,max_num), min(max_stack, soporta));
	

	//No puede exceder el peso maximo del item de abajo
	//Si excedo el peso cargado del camión le quito items.
	while ((num_items * it->weight + peso_pila + itc->peso_cargado) > ((double)itc->max_loading_weight-(double)0.001))
	{
		num_items--;
	}
	//Si excedo la densidad de la pila, le quito items.	
	//4.- no superra la densidad máxima
	if (num_items <= 0) return 0;
	double densidad_item = (it->weight) / ((float)it->length/ (float)1000 * (float)it->width/ (float)1000);
	while (((num_items*densidad_item)+densi_pila > itc->max_stack_density) && num_items >=0) 
	{
		num_items--;
	}
	if ((num_items * densidad_item) > itc->max_stack_density)
		int kk = 9;
	//Que no exceda el volumen máximo del camion
/*	while ((double)(num_items * it->volumen + volp + itc->volumen_ocupado) > (double)(itc->vol_maximo))
	{
		num_items--;
	}*/
	//cout << "altura:" << por_altura << " max_pila:" << max_stack << " soporte_abajo:" << soporta << " densidad:" << densidad << " minima:" << num_items << endl;
	if (num_items <= 0) num_items = 0;
	else
	{
		bool fin = false;

		do 
		{
			//Calculo cuantos me caben para que no exceda el peso de los ejes
			

			double pesot = ts.tmt + (num_items * it->weight)+peso_pila;
			int largo = 0;
			if (orienta == 1)largo = (it->length / 2);
			else largo = (it->width) / 2;
			double peso_x_medio = ts.peso_x_medio + (static_cast<double>(esp.x + largo) * ((num_items*it->weight)+peso_pila));
			double eje = peso_x_medio / pesot;
			double ejr = itc->distance_star_trailer_harness + itc->distance_harness_rear_axle - eje;
			double emh = ((pesot * ejr) + (itc->weight_empty_truck * itc->distance_cog_tractor_rear_axle)) / itc->distance_harness_rear_axle;
			double emr = pesot + itc->weight_empty_truck - emh;
			//TO DO en la VA itc->distance_front_midle_axle es 0
			double emm = 0;
			if (itc->distance_front_midle_axle>0)
				emm= ((itc->weight_tractor * itc->distance_front_axle_cog_tractor) + (emh * itc->distnace_front_axle_harness_tractor)) / itc->distance_front_midle_axle;


			if ((2*itc->Intensidad_Pallets_Medios)>itc->Total_Iter_Packing && (emm > itc->max_weight_middle_axle || emr > itc->max_weight_rear_axle))
			{
				//si está en la primera parte continua
				if (3 * esp.x < (itc->length) && num_items==1)
				{
					fin = true;
					continue;
				}
				do 
				{
					
					num_items--;
					pesot = ts.tmt + (num_items * it->weight) + peso_pila;

					peso_x_medio = ts.peso_x_medio + (static_cast<double>(esp.x + largo) * ((num_items * it->weight) + peso_pila));
					eje = peso_x_medio / pesot;
					ejr = itc->distance_star_trailer_harness + itc->distance_harness_rear_axle - eje;
					emh = ((pesot * ejr) + (itc->weight_empty_truck * itc->distance_cog_tractor_rear_axle)) / itc->distance_harness_rear_axle;
					emr = pesot + itc->weight_empty_truck - emh;
					//TO DO en la VA itc->distance_front_midle_axle es 0
					emm = 0;
					if (itc->distance_front_midle_axle > 0)
						emm = ((itc->weight_tractor * itc->distance_front_axle_cog_tractor) + (emh * itc->distnace_front_axle_harness_tractor)) / itc->distance_front_midle_axle;


				} while ((emm > itc->max_weight_middle_axle || emr > itc->max_weight_rear_axle) && num_items > 0);

			}
			else fin=true;
		} while (!fin && num_items >= 0);

	}
	if (num_items > 0) return num_items;
	else return 0;
}
void Subproblema::ComprobarPesosCamion(truck_sol& ts)
{
	for (auto it : lista_camiones)
	{
		//			strcmp(itc->id_truck, "P201654305") == 0)

		if (strcmp(it->id_truck, ts.id_truc.c_str()) == 0)
		{
			
			if (strcmp(ts.id_truc.c_str(), "P222706601") == 0)
				int kk = 9;
			if (ts.emm != it->emm_colocado)
			{
				cout << ts.emm << "Otro" << it->emm_colocado << endl;
//				PintarProblema(55);
			}
			it->ComprobarPesos();
		}
	}
};


void Subproblema::Crear_pila(int xi, int yi, int zi, int Xf, int Yf, int num_items, item_modelo& itm, truck* itc, int pos, int orienta, truck_sol& ts, int cpila, bool actualiza)
{
	//Creo un item para cada uno de los que pueda meter, en la lista de items
	//int xi = esp.x, yi = esp.first, zi = 0;
	list<item_sol> itsol;
	item* it = itm.id_item;
	int Zf = zi;
	int nit = 0;
	int coste_inv_pila = 0;
	double volp = 0;
	int inicial2 = itm.num_items;
	int inicial1 = itm.id_item->num_items;
	if (actualiza)
	{
		nit = (*itc).pilas_solucion.back().id_pila.num_items;
	}
	if (zi==0 && num_items == 1)
	{
		Zf += it->height ;
		item_sol items=item_sol(it,it->CodId,it->Id, ts.id_truc, cpila, to_string((int)sol.listado_items.size() + 1), nit + num_items, xi, yi, zi, Xf, Yf, Zf, it->weight,pos,it->volumen);
		volp += it->volumen;
		//Calculo el coste de inventario
		int difday = Days_0001(it->latest_arrival_tm_ptr.tm_year, it->latest_arrival_tm_ptr.tm_mon + 1, it->latest_arrival_tm_ptr.tm_mday) -
			Days_0001(itc->arrival_time_tm_ptr.tm_year, itc->arrival_time_tm_ptr.tm_mon + 1, itc->arrival_time_tm_ptr.tm_mday);
		itc->coste_inventario_item += it->inventory_cost;
		if (difday > 0)
		{
			itc->coste_invetario_items_por_colocar += it->inventory_cost * difday;
			coste_inventario += it->inventory_cost * difday;
			itc->coste_inventario_camion += it->inventory_cost * difday;
			coste_inv_pila += it->inventory_cost * difday;
			items.inventory_cost = it->inventory_cost * difday;
			
			
		}
		else
		{
			if (difday < 0)
				PintarProblema(2);
		}
		
		

		//sol.listado_items.push_back((*items));
		itsol.push_back(items);
	}
	else
	{
		for (int ni = 1; ni < num_items; ni++)
		{
			//1.- los que me caben por altura
//			double aux_h = (alto - it->nesting_height) / (it->height - it->nesting_height);
//			int por_altura = (int)floor(aux_h);
			//Zf =  (ni * (it->height - it->nesting_height));
			if (zi == 0 && ni==1)Zf += it->height;
			else Zf += (it->height - it->nesting_height);
			item_sol items=item_sol(it,it->CodId, it->Id, ts.id_truc, cpila, to_string((int)sol.listado_items.size() + 1), nit + ni, xi, yi, zi, Xf, Yf, Zf, it->weight,pos,it->volumen);
			volp += it->volumen;
			itc->coste_inventario_item += it->inventory_cost;
			//Calculo el coste de inventario
			int difday = Days_0001(it->latest_arrival_tm_ptr.tm_year, it->latest_arrival_tm_ptr.tm_mon+1, it->latest_arrival_tm_ptr.tm_mday) - 
				Days_0001(itc->arrival_time_tm_ptr.tm_year, itc->arrival_time_tm_ptr.tm_mon + 1, itc->arrival_time_tm_ptr.tm_mday);
			if (difday > 0)
			{
				//cout << it->latest_arrival << " ; " << itc->arrival_time << endl;
				itc->coste_invetario_items_por_colocar += it->inventory_cost * difday;
				coste_inventario += it->inventory_cost * difday;
				itc->coste_inventario_camion += it->inventory_cost * difday;
				coste_inv_pila += it->inventory_cost * difday;
				items.inventory_cost = it->inventory_cost * difday;
				
				
			}
			else
			{
				
				if (difday < 0)
				{
					cout << "X_" << it->CodIdCC << ": " << it->earliest_arrival << " - " << it->latest_arrival << endl;
					cout << "camion:" << itc->arrival_time << endl;
					PintarProblema(2);
				}
			}
			
			//sol.listado_items.push_back((*items));
			itsol.push_back(items);
			zi = Zf;
		}

		Zf += it->height - it->nesting_height;
		item_sol items=item_sol(it,it->CodId, it->Id, ts.id_truc, cpila, to_string((int)sol.listado_items.size() + 1), nit + num_items, xi, yi, zi, Xf, Yf, Zf, it->weight,pos,it->volumen);
		volp += it->volumen;

		//Calculo el coste de inventario
		int difday = Days_0001(it->latest_arrival_tm_ptr.tm_year, it->latest_arrival_tm_ptr.tm_mon + 1, it->latest_arrival_tm_ptr.tm_mday) -
			Days_0001(itc->arrival_time_tm_ptr.tm_year, itc->arrival_time_tm_ptr.tm_mon + 1, itc->arrival_time_tm_ptr.tm_mday);
		itc->coste_inventario_item += it->inventory_cost;
		if (difday > 0)
		{
			itc->coste_invetario_items_por_colocar += it->inventory_cost * difday;
			coste_inventario += it->inventory_cost * difday;
			itc->coste_inventario_camion += it->inventory_cost * difday;
			coste_inv_pila += it->inventory_cost * difday;
			items.inventory_cost = it->inventory_cost * difday;
			
			
		}
		else
		{
			//if(difday < 0)
			//	int hh = 0;
		}
				
		//sol.listado_items.push_back((*items));
		itsol.push_back(items);
	}

	//creo el stack
	if (actualiza)
	{
		/*sol.listado_pilas.back().z_fin=Zf;
		sol.listado_pilas.back().peso += num_items * it->weight;
		sol.listado_pilas.back().densidad = sol.listado_pilas.back().peso / (((Xf - xi)/1000) * ((Yf - yi)/1000));
		sol.listado_pilas.back().num_items += num_items;*/
		(*itc).pilas_solucion.back().id_pila.z_fin = Zf;
		if (Zf > itc->height)
		{
			PintarProblema(321);
		}
		if ((*itc).pilas_solucion.back().id_pila.max_stack_item_abajo > it->max_stackability)
			(*itc).pilas_solucion.back().id_pila.max_stack_item_abajo = it->max_stackability;
		(*itc).pilas_solucion.back().id_pila.peso += num_items * it->weight;
		(*itc).pilas_solucion.back().id_pila.volumen += num_items * it->volumen;
		(*itc).pilas_solucion.back().id_pila.densidad = (*itc).pilas_solucion.back().id_pila.peso / (((float)(Xf - xi) / (float)1000) * ((float)(Yf - yi) / (float)1000));
		if ((*itc).pilas_solucion.back().id_pila.densidad > 10000)
			int kk = 9;
		(*itc).pilas_solucion.back().id_pila.num_items += num_items;
		(*itc).pilas_solucion.back().id_pila.inventory_cost += coste_inv_pila;
		(*itc).pilas_solucion.back().id_pila.peso_encima_item_abajo+= num_items * it->weight;
		for (auto iti = itsol.begin(); iti != itsol.end(); iti++)
		{
			(*itc).pilas_solucion.back().items.push_back((*iti));
		}
		
	}
	else
	{
		stack_sol pila=stack_sol(ts.id_truc, cpila, to_string(cpila), xi, yi, 0, Xf, Yf, Zf, num_items * it->weight, it->max_stackability, orienta,num_items, coste_inv_pila, num_items * it->volumen,(*itc).pesos_bottom_prod[it->product_code]);
		//sol.listado_pilas.push_back((*pila));
		pila.peso_encima_item_abajo = (num_items - 1) * it->weight;
		pilas_cargadas pc(pila,itsol);
		if (pila.densidad > 10000)
			int kk = 9;
		(*itc).pilas_solucion.push_back(pc);
		
	}

	//Actualizo el numero de items que queda
	itm.num_items -= num_items;
	itm.colocado += num_items;
	itm.id_item->num_items -= num_items;

	/*if (strcmp(itm.id_item->Id, "0090016100_27022023003354") == 0)
	{
		cout << "Descuento X_49=" << itm.num_items << " Colocado:" << itm.colocado << endl;
	}
	if (strcmp(itm.id_item->Id, "0090016100_27022023001411") == 0)
	{
		cout << "Descuento X_109=" << itm.num_items << " Colocado:" << itm.colocado << endl;
	}*/
	//int kk2 = itm.id_item->num_items;

	if (itm.id_item->num_items < 0)
	{
		cout << "Inicial2 " << inicial2 << "Inicial1 "<< inicial1 << "he quitado "<< num_items<< " y quedan " << itm.id_item << " NI " << itm.id_item->num_items << endl;
		if (Pruebas) cout <<"Coloca más de los posibles;"<< itm.id_item->Id << ";"<<itm.id_item->CodIdCC<< ";" << itm.id_item->num_items << ";" << itm.num_items << endl;
		PintarProblema(11);
	}
		
}
//Modifico la función si no cabe nada la pongo lo más al centro posible
//
bool Subproblema::apilar_old(vector<item_modelo>& vitm, truck* itc, int orienta, int i, Tripleta<int> esp, truck_sol& ts, int cpila)
{
	bool puede_llevar_mas = false;
	int Xf = 0, Yf = 0, Yi = 0;
	item_modelo& itm = vitm[i];
	item* it = itm.id_item;
	int ancho = it->width;
	int largo = it->length;
	if (orienta == 2)
	{
		ancho = it->length;
		largo = it->width;
	}
	//	float peso = 0;
		//Calcular el tama o de los perfiles

		//colocar en el centro
	int cantidad = 0;
	double vol_queda = (((double)(itc->length - esp.x) / (double)1000) * ((double)(itc->height) / (double)1000) * ((double)(itc->width_kp) / (double)1000));
	//Valor al azar para poner de esta forma u otra
	  // definimos un mínimo y un máximo
//	double num_alea =(double) get_random(1,2*itc->Intensidad_Pallets_Medios)/(double)10;

	//int num_alea2 = 2 * itc->Intensidad_Pallets_Medios - 2*get_random(1, itc->Intensidad_Pallets_Medios);
	double num_alea2 =  (double)get_random(1, itc->Intensidad_Pallets_Medios);
	En_Medio = false;
	int num = (esp.x) / ((itc->distance_front_midle_axle) / 5);
	//	if (itc->Empezando_Camion==true && num_alea*esp.x< itc->length && vol_queda> num_alea2 * ((itc->volumen_a_cargar - ts.volumen_temp_ocupado)))
	if ((num_alea2 > INTENSITY_MIDDLES) && esp.x < (itc->distance_front_midle_axle + ((itc->distance_harness_rear_axle - itc->distance_front_midle_axle) / 2)) && (vol_queda > (num_alea2 + 1) * ((itc->volumen_a_cargar - ts.volumen_temp_ocupado)) || (itc->Intensidad_Pallets_Medios >= (INTENSITY_MIDDLES + num))))
	{
		//queda 
		int sobra = ((esp.y_fin - esp.y_ini) - ancho);
		Xf = esp.x + largo; Yi = esp.y_ini + (sobra / 2); Yf = Yi + ancho;
		if (esp.CantidadTocaPerfil(Yi, Yf) > 0)
			En_Medio = true;



	}
	if (En_Medio == false)
	{

		itc->Empezando_Camion = false;

		if (esp.x == 0)
		{
			cantidad = (esp.y_fin - esp.y_ini);
		}
		else
		{
			for (auto it1 = esp.perfiles.begin(); it1 != esp.perfiles.end(); it1++)
			{
				cantidad += (min((*it1).y_fin, esp.y_fin) - max((*it1).y_ini, esp.y_ini));
			}
		}
		//Si puede ocasionar que un item no toque el anterior
		if (((esp.y_fin - esp.y_ini) - cantidad) > (*itc).dim_minima)
		{
			//La voy a colocar en el lado que menos cantidad toque de segmento
			//La coloca abajo
			Xf = esp.x + largo; Yf = esp.y_ini + ancho; Yi = esp.y_ini;
			int cantidad_abajo = esp.CantidadTocaPerfil(Yi, Yf);
			//Arriba
			Xf = esp.x + largo; Yf = esp.y_fin; Yi = esp.y_fin - ancho;
			int cantidad_arriba = esp.CantidadTocaPerfil(Yi, Yf);;
			//si se cumple esto va abajo y en otro caso va arriba
			if ((cantidad_abajo < cantidad_arriba && cantidad_abajo != 0) || cantidad_arriba == 0)
			{
				//Lo pongo abajo
				Xf = esp.x + largo; Yf = esp.y_ini + ancho; Yi = esp.y_ini;
			}
			//Si no toca con nada poniendolo pegado arriba o abajo
			if (cantidad_abajo == 0 && cantidad_arriba == 0)
			{
				int y_ini = (*esp.perfiles.begin()).y_ini;
				int y_fin = (*esp.perfiles.begin()).y_fin;
				//Vamos a colocar en el primer perfil que encontremos
				for (auto it4 = esp.perfiles.begin(); it4 != esp.perfiles.end(); it4++)
				{
					y_ini = (*it4).y_ini;
					y_fin = (*it4).y_fin;
					break;

				}
				int solape = 0;
				//Al lado más alejado de la esquina
				if (((y_ini - esp.y_ini)) > ((esp.y_fin - y_fin)))
				{
					Xf = esp.x + largo; Yf = y_ini + solape; Yi = y_ini + solape - ancho;
				}
				else
				{
					Xf = esp.x + largo; Yf = y_fin - solape + ancho; Yi = y_fin - solape;
				}

			}
		}
		else
		{
			//dimensiones de la pila seg n la orientaci n del item seleccionado.
			Xf = esp.x + largo; Yf = esp.y_ini + ancho; Yi = esp.y_ini;
			//Voy a ir colocando una vez a cada lado
			if ((esp.y_fin - esp.y_ini) == (itc->width))
			{
				if (itc->Abajo == false)
				{
					Xf = esp.x + largo; Yf = esp.y_fin; Yi = esp.y_fin - ancho;
					itc->Abajo = true;
				}
				else
				{
					itc->Abajo = false;
				}
			}
			//Si lo que queda es nada pongo la pila lo m s pegada al centro
			int dif_esp_pieza = (esp.y_fin - esp.y_ini) - (Yf - Yi);
			if (dif_esp_pieza < (*itc).dim_minima && dif_esp_pieza>0)
			{
				if (((itc->width) - esp.y_fin) > esp.y_ini) //arriba
				{
					Xf = esp.x + largo; Yf = esp.y_fin; Yi = esp.y_fin - ancho;
				}
			}
		}
	}
	//calcualo cuantos cabe en altura
	double altura_max = itc->height;

	//Hay que poner que dependa del valor
	int num_intensity = (esp.x) / ((itc->distance_front_midle_axle) / 5);
	if (itc->Intensidad_Pallets_Medios > ((2) * INTENSITY_MIDDLES + num_intensity) && esp.x < itc->distance_front_midle_axle)
	{
		altura_max = (itc->height) * (itc->volumen_a_cargar) / (itc->volumen);
		//Comprobar altura maxima
		if (altura_max > (itc->height))
			altura_max = (itc->height);
		altura_max = altura_max / (itc->Intensidad_Pallets_Medios / INTENSITY_MIDDLES);
		altura_max = max(altura_max, (double)it->height);

	}

	int num_items = Cuantos_apilar(itc, it, altura_max, it->max_stackability, ts, 0, esp, orienta, 0, itm.num_items, itc->pesos_bottom_prod[it->product_code], 0);
	if (num_items == 0) return false;




	//Creo un item para cada uno de los que pueda meter, en la lista de items
	Crear_pila(esp.x, Yi, 0, Xf, Yf, num_items, itm, itc, i, orienta, ts, cpila, false);

	//Si el item de abajo no puede llevar nada más encima se cierra la pila.
	if (itc->pilas_solucion.back().id_pila.max_stack_item_abajo == itc->pilas_solucion.back().items.size())
	{
		puede_llevar_mas = false;
	}
	else puede_llevar_mas = true;




	if (puede_llevar_mas)
	{
		//busco más items que pueda llevar encima
		for (int j = i; j < vitm.size() && puede_llevar_mas; j++)
		{
			if (vitm[j].num_items == 0) continue;

			if ((vitm[j].id_item->stackability_code_int == it->stackability_code_int)
				&& (vitm[j].orden_ruta == itm.orden_ruta)
				&& (vitm[j].id_item->suplier_code_int == it->suplier_code_int)
				&& (vitm[j].id_item->plant_code_int == it->plant_code_int)
				&& (vitm[j].id_item->sup_cod_dock_int == it->sup_cod_dock_int)
				&& (vitm[j].id_item->max_stackability >= itc->pilas_solucion.back().id_pila.max_stack_item_abajo)
				&& (vitm[j].id_item->forced_orientation_int == it->forced_orientation_int)
				)
			{
				//si el camion solo puede llevar pilas de un plant dock
				if (itc->stack_with_multiple_docks == 0 && vitm[j].id_item->plant_dock_int != it->plant_dock_int)
					continue;
				else
				{
					//si el camion solo puede llevar 1 pila de dos plant docks y ya la he puesto y el item tiene otro plant dock sigo buscando
					if (itc->stack_with_multiple_docks == 1 && itc->pila_multi_dock == true && vitm[j].id_item->plant_dock_int != it->plant_dock_int)
						continue;
					else
					{
						//Coloco más items encima.
						stack_sol pila(itc->pilas_solucion.back().id_pila);
						//si solo hay una pieza en la base y esta puede llevar moenos que la de encima, intento ponerla en orden inverso
						int max_arriba = itc->pesos_bottom_prod[vitm[j].id_item->product_code];
						//Si caben por altura
						int altoo = pila.z_fin + vitm[j].id_item->height;
						if ((itc->pilas_solucion.back().id_pila).max_weight_item_abajo < max_arriba && (itc->pilas_solucion.back().id_pila).num_items == 1 && altoo < itc->height)
						{
							//PintarProblema(678);


							int xj = (itc->pilas_solucion.back().id_pila).x_ini;
							int yj = (itc->pilas_solucion.back().id_pila).y_ini;
							int zj = (itc->pilas_solucion.back().id_pila).z_ini;
							int orientap = (itc->pilas_solucion.back().id_pila).orientacion;
							int nit_ps = (*itc).pilas_solucion.back().items.size();
							(*itc).pilas_solucion.back().items.clear();
							itc->pilas_solucion.pop_back();

							//Cambiar la base, creo una pila nueva.
							if (orientap == 1)
							{

								Crear_pila(xj, yj, zj, xj + vitm[j].id_item->length, yj + vitm[j].id_item->width, 1, vitm[j], itc, j, orienta, ts, cpila, false);

							}
							else
							{

								Crear_pila(xj, yj, zj, xj + vitm[j].id_item->width, yj + vitm[j].id_item->length, 1, vitm[j], itc, j, orienta, ts, cpila, false);

							}
							pila = (itc->pilas_solucion.back().id_pila);
							//poner el otro encima
							vitm[i].id_item->num_items += nit_ps;
							vitm[i].num_items += nit_ps;
							vitm[i].colocado -= nit_ps;


							int coste_inv_antes = itc->coste_invetario_items_por_colocar;


							Crear_pila(pila.x_ini, pila.y_ini, pila.z_fin, Xf, Yf, nit_ps, vitm[i], itc, i, orienta, ts, cpila, true);

							itc->coste_inventario_item -= vitm[i].id_item->inventory_cost;
							int coste_inv_desp = itc->coste_invetario_items_por_colocar;
							int dif_coste = coste_inv_desp - coste_inv_antes;
							itc->coste_invetario_items_por_colocar -= dif_coste;
							coste_inventario -= dif_coste;
							itc->coste_inventario_camion -= dif_coste;
							pila = (itc->pilas_solucion.back().id_pila);
							//ver si se puede apilar más del base encima.
							num_items = Cuantos_apilar(itc, vitm[j].id_item, altura_max - pila.z_fin, pila.max_stack_item_abajo - pila.num_items, ts, pila.densidad, esp, orienta, pila.peso, vitm[j].num_items, pila.max_weight_item_abajo, pila.volumen);
							veces_pila_cambia++;
						}
						else
						{
							num_items = Cuantos_apilar(itc, vitm[j].id_item, altura_max - pila.z_fin, pila.max_stack_item_abajo - pila.num_items, ts, pila.densidad, esp, orienta, pila.peso, vitm[j].num_items, pila.max_weight_item_abajo, pila.volumen);
						}


						if (num_items > 0)
						{
							while (num_items > 0 && (num_items * vitm[j].id_item->weight) + pila.peso_encima_item_abajo > pila.max_weight_item_abajo)
							{
								num_items--;
							}
							
							double densidad_item = (vitm[j].id_item->weight) / ((float)vitm[j].id_item->length / (float)1000 * (float)vitm[j].id_item->width / (float)1000);
							while (((num_items * densidad_item) + pila.densidad > itc->max_stack_density) && num_items > 0)
							{
								num_items--;
							}
							if (num_items > 0)
							{

								puede_llevar_mas = true;


								Crear_pila(pila.x_ini, pila.y_ini, pila.z_fin, Xf, Yf, num_items, vitm[j], itc, j, orienta, ts, cpila, true);
								if (vitm[j].id_item->plant_dock_int == it->plant_dock_int) itc->pila_multi_dock = true;

								//Si el item de abajo no puede llevar nada más encima se cierra la pila.
								if (itc->pilas_solucion.back().id_pila.max_stack_item_abajo == itc->pilas_solucion.back().items.size())
								{
									puede_llevar_mas = false;
								}
								else puede_llevar_mas = true;
							}

						}
						//						else 
						//							return true;
					}
				}

			}
		}
	}
	return true;

}


bool Subproblema::apilar(vector<item_modelo>& vitm, truck* itc, int orienta, int i, Tripleta<int> esp, truck_sol& ts, int cpila)
{
	bool puede_llevar_mas = false;
	int Xf=0, Yf=0, Yi=0;
	item_modelo& itm = vitm[i];
	item* it = itm.id_item;
	int ancho = it->width;
	int largo = it->length;
	if (orienta == 2)
	{
		ancho = it->length;
		largo = it->width;
	}
//	float peso = 0;
	//Calcular el tama o de los perfiles

	//colocar en el centro
	int cantidad = 0;
	double vol_queda = (((double)(itc->length - esp.x) / (double)1000) * ((double)(itc->height) / (double)1000) * ((double)(itc->width_kp) / (double)1000));
	//Valor al azar para poner de esta forma u otra
	  // definimos un mínimo y un máximo
//	double num_alea =(double) get_random(1,2*itc->Intensidad_Pallets_Medios)/(double)10;

	//int num_alea2 = 2 * itc->Intensidad_Pallets_Medios - 2*get_random(1, itc->Intensidad_Pallets_Medios);
	double num_alea2=get_random(1, itc->Intensidad_Pallets_Medios);
	En_Medio = false;
	int num = (esp.x) / ((itc->distance_front_midle_axle) / 5);
//	if (itc->Empezando_Camion==true && num_alea*esp.x< itc->length && vol_queda> num_alea2 * ((itc->volumen_a_cargar - ts.volumen_temp_ocupado)))
	if ( (num_alea2> INTENSITY_MIDDLES) && esp.x<( itc->distance_front_midle_axle+((itc->distance_harness_rear_axle-itc->distance_front_midle_axle)/2)) && (vol_queda> (num_alea2+1) * ((itc->volumen_a_cargar - ts.volumen_temp_ocupado)) || ( itc->Intensidad_Pallets_Medios>=(INTENSITY_MIDDLES+num))))
	{
		//queda 
		int sobra = ((esp.y_fin - esp.y_ini) - ancho);
		Xf = esp.x + largo; Yi = esp.y_ini+ (sobra/2); Yf = Yi + ancho;
		if (esp.CantidadTocaPerfil(Yi, Yf) >0)
			En_Medio = true;



	}
	//Si no lo colocamos en medio
	if (En_Medio==false)
	{

		itc->Empezando_Camion = false;

		if (esp.x == 0)
		{
			cantidad = (esp.y_fin - esp.y_ini);
		}
		else
		{
			for (auto it1 = esp.perfiles.begin(); it1 != esp.perfiles.end(); it1++)
			{
				cantidad += (min((*it1).y_fin, esp.y_fin) - max((*it1).y_ini, esp.y_ini));
			}
		}
		//Si puede ocasionar que un item no toque el anterior
		if (((esp.y_fin - esp.y_ini) - cantidad) > (*itc).dim_minima)
		{
			//La voy a colocar en el lado que menos cantidad toque de segmento
			//La coloca abajo
			Xf = esp.x + largo; Yf = esp.y_ini + ancho; Yi = esp.y_ini;
			int cantidad_abajo = esp.CantidadTocaPerfil(Yi, Yf);
			//Arriba
			Xf = esp.x + largo; Yf = esp.y_fin; Yi = esp.y_fin - ancho;
			int cantidad_arriba = esp.CantidadTocaPerfil(Yi, Yf);;
			//si se cumple esto va abajo y en otro caso va arriba
			if ((cantidad_abajo < cantidad_arriba && cantidad_abajo != 0) || cantidad_arriba == 0)
			{
				//Lo pongo abajo
				Xf = esp.x + largo; Yf = esp.y_ini + ancho; Yi = esp.y_ini;
			}
			//Si no toca con nada poniendolo pegado arriba o abajo
			if (cantidad_abajo == 0 && cantidad_arriba == 0)
			{
				int y_ini = (*esp.perfiles.begin()).y_ini;
				int y_fin = (*esp.perfiles.begin()).y_fin;
				//Vamos a colocar en el primer perfil que encontremos
				for (auto it4 = esp.perfiles.begin(); it4 != esp.perfiles.end(); it4++)
				{
					y_ini = (*it4).y_ini;
					y_fin = (*it4).y_fin;
					break;

				}
				int solape = 0;
				//Al lado más alejado de la esquina
				if (((y_ini - esp.y_ini)) > ((esp.y_fin - y_fin)))
				{
					Xf = esp.x + largo; Yf = y_ini + solape; Yi = y_ini + solape - ancho;
				}
				else
				{
					Xf = esp.x + largo; Yf = y_fin - solape + ancho; Yi = y_fin - solape;
				}

			}
		}
		else
		{
			//dimensiones de la pila seg n la orientaci n del item seleccionado.
			Xf = esp.x + largo; Yf = esp.y_ini + ancho; Yi = esp.y_ini;
			//Voy a ir colocando una vez a cada lado
			if ((esp.y_fin - esp.y_ini) == (itc->width))
			{
				if (itc->Abajo == false)
				{
					Xf = esp.x + largo; Yf = esp.y_fin; Yi = esp.y_fin - ancho;
					itc->Abajo = true;
				}
				else
				{
					itc->Abajo = false;
				}
			}
			//Si lo que queda es nada pongo la pila lo m s pegada a un lado
			int dif_esp_pieza = (esp.y_fin - esp.y_ini) - (Yf - Yi);
			if (dif_esp_pieza < (*itc).dim_minima && dif_esp_pieza>0)
			{
				if (((itc->width) - esp.y_fin) > esp.y_ini) //arriba
				{
					Xf = esp.x + largo; Yf = esp.y_fin; Yi = esp.y_fin - ancho;
				}
				if (esp.y_fin == (itc->width))
				{
					Xf = esp.x + largo; Yf = esp.y_fin; Yi = esp.y_fin - ancho;
				}
				if (esp.y_ini == 0)
				{
					Xf = esp.x + largo; Yf = esp.y_ini+ancho; Yi = esp.y_ini ;
				}
				

			}
		}
	}
	//calcualo cuantos cabe en altura
	double altura_max = itc->height;

	//Hay que poner que dependa del valor
	//Este trozo es para que no ponga todos en altura
	int num_intensity = (esp.x)/( (itc->distance_front_midle_axle)/5);
	if (itc->Intensidad_Pallets_Medios >((2)*INTENSITY_MIDDLES+ num_intensity) && esp.x < itc->distance_front_midle_axle)
	{
		altura_max = (itc->height) * (itc->volumen_a_cargar) / (itc->volumen);
		//Comprobar altura maxima
		if (altura_max > (itc->height))
			altura_max = (itc->height);
		altura_max = altura_max / (itc->Intensidad_Pallets_Medios / INTENSITY_MIDDLES);
		altura_max = max(altura_max, (double) it->height);

	}
	
	int num_items = Cuantos_apilar(itc, it, altura_max, it->max_stackability, ts, 0, esp, orienta,0, itm.num_items, itc->pesos_bottom_prod[it->product_code],0);
	//Si no puede poner ninguno
	
	if (num_items == 0) 
		return false;
	
	
	

	//Creo un item para cada uno de los que pueda meter, en la lista de items
	Crear_pila(esp.x, Yi, 0, Xf, Yf, num_items, itm, itc, i, orienta, ts, cpila, false);
	if ((itc->pilas_solucion.back().id_pila.max_stack_item_abajo) < (itc->pilas_solucion.back().id_pila.num_items))
	{
		PintarProblema(444);
	}
	//Si el item de abajo no puede llevar nada más encima se cierra la pila.
	if (itc->pilas_solucion.back().id_pila.max_stack_item_abajo == itc->pilas_solucion.back().id_pila.num_items)
	{

		return true;
	}
	int puestas = 0;
	//busco más items que pueda llevar encima
	int elegido = -1;
	int num_items_elegido = 0;
	int altura_max_temp = 0;
	bool elegido_obligado = false;
	bool fin = false;
	if ((itc->peso_cargado+(itc->pilas_solucion.back().id_pila.peso)) > itc->max_loading_weight)
	{
		int kk = 0;
	}
	bool original = true;
	for (int j = i+1; j < vitm.size() && fin!=true && (altura_max - itc->pilas_solucion.back().id_pila.z_fin)>itc->minima_altura; j++)
	{

		//Si no quedan items
		if (vitm[j].num_items == 0) continue;
		//Si cumple condiciones de apilamiento
		if ((vitm[j].id_item->stackability_code_int == it->stackability_code_int)
			&& (vitm[j].orden_ruta == itm.orden_ruta)
			&& (vitm[j].id_item->suplier_code_int == it->suplier_code_int)
			&& (vitm[j].id_item->plant_code_int == it->plant_code_int)
			&& (vitm[j].id_item->sup_cod_dock_int == it->sup_cod_dock_int)
			&& (vitm[j].id_item->max_stackability >= itc->pilas_solucion.back().id_pila.max_stack_item_abajo)
			&& (vitm[j].id_item->forced_orientation_int == it->forced_orientation_int)
			)
		{
			//si el camion solo puede llevar pilas de un plant dock
			if (itc->stack_with_multiple_docks == 0 && vitm[j].id_item->plant_dock_int != it->plant_dock_int)
				continue;
			//si el camion solo puede llevar 1 pila de dos plant docks y ya la he puesto y el item tiene otro plant dock sigo buscando
			if (itc->stack_with_multiple_docks == 1 && itc->pila_multi_dock == true && vitm[j].id_item->plant_dock_int != it->plant_dock_int)
				continue;

			//Coloco más items encima.
			stack_sol pila(itc->pilas_solucion.back().id_pila);
			//si solo hay una pieza en la base y esta puede llevar moenos que la de encima, intento ponerla en orden inverso
			int max_peso_soporta_esta_arriba = itc->pesos_bottom_prod[vitm[j].id_item->product_code];
			//Si caben por altura
			int altura = pila.z_fin + (vitm[j].id_item->height- vitm[j].id_item->nesting_height);
			if (altura > itc->height)
				continue;
			if (pila.densidad > 100000)
				int kk = 9;
			//Añdido el  <= 26/12/2023
			double densidad_item = (vitm[j].id_item->weight) / ((float)vitm[j].id_item->length / (float)1000 * (float)vitm[j].id_item->width / (float)1000);
			if  ((1 * densidad_item) + (itc->pilas_solucion.back().id_pila).densidad > itc->max_stack_density)
				continue;
			if (((1 * vitm[j].id_item->weight) + pila.peso + itc->peso_cargado) >= itc->max_loading_weight)
			{
				continue;
			}

			//Cambiado < por <= 21/12/2023
			
			if (elegido<0 && (itc->pilas_solucion.back().id_pila).max_weight_item_abajo <= max_peso_soporta_esta_arriba && (itc->pilas_solucion.back().id_pila).num_items == 1 
				&& max_peso_soporta_esta_arriba>(itc->pilas_solucion.back().id_pila).peso)
			{
				//PintarProblema(678);

				original = false;
				int xj = (itc->pilas_solucion.back().id_pila).x_ini;
				int yj = (itc->pilas_solucion.back().id_pila).y_ini;
				int zj = (itc->pilas_solucion.back().id_pila).z_ini;
				int orientap = (itc->pilas_solucion.back().id_pila).orientacion;
				int nit_ps = (*itc).pilas_solucion.back().items.size();
				(*itc).pilas_solucion.back().items.clear();
				itc->pilas_solucion.pop_back();

				//Cambiar la base, creo una pila nueva.
				if (orientap == 1)
				{

					Crear_pila(xj, yj, zj, xj + vitm[j].id_item->length, yj + vitm[j].id_item->width, 1, vitm[j], itc, j, orienta, ts, cpila, false);

				}
				else
				{

					Crear_pila(xj, yj, zj, xj + vitm[j].id_item->width, yj + vitm[j].id_item->length, 1, vitm[j], itc, j, orienta, ts, cpila, false);

				}
				pila = (itc->pilas_solucion.back().id_pila);
				//poner el otro encima
				vitm[i].id_item->num_items += nit_ps;
				vitm[i].num_items += nit_ps;
				vitm[i].colocado -= nit_ps;


				int coste_inv_antes = itc->coste_invetario_items_por_colocar;


				Crear_pila(pila.x_ini, pila.y_ini, pila.z_fin, Xf, Yf, nit_ps, vitm[i], itc, i, orienta, ts, cpila, true);

				itc->coste_inventario_item -= vitm[i].id_item->inventory_cost;
				int coste_inv_desp = itc->coste_invetario_items_por_colocar;
				int dif_coste = coste_inv_desp - coste_inv_antes;
				itc->coste_invetario_items_por_colocar -= dif_coste;
				coste_inventario -= dif_coste;
				itc->coste_inventario_camion -= dif_coste;
				pila = (itc->pilas_solucion.back().id_pila);
				//ver si se puede apilar más del base encima.
				num_items = Cuantos_apilar(itc, vitm[j].id_item, altura_max - pila.z_fin, pila.max_stack_item_abajo - pila.num_items, ts, pila.densidad, esp, orienta, pila.peso, vitm[j].num_items, pila.max_weight_item_abajo, pila.volumen);
				veces_pila_cambia++;
//				elegido=

			}
			else
			{
				num_items = Cuantos_apilar(itc, vitm[j].id_item, altura_max - pila.z_fin, pila.max_stack_item_abajo - pila.num_items, ts, pila.densidad, esp, orienta, pila.peso, vitm[j].num_items, pila.max_weight_item_abajo, pila.volumen);
			}
			if ((itc->peso_cargado + (itc->pilas_solucion.back().id_pila.peso)) > itc->max_loading_weight)
			{
				int kk = 0;
			}

			if (num_items > 0)
			{
				while (num_items > 0 && (num_items * vitm[j].id_item->weight) + pila.peso_encima_item_abajo > pila.max_weight_item_abajo)
				{
					num_items--;
				}
				while (num_items > 0 && (((num_items * vitm[j].id_item->weight) + pila.peso + itc->peso_cargado) >= itc->max_loading_weight))
				{
					num_items--;
				}

				if (num_items > 0)
				{
					double densidad_item = (vitm[j].id_item->weight) / ((float)vitm[j].id_item->length / (float)1000 * (float)vitm[j].id_item->width / (float)1000);
					while (((num_items * densidad_item) + pila.densidad > itc->max_stack_density) && num_items > 0)
					{
						num_items--;
					}
				}
				if (num_items > 0)
				{
					if (puestas == 1)
					{
						int kk = 0;
					}
					puede_llevar_mas = true;
					puestas++;
					//Si ya es la última que voy a poner
					if (altura > (itc->height - (itc->minima_altura)))
					{
						//Cambiado
					if (altura > altura_max_temp && elegido_obligado== vitm[j].obligado)
//							if (altura > altura_max_temp )
							{
							//Si es en la primera parte el primero que entra
//							if (2 * (itc->pilas_solucion.back().id_pila.x_ini) < itc->length)
// 								fin = true;
							if (altura_max_temp > 0)
								int kk = 0;

							altura_max_temp = altura;
							elegido = j;
							num_items_elegido = num_items;
							elegido_obligado = vitm[j].obligado;
							if (elegido == 4 && itc->CodId == 4976)
								int kk = 9;
							
						}

					}
					else
					{
						if (elegido < 0)
						{
							Crear_pila(pila.x_ini, pila.y_ini, pila.z_fin, Xf, Yf, num_items, vitm[j], itc, j, orienta, ts, cpila, true);
							elegido = -1;
							num_items_elegido = 0;
							altura_max_temp = 0;
							if (vitm[j].id_item->plant_dock_int == it->plant_dock_int) itc->pila_multi_dock = true;

							//Si el item de abajo no puede llevar nada más encima se cierra la pila.
							if (itc->pilas_solucion.back().id_pila.max_stack_item_abajo == itc->pilas_solucion.back().items.size())
							{
								if ((itc->peso_cargado + (itc->pilas_solucion.back().id_pila.peso)) > itc->max_loading_weight)
								{
									int kk = 0;
								}
								if ((itc->pilas_solucion.back().id_pila.max_stack_item_abajo) < (itc->pilas_solucion.back().id_pila.num_items))
								{
									PintarProblema(444);
								}
								return true;
							}
						}
					}

				}

			}
			//						else 
			//							return true;



		}
	}
	if (elegido == 4 && itc->CodId == 4976)
		int kk = 9;
	//coloca el item más grande de los restantes
	if (elegido >= 0)
	{
		stack_sol pila(itc->pilas_solucion.back().id_pila);

		Crear_pila(pila.x_ini, pila.y_ini, pila.z_fin, Xf, Yf, num_items_elegido, vitm[elegido], itc, elegido, orienta, ts, cpila, true);
		if (vitm[elegido].id_item->plant_dock_int == it->plant_dock_int) 
			itc->pila_multi_dock = true;
		if ((itc->peso_cargado + (itc->pilas_solucion.back().id_pila.peso)) > itc->max_loading_weight)
		{
			int kk = 0;
		}

	}
	if ((itc->peso_cargado + (itc->pilas_solucion.back().id_pila.peso)) > itc->max_loading_weight)
	{
		int kk = 0;
	}
	if ((itc->pilas_solucion.back().id_pila.max_stack_item_abajo) < (itc->pilas_solucion.back().id_pila.num_items))
	{
		PintarProblema(444);
	}
//	if (strcmp(itc->pilas_solucion.back().id_pila.id_stack.c_str(), "P073350001_3") == 0 && itc->pilas_solucion.back().id_pila.peso>850 
//		&& itc->pilas_solucion.back().id_pila.max_weight_item_abajo <= 750)
//		int kk = 0;
	return true;

}
//Devuelve 0 si no ha podido colocar por espacio
//Devuelve 1 si ha podido colocar
//Devuelve 2 si no ha podido colocar por orden
//Devuelve 3 si no ha podido colocar por peso
int Subproblema::Busco_pila(Tripleta<int> its, truck* itc, truck_sol& ts, vector<item_modelo> &vec_item, int cpila,string idcam)
{
	if (!(!(its.perfiles.size() == 0 && its.x != 0)))
		return 0;
//	int orienta = 0;
	bool ha_apilado = false;

	if (((its.y_fin - its.y_ini) < (*itc).dim_minima) || (((*itc).length - its.x) < (*itc).dim_minima))
	{
		return 0;
	}

	int orden = 10000;
	int primero = -1;
	//para todos los items del modelo, mientras haya espacios donde colocar


	for (int i = 0; i < vec_item.size(); i++)
	{
		
		if (NoMeteObligados && i < PrimeroNoMeteObligados)
			continue;
		//Busco el primero que tenga items por meter
		if (vec_item[i].num_items == 0) continue;
		else
		{
			//Cambiado
//			if (NoMeteObligados && (4*i)<3*vec_item.size())
			if (NoMeteObligados )
			{
				
				if (vec_item[i].obligado != 1 && get_random(0, 10) >= (10 - Prob_Saltar_No_Obligado))
				{
					//Solamente salta si no es el último
					if (vec_item[i].orden_ruta!=vec_item.back().orden_ruta)
					continue;
				}
				
			}
			PrimeroNoMeteObligados = i;
			orden = vec_item[i].orden_ruta;
			primero = i;
			break;
		}
	}
	int ultimo = -2;
	for (int i = 0; i < vec_item.size(); i++)
	{
		if (vec_item[i].colocado > 0)
			ultimo = i;
	}
	if (primero > -1 && primero < ultimo && NoMeteObligados)
		int kk = 9;
	//Si no ha encontrado
	if(primero>-1)
	{
		for (int i = primero; i < vec_item.size(); i++)
		{
			if (vec_item[i].num_items == 0) continue;
			//si el item cabe en el espacio, en cualquiera de la orientaciones permitidas
			int orientacion = 0;
			if (cabe_item_en_espacio(its, vec_item[i].id_item, orientacion, itc))
			{
				
				//si el que cabe tiene un orden superior al primero de la lista que tiene items por poner,no se puede colocar.
				if (orden < vec_item[i].orden_ruta)
				{
					return 2;
				}
				/*if (ts.orden_por_el_que_voy <= vec_item[i].orden_ruta)
				{*/
				
				
				
				if (primero == 15 && veces_pila == 8 && i == 18)
					int kk = 0;
					ha_apilado = apilar(vec_item, itc, orientacion, i, its, ts, cpila);
					if (ha_apilado)
					{

						//Reviso el peso en el eje trasero
	
						double peso_cam = ts.tmt;
						peso_cam += itc->pilas_solucion.back().id_pila.peso;
						double  peso_x_medio = ts.peso_x_medio;
						peso_x_medio+= (static_cast<double>(itc->pilas_solucion.back().id_pila.x_ini) + (static_cast<double>(itc->pilas_solucion.back().id_pila.x_fin - itc->pilas_solucion.back().id_pila.x_ini) / 2)) * itc->pilas_solucion.back().id_pila.peso;
						double eje = peso_x_medio /peso_cam;
						double ejr = itc->distance_star_trailer_harness + itc->distance_harness_rear_axle - eje;
						double emh= ((peso_cam * ejr) + (itc->weight_empty_truck * itc->distance_cog_tractor_rear_axle)) / itc->distance_harness_rear_axle;
						double emr = peso_cam + itc->weight_empty_truck - emh;
						if (emr > itc->max_weight_rear_axle)
						{
							for (auto itemit : itc->pilas_solucion.back().items)
							{
								for (int i = 0; i < vec_item.size(); i++)
								{
									if (vec_item[i].id_item->CodId == itemit.id_CodId)
									{
										vec_item[i].colocado--;
										vec_item[i].num_items++;
										vec_item[i].id_item->num_items++;
										break;
									}
								}

							}
							itc->pilas_solucion.pop_back();
							return 3;
						}
							int kk = 9;
						
//						ts.emh = ((ts.tmt * ts.ejr) + (itc->weight_empty_truck * itc->distance_cog_tractor_rear_axle)) / itc->distance_harness_rear_axle;
//						ts.emm_anterior = ts.emm;
//						ts.emr_anterior = ts.emr;

						
					
						veces_pila++;
						//Actualizo el camion
						(*itc).orden_por_el_que_voy = vec_item[i].orden_ruta;
						ts.orden_por_el_que_voy= vec_item[i].orden_ruta;
						ts.weight_loaded += itc->pilas_solucion.back().id_pila.peso;
						itc->emm_colocado_anterior = itc->emm_colocado;
						itc->emr_colocado_anterior = itc->emr_colocado;
						itc->peso_cargado += itc->pilas_solucion.back().id_pila.peso;
						if (itc->peso_cargado > itc->max_loading_weight)
						{
							int kk = 0;
						}
						float v1 = ((float)(itc->pilas_solucion.back().id_pila.x_fin - itc->pilas_solucion.back().id_pila.x_ini)) / 1000;
						float v2 = ((float)(itc->pilas_solucion.back().id_pila.y_fin - itc->pilas_solucion.back().id_pila.y_ini)) / 1000;
						float v3 = ((float)(itc->pilas_solucion.back().id_pila.z_fin - itc->pilas_solucion.back().id_pila.z_ini)) / 1000;
						ts.vol_loaded += v1 * v2 * v3;
						itc->volumen_ocupado += v1 * v2 * v3;
						
						if (ts.load_length < itc->pilas_solucion.back().id_pila.x_fin)
						{
							itc->load_length = itc->pilas_solucion.back().id_pila.x_fin;
							ts.load_length = itc->pilas_solucion.back().id_pila.x_fin;
						}
						ts.tmt += itc->pilas_solucion.back().id_pila.peso;
						ts.volumen_temp_ocupado += itc->pilas_solucion.back().id_pila.volumen;
						ts.peso_x_medio += (static_cast<double>(itc->pilas_solucion.back().id_pila.x_ini) + (static_cast<double>(itc->pilas_solucion.back().id_pila.x_fin - itc->pilas_solucion.back().id_pila.x_ini) / 2)) * itc->pilas_solucion.back().id_pila.peso;
						ts.eje = ts.peso_x_medio / ts.tmt;
						ts.ejr = itc->distance_star_trailer_harness + itc->distance_harness_rear_axle - ts.eje;
						ts.emh = ((ts.tmt * ts.ejr) + (itc->weight_empty_truck * itc->distance_cog_tractor_rear_axle)) / itc->distance_harness_rear_axle;
						ts.emm_anterior = ts.emm;
						ts.emr_anterior = ts.emr;

						ts.emr = ts.tmt + itc->weight_empty_truck - ts.emh;
						if (emr != ts.emr)
						{
							int kk = 9;
						}
						if (ts.emr > itc->max_weight_rear_axle)
							int kk = 9;
						ts.emm = ((itc->weight_tractor * itc->distance_front_axle_cog_tractor) + (ts.emh * itc->distnace_front_axle_harness_tractor)) / itc->distance_front_midle_axle;
						itc->emm_colocado = ts.emm;
						itc->emr_colocado = ts.emr;
						if (itc->pilas_solucion.back().id_pila.densidad > itc->max_stack_density)
							int kk = 9;
						return 1;
		
					}

					
				//}
				////else return 2;

			}
			//Si no cabe pero es el anterior
			else
			{
//				if (i == PrimeroNoMeteObligados)
//					PrimeroNoMeteObligados--;
			}
		}
		//si finalizo y no he encontrado ninguno
		return 2;
	}
	return 0;
}
void Subproblema::MejorarNumExtras()
{
	return ;
	Total_Iter_Packing = 5 * Total_Iter_Packing;
	for (auto itc = lista_camiones.begin(); itc != lista_camiones.end(); itc++)
	{
		/*
		(*itc)->dim_minima = (*itc)->length;
		(*itc)->vol_maximo = (*itc)->volumen;
		(*itc)->volumen_heuristico = 0;
		(*itc)->peso_heuristico = 0;
		//		(*itc)->peso_heuristico = 0;
		for (auto it = (*itc)->items_originales.begin(); it != (*itc)->items_originales.end(); it++)
		{
			(*itc)->items_modelo.push_back(item_modelo(*it, (*it)->num_items_original, 0, 0));
			(*itc)->num_items_modelo += (*it)->num_items_original;

			if ((*it)->forced_orientation_int == 2)
			{
				if ((*it)->width < (*itc)->dim_minima)
					(*itc)->dim_minima = (*it)->width;
			}
			else
			{
				if ((*it)->forced_orientation_int == 1)
				{
					if ((*it)->length < (*itc)->dim_minima)
						(*itc)->dim_minima = (*it)->length;
				}
				else
				{
					if ((*it)->width < (*itc)->dim_minima)
						(*itc)->dim_minima = (*it)->width;
					if ((*it)->length < (*itc)->dim_minima)
						(*itc)->dim_minima = (*it)->length;
				}


			}



		}

		//Por volumen
		
		CargoCamionIni((*itc), (*itc)->items_modelo, (*itc)->id_truck, (*itc)->num_items_modelo, 0, 1);

		(*itc)->items_modelo.clear();
		(*itc)->num_items_modelo = 0;
		int cot_1_pes = (int)ceil(((*itc)->peso_total_items) / ((*itc)->peso_heuristico));
		int cot_1_vol = (int)ceil(((*itc)->volumen_total_items) / ((*itc)->volumen_heuristico));
		int cot_1 = std::max(cot_1_pes, cot_1_vol);
		int new_cota_extra = (double)(1.2 * (cot_1 + 1));
		if (new_cota_extra > (*itc)->cota_extra)
			int kk = new_cota_extra - (*itc)->cota_extra;
		(*itc)->cota_extra = new_cota_extra;
		if (((*itc)->volumen_heuristico) < 1)
			int kk = 9;
	*/
		
		(*itc)->volumen_heuristico = (*itc)->volumen_total_items;
		(*itc)->peso_heuristico = (*itc)->max_loading_weight;
	}
	Total_Iter_Packing =  Total_Iter_Packing/5;
}

bool Subproblema::ComprobarCostes()
{
	//VAmos a comprobar que el coste de transporte es el que tengo
	//int coste = 0;
	for (auto it = sol.listado_camiones.begin(); it != sol.listado_camiones.end(); it++)
	{
//		coste += it->coste;
	}
}
int Subproblema::BuscarIDCC(int a)
{
	for (auto it : lista_items)
	{
		if (it->CodId==a)
			return it->CodIdCC;
	}
	PintarProblema(66);
	return 0;
}
void Subproblema::ComprobarItemsColocados()
{
	if (Pruebas) cout << "Comprueba " << endl;
	if (!Pruebas) return;
	vector<int> vector_items(lista_items.size());
	for (auto it : sol.listado_items)
	{
		int CodIdCC = BuscarIDCC(it.id_CodId);
		vector_items[CodIdCC]++;
	}
	for (auto it2 : lista_items)
	{
		bool problema_num = false;
		if (vector_items[it2->CodIdCC] != it2->num_items_original)
		{
/*			for (auto it : sol.listado_items)
			{
				if (it2->CodIdCC==it.id_CodId)
					if (Pruebas) cout << "camión"
			}*/
			problema_num = true;
			if (Pruebas) cout << "Error en el numero de items colocados , en item " << it2->CodIdCC << " pone " << vector_items[it2->CodIdCC] << " debería poner " << it2->num_items_original << endl;
			

		}
		if (problema_num) PintarProblema(14);
	}
//	if (Pruebas) cout << "Comprueba2 " << endl;
}
bool Subproblema::PackingModelo(int opcion, string dir, parameters param)
{
	if (opcion == 1)
	{
		/*colocar cada uno de estos items en camiones asignados, diferentes del inicialmente asignado,
		por si queda en alguno algun espacio libre que se pueda utilizar.
		Con los items que queden sin cargar, hemos de resolver un problema residual con esos items
		y el resto de camiones no utilizados. Se puede volver a resolver el modelo o intentar resolver
		heuristicamente, si quedan pocos items
	   */
		//ordenar la lista de items de cada camión por 
		//proveedor, dock, tamaño de la lista de camiones en las que puede ir (1, y luego el resto)
		//coste de inventario
		auto itc = lista_camiones.begin();
		bool alguno_queda = false;
		int cuantos_quedaban = 0;
		int iter = 0;
		do
		{
			alguno_queda = false;
			int cuantos_quedan = 0;
			int num_camiones_a_cargar = (int)ceil(param.porcentaje* num_camiones_modelo);
			//&& num_camiones_a_cargar >0
			for (itc = lista_camiones.begin(); itc != lista_camiones.end() ; itc++)
			{
				
	
				(*itc)->Total_Iter_Packing = Total_Iter_Packing;
				
				if ((*itc)->items_modelo.size() != 0)
				{

					//Creo el camion y lo añado a la solución
					truck_sol camion((*itc)->id_truck);
					camion.peso_max = (*itc)->max_loading_weight;
					camion.volumen_max = (*itc)->volumen;
					sol.listado_camiones.push_back(camion);

					coste_transporte += (*itc)->cost;
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
					(*itc)->peso_a_cargar= (*itc)->peso_total_items_modelo;
					(*itc)->volumen_a_cargar= (*itc)->volumen_total_items_modelo;
					int antes = (*itc)->total_items_modelos;

					CargoCamion((*itc), (*itc)->items_modelo, (*itc)->id_truck, (*itc)->num_items_modelo, 0);
					num_camiones_a_cargar--;

					if (Pruebas)
					{
						if ((*itc)->peso_cargado > (*itc)->max_loading_weight)
						{
							cout << "El camion:" << (*itc)->id_truck << " tiene cargado " << (*itc)->peso_cargado << " y el máximo es " << (*itc)->max_loading_weight << " la diferencia: " << (*itc)->peso_cargado - (*itc)->max_loading_weight << endl;
							cout << "El camion:" << (*itc)->id_truck << " tiene vol cargado " << (*itc)->volumen_ocupado << " y el vol máximo es " << (*itc)->volumen_kp << "la diferencia: " << (*itc)->volumen_ocupado - (*itc)->volumen_kp << endl;
							
							(*itc)->max_loading_weight = (*itc)->peso_cargado;
							(*itc)->volumen_kp = (*itc)->volumen_ocupado;

							
							
							//PintarProblema(211);
						}
						if((*itc)->vol_antes > (*itc)->volumen_ocupado)
							cout << "Vol:"<<(*itc)->id_truck << ":" << (*itc)->vol_antes << "-" << (*itc)->volumen_ocupado << endl;
						if ((*itc)->peso_antes > (*itc)->peso_cargado)
							cout <<"Peso:"<< (*itc)->id_truck << ":" << (*itc)->peso_antes << "-" << (*itc)->peso_cargado << endl;
					}
					if ((*itc)->total_items_modelos < 0)
						PintarProblema(111);
					if (antes == (*itc)->total_items_modelos)
						PintarProblema(6);
					
				}
		
				auto itp = (*itc)->peso_total_items_modelo_extra.begin();
				auto itv = (*itc)->volumen_total_items_modelo_items_extra.begin();
				auto itc_extra = (*itc)->pcamiones_extra.begin();
				int cont_cam = 0;
				for (auto itl = (*itc)->items_extras_modelo.begin(); itl != (*itc)->items_extras_modelo.end(); itl++, itp++, itv++,itc_extra++)
				{
					cont_cam++;
					if ((*itl).size() == 0)
					{
						continue;
					}
					(*itc)->Modelo_pesado = false;
					
					string idt ((*itc_extra)->id_truck);
					size_t tok = idt.find("_");
					string dos = idt.substr(tok+1);


							
					truck_sol camion((*itc_extra)->id_truck);
					camion.volumen_max = (*itc_extra)->volumen;
					camion.peso_max = (*itc_extra)->max_loading_weight;
					sol.listado_camiones.push_back(camion);
					coste_transporte += (*itc)->coste_extra;
					
					/*if (strcmp((*itc_extra)->id_truck, "Q206417103_1") == 0 || strcmp((*itc_extra)->id_truck, "Q252417103_1") == 0)
					{
						int hh = 0;
					}*/
					double peso = ((*itc_extra)->max_loading_weight / (*itp));
					double vol = (*itc_extra)->volumen / (*itv);
					if (peso < vol && peso < 2)
					{
						(*itc_extra)->Modelo_pesado = true;
						(*itc_extra)->peso_total_items_modelo_asignados = (*itp);
						(*itc_extra)->volumen_total_items_modelo_asignados = (*itv);
					}
					(*itc_extra)->peso_a_cargar = (*itp);
					(*itc_extra)->volumen_a_cargar = (*itv);
					int antes = (*itc)->num_items_extra.at(cont_cam - 1);
					CargoCamion((*itc_extra), (*itl), (*itc_extra)->id_truck, (*itc)->num_items_extra.at(cont_cam - 1), (*itc)->num_extras_creados);
					num_camiones_a_cargar--;
					if (Pruebas)
					{
						if ((*itc_extra)->vol_antes > (*itc_extra)->volumen_ocupado)
							cout << "Vol:" << (*itc_extra)->id_truck << ":" << (*itc_extra)->vol_antes << "-" << (*itc_extra)->volumen_ocupado << endl;
						if ((*itc_extra)->peso_antes > (*itc_extra)->peso_cargado)
							cout << "Peso:" << (*itc_extra)->id_truck << ":" << (*itc_extra)->peso_antes << "-" << (*itc_extra)->peso_cargado << endl;
						if ((*itc)->pilas_solucion.size() <= 0 && (*itc)->pcamiones_extra.size() > 0)
						{
							cout << "El camion " << (*itc)->id_truck << "%s no tiene solución y su extra" << (*itc_extra)->id_truck << " %s si-" << endl;
							//int kk = 9;
							PintarProblema(110);
						}
					}

					(*itc)->total_items_modelos -= (antes - (*itc)->num_items_extra.at(cont_cam - 1));
					(*itc_extra)->num_items_modelo = (*itc)->num_items_extra.at(cont_cam - 1);

					if ((*itc)->total_items_modelos < 0)
						PintarProblema(111);
					if (antes == (*itc)->num_items_extra.at(cont_cam - 1))
						PintarProblema(6);
				}

				if ((*itc)->total_items_modelos > 0)
				{
					alguno_queda = true;
					cuantos_quedan += (*itc)->total_items_modelos;
				}
				
			}

			/*for (auto it1 = lista_camiones.begin(); it1 != lista_camiones.end(); it1++)
			{
				if ((*it1)->lleno) continue;
				int total = 0;
				for (auto it2 = (*it1)->pcamiones_extra.begin(); it2 != (*it1)->pcamiones_extra.end(); it2++)
				{
					int cont = 0;
					for (auto it3 = (*it2)->items_modelo.begin(); it3 != (*it2)->items_modelo.end(); it3++)
					{
						cont += (*it3).num_items;
					}
					if (cont != (*it2)->num_items_modelo)
					{
						cout << (*it2)->id_truck << ": " << cont << " - " << (*it2)->num_items_modelo << endl;
					}
					total += cont;
				}
				int cont = 0;
				for (auto it3 = (*it1)->items_modelo.begin(); it3 != (*it1)->items_modelo.end(); it3++)
				{
					cont += (*it3).num_items;
				}
				total += cont;
				if (cont != (*it1)->num_items_modelo)
				{
					cout << (*it1)->id_truck << ": " << cont << " - " << (*it1)->num_items_modelo << endl;
				}
				if (total != (*it1)->total_items_modelos)
				{
					cout << (*it1)->id_truck << ": " << total << " - " << (*it1)->total_items_modelos << endl;
					(*it1)->total_items_modelos = total;
				}
			}*/
//			if (cuantos_quedan != 0 && cuantos_quedan == cuantos_quedaban)
//			{
//				int kk = 0;
////				PintarProblema(111);
//			}
				
			cuantos_quedaban = cuantos_quedan;
			cplex_trozo = true;
			if (alguno_queda)
			{
				cout << endl;
				for (auto it_i = lista_items.begin(); it_i != lista_items.end(); it_i++) //Recorrer la lista de tipo items
				{

					if ((*it_i)->num_items <= 0)
						continue;
					/*if (Pruebas) cout <<  (*it_i)->CodIdCC << "=" << (*it_i)->num_items << " cams:";
					for (auto cam = (*it_i)->camiones.begin(); cam != (*it_i)->camiones.end(); cam++)
					{
						if (Pruebas) cout << (*cam)->CodIdCC << " - ";
					}
					if (Pruebas) cout << endl;*/
				}
				/*for (auto itcc=lista_camiones.begin(); itcc!= lista_camiones.end();itcc++)
				{
					if ((*itcc)->CodIdCC == 6 || (*itcc)->CodIdCC == 2)
					{
						for (auto itemi = (*itcc)->items.begin(); itemi != (*itcc)->items.end(); itemi++)
						{
							cout << "cam:" << (*itcc)->CodIdCC << " - " <<(*itemi)->CodIdCC<< " - " << (*itemi)->num_items << endl;
						}
					}
				}*/
				/*if (iter == 0)
				{*/
					for (auto itcc = lista_camiones.begin(); itcc != lista_camiones.end(); itcc++)
					{
						if ((*itcc)->pilas_solucion.empty() && (*itcc)->peso_cargado > 0)
						{
							//int kk = 9;
							PintarProblema(210);
						}
						if (((*itcc)->peso_cargado > (*itcc)->max_loading_weight) || ((*itcc)->volumen_ocupado > (*itcc)->volumen_kp))
						{

							cout << "El camion:" << (*itcc)->id_truck << " tiene cargado " << (*itcc)->peso_cargado << " y el máximo es " << (*itcc)->max_loading_weight << " la diferencia: "<< (*itcc)->peso_cargado -(*itcc)->max_loading_weight << endl;
							cout << "El camion:" << (*itcc)->id_truck << " tiene vol cargado " << (*itcc)->volumen_ocupado << " y el vol máximo es " << (*itcc)->volumen_kp << "la diferencia: "<< (*itcc)->volumen_ocupado- (*itcc)->volumen_kp << endl;
							PintarProblema(211);
						}
					}
				//}*/
				if (Pruebas) cout << endl<< "Quedan:"<< cuantos_quedan << " " << cam_llenos << " " << sol.listado_camiones.size() << " Iter Modelo "<< Iteracion_Modelo << endl;

				Roadef2022ExtraCplex_V2(dir, param, this);
				//Otro modelo más
				Iteracion_Modelo++;
				if (TieneSol == false)
				{
					TiempoExtra += 5;
					return false;
				}
				iter++;
				//Roadef2022ExtraCplex(dir, param, this);
			}
				
		} while (alguno_queda);
//		ComprobarCostes();
		//cout << "quedan por colocar: " << (*itc)->total_items_modelos << endl;
//		if ((*itc)->total_items_modelos == 0)
//			continue;

		//Aquí tendría que poner de nuevo el modelo con lo restante
		// TODO Quitar
		//SI hay items en los camiones extra los pongo todos en items_modelo
		//Los ordeno y los meto en camiones extra de ese tipo
		bool facil = false;
		int cont_cam = 0;
		if (facil)
		{
			if ((*itc)->items_extras_modelo.size() > 0)
			{
				for (auto itl = (*itc)->items_extras_modelo.begin(); itl != (*itc)->items_extras_modelo.end(); itl++)
				{
					for (auto itv = (*itl).begin(); itv != (*itl).end(); itv++)
					{
						if ((*itv).num_items == 0) continue;
						(*itc)->items_modelo.push_back((*itv));
						(*itc)->num_items_modelo += (*itv).num_items;
					}
				}
			}
//			sort((*itc)->items_modelo.begin(), (*itc)->items_modelo.end());
			//Tengo todos los que quedan en una lista
			while ((*itc)->total_items_modelos > 0)
			{
				cont_cam++;
				string idcamion = (*itc)->id_truck;
				idcamion += "_" + to_string(cont_cam);
				idcamion[0] = 'Q';
				truck_sol camion(idcamion);
				camion.peso_max = (*itc)->max_loading_weight;
				camion.volumen_max = (*itc)->volumen;
					sol.listado_camiones.push_back(camion);


				CargoCamion((*itc), (*itc)->items_modelo, idcamion, (*itc)->num_items_modelo,cont_cam);


			}
		}

		//cout << "Ahora quedan por colocar: " << (*itc)->total_items_modelos << endl;			

	}

	else
	{
		if (opcion == 2)
		{
//			int extra = 0;
			/*ordenar los camiones por fecha
			* empaquetar los items en los acamiones asignados
			* si se quedan items por colocar ponerlos en el siguiente camion, por fecha
			* si no se consigue empaquetar todo. Lanzar modelo sin los items asignados y camiones llenos.
			*/
			
			list<truck*>::iterator itcsig;
			for (auto itc = lista_camiones.begin(); itc != lista_camiones.end(); itc++)
			{
				//Ordenamos los items de cada camión.	
				//sort((*itc)->items_modelo.begin(), (*itc)->items_modelo.end());
				//Creo el camion y lo añado a la solución
				truck_sol camion((*itc)->id_truck);
				camion.peso_max = (*itc)->max_loading_weight;
				camion.volumen_max = (*itc)->volumen;
				sol.listado_camiones.push_back(camion);
				CargoCamion((*itc), (*itc)->items_modelo, (*itc)->id_truck, (*itc)->num_items_modelo,0);
				//si han quedado por colocar
				itcsig = itc;
				itcsig++;
				while((*itc)->total_items_modelos > 0 && (*itc)->mismo_dia((*itcsig)) && (*itcsig)!=lista_camiones.back())
				{					
					//Añado los items que no se han empaquetado al siguiente camión
					//
					for (auto it = (*itc)->items_modelo.begin(); it != (*itc)->items_modelo.end(); it++)
					{
						if (it->num_items>0 )
						{
							(*itcsig)->items_modelo.push_back(*it);
							(*itcsig)->total_items_modelos+=it->num_items;
							(*itcsig)->num_items_modelo+=it->num_items;
						}
					}
					//intentar colocarlos en el siguiente camión
					//sort((*itcsig)->items_modelo.begin(), (*itcsig)->items_modelo.end());
					//Creo el camion y lo añado a la solución
					truck_sol camion1((*itcsig)->id_truck);
					camion1.peso_max = (*itcsig)->max_loading_weight;
					camion1.volumen_max = (*itcsig)->volumen;
					sol.listado_camiones.push_back(camion1);
					CargoCamion((*itcsig), (*itcsig)->items_modelo, (*itcsig)->id_truck, (*itcsig)->num_items_modelo,0);
					itc = itcsig;
					itcsig++;
				}
				if ((*itcsig)->total_items_modelos > 0)
				{
					
					//Lanzo el modelo con los items y camiones restantes.
					Roadef2022ExtraCplex(dir, param,this);
					return true;
				}
			}
			return true;
		}
		else
		{
			if (opcion == 3)
			{
				/*ordenar los camiones por fecha
				* Se empaqueta camiones
				* si se quedan items por colocar ponerlos en el siguiente camion, por fecha
				* Cuando termine el dia si han quedado items por colocar:
				* a ) Si quedan camiones planificados sin usar ese dia y el coste total de inventario de esos items
				* es mayor que el coste del camion, activar uno de estos camiones 
				* y cargar en ellos los items pendientes.
				* b ) Si no quedan camiones planificados, pero el coste total de inventario de esos items es mayor
				* que el coste de un camion extra, se crea el camion extra y se intenta cargar con esos items.
				*/
				list<truck*>::iterator itcsig;
				for (auto itc = lista_camiones.begin(); itc != lista_camiones.end(); itc++)
				{
					//Ordenamos los items de cada camión.	
//					sort((*itc)->items_modelo.begin(), (*itc)->items_modelo.end());
					//Creo el camion y lo añado a la solución
					truck_sol camion((*itc)->id_truck);
					camion.peso_max = (*itc)->max_loading_weight;
					camion.volumen_max = (*itc)->volumen;
					sol.listado_camiones.push_back(camion);
					CargoCamion((*itc), (*itc)->items_modelo, (*itc)->id_truck, (*itc)->num_items_modelo,0);
					//si han quedado por colocar
					itcsig = itc;
					itcsig++;
					while ((*itc)->total_items_modelos > 0 && (*itc)->mismo_dia((*itcsig)) && (*itcsig) != lista_camiones.back())
					{					
						//Añado los items que no se han empaquetado al siguiente camión
						for (auto it = (*itc)->items_modelo.begin(); it != (*itc)->items_modelo.end(); it++)
						{
							if (it->num_items > 0
								&& it->id_item->earliest_arrival <= (*itcsig)->arrival_time
								&& it->id_item->latest_arrival >= (*itcsig)->arrival_time)
							{
								(*itcsig)->items_modelo.push_back(*it);
								(*itcsig)->total_items_modelos += it->num_items;
								(*itcsig)->num_items_modelo += it->num_items;
							}
						}
						//intentar colocarlos en el siguiente camión
//						sort((*itcsig)->items_modelo.begin(), (*itcsig)->items_modelo.end());
						//Creo el camion y lo añado a la solución
						truck_sol camion1((*itcsig)->id_truck);
						camion1.peso_max = (*itcsig)->max_loading_weight;
						camion1.volumen_max = (*itcsig)->volumen;
						sol.listado_camiones.push_back(camion1);
						CargoCamion((*itcsig), (*itcsig)->items_modelo, (*itcsig)->id_truck, (*itcsig)->num_items_modelo,0);
						itc = itcsig;
						itcsig++;
					}
					//si no son del mismo dia
					if ((*itc)->total_items_modelos > 0 && (*itc)->mismo_dia((*itcsig)) == false)
					{
						//calculo el coste de inventario
						if ((*itc)->coste_invetario_items_por_colocar > (*itc)->coste_extra)
						{
							//creo uno extra
							string idcamion = (*itc)->id_truck;
							idcamion += "_" + to_string(1);
							idcamion[0] = 'Q';
							truck_sol camion1(idcamion);
							camion1.peso_max = (*itc)->max_loading_weight;
							camion1.volumen_max = (*itc)->volumen;
							sol.listado_camiones.push_back(camion1);
							CargoCamion((*itc), (*itc)->items_modelo, idcamion, (*itc)->num_items_modelo,1);
						}
						else
						{
							//miro si puedo ponerlo en el siguinte 
						}
					}
				}
			}
			
		}
	}
	

	
	return true;
}
//TODO modificar la estructura de datos para que sea mas eficiente
//Los items deben estar asociados a las pilas, no independientes, de igual modo las pilas	
//deben estar asociadas a los camiones
void Subproblema::ModificarYStackItems(stack_sol &stack )
{
	for (auto it_i = sol.listado_items.rbegin(); it_i != sol.listado_items.rend(); it_i++)
	{
		if (strcmp(stack.id_stack.c_str(), (*it_i).id_stack.c_str()) == 0)
		{
			(*it_i).y_ini = stack.y_ini;
			(*it_i).y_fin = stack.y_fin;
		}

	}
}
void Subproblema::CalcularMinimo(truck* camion, vector<item_modelo>& vec_items)
{
	int cont = 0;
	for (auto it = vec_items.begin(); it != vec_items.end(); it++,cont++)
	{
		if ((*it).num_items == 0) continue;
//		int kk1 = (*it).id_item->height;
//		int kk2 = (*it).id_item->nesting_height;
		if (((*it).id_item->height - (*it).id_item->nesting_height )<= camion->minima_altura)
		{
			camion->minima_altura = (*it).id_item->height-(*it).id_item->nesting_height;
			
		}
		if ((*it).id_item->length <= camion->dim_minima)
		{
			camion->dim_minima = (*it).id_item->length;
			camion->index_minimo = cont;
		}
		if ((*it).id_item->width <= camion->dim_minima)
		{
			camion->dim_minima = (*it).id_item->width;
			camion->index_minimo = cont;
		}
	}
}
void Subproblema::QuitarPerfiles(list<Tripleta<int>>& lista, stack_sol& ultima, Tripleta<int> &triplet)
{
	for (auto itl = lista.begin(); itl != lista.end(); itl++)
	{
		for (auto itl2 = (*itl).perfiles.begin(); itl2 != (*itl).perfiles.end();)
		{
			if (((*itl2).y_ini == ultima.y_ini)
				&& ((*itl2).y_fin == ultima.y_fin)
				&& ((*itl2).x == ultima.x_fin))
			{
				itl2 = (*itl).perfiles.erase(itl2);
				break;
			}
			else
				itl2++;
		}
	}
	int anchopila = ultima.y_fin -ultima.y_ini;
	int anchoespacio = ultima.y_fin - triplet.y_ini;
	ultima.y_fin -= (anchoespacio - anchopila) / 2;
	ultima.y_ini = ultima.y_fin - anchopila;
	triplet.x = ultima.x_fin;
	triplet.perfiles.clear();
	triplet.perfiles.push_back(Tripleta<int>(ultima.y_ini, ultima.y_fin, ultima.x_fin));

}
void Subproblema::LimpiarPerfiles(list<Tripleta<int>>& lista)
{
	for (auto itl = lista.begin(); itl != lista.end(); itl++)
	{
		//Limpiar los que sean de antes
		auto itl2 = (*itl).perfiles.begin();
		while (itl2 != (*itl).perfiles.end())
		{
			//Si es viejo o se sale del espacio
			if (((*itl2).x < (*itl).x) || ((*itl2).y_ini > (*itl).y_fin) || ((*itl2).y_fin < (*itl).y_ini))
			{
				itl2 = (*itl).perfiles.erase(itl2);
			}
			else
			{
				if ((*itl2).y_fin > (*itl).y_fin) 
					(*itl2).y_fin = (*itl).y_fin;
				if ((*itl2).y_ini < (*itl).y_ini) 
					(*itl2).y_ini = (*itl).y_ini;
				itl2++;
			}
		}


	}
}
void Subproblema::PintarVector(vector<item_modelo>& vec_items,int kiter)
{
	for (auto it = vec_items.begin(); it != vec_items.end(); it++)
	{
		cout <<kiter<< (*it).id_item->CodIdCC << " " << (*it).id_item->densidad << " " << (*it).id_item->inventory_cost<<" ";
		cout << (*it).id_item->weight << " " << (*it).id_item->volumen << endl;
	}
}
//esta función revisa si hay varias rutas y si hay obligados
void Subproblema::ComprobarObligadosyClientes(vector<item_modelo>& vec_items)
{
	bool obligados = false;
	bool varias_rutas = false;
	int anterior_orden_ruta = vec_items.begin()->orden_ruta;
	for (auto& it : vec_items)
	{
			
		if (it.obligado) obligados=true;
		if (it.orden_ruta!=anterior_orden_ruta) varias_rutas = true;
//		int anterio_orden_ruta = it.orden_ruta;
	}
//	if (obligados && varias_rutas)
//		int kk = 9;

}

//esta función revisa si en la solución hay obligados que no están colocado
bool Subproblema::ObligadosSinColocar(vector<item_modelo>& vec_items)
{
	bool obligados_sin_colocar = false;
	bool varias_rutas = false;
	int anterior_orden_ruta = vec_items.begin()->orden_ruta;
	for (auto& it : vec_items)
	{

		if (it.obligado && it.num_items > 0)
		{
			return true;
		}
//		if (it.orden_ruta != anterior_orden_ruta) 
//				varias_rutas = true;
//		if (obligados_sin_colocar && varias_rutas)
//			return true;
//		int anterio_orden_ruta = it.orden_ruta;
	}
	return false;
}
void Subproblema::AleatorizarVector(vector<item_modelo>& vec_items)
{
	if (get_random(0, 1) == 0)
	{
		for (auto &it :vec_items)
		{
			//		cout << (*it).id_item->CodIdCC << " " << (*it).id_item->densidad << " " << (*it).id_item->inventory_cost<<" ";
			//		cout << (*it).id_item->weight << " " << (*it).id_item->volumen << endl;
			it.id_item->Aleatorizado_densidad = 4 * it.id_item->densidad + it.id_item->densidad * get_random(-2, 2);
			it.id_item->Aleatorizado_inventory_cost = 4 * it.id_item->inventory_cost + it.id_item->inventory_cost * get_random(-2, 2);
			it.id_item->Aleatorizado_weight = 4 * it.id_item->weight + it.id_item->weight * get_random(-1, 1);
			it.id_item->Aleatorizado_volumen = 10 * it.id_item->volumen + it.id_item->volumen * get_random(-3, 3);
			//		cout << (*it).id_item->CodIdCC << " " << (*it).id_item->Aleatorizado_densidad << " " << (*it).id_item->Aleatorizado_inventory_cost << " ";
			//		cout << (*it).id_item->Aleatorizado_weight << " " << (*it).id_item->Aleatorizado_volumen << endl;
		}
	}
	else
	{
		for (auto& it : vec_items)
		{
			it.id_item->Aleatorizado_densidad = get_random(1, it.id_item->densidad + 1);
			it.id_item->Aleatorizado_inventory_cost = get_random(1, it.id_item->inventory_cost + 1);
			it.id_item->Aleatorizado_weight = get_random(1, it.id_item->weight + 1);
			it.id_item->Aleatorizado_volumen = get_random(1, it.id_item->volumen + 1);

		}
	}

}
bool Subproblema::orden_items(const item_modelo& a, const item_modelo& b)
{

	if (a.orden_ruta < b.orden_ruta) return true;
	else
	{
		if (a.orden_ruta == b.orden_ruta)
		{
			if (a.id_item->lista_camiones.size() == 1 && b.id_item->lista_camiones.size() == 1)
			{
				if (a.id_item->inventory_cost > b.id_item->inventory_cost) return true;
				else
				{
					if (a.id_item->densidad < b.id_item->densidad)
					{
						return true;

					}
					else
					{
						if (max(a.id_item->width, a.id_item->length) > max(b.id_item->width, b.id_item->length))
							return true;
						else return false;
					}
				}

			}
			else
			{
				if (a.id_item->lista_camiones.size() == 1) return true;
				else
				{
					if (b.id_item->lista_camiones.size() == 1) return false;
					else
					{
						if (a.id_item->lista_camiones.size() < b.id_item->lista_camiones.size())
						{
							return true;
						}
						if (a.id_item->inventory_cost > b.id_item->inventory_cost) return true;
						else
						{
							if (a.id_item->densidad < b.id_item->densidad)
							{
								return true;

							}
							else
							{
								if (max(a.id_item->width, a.id_item->length) > max(b.id_item->width, b.id_item->length))
									return true;
								else return false;
							}
						}
					}
				}
			}
		}
		else return false;
	}
};

void Subproblema::CargoCamion(truck* camion, vector<item_modelo>& vec_items, string idcamion, int& num_items_camion, int numcamion)
{
	int tipo_carga = 0;
	if (tranportation_cost > inventory_cost) tipo_carga = 2;
	else
	{
		if (tranportation_cost < inventory_cost) tipo_carga = 1;
		else
		{
			if (tranportation_cost == inventory_cost) tipo_carga = 0;
		}
	}
	switch(tipo_carga)
	{
	case 0:
		{
			int alea = get_random(0, 5);
			if (alea == 0) tipo_carga = 0;
			else tipo_carga = 1;
		}
	case 1:
		{
			int alea = get_random(0, inventory_cost);
			if (alea > 0) tipo_carga = 0;
			else tipo_carga = 1;
		}
	case 2:
		{
			int alea = get_random(0, tranportation_cost);
			if (alea == 0) tipo_carga = 0;
			else tipo_carga = 1;
		}
	}
	tipo_carga = 1;
	CargoCamion(camion, vec_items, idcamion, num_items_camion, numcamion, tipo_carga);
};
void Subproblema::PonerCantidadesOriginal(vector < item_modelo> &vect)
{
	for (auto it = vect.begin(); it != vect.end(); it++)
	{

		it->num_items = it->id_item->num_items;

	}
}

double Subproblema::Volumen(vector < item_modelo>& vect)
{
	double vol = 0;
	for (auto it : vect)
	{
		vol += it.colocado * it.id_item->vol_con_nest;
	}
	return vol;
}
double Subproblema::VolumenP(vector < item_modelo>& vect)
{
	double vol = 0;
	for (auto it : vect)
	{
		vol += it.num_items * it.id_item->vol_con_nest;
	}
	return vol;
}
double Subproblema::VolumenPO(vector < item_modelo>& vect)
{
	double vol = 0;
	for (auto it : vect)
	{
		vol += it.colocado * it.id_item->vol_con_nest;
	}
	return vol;
}
void Subproblema::CambiarCantidadesOriginal(vector < item_modelo>& vect)
{
	for (auto it : vect)
	{
		it.id_item->num_items = it.num_items;

	}
}
/*
void Subproblema::CargoCamion(truck* camion, vector<item_modelo>& vec_items, string idcamion, int& num_items_camion, int numcamion, int tipo_carga)
{
	//int tipo_carga = get_random(0, 1);
	//0 es carga por inventario
	// 1 es carga por volumen
	// 2 por peso
	//if (Pruebas) cout << "Inicio Cargo" << endl;
	//Inicializo pesos ejes;
	camion->InicializarPesoEjes(Total_Iter_Packing);
	
	//Guardamos el camión actual y las 
	truck* best_camion = new truck(*camion);
	truck* camion_orig = new truck(*best_camion);

	camion->Intensidad_Pallets_Medios = INTENSITY_MIDDLES;
	//guardo la lista de items 
	int antiguo_coste_inventario = coste_inventario;
	int num_items_original = num_items_camion;
	int best_num_items = num_items_camion;
	int best_coste_inventario = coste_inventario;
	int best_coste_inventario_item = 0;
	int coste_original = coste_inventario;
	truck_sol best_ts = sol.listado_camiones.back();
	truck_sol orig_ts = sol.listado_camiones.back();

	vector < item_modelo> vec_items_original = vec_items;
	AleatorizarVector(vec_items);
	sort(vec_items.begin(), vec_items.end(), item_modelo::camiones());

	list<pilas_cargadas> best_pilas;
	list<item_sol> best_items;4
	//Aquí guardamos los números que tienen esos items ahora mismo
	vector < item_modelo> cantidad_items_inicio = vec_items;
	//if (Pruebas) cout << "Inicio Cargo 2" << endl;
	PonerCantidadesOriginal(cantidad_items_inicio);

	vector < item_modelo> best_cantidad_items = cantidad_items_inicio;
	//ordenamos el vector de items

	camion->SetMultipleCliente(vec_items);


	//Aqui tengo los items que había en los items al entrar
	int minimo = camion->dim_minima;
	int cont_pilas = 0;


	//mientras haya espaios donde colocar y haya items del camión.
	int iter = 0;
	double best_volumen = 0;
	double best_peso = 0;
	bool sol_mejorada = false;
	bool he_pasado = false;
	//if (Pruebas) cout << "Inicio Cargo 4" << endl;
	do
	{
		//if (Pruebas) cout << "Inicio Cargo 4.5:" << iter << endl;

		minimo = camion->dim_minima;
		CalcularMinimo(camion, vec_items);
		cont_pilas = 0;

		camion->crea_espacios(numcamion);
		int orden_anterior = vec_items.front().orden_ruta;
		bool cumple_peso_ejes = false;
		//if (Pruebas) cout << "Inicio Cargo 5:" << iter << endl;
		while (!camion->lesp.at(numcamion).lespacios.empty() && num_items_camion > 0)
		{
			//elegir el espacio
			Tripleta<int>* its = camion->lesp.at(numcamion).Busca_sig_esp();
//			
			int coloco = 0;

							//Buscar pila para segmento, siempre coloca abajo
			coloco = Busco_pila(*its, camion, sol.listado_camiones.back(), vec_items, cont_pilas, idcamion);

			if (coloco == 1)
			{
				//Comprobar peso ejes pero tengo que comprobar con el anterior
				if (camion->orden_por_el_que_voy != orden_anterior)
				{
					orden_anterior = camion->orden_por_el_que_voy;
					if (comprobarPesoEjesActualizarIntensidad(camion, sol, true))
					{
						goto actualizar;
					}

				}
				cont_pilas++;
				num_items_camion -= camion->pilas_solucion.back().id_pila.num_items;
				camion->total_items_modelos -= camion->pilas_solucion.back().id_pila.num_items;
				//Actualizo el mínimo
				if (vec_items[camion->index_minimo].num_items == 0)
				{
					camion->index_minimo = -1;
					camion->dim_minima = camion->length;
					CalcularMinimo(camion, vec_items);
				}
				//Genero los nuevos espacios y actualizar
				camion->lesp.at(numcamion).Generaryactualizar_espacios((*camion->pilas_solucion.back().id_pila), *its, camion->dim_minima,En_Medio);
				//lesp.Generaryactualizar_espacios(sol.listado_pilas.back(), *its, camion->dim_minima);
				//
				//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
				ArreglarEspacio(camion,numcamion);

			}
			else
			{
			
				//Comprobar peso ejes
				// ya que voy a cambiar de supplier
				if (coloco == 2)
				{
					if (comprobarPesoEjesActualizarIntensidad(camion, sol, false))
					{
						goto actualizar;
					}

				}
				//actualiza espacio
				if (coloco == 3)
				{
					camion->lesp.at(numcamion).lespacios.clear();
					continue;
				}
				QuitarRestos(camion, numcamion, its);
			}
		}
		//if (Pruebas) cout << "Fin Cargo 1:" << iter << endl;
		if (num_items_original == num_items_camion)
			PintarProblema(8);


		if (camion->emm_colocado > camion->max_weight_middle_axle || camion->emr_colocado > camion->max_weight_rear_axle)
		{
			camion->Intensidad_Pallets_Medios++;
			num_items_camion = num_items_original;
			cumple_peso_ejes = false;
		}
		else
		{
			cumple_peso_ejes = true;
			if (camion->Intensidad_Pallets_Medios > 1)
				camion->Intensidad_Pallets_Medios--;
			he_pasado = true;
		}

		//if (Pruebas) cout << "Fin Cargo 2:" << iter << endl;
		if (camion->Total_Iter_Packing > 1 && num_items_camion != 0)
		{
			////Parte reactiva
			//if (Pruebas) cout << "Fin Cargo 2.1:" << iter << endl;
			if (cumple_peso_ejes && (((tipo_carga==0) &&
				(((camion->coste_inventario_item  == best_coste_inventario_item) && sol.listado_camiones.back().volumen_temp_ocupado > best_volumen) ||
					(camion->coste_inventario_item > best_coste_inventario_item))) || 
				((tipo_carga == 1) &&
					(sol.listado_camiones.back().volumen_temp_ocupado > best_volumen) )||
				((tipo_carga==2)&&(sol.listado_camiones.back().weight_loaded > best_peso))
				))
			{
				sol_mejorada = true;
				best_camion->actualiza(camion);
				if (tipo_carga==1)
					best_volumen = sol.listado_camiones.back().volumen_temp_ocupado;
				else
				{
					if (tipo_carga == 2)
					{
						if(sol.listado_camiones.back().weight_loaded > best_peso)
							best_peso = sol.listado_camiones.back().weight_loaded;
					}
					else
					{
						if (sol.listado_camiones.back().volumen_temp_ocupado > best_volumen)
							best_volumen = sol.listado_camiones.back().volumen_temp_ocupado;
					}
				}
					
				best_ts = sol.listado_camiones.back();
				best_pilas.clear();
				best_items.clear();
				best_pilas.insert(best_pilas.begin(), camion->pilas_solucion.begin(), camion->pilas_solucion.end());
				best_coste_inventario_item = camion->coste_inventario_item;
				best_coste_inventario = coste_inventario;
				best_num_items = num_items_camion;
				//Guardo los numeros originales que quedaba
				PonerCantidadesOriginal(best_cantidad_items);

			}


actualizar:
			camion->pilas_solucion.clear();
			sol.listado_camiones.back().reset();
			coste_inventario= antiguo_coste_inventario;
			camion->actualiza(camion_orig);
			vec_items = vec_items_original;
			if (iter==5)
			AleatorizarVector(vec_items);
			num_items_camion = num_items_original;
			//Restablecer a números originales
			PonerCantidadesOriginal(cantidad_items_inicio);

			if (he_pasado)
			{
				if (iter % 4 == 0)
					sort(vec_items.begin(), vec_items.end(), item_modelo::densidad());
				if (iter % 4 == 1)
					sort(vec_items.begin(), vec_items.end(), item_modelo::weight());
				if (iter % 4 == 2)
					sort(vec_items.begin(), vec_items.end(), item_modelo::inventario());
				if (iter % 4 == 3)
					sort(vec_items.begin(), vec_items.end(), item_modelo::camiones());
			}
			else
			{
				if (iter % 2 == 0)
					sort(vec_items.begin(), vec_items.end(), item_modelo::weight());
				else
					sort(vec_items.begin(), vec_items.end(), item_modelo::densidad());
					

			}
				

			

		}
		iter++;
		//if (Pruebas) cout << "Fin Cargo 3:" << iter << endl;
		
	}while(num_items_camion > 0 && iter < camion->Total_Iter_Packing);
	//if (Pruebas) cout << "Fin Cargo 1" << endl;
	if (he_pasado == false)
	{
		PintarProblema(100);

	}

	if (camion->Total_Iter_Packing > 1 && sol_mejorada && num_items_camion!=0)
	{
		int kkx = best_pilas.back().id_pila.x_fin;
		camion->actualiza(best_camion);
//		camion->total_items_modelos = best_items_modelo_original;
		num_items_camion = best_num_items;
		if (Iteracion_Modelo == 0 && best_volumen> camion->volumen_heuristico)
		camion->volumen_heuristico = best_volumen;
		coste_inventario = best_coste_inventario;
		sol.listado_camiones.back() = best_ts;
		//pasar la best_pila a la solcucion
		camion->pilas_solucion = best_pilas; //poner best_pilas en la pilas_solucion del camion

		for (auto it = best_cantidad_items.begin(); it != best_cantidad_items.end(); it++)
		{
			it->id_item->num_items = it->num_items;
		}
	}

	delete (best_camion);
	delete (camion_orig);
	if (iter == 10 && sol_mejorada == false)
		int kk = 9;
	if (Pruebas)
	veces_cargocamion[Iteracion_Modelo].first++;
	if (sol_mejorada && num_items_camion > 0 )
	{
		if (Pruebas)
		{
			ActualizarEstadisticas(camion);

		}
		camion->lleno = true;
		cam_llenos++;

	}
	else
	{
		if (iter >= camion->Total_Iter_Packing && sol_mejorada == false)
			PintarProblema(123);
	}
		
	


}*/

void Subproblema::PonerParams(parameters& params)
{
	param_file = params.param_file;
	input_trucks = params.input_trucks;
	input_items = params.input_items;
	output_items = params.output_items;
	output_trucks = params.output_trucks;
	output_stack = params.output_stack;
	p_inventario = params.inventory_cost;
	p_transporte = params.transportation_cost;
}
	
bool Subproblema::QuedaAlguno(vector<item_modelo>& vec_items)
{
	for (int i=max(PrimeroNoMeteObligados,0);i< vec_items.size();i++)
	{
		if (vec_items[i].num_items > 0)
			return true;
	}
	return false;
}

void Subproblema::CargoCamion(truck* camion, vector<item_modelo>& vec_items, string idcamion, int& num_items_camion, int numcamion, int tipo_carga)
{
//	cout << "CargoUnCamion" << endl;
	//int tipo_carga = get_random(0, 1);
	//0 es carga por inventario
	// 1 es carga por volumen
	// 2 por peso
	//if (Pruebas) cout << "Inicio Cargo" << endl;
	//Inicializo pesos ejes;

		int kk = 9;
	double paramBV = 0.6;
	if (p_inventario>p_transporte)
		paramBV = 0.1;
	if (camion->CodId==1330 && num_items_camion==111)
		int kk = 9;
	camion->InicializarPesoEjes(Total_Iter_Packing);
	
	//Guardamos el camión actual y las 
	truck* best_camion = new truck(*camion);
	truck* camion_orig = new truck(*best_camion);
	NoMeteObligados = false;

	camion->Intensidad_Pallets_Medios = INTENSITY_MIDDLES;
	//guardo la lista de items 
	int antiguo_coste_inventario = coste_inventario;
	int num_items_original = num_items_camion;
	int best_num_items = num_items_camion;
	int best_coste_inventario = coste_inventario;
	int best_coste_inventario_por_colocar = INT_MAX;
	int best_coste_inventario_item = -1;
	truck_sol best_ts = sol.listado_camiones.back();
	truck_sol orig_ts = sol.listado_camiones.back();

	vector < item_modelo> vec_items_original = vec_items;

	AleatorizarVector(vec_items);
	sort(vec_items.begin(), vec_items.end(), item_modelo::camiones());
	bool multiples_destinos = false;
	if ((vec_items.begin()->orden_ruta) != (vec_items.back().orden_ruta ))
	{
		multiples_destinos = true;
	}
//	if (Pruebas) ComprobarObligadosyClientes(vec_items);

	list<pilas_cargadas> best_pilas;
	list<item_modelo> best_items;
	list<item_modelo> best_item_sol;
	//Aquí guardamos los números que tienen esos items ahora mismo
	vector < item_modelo> cantidad_items_inicio = vec_items;
	vector < item_modelo> best_vec_items = vec_items;
	//if (Pruebas) cout << "Inicio Cargo 2" << endl;
	PonerCantidadesOriginal(cantidad_items_inicio);
	
	vector < item_modelo> best_cantidad_items = cantidad_items_inicio;
	//ordenamos el vector de items

	camion->SetMultipleCliente(vec_items);
	//if (Pruebas) cout << "orden clientes" << endl;
	bool Obligados_Sin_Colocar=true;
	bool Best_Obligados_Sin_Colocar = true;
	//Aqui tengo los items que había en los items al entrar
	int minimo = camion->dim_minima;
	int cont_pilas = 0;
	int casi_coste_inventario_por_colocar = 0;
//	double casi_best_volumen = 0;
	//mientras haya espaios donde colocar y haya items del camión.
	int iter = 0;
	double best_volumen = 0;
	double best_peso = 0;
	bool sol_mejorada = false;
	bool he_pasado = false;

//	if (Pruebas) cout << "Inicio Cargo 4" << endl;
	do
	{
//		if (Pruebas) cout << "Inicio Cargo 4.5:" << iter << endl;

		minimo = camion->dim_minima;
		CalcularMinimo(camion, vec_items);
		cont_pilas = 0;

		camion->crea_espacios(numcamion);
		int orden_anterior = vec_items.front().orden_ruta;
		bool cumple_peso_ejes = false;
//		if (Pruebas) cout << "Inicio Cargo 5:" << iter << endl;
		while (!camion->lesp.at(numcamion).lespacios.empty() && num_items_camion > 0 && 
			(NoMeteObligados==false || (NoMeteObligados && QuedaAlguno(vec_items))))
		{
			if (veces_pila == 263)
				int kk = 9;
			//elegir el espacio
			Tripleta<int>* its = camion->lesp.at(numcamion).Busca_sig_esp();
			//			
			int coloco = 0;
			//if (Pruebas) cout << "Inicio Cargo 6:" << iter << endl;
			if (!((*its).perfiles.size() == 0 && (*its).x != 0))
			{

				//Buscar pila para segmento, siempre coloca abajo
				coloco = Busco_pila(*its, camion, sol.listado_camiones.back(), vec_items, cont_pilas, idcamion);
			}
			//if (Pruebas) cout << "Inicio Cargo 7:" << iter << endl;
			if (coloco == 1)
			{
				//Comprobar peso ejes pero tengo que comprobar con el anterior
				if (camion->orden_por_el_que_voy != orden_anterior)
				{
					orden_anterior = camion->orden_por_el_que_voy;
					//Si pasa el peso medio de los ejes
					if ((camion->emm_colocado_anterior > camion->max_weight_middle_axle || camion->emr_colocado_anterior > camion->max_weight_rear_axle)
						|| (sol.listado_camiones.back().emm_anterior > camion->max_weight_middle_axle || sol.listado_camiones.back().emr_anterior > camion->max_weight_rear_axle))
					{
						camion->Intensidad_Pallets_Medios++;
						if ((2 * camion->Intensidad_Pallets_Medios) > camion->Total_Iter_Packing && he_pasado == false)
							int kk = 9;

						goto actualizar;
					}

				}
				//if (Pruebas) cout << "Pila:" << cont_pilas << endl;
				cont_pilas++;
				num_items_camion -= camion->pilas_solucion.back().id_pila.num_items;
				camion->total_items_modelos -= camion->pilas_solucion.back().id_pila.num_items;




				//Actualizo el mínimo
				if (vec_items[camion->index_minimo].num_items == 0)
				{
					camion->index_minimo = -1;
					camion->dim_minima = camion->length;
					CalcularMinimo(camion, vec_items);
				}
				//Genero los nuevos espacios y actualizar
				camion->lesp.at(numcamion).Generaryactualizar_espacios((camion->pilas_solucion.back().id_pila), *its, camion->dim_minima, En_Medio);
				//lesp.Generaryactualizar_espacios(sol.listado_pilas.back(), *its, camion->dim_minima);
				//
				//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
				for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
				{
					if ((*itl).y_ini <= camion->pilas_solucion.back().id_pila.y_ini &&
						(*itl).y_fin >= camion->pilas_solucion.back().id_pila.y_fin)
					{
						//Limpiar los que sean de antes
						auto itl2 = (*itl).perfiles.begin();
						while (itl2 != (*itl).perfiles.end())
						{
							if ((*itl2).x < camion->pilas_solucion.back().id_pila.x_fin)
							{
								itl2 = (*itl).perfiles.erase(itl2);
							}
							else
							{
								itl2++;
							}
						}
						//Insertamos el último si no estaba
						bool no_meter = false;
						for (auto itp = (*itl).perfiles.begin(); itp != (*itl).perfiles.end() && no_meter != true; itp++)
						{
							if (((*itp).y_ini == camion->pilas_solucion.back().id_pila.y_ini) &&
								((*itp).y_fin == camion->pilas_solucion.back().id_pila.y_fin) &&
								((*itp).x == camion->pilas_solucion.back().id_pila.x_fin))
								no_meter = true;
						}
						if (no_meter == false)
							(*itl).perfiles.push_back(Tripleta<int>(camion->pilas_solucion.back().id_pila.y_ini, camion->pilas_solucion.back().id_pila.y_fin, camion->pilas_solucion.back().id_pila.x_fin));
						break;
					}
				}
				//Limpio perfiles antiguos
				//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
				for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
				{
					//Limpiar los que sean de antes
					auto itl2 = (*itl).perfiles.begin();
					while (itl2 != (*itl).perfiles.end())
					{
						//Si es viejo o se sale del espacio
						if (((*itl2).x < (*itl).x) || ((*itl2).y_ini > (*itl).y_fin) || ((*itl2).y_fin < (*itl).y_ini))
						{
							itl2 = (*itl).perfiles.erase(itl2);
						}
						else
						{
							if ((*itl2).y_fin > (*itl).y_fin) (*itl2).y_fin = (*itl).y_fin;
							if ((*itl2).y_ini < (*itl).y_ini) (*itl2).y_ini = (*itl).y_ini;
							if ((*itl2).y_ini == (*itl2).y_fin)
								itl2 = (*itl).perfiles.erase(itl2);
							else
								itl2++;
						}
					}


				}

			}
			else
			{

				//Comprobar peso ejes
				// ya que voy a cambiar de supplier
				if (coloco == 2)
				{
					if ((camion->emm_colocado > camion->max_weight_middle_axle || camion->emr_colocado > camion->max_weight_rear_axle)
						|| (sol.listado_camiones.back().emm > camion->max_weight_middle_axle || sol.listado_camiones.back().emr > camion->max_weight_rear_axle))
					{
						camion->Intensidad_Pallets_Medios++;
						if ((2 * camion->Intensidad_Pallets_Medios) > camion->Total_Iter_Packing && he_pasado==false)
							int kk = 9;
						//Quitar la última pila
//						if ((camion->emr_colocado > camion->max_weight_rear_axle) && (camion->emr_colocado_anterior < camion->max_weight_rear_axle))
//							camion->QuitarUltimaPila();
						goto actualizar;
					}

				}
				//actualiza espacio
				if (coloco == 3)
				{
					camion->lesp.at(numcamion).lespacios.clear();
					continue;
				}
				if (camion->lesp.at(numcamion).lespacios.size() == 1)
				{
					camion->lesp.at(numcamion).clear();
				}
				else camion->lesp.at(numcamion).actualizo_espacios(*its);
				//Limpio perfiles antiguos
				//
				//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
				for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
				{
					//Limpiar los que sean de antes
					auto itl2 = (*itl).perfiles.begin();
					while (itl2 != (*itl).perfiles.end())
					{
						//Si es viejo o se sale del espacio
						if (((*itl2).x < (*itl).x) || ((*itl2).y_ini > (*itl).y_fin) || ((*itl2).y_fin < (*itl).y_ini))
						{
							itl2 = (*itl).perfiles.erase(itl2);
						}
						else
						{
							if ((*itl2).y_fin > (*itl).y_fin) (*itl2).y_fin = (*itl).y_fin;
							if ((*itl2).y_ini < (*itl).y_ini) (*itl2).y_ini = (*itl).y_ini;
							if ((*itl2).y_ini == (*itl2).y_fin)
								itl2 = (*itl).perfiles.erase(itl2);
							else
								itl2++;
						}
					}


				}
			}
			//Comprobar Espacios
			for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
			{
				auto itl2 = itl;
				itl2++;
				if (itl2!= camion->lesp.at(numcamion).lespacios.end())
				{
					if (itl2->y_ini != itl->y_fin)
						PintarProblema(126);
				}

			}
		}

		if (best_volumen>0 && (2 * iter >= Total_Iter_Packing))
		{
			//Si puedo salta si hay obligados o es del tipo de carga 2
			if (ObligadosSinColocar(vec_items)  )
			{

				NoMeteObligados = true;
				Prob_Saltar_No_Obligado += 1;
				if (Prob_Saltar_No_Obligado > 10)
					Prob_Saltar_No_Obligado = 10;

			}
			else
			{
				Prob_Saltar_No_Obligado -= 1;
				if (Prob_Saltar_No_Obligado < 0)
					Prob_Saltar_No_Obligado = 0;
			}
			if (tipo_carga == 2 && multiples_destinos == true)
			{
				NoMeteObligados = true;
				Prob_Saltar_No_Obligado= 0;
			}

		}

		//if (Pruebas) cout << "Fin Cargo 1:" << iter << endl;
		if (num_items_original == num_items_camion)
		{
			cout<<"Pilas sol:"<<camion->pilas_solucion.size()<< "Intensidad "<<camion->Intensidad_Pallets_Medios << endl;
			cout<<"Problema Volumen_cam:"<<camion->vol_maximo<< "Vol_cargado:"<<camion->volumen_ocupado<< " Num items original"<< num_items_original<<endl;
//			PintarProblema(8);
		}

		if (iter >= (camion->Total_Iter_Packing - 4) && sol_mejorada == false)
			int kk = 9;
		if (camion->emm_colocado > camion->max_weight_middle_axle || camion->emr_colocado > camion->max_weight_rear_axle)
		{
			camion->Intensidad_Pallets_Medios++;
			if (camion->Intensidad_Pallets_Medios > 40 && he_pasado==false)
			{
				cout << "Pilas sol:" << camion->pilas_solucion.size() << "Intensidad " << camion->Intensidad_Pallets_Medios << endl;
				cout << "Problema Volumen_cam:" << camion->vol_maximo << "Vol_cargado:" << camion->volumen_ocupado << " Num items original" << num_items_original << endl;
				cout << "Peso Ejes:" << camion->emm_colocado << " " << camion->emr_colocado << endl;
				cout << "peso maximo" << camion->max_weight_middle_axle << " " << camion->max_weight_rear_axle << endl;
			}
			num_items_camion = num_items_original;
			cumple_peso_ejes = false;
//			if (2* iter>camion->Total_Iter_Packing && he_pasado==false)
//			camion->DibujarTruck();
		}
		else
		{
			cumple_peso_ejes = true;
			if (camion->Intensidad_Pallets_Medios > 1)
				camion->Intensidad_Pallets_Medios--;
			he_pasado = true;
			if (best_volumen > camion->volumen_heuristico)
				int kk = 9;
		}

//		if (Pruebas) cout << "Fin Cargo 2:" << iter << endl;
		if (camion->Total_Iter_Packing > 1 && num_items_camion != 0)
		{
			////Parte reactiva
//			if (Pruebas) cout << "Fin Cargo 2.1:" << iter << endl;
/*			casi_coste_inventario_por_colocar = ((double)get_random(100, 103) / (double)100) * best_coste_inventario_por_colocar;
			casi_best_volumen = ((double)get_random(97, 100) / (double)100)* best_volumen;
			if (cumple_peso_ejes && (((tipo_carga == 0) &&
				((((camion->coste_invetario_items_por_colocar) <= (casi_coste_inventario_por_colocar)) && camion->volumen_ocupado > best_volumen) ||
					(camion->coste_invetario_items_por_colocar < best_coste_inventario_por_colocar))) ||
				((tipo_carga == 1) &&
					((camion->volumen_ocupado > (best_volumen + DBL_EPSILON)) ||
						((camion->volumen_ocupado >= (casi_best_volumen)) && (camion->coste_invetario_items_por_colocar < best_coste_inventario_por_colocar))))))
			{
			*/	

				Obligados_Sin_Colocar = ObligadosSinColocar(vec_items);
                if (cumple_peso_ejes && 
                    ((tipo_carga!=2 && (Obligados_Sin_Colocar == Best_Obligados_Sin_Colocar)  && camion->volumen_ocupado > best_volumen)
                    || (tipo_carga!=2 && !Obligados_Sin_Colocar && Best_Obligados_Sin_Colocar && camion->volumen_ocupado > paramBV * best_volumen)
                    || (tipo_carga == 2 && ((Obligados_Sin_Colocar == Best_Obligados_Sin_Colocar ) || (!Obligados_Sin_Colocar && Best_Obligados_Sin_Colocar) ) && camion->coste_inventario_item > best_coste_inventario_item)))
				{
					if (Obligados_Sin_Colocar == false && Best_Obligados_Sin_Colocar)
						int kk = 9;
					Best_Obligados_Sin_Colocar = Obligados_Sin_Colocar;

//					camion->DibujarTruck();

//				camion->ComprobarPesos();
				sol_mejorada = true;
				best_camion->actualiza(camion);
				if (camion->coste_inventario_item > best_coste_inventario_item)
					best_coste_inventario_item = camion->coste_inventario_item;
				if (camion->coste_invetario_items_por_colocar < best_coste_inventario_por_colocar)
					best_coste_inventario_por_colocar = camion->coste_invetario_items_por_colocar;

					best_volumen = camion->volumen_ocupado;
				if (camion->peso_cargado > best_peso)
					best_peso = camion->peso_cargado;
	//			if (Pruebas) cout << "BEST" << endl;
				best_ts = sol.listado_camiones.back();
				best_pilas.clear();
				best_items.clear();
				best_pilas.insert(best_pilas.begin(), camion->pilas_solucion.begin(), camion->pilas_solucion.end());
				best_items.insert(best_items.begin(),camion->items_modelo.begin(),camion->items_modelo.end());
				best_item_sol.clear();
				best_item_sol.insert(best_item_sol.begin(), vec_items.begin(), vec_items.end());
				best_coste_inventario = coste_inventario;
				best_num_items = num_items_camion;
				//Guardo los numeros originales que quedaba
				best_vec_items = vec_items;
				PonerCantidadesOriginal(best_cantidad_items);
	

			}


			actualizar:
				PrimeroNoMeteObligados = -1;
			camion->pilas_solucion.clear();
			sol.listado_camiones.back().reset();
			coste_inventario = antiguo_coste_inventario;
			camion->actualiza(camion_orig);
			vec_items = vec_items_original;
			//if (Pruebas) cout << "Actualizar" << endl;


			num_items_camion = num_items_original;
			//Restablecer a números originales
			CambiarCantidadesOriginal(cantidad_items_inicio);
			//if (Pruebas) cout << "Cambiar a original" << endl;

			if (he_pasado)
			{
				if (tipo_carga == 2 && iter % 3 == 1)
				{

					sort(vec_items.begin(), vec_items.end(), item_modelo::inventario());
                    
//                  for (const auto& item : vec_items) 
//					{
//                            std::cout << "Inventory Cost: " << item.id_item->inventory_cost << std::endl;

//                    }
				}
				else
				{
					if (iter % 6 == 1)
						sort(vec_items.begin(), vec_items.end(), item_modelo::fecha());
					if (iter % 6 == 2)
						sort(vec_items.begin(), vec_items.end(), item_modelo::densidad());
					if (iter % 6 == 3)
					{
						AleatorizarVector(vec_items);
						if (tipo_carga == 0)
							sort(vec_items.begin(), vec_items.end(), item_modelo::inventario());
						else
							sort(vec_items.begin(), vec_items.end(), item_modelo::volumen());
					}
					if (iter % 6 == 4)
						sort(vec_items.begin(), vec_items.end(), item_modelo::weight());
					if (iter % 6 == 5)
					{
						if (iter > 0)
							AleatorizarVector(vec_items);
						sort(vec_items.begin(), vec_items.end(), item_modelo::volumen());
					}
					if (iter % 6 == 0)
						sort(vec_items.begin(), vec_items.end(), item_modelo::camiones());
				}
//				PintarVector(vec_items, iter);
			}
			else
			{
				if (iter % 2 == 0)
					sort(vec_items.begin(), vec_items.end(), item_modelo::weight());
				else
					sort(vec_items.begin(), vec_items.end(), item_modelo::densidad());


			}

			//if (Pruebas) cout << "sort" << endl;


		}
		iter++;
		//if (Pruebas) cout << "Fin Cargo 3:" << iter << endl;

	} while (num_items_camion > 0 && iter < camion->Total_Iter_Packing);
	//if (Pruebas) cout << "Fin Cargo 1" << endl;
	if (num_items_camion > 0 && best_volumen < 20)
		int kk = 9;
//	if (Last_Ramon )
//	{
		if (!Last_Ramon && max(camion->volumen_ocupado, best_volumen) > camion->volumen_heuristico + DBL_EPSILON)
		{
			double Vol = 0;
			if (num_items_camion == 0)
				Vol = Volumen(vec_items);
			else
			{
				
//				if (Best_Obligados_Sin_Colocar)
//					Vol = 0;
//				else
					Vol = Volumen(best_vec_items); 
			}
			if (Vol > camion->volumen_heuristico + DBL_EPSILON)
			{
				MejoraCotas = true;
				camion->volumen_heuristico = Vol;
				best_camion->volumen_heuristico = Vol;
				for (auto& it : lista_camiones)
				{
					if (it->CodIdCC == camion->CodIdCC)
					{
						it->volumen_heuristico = Vol;
						break;
					}
				}
			}


		}
		if (!Last_Ramon && camion->peso_cargado > camion->peso_heuristico + DBL_EPSILON)
		{
			//		MejoraCotas = true;
			camion->peso_heuristico = camion->peso_cargado;
			best_camion->peso_heuristico = camion->peso_cargado;
			for (auto& it : lista_camiones)
			{
				if ((it->CodIdCC) == camion->CodIdCC)
				{
					it->peso_heuristico = camion->peso_cargado;
					break;
				}
			}
		}
//	}
	if (Pruebas && (iter >= camion->Total_Iter_Packing) && Best_Obligados_Sin_Colocar)
	{
		int obligados = 0;
		int destinos = 0;
		int destino_inicial = 0;
		for (auto it : vec_items)
		{
			if (it.obligado)
				obligados++;
			if (it.orden_ruta != destino_inicial)
			{
				destinos++;
				destino_inicial = it.orden_ruta;
			}

		}
		printf("No mete todos los obligado en este camion Porcentaje %.2f Destinos %d NObligados %d Titems %d\n",paramBV,destinos,obligados,vec_items.size());
	}

	int kk1 = 9;
	if (Pruebas && he_pasado == false)
	{
		cout << "Pilas sol:" << camion->pilas_solucion.size() << "Intensidad " << camion->Intensidad_Pallets_Medios << endl;
		cout << "Problema Volumen_cam:" << camion->vol_maximo << "Vol_cargado:" << camion->volumen_ocupado << " Num items original" << num_items_original << endl;
		
//		PintarProblema(100);

	}
	if (num_items_camion == 0 )
	{

			camion->items_sol.clear();
			camion->items_sol.insert(camion->items_sol.begin(), vec_items.begin(), vec_items.end());


	}
	if (camion->Total_Iter_Packing > 1 && sol_mejorada && best_num_items> 0 )
	{

		if (Last_Ramon)
		{
			//Cambiado cuando Last_Ramon	
		//		if ( camion->peso_total_items_modelo < camion->max_loading_weight &&
		//			VolumenP(vec_items) < camion->volumen_heuristico)
			double Vol = Volumen(best_vec_items);
			if (Vol < camion->volumen_heuristico - 0.02)

			{
				no_colocados++;

				MejoraCotas = true;

				camion->volumen_heuristico = Vol;
				best_camion->volumen_heuristico = Vol;
				for (auto& it : lista_camiones)
				{
					if (it->CodIdCC == camion->CodIdCC)
					{
						it->volumen_heuristico = Vol;
						if (Best_Obligados_Sin_Colocar)
						{
							for (auto it : best_vec_items)
							{
								if (it.obligado && it.num_items > 0)
//								||(!it.obligado && it.colocado > 0))
									camion->prohibidos.insert(it.id_item->CodIdCC);
							}
						}
						break;
					}
				}






			}
			else
			{
				for (auto& it : lista_camiones)
				{
					if (it->CodIdCC == camion->CodIdCC)
					{
						if (Best_Obligados_Sin_Colocar)
						{
							for (auto it : best_vec_items)
							{
								if (it.obligado  && it.num_items > 0)
//									|| (!it.obligado && it.colocado > 0))
//								if (!it.obligado && it.colocado > 0)
									camion->prohibidos.insert(it.id_item->CodIdCC);
							}
						}
						break;
					}
				}
			}
		}
		else
		{
			if (camion->peso_total_items_modelo < camion->max_loading_weight &&
				VolumenP(vec_items) < camion->volumen_heuristico)
			{
				no_colocados++;
			}
		}
		
		if (tipo_carga == 0)
			int kk = 9;
		//int kkx = best_pilas.back().id_pila.x_fin;
		camion->actualiza(best_camion);
//		if (best_volumen > camion->volumen_heuristico)
//			int kk = 9;
		//		camion->total_items_modelos = best_items_modelo_original;
		//if (Pruebas) cout << "actualizar de mejorarl" << endl;
		num_items_camion = best_num_items;

		coste_inventario = best_coste_inventario;
		sol.listado_camiones.back() = best_ts;
		//pasar la best_pila a la solcucion
		camion->pilas_solucion = best_pilas; //poner best_pilas en la pilas_solucion del camion
		CambiarCantidadesOriginal(best_cantidad_items);
		camion->items_sol = best_item_sol;
		vec_items = best_vec_items;

	}


	delete (best_camion);
	delete (camion_orig);
	//if (Pruebas) cout << "delete" << endl;

	if (Pruebas)
		veces_cargocamion[Iteracion_Modelo].first++;
	if (sol_mejorada && num_items_camion > 0 )
	{
		if (Pruebas)
		{
			ActualizarEstadisticas(camion);

		}
		if (!Last_Ramon)
		{
			camion->lleno = true;
			cam_llenos++;
		}
/*		if (best_volumen > 0.75 * camion->volumen_heuristico)
		{
			camion->lleno = true;
			cam_llenos++;
		}
		else
			int kk = 9;
			*/
	}
	else
	{
		if (iter >= camion->Total_Iter_Packing && sol_mejorada == false)
		{
			cout << "No mete nada"<<endl;
			cout << best_coste_inventario_por_colocar << " " << camion->volumen_ocupado << " "<< camion_orig->volumen_ocupado << endl;
			cout << "Best Volumen " << best_volumen << " Iter " << iter << " Iter " << camion->Total_Iter_Packing << endl;
//			PintarProblema(123);
		}
	}



}
void Subproblema::CargoCamionIni(truck* camion, vector<item_modelo>& vec_items, string idcamion, int& num_items_camion, int numcamion)
{
	//Inicializo pesos ejes;
//	if (Pruebas) cout << "Inicio Cargo 4.5:" << " " << Total_Iter_Packing << endl;
	camion->InicializarPesoEjes(Total_Iter_Packing);
//	if (Pruebas) cout << "Inicio Cargo 4.5:" << " " << camion->Total_Iter_Packing <<" " << Total_Iter_Packing << endl;
	double best_peso = 0;
	double best_volumen = 0;
	//Guardamos el camión actual y las 

	truck* camion_orig = new truck(*camion);
	truck_sol camion_ts(camion->id_truck);
	camion_ts.peso_max = camion->max_loading_weight;
	camion_ts.volumen_max = camion->volumen;
	camion->Intensidad_Pallets_Medios = INTENSITY_MIDDLES;
	//guardo la lista de items 

	int num_items_original = num_items_camion;


	vector < item_modelo> vec_items_original = vec_items;

	AleatorizarVector(vec_items);
	sort(vec_items.begin(), vec_items.end(), item_modelo::camiones());




	//Aquí guardamos los números que tienen esos items ahora mismo
	vector < item_modelo> cantidad_items_inicio = vec_items;
	//if (Pruebas) cout << "Inicio Cargo 2" << endl;
	PonerCantidadesOriginal(cantidad_items_inicio);

	vector < item_modelo> best_cantidad_items = cantidad_items_inicio;
	//ordenamos el vector de items

	camion->SetMultipleCliente(vec_items);
	//if (Pruebas) cout << "orden clientes" << endl;

	//Aqui tengo los items que había en los items al entrar
	int minimo = camion->dim_minima;
	int cont_pilas = 0;


	//mientras haya espaios donde colocar y haya items del camión.
	int iter = 0;

	bool sol_mejorada = false;
	bool he_pasado = false;
//	if (Pruebas) cout << "Inicio Cargo 4" << endl;
	do
	{
//		if (Pruebas) cout << "Inicio Cargo 4.5:" << iter << 
//			" "<<Total_Iter_Packing << endl;

		minimo = camion->dim_minima;
		CalcularMinimo(camion, vec_items);
		cont_pilas = 0;

		camion->crea_espacios(numcamion);
		int orden_anterior = vec_items.front().orden_ruta;
		bool cumple_peso_ejes = false;
		//if (Pruebas) cout << "Inicio Cargo 5:" << iter << endl;
		while (!camion->lesp.at(numcamion).lespacios.empty() && num_items_camion > 0)
		{
			//elegir el espacio
			Tripleta<int>* its = camion->lesp.at(numcamion).Busca_sig_esp();
			//			
			int coloco = 0;
			//if (Pruebas) cout << "Inicio Cargo 6:" << iter << endl;
			if (!((*its).perfiles.size() == 0 && (*its).x != 0))
			{

				//Buscar pila para segmento, siempre coloca abajo
				coloco = Busco_pila(*its, camion, camion_ts, vec_items, cont_pilas, idcamion);
			}
			//if (Pruebas) cout << "Inicio Cargo 7:" << iter << endl;
			if (coloco == 1)
			{
				if (camion->peso_cargado > camion->max_loading_weight)
				{
					int kk = 0;
				}
				//Comprobar peso ejes pero tengo que comprobar con el anterior
				if (camion->orden_por_el_que_voy != orden_anterior)
				{
					orden_anterior = camion->orden_por_el_que_voy;
					if ((camion->emm_colocado_anterior > camion->max_weight_middle_axle || camion->emr_colocado_anterior > camion->max_weight_rear_axle)
						|| (camion_ts.emm_anterior > camion->max_weight_middle_axle || camion_ts.emr_anterior > camion->max_weight_rear_axle))
					{
						camion->Intensidad_Pallets_Medios++;

						goto actualizar;
					}

				}
				//if (Pruebas) cout << "Pila:" << cont_pilas << endl;
				cont_pilas++;
				num_items_camion -= camion->pilas_solucion.back().id_pila.num_items;
				camion->total_items_modelos -= camion->pilas_solucion.back().id_pila.num_items;



				//Actualizo el mínimo
				if (vec_items[camion->index_minimo].num_items == 0)
				{
					camion->index_minimo = -1;
					camion->dim_minima = camion->length;
					CalcularMinimo(camion, vec_items);
				}
				//Genero los nuevos espacios y actualizar
				camion->lesp.at(numcamion).Generaryactualizar_espacios((camion->pilas_solucion.back().id_pila), *its, camion->dim_minima, En_Medio);
				//lesp.Generaryactualizar_espacios(sol.listado_pilas.back(), *its, camion->dim_minima);
				//
				//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
				for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
				{
					if ((*itl).y_ini <= camion->pilas_solucion.back().id_pila.y_ini &&
						(*itl).y_fin >= camion->pilas_solucion.back().id_pila.y_fin)
					{
						//Limpiar los que sean de antes
						auto itl2 = (*itl).perfiles.begin();
						while (itl2 != (*itl).perfiles.end())
						{
							if ((*itl2).x < camion->pilas_solucion.back().id_pila.x_fin)
							{
								itl2 = (*itl).perfiles.erase(itl2);
							}
							else
							{
								itl2++;
							}
						}
						//Insertamos el último si no estaba
						bool no_meter = false;
						for (auto itp = (*itl).perfiles.begin(); itp != (*itl).perfiles.end() && no_meter != true; itp++)
						{
							if (((*itp).y_ini == camion->pilas_solucion.back().id_pila.y_ini) &&
								((*itp).y_fin == camion->pilas_solucion.back().id_pila.y_fin) &&
								((*itp).x == camion->pilas_solucion.back().id_pila.x_fin))
								no_meter = true;
						}
						if (no_meter == false)
							(*itl).perfiles.push_back(Tripleta<int>(camion->pilas_solucion.back().id_pila.y_ini, camion->pilas_solucion.back().id_pila.y_fin, camion->pilas_solucion.back().id_pila.x_fin));
						break;
					}
				}
				//Limpio perfiles antiguos
				//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
				for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
				{
					//Limpiar los que sean de antes
					auto itl2 = (*itl).perfiles.begin();
					while (itl2 != (*itl).perfiles.end())
					{
						//Si es viejo o se sale del espacio
						if (((*itl2).x < (*itl).x) || ((*itl2).y_ini > (*itl).y_fin) || ((*itl2).y_fin < (*itl).y_ini))
						{
							itl2 = (*itl).perfiles.erase(itl2);
						}
						else
						{
							if ((*itl2).y_fin > (*itl).y_fin) (*itl2).y_fin = (*itl).y_fin;
							if ((*itl2).y_ini < (*itl).y_ini) (*itl2).y_ini = (*itl).y_ini;
							if ((*itl2).y_ini == (*itl2).y_fin)
								itl2 = (*itl).perfiles.erase(itl2);
							else
								itl2++;
						}
					}


				}

			}
			else
			{

				//Comprobar peso ejes
				// ya que voy a cambiar de supplier
				if (coloco == 2)
				{
					if ((camion->emm_colocado > camion->max_weight_middle_axle || camion->emr_colocado > camion->max_weight_rear_axle)
						|| (camion_ts.emm > camion->max_weight_middle_axle || camion_ts.emr > camion->max_weight_rear_axle))
					{
						camion->Intensidad_Pallets_Medios++;

						goto actualizar;
					}

				}
				//actualiza espacio
				if (coloco == 3)
				{
					camion->lesp.at(numcamion).lespacios.clear();
					continue;
				}
				if (camion->lesp.at(numcamion).lespacios.size() == 1)
				{
					camion->lesp.at(numcamion).clear();
				}
				else camion->lesp.at(numcamion).actualizo_espacios(*its);
				//Limpio perfiles antiguos
				//
				//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
				for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
				{
					//Limpiar los que sean de antes
					auto itl2 = (*itl).perfiles.begin();
					while (itl2 != (*itl).perfiles.end())
					{
						//Si es viejo o se sale del espacio
						if (((*itl2).x < (*itl).x) || ((*itl2).y_ini > (*itl).y_fin) || ((*itl2).y_fin < (*itl).y_ini))
						{
							itl2 = (*itl).perfiles.erase(itl2);
						}
						else
						{
							if ((*itl2).y_fin > (*itl).y_fin) (*itl2).y_fin = (*itl).y_fin;
							if ((*itl2).y_ini < (*itl).y_ini) (*itl2).y_ini = (*itl).y_ini;
							if ((*itl2).y_ini == (*itl2).y_fin)
								itl2 = (*itl).perfiles.erase(itl2);
							else
								itl2++;
						}
					}


				}
			}
		}
		//if (Pruebas) cout << "Fin Cargo 1:" << iter << endl;
/*		if (num_items_original == num_items_camion)
		{
			cout << "Pilas sol:" << camion->pilas_solucion.size() << " Intensidad " << camion->Intensidad_Pallets_Medios << endl;
			cout << "Problema Volumen_cam:" << camion->vol_maximo << " Vol_cargado: " << camion->volumen_ocupado << " Num items original" << num_items_original << endl;
//			PintarProblema(8);
		}*/


		if (camion->emm_colocado > camion->max_weight_middle_axle || camion->emr_colocado > camion->max_weight_rear_axle)
		{
			camion->Intensidad_Pallets_Medios++;
			num_items_camion = num_items_original;
			cumple_peso_ejes = false;
		}
		else
		{
			cumple_peso_ejes = true;
			if (camion->Intensidad_Pallets_Medios > 1)
				camion->Intensidad_Pallets_Medios--;
			he_pasado = true;
		}

//		if (Pruebas) cout << "Fin Cargo 21:" << iter << endl;
//		if (camion->Total_Iter_Packing > 1 && num_items_camion != 0)
//		{
			////Parte reactiva
			//if (Pruebas) cout << "Fin Cargo 2.1:" << iter << endl;
			if (cumple_peso_ejes) 
			{
//				camion->ComprobarPesos();
//				camion->DibujarTruck();
				double volumen = Volumen(vec_items);

//				if (Pruebas) cout << "Fin Cargo 23:" << iter << endl;
				if (volumen > best_volumen)
				{
					best_volumen = volumen;
					
				}
				if (camion->peso_cargado > best_peso)
					best_peso = camion->peso_cargado;
				if (camion->peso_cargado > camion->max_loading_weight)
					int kk = 9;
				sol_mejorada = true;

			}

//			if (Pruebas) cout << "Fin Cargo 24:" << iter << endl;
		actualizar:
//			if (Pruebas) cout << "Fin Cargo 25:" << iter << endl;
			camion->pilas_solucion.clear();
//			if (Pruebas) cout << "Fin Cargo 250:" << iter << endl;

//			if (Pruebas) cout << "Fin Cargo 251:" << iter << endl;
			camion_ts.reset();
//			if (Pruebas) cout << "Fin Cargo 252:" << iter << endl;
			camion->actualiza(camion_orig);
//			if (Pruebas) cout << "Fin Cargo 253:" << iter << endl;
			vec_items = vec_items_original;


//			if (Pruebas) cout << "Actualizar" << endl;
						//Restablecer a números originales
//			if (Pruebas) cout << "Fin Cargo 26:" << iter << endl;
			CambiarCantidadesOriginal(cantidad_items_inicio);

			if (num_items_camion > 0)
			{

				num_items_camion = num_items_original;

				//if (Pruebas) cout << "Cambiar a original" << endl;
				if (iter % 5 == 1)
				{
					AleatorizarVector(vec_items);
					sort(vec_items.begin(), vec_items.end(), item_modelo::weight());
				}
					
				if (iter % 5 == 2)
					sort(vec_items.begin(), vec_items.end(), item_modelo::volumen());
				if (iter % 5 == 3)
				{
					sort(vec_items.begin(), vec_items.end(), item_modelo::width_vol_ini());

				}
				if (iter % 5 == 4)
					sort(vec_items.begin(), vec_items.end(), item_modelo::weight());
				if (iter % 5 == 0 )
				{
					if (iter>0)
					AleatorizarVector(vec_items);
					sort(vec_items.begin(), vec_items.end(), item_modelo::volumen());
				}



			}

			//if (Pruebas) cout << "sort" << endl;


//		}
		iter++;
//		if (Pruebas) cout << "Fin Cargo 3:" << iter << endl;

	} while (num_items_camion > 0 && iter < camion->Total_Iter_Packing);
	if (camion->volumen_heuristico<best_volumen)
		camion->volumen_heuristico = best_volumen+0.01;
	if (camion->peso_heuristico < best_peso)
		camion->peso_heuristico = min(best_peso+0.01,camion->max_loading_weight);
	//if (Pruebas) cout << "Fin Cargo 1" << endl;
	if (he_pasado == false)
	{
		cout << "Pilas sol:" << camion->pilas_solucion.size() << "Intensidad Ini" << camion->Intensidad_Pallets_Medios << endl;
		cout << "Problema Volumen_cam:" << camion->vol_maximo << "Vol_cargado:" << camion->volumen_ocupado << " Num items original" << num_items_original << endl;

		PintarProblema(100);

	}


	delete (camion_orig);
	//if (Pruebas) cout << "delete" << endl;

//	if (Pruebas)
//		veces_cargocamion[Iteracion_Modelo].first++;





}
void Subproblema::ActualizarEstadisticas(truck* camion)
{
	veces_cargocamion[Iteracion_Modelo].second++;
	if (veces_cargocamion[Iteracion_Modelo].second > 1)
	{
		cargocamion_char[Iteracion_Modelo][0] = (((double)(veces_cargocamion[Iteracion_Modelo].second - 1) * cargocamion_char[Iteracion_Modelo][0]) + camion->peso_cargado) / ((double)veces_cargocamion[Iteracion_Modelo].second);
		cargocamion_char[Iteracion_Modelo][1] = (((double)(veces_cargocamion[Iteracion_Modelo].second - 1) * cargocamion_char[Iteracion_Modelo][1]) + camion->volumen_ocupado) / ((double)veces_cargocamion[Iteracion_Modelo].second);
		cargocamion_char[Iteracion_Modelo][2] = (((double)(veces_cargocamion[Iteracion_Modelo].second - 1) * cargocamion_char[Iteracion_Modelo][2]) + camion->peso_heuristico) / ((double)veces_cargocamion[Iteracion_Modelo].second);
		cargocamion_char[Iteracion_Modelo][3] = (((double)(veces_cargocamion[Iteracion_Modelo].second - 1) * cargocamion_char[Iteracion_Modelo][3]) + camion->volumen_heuristico) / ((double)veces_cargocamion[Iteracion_Modelo].second);

	}
	else
	{
		cargocamion_char[Iteracion_Modelo][0] = camion->peso_cargado;
		cargocamion_char[Iteracion_Modelo][1] = camion->volumen_ocupado;
		cargocamion_char[Iteracion_Modelo][2] = camion->peso_heuristico;
		cargocamion_char[Iteracion_Modelo][3] = camion->volumen_heuristico;
	}
}
void Subproblema::ArreglarEspacio(truck* camion, int numcamion)
{
	for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
	{
		if ((*itl).y_ini <= camion->pilas_solucion.back().id_pila.y_ini &&
			(*itl).y_fin >= camion->pilas_solucion.back().id_pila.y_fin)
		{
			//Limpiar los que sean de antes
			auto itl2 = (*itl).perfiles.begin();
			while (itl2 != (*itl).perfiles.end())
			{
				if ((*itl2).x < camion->pilas_solucion.back().id_pila.x_fin)
				{
					itl2 = (*itl).perfiles.erase(itl2);
				}
				else
				{
					itl2++;
				}
			}
			//Insertamos el último si no estaba
			bool no_meter = false;
			for (auto itp = (*itl).perfiles.begin(); itp != (*itl).perfiles.end() && no_meter != true; itp++)
			{
				if (((*itp).y_ini == camion->pilas_solucion.back().id_pila.y_ini) &&
					((*itp).y_fin == camion->pilas_solucion.back().id_pila.y_fin) &&
					((*itp).x == camion->pilas_solucion.back().id_pila.x_fin))
					no_meter = true;
			}
			if (no_meter == false)
				(*itl).perfiles.push_back(Tripleta<int>(camion->pilas_solucion.back().id_pila.y_ini, camion->pilas_solucion.back().id_pila.y_fin, camion->pilas_solucion.back().id_pila.x_fin));
			break;
		}
	}
	//Limpio perfiles antiguos
	//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
	for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
	{
		//Limpiar los que sean de antes
		auto itl2 = (*itl).perfiles.begin();
		while (itl2 != (*itl).perfiles.end())
		{
			//Si es viejo o se sale del espacio
			if (((*itl2).x < (*itl).x) || ((*itl2).y_ini > (*itl).y_fin) || ((*itl2).y_fin < (*itl).y_ini))
			{
				itl2 = (*itl).perfiles.erase(itl2);
			}
			else
			{
				if ((*itl2).y_fin > (*itl).y_fin) (*itl2).y_fin = (*itl).y_fin;
				if ((*itl2).y_ini < (*itl).y_ini) (*itl2).y_ini = (*itl).y_ini;
				if ((*itl2).y_ini == (*itl2).y_fin)
					itl2 = (*itl).perfiles.erase(itl2);
				else
					itl2++;
			}
		}


	}
}
void Subproblema::QuitarRestos(truck* camion, int numcamion, Tripleta<int> *its)
{
	if (camion->lesp.at(numcamion).lespacios.size() == 1)
	{
		camion->lesp.at(numcamion).clear();
	}
	else camion->lesp.at(numcamion).actualizo_espacios(*its);
	//Limpio perfiles antiguos
	//
	//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
	for (auto itl = camion->lesp.at(numcamion).lespacios.begin(); itl != camion->lesp.at(numcamion).lespacios.end(); itl++)
	{
		//Limpiar los que sean de antes
		auto itl2 = (*itl).perfiles.begin();
		while (itl2 != (*itl).perfiles.end())
		{
			//Si es viejo o se sale del espacio
			if (((*itl2).x < (*itl).x) || ((*itl2).y_ini > (*itl).y_fin) || ((*itl2).y_fin < (*itl).y_ini))
			{
				itl2 = (*itl).perfiles.erase(itl2);
			}
			else
			{
				if ((*itl2).y_fin > (*itl).y_fin) (*itl2).y_fin = (*itl).y_fin;
				if ((*itl2).y_ini < (*itl).y_ini) (*itl2).y_ini = (*itl).y_ini;
				if ((*itl2).y_ini == (*itl2).y_fin)
					itl2 = (*itl).perfiles.erase(itl2);
				else
					itl2++;
			}
		}


	}
}
bool Subproblema::comprobarPesoEjesActualizarIntensidad(truck* camion, solucion& solu, bool checkAnterior) {
	bool result = false;
	if (checkAnterior) {
		result = (camion->emm_colocado_anterior > camion->max_weight_middle_axle || camion->emr_colocado_anterior > camion->max_weight_rear_axle) ||
			(sol.listado_camiones.back().emm_anterior > camion->max_weight_middle_axle || solu.listado_camiones.back().emr_anterior > camion->max_weight_rear_axle);
	}
	else {
		result = (camion->emm_colocado > camion->max_weight_middle_axle || camion->emr_colocado > camion->max_weight_rear_axle) ||
			(sol.listado_camiones.back().emm > camion->max_weight_middle_axle || solu.listado_camiones.back().emr > camion->max_weight_rear_axle);
	}
	if (result) {
		camion->Intensidad_Pallets_Medios++;
	}
	return result;
}
void Subproblema::escribe(string file)
{
	

	for (auto itc = sol.listado_camiones.begin(); itc != sol.listado_camiones.end(); itc++)
		if((*itc).weight_loaded>0) itc->escribe(file);
	
	
	for (auto itc = lista_camiones.begin(); itc != lista_camiones.end(); itc++)
	{
		for (auto its = (*itc)->pilas_solucion.begin(); its != (*itc)->pilas_solucion.end(); its++)
		{
			its->id_pila.escribe(file);
			for (auto iti = its->items.begin(); iti != its->items.end(); iti++)
			{
				(*iti).escribe(file);
			}
		}
	}
	

}

