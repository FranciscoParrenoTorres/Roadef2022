#include "kosaraju.h"


//bool cmp(pair<string, int>& a,pair<string, int>& b)
//{
//	return a.second < b.second;
//}
void Calcula_componentes_conexas(map<string, int> proveedores, list<truck*> lista_camiones, map<int, list<string>>& componentes)
{
	Graph g((int)proveedores.size());
	map<int, string> int_provee = reverseMap(proveedores);
	
	for (list<truck*>::iterator itt = lista_camiones.begin(); itt != lista_camiones.end(); itt++)
	{
		
		
		//(*itt)->int_order_ruta = reverseMap((*itt)->orden_ruta);
		multimap<int, string> intorde= invertMap((*itt)->orden_ruta);

		
	//	map<int,string>::iterator itr =	(*itt)->int_order_ruta.begin();
		multimap<int, string>::iterator itr = intorde.begin();
		
		int pos1 = proveedores[(*itr).second];
		int ini = pos1;
		itr++;
		//for (itr; itr != (*itt)->int_order_ruta.end(); itr++)
		for (itr; itr != intorde.end(); itr++)
		{
			int pos2 = proveedores[(*itr).second];
			g.addEdge(pos1, pos2);
			pos1 = pos2;
		}
		g.addEdge(pos1, ini);
	}
	g.SCC(int_provee,componentes);
}
void Calcula_componente_conex3(Problema P1, list<truck*> lista_camiones, list<item*>lista_item, Problema& Prob, map <string, truck*> mapa_camiones)
{
	map<int, list<string>> componentes2;
	map<int, string> int_cam;
	map<string, int> cam_int;
	map<string, list<item*>> cam_item;

	for (auto its = P1.subproblemas.begin(); its != P1.subproblemas.end(); its++)
	{
		componentes2.clear();
		cam_int.clear();
		
		for (auto itc = (*its)->lista_camiones.begin(); itc != (*its)->lista_camiones.end(); itc++)
		{
			cam_int.insert(pair<string, int>((*itc)->id_truck, (*itc)->CodIdCC));
			
		}
		int_cam.clear();
		int_cam = reverseMap(cam_int);
		
		Graph g((int)(*its)->lista_camiones.size());
		int cont_cam = 0;
		for (auto itc = (*its)->lista_camiones.begin(); itc != (*its)->lista_camiones.end(); itc++)
		{			
			int ini = (*itc)->CodIdCC;
			int pos1 = ini;
			for (auto iti = (*itc)->items.begin(); iti != (*itc)->items.end(); iti++)
			{

				if (cam_item.count((*itc)->id_truck) == 0)
				{
					(*itc)->CodIdCC = cont_cam;
					list<item*> auxit;
					auxit.push_back((*iti));
					cam_item.insert(pair<string, list<item*>>((*itc)->id_truck, auxit));
					cont_cam++;
				}
				else
				{
					cam_item[(*itc)->id_truck].push_back((*iti));
				}
				for (auto itc2 = (*iti)->camiones.begin(); itc2 != (*iti)->camiones.end(); itc2++)
				{
					if ((*itc2)->CodIdCC <= ini) continue;
					else
					{
						int pos2 = (*itc2)->CodIdCC;
						g.addEdge(pos1, pos2);
						pos1 = pos2;
					}
				}
				g.addEdge(pos1, ini);
			}
		}
		g.SCC(int_cam, componentes2);
		Prob.Add_componente(componentes2, mapa_camiones,cam_item);
	}
}
void Calcula_componentes_conexas2(map<string, int> proveedores, list<truck*> lista_camiones, map<int, list<string>>& componentes, list<item*>lista_item, map <string, list<item*>>& proveedor_items)
{
	//para cada componente
	map<int, list<string>>::iterator itc = componentes.begin();
	map<int, list<int>> item_cam;
	map<int, string> int_cam;
	map<string,int> cam_int;
	map<string,list<item*>> cam_item;
	map<int, list<string>> componentes2;
	int contc = 0;
	//Para cada componente conexa 
	for (itc; itc != componentes.end(); itc++)
	{
		contc++;
		list<string>::iterator itp = (*itc).second.begin();


		int cont_cam = 0;
		list<item*> list_items;
		list<truck*> list_trucks;
		item_cam.clear();
		int_cam.clear();
		list_items.clear();
		list_trucks.clear();
		cam_item.clear();
		cam_int.clear();
		componentes2.clear();

		for (itp; itp != (*itc).second.end(); itp++)
		{
			list<item*>::iterator iti = proveedor_items[*itp].begin();
			//lista de items
			for (iti; iti != proveedor_items[*itp].end(); iti++)
			{
				list_items.push_back((*iti));
				if ((*iti)->camiones.size() > 1)
				{
					//int kk = 0;
				}
				//lista de camiones				
				for (list<truck*>::iterator itcam = (*iti)->camiones.begin(); itcam != (*iti)->camiones.end(); itcam++)
				{
					
					if (cam_item.count((*itcam)->id_truck) == 0)
					{
						(*itcam)->CodIdCC = cont_cam;
						list<item*> auxit;
						auxit.push_back((*iti));
						cam_item.insert(pair<string, list<item*>>((*itcam)->id_truck, auxit));
						cam_int.insert(pair<string,int>((*itcam)->id_truck,(*itcam)->CodIdCC));
						list_trucks.push_back((*itcam));
						cont_cam++;
					}
					else
					{
						cam_item[(*itcam)->id_truck].push_back((*iti));
					}
					if (item_cam.count((*iti)->CodId) == 0)
					{
						list<int> aux;
						aux.push_back((*itcam)->CodIdCC);
						item_cam.insert(pair<int, list<int>>((*iti)->CodId, aux));
					}
					else
						item_cam[(*iti)->CodId].push_back((*itcam)->CodIdCC);
				}
			}
		}

		int_cam.clear();
		int_cam = reverseMap(cam_int);
		Graph g((int)int_cam.size());
		for (auto iti = item_cam.begin(); iti != item_cam.end(); iti++)
		{
			list<int>::iterator itcam = (*iti).second.begin();
			int ini = (*itcam);
			int pos1 = (*itcam);
			int pos2;
			while( itcam != (*iti).second.end())
			{				
				itcam++;
				if (itcam != (*iti).second.end())
				{
					pos2= (*itcam);
					g.addEdge(pos1, pos2);
					pos1 = pos2;
				}
				else
				{
					//uno el ultimo con el primero.
					g.addEdge(ini, pos1);
				}
			}
		}
		g.SCC(int_cam, componentes2);
		//Prob.Add_componente(componentes2,cam_item);
	}
}