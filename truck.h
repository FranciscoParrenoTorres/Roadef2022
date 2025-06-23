#include "encabezados.h"

#ifndef TRUCK_H
#define TRUCK_H

class pilas_cargadas;
class espacios;
class item_modelo;

class truck
{
	
public:
	//vector<string> ruta_proveedores; // ruta  cod_supplier|doc supplier|dockplant
	//int cuantos_proveedores_en_ruta=0;
	set<int> dimensiones_posibles_base; //Número de posibles valores de los pallets que van en el camión
	set<int> dimensiones_posibles_height; //Número de posibles valores de los pallets que van en el camión
	set<int> prohibidos;
	//vector<int> supplier_loading_order; // orden de carga del proveedor
	//vector<string> supplier_dock; // muelle del proveedor
	//vector<int> supplier_dock_loading_order; // orden de carga del muelle del proveedor
	//char plant_code[30]; // codigo de la planta
	//vector<string> plant_dock; // muelle de la planta
	//vector<int> plant_dock_loading_order; // orden de carga del muelle de la planta
	map<string,int> orden_ruta; //ruta por suplier-suplier dock - plant dock y numero orden
	map<int, string> int_order_ruta; //orden de la ruta, supplier por orden.
	set<int, string>int_order_ruta_set;
	list<string> code_product; // lista de items
	long long int arrival_time; // tiempo de llegada 
	time_t arrival_time_tm; // tiempo de llegada en formato tm
	tm arrival_time_tm_ptr; // puntero al tiempo de llegada en formato tm
	char id_truck[30]; // id del camion
	int length; // longitud
	int width; // anchura
	int height; // altura 	
	int length_kp; // longitud knapsack
	int width_kp; // anchura knapsack
	int height_kp; // altura knapsack
	int min_dim_width=99999;
	int min_dim_length=99999;
	int ndestinos = 1;
	int CodId; //número de identificación del truck de 0 a el número de trucks
	int CodIdCC; //número de identificación del truck de 0 a el número de trucks en la CC
	int CodIdCCE; //número de identificación del truck de 0 a el número de trucks en la CC y el número de extra
	double max_loading_weight; // peso maximo de carga
	double max_loading_weight_orig; // peso maximo de carga
	int stack_with_multiple_docks; //pila que va varios muelles
	int max_stack_density; //densidad máxima de la pila
	int max_weight_bottom_item_stack; // peso maximo del item inferior de la pila
	int cost; //coste C_j
	float coste_extra; //coste de poner un camion extra de este camión
	int max_weight_middle_axle;	// peso maximo del eje medio
	int max_weight_rear_axle; // peso maximo del eje trasero
	float weight_tractor; //peso máximo eje tractor
	int distance_front_midle_axle; // distancia del eje delantero al eje medio
	int distance_front_axle_cog_tractor; //distancia del eje delantero al centro de gravedad tractor
	int distnace_front_axle_harness_tractor; // distancia del eje delantero al enganche tractor
	float weight_empty_truck; // peso vacio del camion
	int distance_harness_rear_axle; // distancia del enganche al eje trasero
	int distance_cog_tractor_rear_axle; // distancia del centro de gravedad tractor al eje trasero 
	int distance_star_trailer_harness; // distancia del comienzo del camión al remolque
	list<item*> items; // lista de items que pueden ir en el camion
	list<item*> items_originales; // lista de items que pueden ir en el camion originales
	vector<item_modelo> items_modelo; // lista de items que pueden ir en el camion según modelo
	list<vector<item_modelo>> items_extras_modelo; // lista de items por copia extra que pueden ir en el camion según modelo
	double volumen; // volumen del camion
	double volumen_inicial;
	double peso_inicial;
	int mas_extras = 0;//Si en la siguiente iteracion hay que poner más de este tipo
	double volumen_kp; // volumen del camion del knapsack
	double peso_total_items; //Peso total de los items que pueden ir en el camión
	double volumen_total_items; //Volumen total de los items que pueden ir en el camión
	double peso_total_items_modelo;
	double volumen_total_items_modelo;
	double volumen_heuristico; //volumen máximo calculado por el heurístico 
	double peso_heuristico; //peso máximo calculado por el heurístico 
	int cota_extra; //Número máximo de camiones extras que se pueden poner
	int num_items_modelo; //numero de items que asigna el modelo al camion
	vector<int> num_items_extra;
	vector<double> peso_total_items_modelo_extra;
	vector<double> volumen_total_items_modelo_items_extra;
	int dim_minima; //diemnsion mínima de los items que pueden ir en el camión
	int minima_altura;
	bool pila_multi_dock;
	bool Modelo_pesado; //la asignación del modelo mete mucho peso en el camión
	double peso_total_items_modelo_asignados;
	double volumen_total_items_modelo_asignados;
	map<string, int> pesos_bottom_prod;
	int num_extras_creados; //número de extras creados con pallets
	int total_items_modelos; //todos los items que va a poner tanto en el camion como en los extras.
	int coste_invetario_items_por_colocar;
	bool lleno;
	int Intensidad_Pallets_Medios=3; //intensidad de los pallets medios
	bool no_puede_colocar_nada;
	int index_minimo;
	int codigo_cierre;
	int coste_inventario_item;
	int coste_inventario_camion;
	bool Abajo = true;
	list<pilas_cargadas> pilas_solucion;
	int orden_por_el_que_voy;
	double volumen_ocupado;
	double peso_cargado;
	vector<espacios> lesp;
	list<truck*> pcamiones_extra;
	int Total_Iter_Packing; //Total de iteraciones del modelo de packing	
	double volumen_a_cargar = 0; //Volumen que voy a querer cargar en el camion
	double peso_a_cargar = 0; //Peso que voy a querer cargar en el camion
	bool Empezando_Camion ;//es para colocar solamente una fila al principio
	bool multiple_cliente;
	bool pocos_items = false;
	double emm_colocado;
	double emr_colocado;
	double emm_colocado_anterior; //antes de colocar la última pila
	double emr_colocado_anterior; //antes de colocar la última pila
	double vol_antes;
	double peso_antes;
	vector<int>lcompo;
	list<item_modelo> items_sol;
	bool recargar;
	bool fijado;
	map<string, int> items_colocados;
	bool es_extra;
	int kextra;
	double vol_maximo;
	int load_length;
	bool necesito = false;
	bool ComprobarPesos();
#ifdef _WIN32
	int DibujarTruck();
#endif	
	void vol_peso_cam();
	truck()
	{
		//recargar = false;
		fijado = false;
	};
	void actualiza(truck * t1)
	{
		dim_minima = t1->dim_minima; //diemnsion mínima de los items que pueden ir en el camión
		minima_altura = t1->minima_altura;
		pila_multi_dock = t1->pila_multi_dock;
		coste_invetario_items_por_colocar = t1->coste_invetario_items_por_colocar;
		lleno = t1->lleno;
		index_minimo = t1->index_minimo;
		codigo_cierre = t1->codigo_cierre;
		orden_por_el_que_voy =t1->orden_por_el_que_voy;
		volumen_ocupado = t1->volumen_ocupado;
		volumen_heuristico = t1->volumen_heuristico;
		peso_heuristico = t1->peso_heuristico;
		coste_inventario_item = t1->coste_inventario_item;
		coste_inventario_camion = t1->coste_inventario_camion;
		total_items_modelos = t1->total_items_modelos;
		num_extras_creados = t1->num_extras_creados;
		Empezando_Camion = t1->Empezando_Camion;
		peso_cargado = t1->peso_cargado;
		multiple_cliente = t1->multiple_cliente;
		emm_colocado = t1->emm_colocado;
		emr_colocado = t1->emr_colocado;
		pocos_items = t1->pocos_items;
		emm_colocado_anterior = t1->emm_colocado_anterior;
		emr_colocado_anterior = t1->emr_colocado_anterior;
		Total_Iter_Packing = t1->Total_Iter_Packing;
		recargar = t1->recargar;
		fijado = t1->fijado;
		es_extra = t1->es_extra;
		kextra = t1->kextra;
		vol_maximo = t1->vol_maximo;
		load_length = t1->load_length;
		necesito=t1->necesito;
		//TODO cargar lista de la otra sol
//		pilas_cargadas.clear();

	}
	truck(const truck& t1)
	{
		for (auto it = t1.orden_ruta.begin(); it != t1.orden_ruta.end(); it++)
		{
			orden_ruta.insert(pair<string, int>(it->first, it->second));
		}
		for (auto it = t1.int_order_ruta.begin(); it != t1.int_order_ruta.end(); it++)
		{
			int_order_ruta.insert(pair<int, string>(it->first, it->second));
		}
		for (auto it = t1.code_product.begin(); it != t1.code_product.end(); it++)
		{
			code_product.push_back(*it);
		}
		Total_Iter_Packing=t1.Total_Iter_Packing; //Total de iteraciones del modelo de packing	
		volumen_a_cargar = t1.volumen_a_cargar; //Volumen que voy a querer cargar en el camion
		peso_a_cargar = t1.peso_a_cargar; //Peso que voy a querer cargar en el camion
		Empezando_Camion = t1.Empezando_Camion;//es para colocar solamente una fila al principio
		arrival_time=t1.arrival_time; 
		coste_inventario_item = t1.coste_inventario_item;
		coste_inventario_camion = t1.coste_inventario_camion;
		arrival_time_tm=t1.arrival_time_tm; 
		arrival_time_tm_ptr = t1.arrival_time_tm_ptr;
		Intensidad_Pallets_Medios = t1.Intensidad_Pallets_Medios;
		strcpy(id_truck,t1.id_truck); 
		length=t1.length;
		pocos_items = t1.pocos_items;
		width=t1.width; 
		height=t1.height; 	
		length_kp=t1.length_kp; 
		width_kp=t1.width_kp; 
		height_kp=t1.height_kp; 
		CodId=t1.CodId; 
		CodIdCC=t1.CodIdCC; 
		max_loading_weight=t1.max_loading_weight; 
		max_loading_weight_orig = t1.max_loading_weight_orig;
		peso_heuristico = t1.peso_heuristico;
		volumen_heuristico = t1.volumen_heuristico;
		stack_with_multiple_docks=t1.stack_with_multiple_docks; 
		max_stack_density=t1.max_stack_density; 
		max_weight_bottom_item_stack=t1.max_weight_bottom_item_stack; 
		cost=t1.cost; 
		coste_extra=t1.coste_extra; 
		max_weight_middle_axle=t1.max_weight_middle_axle;	
		max_weight_rear_axle=t1.max_weight_rear_axle; 
		weight_tractor=t1.weight_tractor; 
		distance_front_midle_axle=t1.distance_front_midle_axle; 
		distance_front_axle_cog_tractor=t1.distance_front_axle_cog_tractor; 
		distnace_front_axle_harness_tractor=t1.distnace_front_axle_harness_tractor; 
		weight_empty_truck=t1.weight_empty_truck; 
		distance_harness_rear_axle=t1.distance_harness_rear_axle; 
		distance_cog_tractor_rear_axle=t1.distance_cog_tractor_rear_axle; 
		distance_star_trailer_harness=t1.distance_star_trailer_harness; 
		for (auto it = t1.items.begin(); it != t1.items.end(); it++)
		{
			items.push_back((*it));
		}
		necesito = t1.necesito;
		volumen_heuristico = t1.volumen_heuristico; //volumen que se le asigna al camion por la heuristica
		peso_heuristico = t1.peso_heuristico;
		volumen=t1.volumen; // volumen del camion
		volumen_kp=t1.volumen_kp; // volumen del camion del knapsack
		peso_total_items=t1.peso_total_items; //Peso total de los items que pueden ir en el camión
		volumen_total_items=t1.volumen_total_items; //Volumen total de los items que pueden ir en el camión
		peso_total_items_modelo=t1.peso_total_items_modelo;
		volumen_total_items_modelo=t1.volumen_total_items_modelo;
		cota_extra=t1.cota_extra;
		num_items_modelo=t1.num_items_modelo; //numero de items que asigna el modelo al camion
		dim_minima=t1.dim_minima; //diemnsion mínima de los items que pueden ir en el camión
		minima_altura=t1.minima_altura;
		pila_multi_dock=t1.pila_multi_dock;
		Modelo_pesado =t1.Modelo_pesado; //la asignación del modelo mete mucho peso en el camión
		peso_total_items_modelo_asignados=t1.peso_total_items_modelo_asignados;
		volumen_total_items_modelo_asignados=t1.volumen_total_items_modelo_asignados;
		for (auto it = t1.pesos_bottom_prod.begin(); it != t1.pesos_bottom_prod.end(); it++)
		{
			pesos_bottom_prod.insert(pair<string,int>(it->first, it->second));
		}
		num_extras_creados=t1.num_extras_creados; //número de extras creados con pallets
		total_items_modelos=t1.total_items_modelos; //todos los items que va a poner tanto en el camion como en los extras.
		coste_invetario_items_por_colocar=t1.coste_invetario_items_por_colocar;
		lleno=t1.lleno;
		index_minimo=t1.index_minimo;
		codigo_cierre=t1.codigo_cierre;
		orden_por_el_que_voy= t1.orden_por_el_que_voy;
		volumen_ocupado= t1.volumen_ocupado;
		peso_cargado = t1.peso_cargado;
		//if (t1.peso_cargado > 0)
		//{
		//	int kk = 9;
		//}
		multiple_cliente = t1.multiple_cliente;
		emm_colocado = t1.emm_colocado;
		emr_colocado = t1.emr_colocado;
		emm_colocado_anterior = t1.emm_colocado_anterior;
		emr_colocado_anterior = t1.emr_colocado_anterior;
		recargar = t1.recargar;
		fijado = t1.fijado;
		es_extra = t1.es_extra;
		kextra = t1.kextra;
		vol_maximo=t1.vol_maximo;
		load_length=t1.load_length;
		
	};
	int Diferencia_Dias(truck* t2)
	{
		int dif = Days_0001(this->arrival_time_tm_ptr.tm_year, this->arrival_time_tm_ptr.tm_mon + 1, this->arrival_time_tm_ptr.tm_mday) -
			Days_0001(t2->arrival_time_tm_ptr.tm_year, t2->arrival_time_tm_ptr.tm_mon + 1, t2->arrival_time_tm_ptr.tm_mday);
		return dif;
	}
	bool mismo_dia(truck* t2)
	{
		int dif=Days_0001(this->arrival_time_tm_ptr.tm_year, this->arrival_time_tm_ptr.tm_mon + 1, this->arrival_time_tm_ptr.tm_mday) -
			Days_0001(t2->arrival_time_tm_ptr.tm_year, t2->arrival_time_tm_ptr.tm_mon + 1, t2->arrival_time_tm_ptr.tm_mday);
/*		tm lt;
		gmtime(&lt, &this->arrival_time_tm);
		tm lt2;
		gmtime_s(&lt2, &t2->arrival_time_tm);
		//si son del mismo dia
		if (lt.tm_year == lt2.tm_year && lt.tm_mon == lt2.tm_mon && lt.tm_mday == lt2.tm_mday)*/
		if (dif==0)
		{
			return true;
		}
		else
		{
			//if (dif < 0)
			//	int kk = 9;
			return false;

		}
	};
	void crea_espacios(int i)
	{
		lesp.clear();
		if (i >= lesp.size())
		{
			for (int j = (int)lesp.size(); j <= i; j++)
			{
				espacios e;
				lesp.push_back(e);
			}
		}
		
		lesp.at(i).add(Tripleta<int>(0, this->width, 0));
	};
	/*
	void QuitarUltimaPila()
	{
		//Poner los valores de los ítems
		for (auto it : pilas_solucion.back().items)
		{

		}
	}*/
	void reset()
	{
		Empezando_Camion = false;
		items_modelo.clear();
		items_extras_modelo.clear();
		peso_total_items_modelo = 0;
		volumen_total_items_modelo = 0;
		num_items_modelo = 0;
		num_items_extra.clear();
		peso_total_items_modelo_extra.clear();
		volumen_total_items_modelo_items_extra.clear();
		dim_minima = 0;
		minima_altura = height;
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
		prohibidos.clear();
		necesito = false;
		pcamiones_extra.clear();
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
		//recargar = false;
		fijado = false;
		items_colocados.clear();
		items_sol.clear();
		load_length = 0;
		
	};
	static bool comparador (truck  *a,truck *b)
	{

		if (a->arrival_time > b->arrival_time)
			return true;
		else
			return false;
	};
	void clean()
	{
		Empezando_Camion = true;
		items_modelo.clear();
		items_extras_modelo.clear();
		peso_total_items_modelo = 0;
		volumen_total_items_modelo = 0;
		num_items_modelo = 0;
		num_items_extra.clear();
		peso_total_items_modelo_extra.clear();
		volumen_total_items_modelo_items_extra.clear();
		dim_minima = 0;
		minima_altura = height;
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
		pcamiones_extra.clear();
		prohibidos.clear();
		lleno = false;
		pocos_items = false;
		volumen_a_cargar = 0; //Volumen que voy a querer cargar en el camion
		peso_a_cargar = 0; //Peso que voy a querer cargar en el camion
		Empezando_Camion = true;
		peso_cargado = 0;

		no_puede_colocar_nada = false;
		emm_colocado = 0;
		emr_colocado = 0;
		emm_colocado_anterior = 0;
		emr_colocado_anterior = 0;
		vol_antes = 0;
		peso_antes = 0;
		fijado = false;
		load_length = 0;

		
	};
	bool comprobar_coste()
	{
		/*int coste = 0;
		for (auto itp = pilas_solucion.begin(); itp != pilas_solucion.end(); itp++)
		{
			for (auto iti = (*itp).items.begin(); iti != (*itp).items.end(); iti++)
			{
				coste += (*iti)->inventory_cost;
			}
		}
		if (coste_invetario_items_por_colocar != coste)
		{
			return false;
		}
		else return true;*/
	};
	void InicializarPesoEjes(int a);
	void SetMultipleCliente(vector<item_modelo>& vec_items);
	void reanudar();
	void reset2();

	void AddItem(item* iT, int cuantos, int orden, int obli);

	int DiferenciaDiasCamCam(truck* iT);

	int DiferenciaDiasCamItem(item* iT);

	int DiferenciaDiasLastCamItem(item* iT);

	int Diferencia_Dias(item* iT);

	void reanudar_solucion();
};
#endif
