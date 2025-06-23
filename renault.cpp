// renault.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include "encabezados.h"

void escribeG(parameters& params, mejor_sol mej);
double calcularMediana(std::vector<int>& vec,int cuantil);
int main(int argc, char* argv[])
{
	string param_file;
	string input_items;
	string input_trucks;
	string output_items;
	string output_trucks;
	string output_stack;
	string file_to_read;
	string file_to_write;
	int Seconds = 0;
	int seed = 1;
	int nThreads = 4;
	int Gurobi = 0;
	int Tipo = 13;
	bool Pruebas = true;
	struct timeb time_final, time_initial;

	
	//Inicio el contador de tiempo
	ftime(&time_initial);


	if ((argc > 2 && argc!=15)) 
	{          
		
		while (argc > 1)
		{
			if (strcmp(argv[1], "-Time") == 0)
			{ //Tiempo
				sscanf(argv[2], "%d", &Seconds);
			}
			else if (strcmp(argv[1], "-InputFile") == 0)
			{ //File to read
				file_to_read = argv[2];
				file_to_write = argv[2];
			}
			else if (strcmp(argv[1], "-OutputFile") == 0)
			{
				file_to_write = argv[2];
			}
			else if (strcmp(argv[1], "-Seed") == 0)
			{
				sscanf(argv[2], "%d", &seed);
			}
			else if (strcmp(argv[1], "-NThreads") == 0)
			{
				sscanf(argv[2], "%d", &nThreads);
			}
			else if (strcmp(argv[1], "-Optimizer") == 0)
			{
				sscanf(argv[2], "%d", &Gurobi);
			}
			else if (strcmp(argv[1], "-Pruebas") == 0)
			{
				sscanf(argv[2], "%d", &Pruebas);
			}
			else if (strcmp(argv[1], "-Tipo") == 0)
			{
				sscanf(argv[2], "%d", &Tipo);
			}
			argc -= 2;
			argv += 2;
		}
	}
	parameters param;
	if (argc == 7)
	{
		//	//paremters file
		param.param_file = argv[1];
		param.input_items = argv[2];
		param.input_trucks = argv[3];
		param.output_items = argv[4];
		param.output_stack = argv[5];
		param.output_trucks = argv[6];
		

	}
	else
	{
		param.param_file = file_to_read + "input_parameters.csv";
		param.input_items = file_to_read + "input_items.csv";
		param.input_trucks = file_to_read + "input_trucks.csv";
		param.output_stack = file_to_write+ "output_stacks" + to_string(Tipo) + ".csv";
		param.output_items = file_to_write + "output_items" + to_string(Tipo) + ".csv";
		param.output_trucks = file_to_write + "output_trucks" + to_string(Tipo) + ".csv";
		param.report = file_to_write + "report"+ to_string(Tipo) +".csv";
		string input = "instances_";
		input += "_" + to_string(Tipo) + ".csv";
		ofstream f(input);
		f << "input items pathFilename;input trucks pathFilename;input parameters pathFilename;output items pathFilename;output stacks pathFilename;output trucks pathFilename;report pathFilename" << endl;
		//		f << input << "input_items.csv;" << input << "input_trucks.csv;" << input << "input_parameters.csv;" << input << "output_items.csv;" << input << "output_stacks.csv;" << input << "output_trucks.csv;" << input << "report.csv" << endl;
		f << param.input_items << ";" << param.input_trucks << "; " << param.param_file << "; " << param.output_items << "; " << param.output_stack << "; " << param.output_trucks << "; " << param.report << "; " << endl;

		f.close();
	}
	
	


    list<item*> lista_item;
    list<truck*> lista_camiones;
	map <string, truck*> mapa_camiones;
    map<string, int> proveedores;
	map<int, list<string>> componentes;
	map <string, list<item*>> proveedor_items;
	
	
	//me guardo el tiempo inicial
	param.tini = time_initial;

	if (Pruebas)
		cout << "Ha pasado los parametros\n" << endl;
	
	
	//leo los datos y los guardo en las estructuras
	
	leer_datos( param, lista_item, lista_camiones, mapa_camiones, proveedores, proveedor_items);

	if (Pruebas)
		cout << "Ha leido el problema\n" << endl;

	/********* OJO CAMBIAR TIEMPO **************/
	int TMAX = param.time_limit; //SUSTITUIR por param.time
	srand(Tipo);  // generamos una semilla


	param.time_limit =  2*param.time_limit;

	if (Tipo==117) {
		TMAX=4*TMAX;
		param.time_limit=4*param.time_limit;}
	if (Tipo==217) {
		TMAX=8*TMAX;
		param.time_limit=8*param.time_limit;}

	//Calculo las componentes conexas
	Calcula_componentes_conexas(proveedores, lista_camiones,componentes);
	Problema Prob1 = Problema(componentes, proveedor_items, time_initial, TMAX, Pruebas);

	if (Pruebas)
		cout << "Ha leido y ha calculado componentes conexas\n" << endl;

	
	//////Filtro
	//for (auto itp = Prob1.subproblemas.begin(); itp != Prob1.subproblemas.end(); itp++)
	//{
	//	(*itp)->FiltroItems(3);
	//}
	
	Problema Prob(time_initial, TMAX, Pruebas);

	Prob.Tipo = Tipo;
	if (Tipo >= 1001 && Tipo <= 1020) //Para los de varias semillas
		Prob.Tipo = 17;
	if (Prob.Tipo==117 || Prob.Tipo==217) Prob.Tipo=17;
	if (Prob.Tipo==23) Prob.Tipo=13;
	//He puesto 11 para doble tiempo y relax en todos Todos_Relax=True
	if (Prob.Tipo == 11)
	{
		Prob.Todos_Relax = true;
	}
	//He puesto 12 para doble tiempo y por camion Por_Camion=true
	if (Prob.Tipo == 12)
	{
		Prob.Por_Camion = true;
	}
	//He puesto 13 para doble tiempo y el normal
	//He puesto 14 para doble tiempo que sea relax y con todas las variables enteras Todos_Relax=True Todas_Enteras=true
	if (Prob.Tipo == 14)
	{
		Prob.Todos_Relax = true;	
		Prob.Todos_Enteras = true;
	}
	//He puesto 15 para doble tiempo  poner Last_Ramon=True
	if (Prob.Tipo== 15 || Prob.Tipo==16)
	{
		Prob.Last_Ramon = true;
		if (Tipo==16)
			Prob.Fijar= false;
	}


	Calcula_componente_conex3(Prob1,lista_camiones,lista_item,Prob,mapa_camiones);
	if (Pruebas)
		cout << "Hay " << Prob.subproblemas.size() << " subproblemas" << endl;
	int kcont1 = 0;
	for (auto it : Prob.subproblemas)
	{
		
		it->PonerParams(param);
		it->Tipo= Tipo;
		it->CodS = kcont1;
		cout << "Componente:" << kcont1 << " TA " << it->Total_Posibles_Asignaciones << endl;
		kcont1++;
	}
	////dEJARLO COMO ESTÁ.
	//for (auto ic = lista_camiones.begin(); ic != lista_camiones.end(); ic++)
	//{
	//	(*ic)->items.clear();
	//}


	bool cambios = true;
	while (cambios)
	{
		cambios = false;
		for (auto its = Prob.subproblemas.begin(); cambios != true && its != Prob.subproblemas.end()  ; its++)
		{
//			if (Pruebas) cout << (*its)->CodS << " " << (*its)->Total_Posibles_Asignaciones << endl;
	//sort(Prob.subproblemas.begin(), Prob.subproblemas.end(), Subproblema::asignaciones());
			int total_dias_cambios = 0;
			for (auto itc = (*its)->lista_camiones.begin(); itc != (*its)->lista_camiones.end(); itc++)
			{
				auto itc2 = itc;
				itc2++;
				if (itc2 != (*its)->lista_camiones.end())
				{

					int dif_day = Days_0001((*itc)->arrival_time_tm_ptr.tm_year, (*itc)->arrival_time_tm_ptr.tm_mon + 1, (*itc)->arrival_time_tm_ptr.tm_mday)
						- Days_0001((*itc2)->arrival_time_tm_ptr.tm_year, (*itc2)->arrival_time_tm_ptr.tm_mon + 1, (*itc2)->arrival_time_tm_ptr.tm_mday);
					if (dif_day > 0)
						total_dias_cambios++;
				}

			}
			total_dias_cambios++;

			(*its)->total_dias_cambios = total_dias_cambios;
			
			if (((*its)->Total_Posibles_Asignaciones > 500000 || total_dias_cambios>20) && (*its)->NoSePuedeDividir==false)
			{

//				cout << "PAsa" << endl;
//				if (Pruebas) cout << (*its)->CodS << " " << (*its)->Total_Posibles_Asignaciones << endl;
//				cout << "PAsa2"<< endl;
				bool Divide=Prob.divide_subproblema((* its));


				if (Divide == true)
				{
					cambios = true;
					break;
				}
			}
		}
		for (auto its = Prob.subproblemas.begin();  its != Prob.subproblemas.end() && cambios==true; its++)
		{
			if ((* its)->CodS == 99999)
			{
				its=Prob.subproblemas.erase(its);
				break;
			}
			
		}
	}
	Prob.TiempoModelo = (TMAX - Prob.subproblemas.size()) / 12;
	sort(Prob.subproblemas.begin(), Prob.subproblemas.end(), Subproblema::asignaciones());
	//MAITE
	int cont2 = 0;
	for (auto itsub = Prob.subproblemas.begin(); itsub != Prob.subproblemas.end(); itsub++)
	{
		for (auto itsit = (*itsub)->lista_items.begin(); itsit != (*itsub)->lista_items.end(); itsit++)
		{
/*			if (strcmp((*itsit)->Id, "0090016800_08062023050730") == 0)
			{
				int kk = 0;
			}
*/		}
		cont2++;
	}
	//MAITE



	if (Pruebas)
		cout << "Hay " << Prob.subproblemas.size() << " subproblemas" << endl;
	Prob.total_items_cuadrado = 0;
	Prob.total_items_camiones = 0;
	for (auto its : Prob.subproblemas)
	{
		Prob.total_items_cuadrado += (int)pow(its->total_items, 2) / 100;
		Prob.total_items_camiones +=its->Total_Posibles_Asignaciones;
		
		for (auto it : its->lista_items)
		{
			it->camiones.sort(truck::comparador);
			it->camiones_originales.clear();
			std::copy(it->camiones.begin(), it->camiones.end(), std::back_inserter(it->camiones_originales));
		}
		
	}
	int kcont = 0;
	int max_camiones = 0, max_items = 0, max_dias = 0;
	int total_camiones = 0, total_items = 0,total_dias=0;
	int  total_extra = 0, max_extra = 0;
	int total_inventory = 0, max_inventory = 0;
	std::vector <int> vdias;
	std::vector <int> vcamiones;
	std::vector <int> vitems;
	for (auto it : Prob.subproblemas)
	{
		it->PonerParams(param);
		cout << "Componente:" << kcont << " TA " << it->Total_Posibles_Asignaciones << endl;
		int total_dias_cambios = 0;
		for (auto iti2 : it->lista_items)
		{
			total_inventory += iti2->inventory_cost;
			if (iti2->inventory_cost > max_inventory)
				max_inventory = iti2->inventory_cost;

		}
		for (auto itc = it->lista_camiones.begin(); itc != it->lista_camiones.end(); itc++)
		{
			total_extra+=(*itc)->cota_extra;
			if ((*itc)->cota_extra > max_extra) 
				max_extra = (*itc)->cota_extra;
			auto itc2 = itc;
			itc2++;
			if (itc2 != it->lista_camiones.end())
			{

				int dif_day = Days_0001((*itc)->arrival_time_tm_ptr.tm_year, (*itc)->arrival_time_tm_ptr.tm_mon + 1, (*itc)->arrival_time_tm_ptr.tm_mday)
					- Days_0001((*itc2)->arrival_time_tm_ptr.tm_year, (*itc2)->arrival_time_tm_ptr.tm_mon + 1, (*itc2)->arrival_time_tm_ptr.tm_mday);
				if (dif_day > 0)
					total_dias_cambios++;
			}

		}
		
		total_dias_cambios++;
		total_dias += total_dias_cambios;
		total_camiones += it->lista_camiones.size();
		
		total_items += it->lista_items.size();
		vdias.push_back(total_dias_cambios);
		vcamiones.push_back(it->lista_camiones.size());
		vitems.push_back(it->lista_items.size());
		if (it->lista_camiones.size() > max_camiones)
			max_camiones = it->lista_camiones.size();
		if (it->lista_items.size() > max_items)
			max_items = it->lista_items.size();
		if (total_dias_cambios > max_dias)
			max_dias = total_dias_cambios;
		
/*		for (auto it2 : it->lista_items)
			if (strcmp((it2)->Id, "0090016900_14062023044992") == 0)
				int kk = 9;*/
//		for (auto it2 : it->lista_camiones)
//			if (strcmp((it2)->id_truck, "P193706601") == 0)
//				int kk = 9;
		kcont++;
	}
//	cout << "compo:" << Prob.subproblemas.size() << " cam:" << Problista_camiones.size() << " items:" << (*it)->total_items << " plani:" << (*it)->planificados << " extras:" << (*it)->extras << " CT:" << (*it)->coste_transporte << " CI:" << (*it)->coste_inventario << " CT:" << (*it)->coste_total << endl;
	cout << file_to_read << " NComponentes "<< Prob.subproblemas.size()<< " Total  camiones: " << total_camiones << " Total items : " << total_items << " Total dias : " << total_dias << " Max camiones : " << max_camiones  << " Max items : " << max_items << " Max dias : " << max_dias  
		<< " Cuantil Camiones :" << calcularMediana(vcamiones, 98) << " Cuantil items:" << calcularMediana(vitems, 98) << " Cuantil dias : " << calcularMediana(vdias, 98)
		<< " Cuantil Camiones :" << calcularMediana(vcamiones, 90) << " Cuantil items:" << calcularMediana(vitems, 90) << " Cuantil dias : " << calcularMediana(vdias, 90)
		<< " Mediana Camiones :" << calcularMediana(vcamiones,50) << " Mediana items:" << calcularMediana(vitems,50) << " Mediana dias : " << calcularMediana(vdias,50)
		<< " Media camiones : " << total_camiones / Prob.subproblemas.size() << " Media items : " << total_items / Prob.subproblemas.size() << " Media dias : " << total_dias / Prob.subproblemas.size() 
		<< " TE " << total_extra << " ME " << max_extra << " MEdia E "<< total_extra/ total_camiones 
		<< " TI " << total_inventory << " MI " << max_inventory << " MEdia I " << total_inventory / total_items << 
		"\t" << param.inventory_cost << "\t"<< param.transportation_cost << "\t"<< param.extra_truck_cost << endl;
	//return 2;


	

	int iteraciones_cuentas = 0;
	double ttotal = 0;
	int iter_totales = 0;
	int solu_anterior=999999999;
	//Por si ya ha llegado al óptimo
	while (ttotal<(Prob.TiempoTotal-20)  && iter_totales<MAX_ITER_GLOBAL)
	{
		
		//Escojo al azar el % de camiones a llenar a partir de la segund aiter
		/*if (iter >= 1) 
			param.porcentaje = (get_random(20, 70)/(double)100);
		else*/ param.porcentaje = 1;
		int salida = 0;
		Prob.Algoritmo_Paralelo = false;
		if (Prob.Algoritmo_Paralelo==false)
			salida=Prob.Resolver(iter_totales, file_to_read, param);
		else
			salida = Prob.Resolver_Paralelo(iter_totales, file_to_read, param);
		ftime(&time_final);
		ttotal = ((double)((time_final.time - time_initial.time) * 1000 + time_final.millitm - time_initial.millitm)) / 1000;
		if (salida == 2)
			break;


		

		

		ftime(&time_final);
		ttotal = ((double)((time_final.time - time_initial.time) * 1000 + 	time_final.millitm - time_initial.millitm)) / 1000;
		if (Pruebas) cout << endl << setprecision(8) << "Tiempo " << ttotal << "Iter " << iteraciones_cuentas << endl;
		iteraciones_cuentas++;
		iter_totales++;
		//Recooro las componentes y me quedo con las que mejore.
		Prob.mejor_solucion.coste_total = param.transportation_cost * Prob.mejor_solucion.coste_transporte + param.inventory_cost * Prob.mejor_solucion.coste_inventario;

		if (Prob.mejor_solucion.coste_total<solu_anterior)
		{
			if (Pruebas) cout<< endl<< "escribe la solucion" << endl;
			Prob.escribeG(param, Prob.mejor_solucion);
#ifdef _WIN32
			Prob.Chequear(param, file_to_read, Tipo);
#endif
			int kk = 9;
		}
//		tvuelta += 600;
		solu_anterior=Prob.mejor_solucion.coste_total;

		Prob.reset();

	}
	int cc2 = 0;
	for (auto it = Prob.subproblemas.begin(); it != Prob.subproblemas.end()&&(*it)->Pruebas; it++)
	{
		if (Pruebas)
			Prob.ActualizarEstadisticas((*it));
		//cout << "compo:" << cc << " cam:" << (*it)->lista_camiones.size() << " items:" << (*it)->total_items << " plani:"<<(*it)->planificados <<" extras:"<<(*it)->extras<<" CT:"<< (*it)->coste_transporte <<" CI:"<<(*it)->coste_inventario <<" CT:"<<(*it)->coste_total << endl;
		cc2++;

	}
	
//	escribeG(param,Prob.mejor_solucion);
	Prob.escribeG(param, Prob.mejor_solucion);
	if (Pruebas)
	{
		ftime(&time_final);
		double ttotal2 = ((double)((time_final.time - time_initial.time) * 1000 + time_final.millitm - time_initial.millitm)) / 1000;
		if (Pruebas) cout << endl << setprecision(8) << "Tiempo " << ttotal << "Iter " << iter_totales << endl;

		string nombreArchivo1 = "Resultados.txt";
		//nombreArchivo1 += file_to_read;
		//	string nombreArchivo1 = "Resultados.txt";
		ofstream archivo;
		archivo.open(nombreArchivo1.c_str(), ofstream::out | ofstream::app);
		archivo << file_to_read << ";";
		archivo << setprecision(8) << Prob.mejor_solucion.coste_transporte << ";" << Prob.mejor_solucion.coste_inventario << ";" << Prob.mejor_solucion.coste_total << ";" << iter_totales<< ";" << ttotal << ";" << ttotal2 << ";" << Prob.total_infeasibles << ";"<< Prob.best_iteracion << ";" <<Prob.total_volumen<<";"<< Prob.total_volumen_camiones << ";" << Prob.total_peso << ";" << Prob.total_peso_camiones << endl;
		archivo.close();
		
		////Chequear
		for (auto& its : Prob.subproblemas)
		{
			if (its->escribe_componente)	
				Prob.ChequearComponente(param, file_to_read,  Prob.mejor_solucion,its->CodS,Tipo);
		}
//		Prob.ChequearComponente(param, file_to_read);
		Prob.Chequear(param,file_to_read,Tipo);
		cout <<endl<< file_to_read << ";\t";
		cout << setprecision(8) << Prob.mejor_solucion.coste_transporte << ";" << Prob.mejor_solucion.coste_inventario << ";" << Prob.mejor_solucion.coste_total << ";" << iter_totales << ";" << ttotal << ";" << ttotal2 << ";" << Prob.total_infeasibles << ";" << Prob.best_iteracion << ";"  << param.transportation_cost << ";" << param.inventory_cost << ";" << param.extra_truck_cost << ";";
		Prob.PrintEstadisticas();
		cout << endl;
	

	}
	return 1;
 }

 void comprobar_componente(list<truck*> lc)
 {
	 for (auto itc = lc.begin(); itc != lc.end(); itc++)
	 {
		 if ((*itc)->lcompo.size() > 1)
		 {
			 PintarProblema(555);
		 }
	 }
 }
 void estadistcias_itesms(list<item*>lista_item,string file)
 {
	 int ncam = 0;
	 int ndias = 0;
	 int ult = 0;
	 int prim = 0;
	 int min = 0;
	 int max = 0;
	 list<truck*>::iterator ic;
	 for (auto it = lista_item.begin(); it != lista_item.end(); it++)
	 {
		 ncam += (int) (*it)->camiones.size();
		 //(*ic)->arrival_time_tm_ptr.tm_year, (*ic)->arrival_time_tm_ptr.tm_mon + 1, (*ic)->arrival_time_tm_ptr.tm_mday);
		 ic = (*it)->camiones.begin();
		 ult = Days_0001((*ic)->arrival_time_tm_ptr.tm_year, (*ic)->arrival_time_tm_ptr.tm_mon + 1, (*ic)->arrival_time_tm_ptr.tm_mday);
		 ic = (*it)->camiones.end();
		 ic--;
		 prim = Days_0001((*ic)->arrival_time_tm_ptr.tm_year, (*ic)->arrival_time_tm_ptr.tm_mon + 1, (*ic)->arrival_time_tm_ptr.tm_mday);
		 
		 int dif = ult - prim;
		 //if (dif < 0)
		 //{
			// int kk = 0;
		 //}
		 if (dif < min)min = dif;
		 if (dif > max) max = dif;
		 ndias += dif;
	 }
	 int mediac = ncam / lista_item.size();
	 int meddias = ndias / lista_item.size();
	 ofstream archivo;
	 archivo.open("dias.txt", ofstream::out | ofstream::app);
	 archivo << file << ";";
	 archivo << mediac << ";" << min << ";" << meddias << ";" << max << ";" << endl;
	 archivo.close();
	 exit(0);
 }
 double calcularMediana(std::vector<int>& vec,int val) {
	 // Ordenar el vector
	 std::sort(vec.begin(), vec.end());

	 int n = vec.size();
	 if (val == 50)
	 {
		 if (n % 2 == 0) {
			 // Si el número de elementos es par, devuelve el promedio de los dos del medio
			 return (vec[n / 2 - 1] + vec[n / 2]) / 2.0;
		 }
		 else {
			 // Si el número de elementos es impar, devuelve el elemento del medio
			 return vec[n / 2];
		 }
	 }
	 else
	 {
		 int pos = val * n / 100;
		 return vec[pos];
	 }
		 
 }
/*
* string nombreArchivo1 = "Resultados.txt";

	ofstream archivo;
	// Abrimos el archivo
	
	archivo.open(nombreArchivo1.c_str(), ofstream::out | ofstream::app);
//	fprintf(archivo, "%s %d %.4f %.4f %d %d %d %d\n", file_to_read ,Prob.subproblemas.size(), Prob.ObjMip,Prob.ObjBound , Prob.num_camiones_modelo , Prob.num_camiones_extra_modelo , Prob.max_cam_extra_tipo_modelo, Gurobi );
	archivo << file_to_read << ";" << Prob.subproblemas.size() << ";";
	archivo << setprecision(8) << Prob.ObjMip << ";" << Prob.ObjBound << ";" << Prob.num_camiones_modelo << ";" << Prob.num_camiones_extra_modelo << ";" << Prob.max_cam_extra_tipo_modelo << ";" << Gurobi<<";" <<tmp_first2 << endl;
	archivo.close();
	//Escribo para cada subproblema los datos.
	escribe_info_componentes(Prob,argv[1],(int)lista_camiones.size(), (int)lista_item.size(), (int)proveedores.size());
	
	//escribe_item(argv[1], mapa_camiones, param, lista_item, lista_camiones);
	//Para llamar al modelo
	//Roadef2022ExtraCplex(argv[1], lista_camiones, lista_item, param);
	// 
*/