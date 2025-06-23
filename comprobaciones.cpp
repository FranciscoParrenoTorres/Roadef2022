#include "encabezados.h"

bool cuadra_num_items(Subproblema* SU)
{
	map<string, int> cant_items;

	for (auto iti = SU->lista_items.begin(); iti != SU->lista_items.end(); iti++)
	{
		cant_items.insert(pair<string, int>((*iti)->Id, (*iti)->num_items));
		if ((*iti)->num_items < 0)
		{
			//int kk = 9;
			return false;
		}
	}
	for (auto it_c = SU->lista_camiones.begin(); it_c != SU->lista_camiones.end(); it_c++)
	{
		for (auto it_i = SU->mapa_camiones[(*it_c)->id_truck].begin(); it_i != SU->mapa_camiones[(*it_c)->id_truck].end(); it_i++)
		{
			if (cant_items.at((*it_i)->Id) == (*it_i)->num_items)
				continue;
			else
			{
				//int dif = cant_items.at((*it_i)->Id) - (*it_i)->num_items;
				//printf("En el camión, %s, el item %s, tiene %d items y en la lista de items %d, difieren en %d\n",(*it_c)->id_truck,(*it_i)->num_items, cant_items.at((*it_i)->Id),dif);
				return false;
			}
		}
	}
	return true;
}
