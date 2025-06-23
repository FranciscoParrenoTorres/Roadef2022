#include "encabezados.h"
//#pragma once
#ifndef Problema_H
#define Problema_H

class Problema
{
	
public:
	vector<Subproblema*> subproblemas;
	double ObjMip=0; //Valor solución posible modelo
	double ObjBound=0; //Valor cota inferior modelo
	int TiempoModelo;//Tiempo para el modelo
	int TiempoTotal;
	int num_camiones_modelo=0;
	int num_camiones_extra_modelo=0;
	int max_cam_extra_tipo_modelo=0;
	double total_volumen;
	double total_peso;
	double total_volumen_camiones;
	double total_peso_camiones;

		
	int total_items_cuadrado=0; //Total del número items de cada compenente al cuadrado
	int total_items_camiones=0; //Total del número items *camiones de cada compontente
	int coste_transporte=0;
	int coste_inventario=0;
	int best_iteracion=0;
	int coste_total=0;
	int total_infeasibles = 0;
	bool Algoritmo_Paralelo = false;
	mejor_sol mejor_solucion;
	timeb TiempoInicio;
	timeb TiempoFin;
	timeb TiempoActual;
	int veces_pila_cambia=0;
	int veces_pila=0;
	//Veces que se ha cargado un camion
	vector<pair<int, int>> veces_cargocamion;
	//Peso y volumen de los camiones cargados
	vector<vector<double>> cargocamion_char;
	int Total_Iter_Packing=50; //Total de iteraciones del modelo de packing
	Problema();
	bool Pruebas=true;//True para probar, false para competición
	Problema(map<int, list<string>> componentes, map <string, list<item*>>& proveedor_items,timeb tini, int  tmax, bool prueba);
//	int Resolver_Paralelo(int& iter, string file_to_read, parameters param, int Gurobi);
	int Resolver_Paralelo(int& iter, string file_to_read, parameters param);
	Problema(timeb tini, int tmax, bool prueba);
	void Add_componente(map<int, list<string>> componentes, map<string, truck*> mapa_camiones, map<string, list<item*>> cam_item);
	void Add_componente2(map<int, list<string>> componentes, map<string, truck*> mapa_camiones, map<string, list<item*>> cam_item);
	~Problema();
	void PrintEstadisticas();
	void ActualizarEstadisticas(Subproblema* sub);
	void reset();
//	int Resolver(int &iter, string file_to_read,parameters param, int Gurobi);
	bool Todos_Relax = false;
	bool Last_Ramon= false;
	bool Todos_Enteras= false;
	bool Por_Camion = false;
	bool Fijar= true;
	int Tipo = 13; //Tipo de algoritmo
	int Resolver(int& iter, string file_to_read, parameters param);
	void escribeG(parameters& params, mejor_sol mej);
	void escribe_cam(Subproblema* su, int cont);
	void Insertar_camiones_ordenado_por_fechas(Subproblema* sub, truck* itcam);	
	void pasar_solucion(Subproblema* it, parameters param);
	void MejorSolucion(parameters param);
	void actualizo_mejor_solucion_paralelo(Subproblema* it, int compo);
	void actualizo_mejor_solucion(Subproblema* it, int compo);
	bool comprobar_solucion();
	bool BorrarItemDelCamion(truck* camion, item* item);
	void escribe(string file_to_write,int tipo)
	{
		ofstream f(file_to_write+"output_trucks"+to_string(tipo) + ".csv");
		f << "Id truck; Loaded length; Weight of loaded items; Volumevof loaded items; emm; emr" << endl;
		f.close();
		ofstream f2(file_to_write + "output_stacks" + to_string(tipo) + ".csv");
		f2 << "Id truck; Id stack; Stack code; X origin; Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;
		f2.close();
		ofstream f3(file_to_write + "output_items" + to_string(tipo) + ".csv");
		f3 << "Item ident;Id truck;Id stack;Item code;X origin;Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;
		f3.close();
		
		for (int i = 0 ; i < subproblemas.size(); i++)
		{
			subproblemas[i]->escribe(file_to_write);
		}
	};
	void ChequearComponente(parameters param, string file_to_read,  mejor_sol mej,int idS,int tipo)
	{
		string output_trucks = "./KK/output_trucks"+to_string(tipo)+".csv";
		string output_stacks = "./KK/output_stacks"+ to_string(tipo) +".csv";
		string output_items = "./KK/output_items"+ to_string(tipo) +".csv";
		string output_report = "./KK/report"+ to_string(tipo) +".csv";
		ofstream ft_c(output_trucks, ios_base::trunc);
		ft_c << "Id truck; Loaded length; Weight of loaded items; Volumevof loaded items; emm; emr" << endl;

		ofstream fs_c(output_stacks, ios_base::trunc);
		fs_c << "Id truck; Id stack; Stack code; X origin; Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;

		ofstream fi_c(output_items, ios_base::trunc);
		fi_c << "Item ident;Id truck;Id stack;Item code;X origin;Y origin; Z origin; X extremity; Y extremity; Z extremity" << endl;


		for (auto& its : subproblemas)
		{
			if (its->CodS == idS)
			{
				//Cambiado
				its->EscribeComponente(param);

				for (auto it = mej.solucion_compo.at(its->CodS).listado_items.begin(); it != mej.solucion_compo.at(its->CodS).listado_items.end(); it++)
				{
					(*it).escribe(fi_c);

				}


				for (auto it = mej.solucion_compo.at(its->CodS).listado_camiones.rbegin(); it != mej.solucion_compo.at(its->CodS).listado_camiones.rend(); it++)
				{
					//			if (camiones_en_sol.count((*it).id_truc) == 0)
					//			{
					//				camiones_en_sol.insert(pair<string, bool>((*it).id_truc, true));
					(*it).escribe(ft_c);
					//			}
					//			else
					//				int kk = 9;

				}


				for (auto it = mej.solucion_compo.at(its->CodS).listado_pilas.begin(); it != mej.solucion_compo.at(its->CodS).listado_pilas.end(); it++)
					(*it).escribe(fs_c);
			}
		}
		ft_c.close();
		fi_c.close();
		fs_c.close();
		string input = "instances_";


		input += "KK.csv";
		ofstream f(input);
		f << "input items pathFilename;input trucks pathFilename;input parameters pathFilename;output items pathFilename;output stacks pathFilename;output trucks pathFilename;report pathFilename" << endl;

		f << "./KK/input_items.csv;./KK/input_trucks.csv;./KK/input_parameters.csv;" + output_items + ";" + output_stacks + ";" + output_trucks + ";" + output_report  << endl;
	//		f << input << "input_items.csv;" << input << "input_trucks.csv;" << input << "input_parameters.csv;" << input << "output_items.csv;" << input << "output_stacks.csv;" << input << "output_trucks.csv;" << input << "report.csv" << endl;
//		f << param.input_items << ";" << param.input_trucks << "; " << param.param_file << "; " << param.output_items << "; " << param.output_stack << "; " << param.output_trucks << "; " << param.report << "; " << endl;

		f.close();

#ifdef _WIN32
		string instruccion = "java -DLOG_FILENAME=\"logs/logchecker\" -Dlogback.configurationFile=logback.xml -jar ./CheckerChallenge-1.10.0.jar ";
		instruccion += input;
#else
		string instruccion = "java -Xmx4g -DLOG_FILENAME=\"logs/logchecker\" -Dlogback.configurationFile=logback.xml -jar ./CheckerChallenge-1.10.0.jar ";
		instruccion += input;
#endif		
		printf("\n%s", instruccion.c_str());
#ifdef _WIN32		
		system(instruccion.c_str());
#endif	
		//Ahora muestra por pantalla lo que ha puesto en ese archivo
		// Abre el archivo en modo lectura 

		ifstream archivo(output_report);
		//		ofstream archivo2("resultados.csv", ios_base::app);
				// Verifica que el archivo se haya abierto correctamente

		if (!archivo) {
			cerr << "Error al abrir el archivo report.csv" << endl;
			return;
		}


		// Declara una variable para almacenar cada línea del archivo

		string linea;
		getline(archivo, linea);
		getline(archivo, linea);
		cout << linea << endl;
		char delimitador = ';';
		string ic;
		stringstream stream(linea);

		// Muestra la línea en la pantalla 
		getline(stream, ic, delimitador);
		if (strcmp(ic.c_str(), "transportation cost") == 0)
		{
			getline(archivo, linea);
			cout << linea << endl;
			//			archivo2 << param.report <<";"<< linea << endl;
		}

		archivo.close();
		//		archivo2.close();



	};
	void Chequear(parameters param,string file_to_read,int tipo)
	{
		string input = "instances_";


#ifdef _WIN32
		size_t ultima_pos = file_to_read.find_last_of("//");
		size_t penultima_pos = file_to_read.find_last_of("//", ultima_pos - 1);
		std::string subcadena1 = file_to_read.substr(penultima_pos-1,1);
		input += subcadena1;
		input += "_";
		std::string subcadena = file_to_read.substr(penultima_pos + 1, ultima_pos - penultima_pos - 1);
		input += subcadena;
#else
		size_t ultima_pos = file_to_read.find_last_of("//");
		size_t penultima_pos = file_to_read.find_last_of("//", ultima_pos - 1);
		std::string subcadena1 = file_to_read.substr(penultima_pos - 1, 1);
		input += subcadena1;
		input += "_";
		std::string subcadena = file_to_read.substr(penultima_pos + 1, ultima_pos - penultima_pos - 1);
		input += subcadena;
#endif		
		input += "_"+to_string(tipo) + ".csv";
		ofstream f(input);
		f << "input items pathFilename;input trucks pathFilename;input parameters pathFilename;output items pathFilename;output stacks pathFilename;output trucks pathFilename;report pathFilename" << endl;
//		f << input << "input_items.csv;" << input << "input_trucks.csv;" << input << "input_parameters.csv;" << input << "output_items.csv;" << input << "output_stacks.csv;" << input << "output_trucks.csv;" << input << "report.csv" << endl;
		f << param.input_items<<";" << param.input_trucks<<"; " <<param.param_file << "; "<< param.output_items << "; " << param.output_stack  << "; " << param.output_trucks  << "; "<< param.report << "; " << endl;

		f.close();
		
#ifdef _WIN32
		string instruccion = "java -DLOG_FILENAME=\"logs/logchecker\" -Dlogback.configurationFile=logback.xml -jar ./CheckerChallenge-1.10.0.jar ";
		instruccion += input;
#else
		string instruccion = "java -Xmx4g -DLOG_FILENAME=\"logs/logchecker\" -Dlogback.configurationFile=logback.xml -jar ./CheckerChallenge-1.10.0.jar ";
		instruccion += input;
#endif		
		printf("\n%s", instruccion.c_str());
#ifdef _WIN32		
		system(instruccion.c_str());
#endif	
		//Ahora muestra por pantalla lo que ha puesto en ese archivo
		// Abre el archivo en modo lectura 

		ifstream archivo(param.report);
//		ofstream archivo2("resultados.csv", ios_base::app);
		// Verifica que el archivo se haya abierto correctamente

		if (!archivo) { 
			cerr << "Error al abrir el archivo report.csv" << endl; 
			return ; 
		}


		// Declara una variable para almacenar cada línea del archivo

		string linea;
		getline(archivo, linea);
		getline(archivo, linea);
		cout << linea << endl;
		char delimitador = ';';
		string ic;
		stringstream stream(linea);
			
		// Muestra la línea en la pantalla 
		getline(stream, ic, delimitador);
		if (strcmp(ic.c_str(), "transportation cost") == 0)
		{
			getline(archivo, linea);
//			archivo2 << param.report <<";"<< linea << endl;
		}
		
		archivo.close();
//		archivo2.close();

		
	
	};
	int DibujarSDL2();
	bool BorrarCamionDelItem(truck* camion, item* item);
	bool divide_subproblema(Subproblema* Sub);
};

#endif