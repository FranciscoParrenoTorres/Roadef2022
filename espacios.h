#pragma once
#include "encabezados.h"

#ifndef ESPACIOS_H
#define ESPACIOS_H


class stack_sol;

class espacios
{
public:
	list <Tripleta<int>> lespacios;

	espacios()
	{
		lespacios.clear();
	}
	espacios(Tripleta<int> t)
	{
		lespacios.push_back(t);
	};
	~espacios()
	{
		lespacios.clear();
	};
	void add(Tripleta<int> t)
	{
		lespacios.push_back(t);
	};
	void add(int x, int y, int z)
	{
		lespacios.push_back(Tripleta<int>(x, y, z));
	};
	void QuitarPerfilesDuplicados()
	{
		for (auto itl = lespacios.begin(); itl != lespacios.end(); itl++)
		{
			auto itl2 = (*itl).perfiles.begin();
			while (itl2 != (*itl).perfiles.end())
			{
				auto itl3 = itl2;
				itl3++;
					while (itl3 != (*itl).perfiles.end())
					{
						if (((*itl3).x == (*itl2).x) &&
							((*itl3).y_ini == (*itl2).y_ini) &&
							((*itl3).y_fin == (*itl2).y_fin))
							itl3 = (*itl).perfiles.erase(itl3);
						else
							itl3++;
					}
					itl2++;

		
			}
		}
	}
	void clear()
	{
		lespacios.clear();
	};
	Tripleta<int>* Busca_sig_esp()
	{
		int min = 100000;
		Tripleta<int>* ptri=NULL;
		for (auto itl = lespacios.begin(); itl != lespacios.end(); itl++)
		{
			if (itl->x < min)
			{
				min = itl->x;
				ptri = &(*itl);
			}
		}
		return ptri;
	};
	int anchoy(Tripleta<int> esp)
	{
		return (esp.y_fin - esp.y_ini);
	};
	void InsertaryActualizar(Tripleta<int>& esp1)
	{
		for (auto itl = lespacios.begin(); itl != lespacios.end(); itl++)
		{
			if (itl->y_ini == esp1.y_fin)
			{
				if (esp1.x != itl->x)
				{
					itl = lespacios.insert(itl, esp1);
					return;
					//					actualizo_espacios_iguales((*itl));
				}
				else
				{
					itl->y_ini = esp1.y_ini;
				}
				break;
			}
			if (itl->y_fin == esp1.y_ini)
			{
				if (esp1.x != itl->x)
				{
					auto itl2 = itl;
					itl2++;
					if (itl2!= lespacios.end())
					itl = lespacios.insert(itl2, esp1);
					else
						lespacios.push_back(esp1);
					return;
					//					actualizo_espacios_iguales((*itl));
				}
				else
				{
					itl->y_fin= esp1.y_fin;
				}
				break;
			}
		}
		lespacios.push_back(esp1);
	};
	void Generaryactualizar_espacios(stack_sol pila, Tripleta<int>& esp, int dim_minima, bool EnMedio)
	{
	//	Tripleta<int> esp1(esp.y_ini, esp.y_fin, esp.x);
		//esp1 es una copia de esp
		 Tripleta<int> esp1(esp);
		 Tripleta<int> esp2(esp);
		//Si tengo más de un espacio en otro caso, puedo poner arriba o abajo
		if (lespacios.size() != 1)
		{
			esp.x = pila.x_fin;
			//Es igual que la pila
			if (esp.y_ini == pila.y_ini && esp.y_fin == pila.y_fin)
			{

				

				actualizo_espacios_iguales(esp);
				return;
			}
			//Hay algo por arriba 
			if ((esp.y_fin - pila.y_fin) > 0) 
			{
				
			//si no cabe nada
				if ((esp.y_fin - pila.y_fin) < dim_minima)
				{
				//Buscar si hay un espacio por encima y ver su inicio
				bool listo = false;
				for (auto itl = lespacios.begin(); itl != lespacios.end() && listo != true; itl++)
				{
					if ((*itl).y_ini == esp.y_fin && (*itl).x < esp.x)
					{
						(*itl).y_ini = pila.y_fin;
						esp.y_fin = pila.y_fin;
						listo = true;
					}
				}
				}
				else
				{
					esp.y_fin = pila.y_fin;
					esp1.y_ini = pila.y_fin;
					InsertaryActualizar(esp1);
				}
			}
			//Algo por debajo que no cabe nada
			if (((pila.y_ini-esp.y_ini) > 0) )
			{

				if (((pila.y_ini - esp.y_ini) < dim_minima))
				{
					//Buscar si hay un espacio por debajo y ver su inicio
									//Buscar si hay un espacio por encima y ver su inicio
					bool listo = false;
					for (auto itl = lespacios.begin(); itl != lespacios.end() && listo != true; itl++)
					{
						if ((*itl).y_fin == esp.y_ini && (*itl).x < esp.x)
						{
							(*itl).y_fin = pila.y_ini;
							esp.y_ini = pila.y_ini;
							listo = true;
						}
					}
				}
				else
				{
					esp.y_ini = pila.y_ini;
				esp2.y_fin = pila.y_ini;
				InsertaryActualizar(esp2);

				}

			}
			actualizo_espacios_iguales(esp);

			/*
				for (auto itl = lespacios.begin(); itl != lespacios.end(); itl++)
				{
					if (itl->y_ini == esp.y_fin)
					{

						itl = lespacios.insert(itl, esp);
						actualizo_espacios_iguales((*itl));
						break;
					}
				}*/

		}
		else
		{ //solamente un espacio
			if (pila.y_fin == esp.y_fin)
			{
				//abajo
				esp1.x = pila.x_fin;
				esp1.y_fin = pila.y_fin;
				esp1.y_ini = pila.y_ini;
				esp.y_fin = pila.y_ini;
				if (anchoy(esp) < dim_minima || EnMedio)
				{
					esp.y_fin = esp1.y_fin;
					esp.x = esp1.x;
				}
				else
					lespacios.push_back(esp1);
			}
			else
			{


				if (pila.y_ini == esp.y_ini)
				{
					//arriba
					esp1.x = pila.x_fin;
					esp1.y_fin = pila.y_fin;
					esp1.y_ini = pila.y_ini;
					esp.y_ini = pila.y_fin;
					if (anchoy(esp) < dim_minima || EnMedio)
					{
						esp.y_ini = esp1.y_ini;
						esp.x = esp1.x;
					}
					else
						lespacios.insert(lespacios.begin(), esp1);
				}
				else
				{
//					Tripleta<int> esp2(esp.y_ini, esp.y_fin, esp.x);
					Tripleta<int> espa2(esp);
					//arriba
					bool meter_esp1 = false;
					esp1.x = pila.x_fin;

					esp1.y_fin = pila.y_fin;
					esp1.y_ini = pila.y_ini;
					esp.y_ini = pila.y_fin;
					if (anchoy(esp) < dim_minima || EnMedio)
					{
						esp.y_ini = esp1.y_ini;
						esp.x = esp1.x;
					}
					else
					{
						meter_esp1 = true;
					}

					//abajo
					espa2.y_fin = pila.y_ini;
					if (anchoy(espa2) < dim_minima || EnMedio)
					{
						
						if (meter_esp1)
						{
							esp1.x = espa2.y_ini;
							lespacios.insert(lespacios.begin(), esp1);
						}
						else
							esp.y_ini = espa2.y_ini;


					}
					else
					{
					/*	lespacios.insert(lespacios.begin(), esp2);
						if (meter_esp1)
							lespacios.push_back(esp1);*/
//								lespacios.insert(lespacios.begin(), esp2);
						if (meter_esp1)
							lespacios.insert(lespacios.begin(), esp1);
						lespacios.insert(lespacios.begin(), espa2);
					}

				}
			}

		}
		for (auto itl = lespacios.begin(); itl != lespacios.end(); itl++)
		{
			if ((*itl).y_ini <= pila.y_ini &&
				(*itl).y_fin >= pila.y_fin)
			{
				//Limpiar los que sean de antes
				auto itl2 = (*itl).perfiles.begin();
				while (itl2 != (*itl).perfiles.end())
				{
					if ((*itl2).x < pila.x_fin)
					{
						itl2 = (*itl).perfiles.erase(itl2);
					}
					else
					{
						itl2++;
					}
				}
				//Insertamos el último
				(*itl).perfiles.push_back(Tripleta<int>(pila.y_ini, pila.y_fin, pila.x_fin));
				break;
			}
		}
		//Limpio perfiles antiguos
		//for (auto itl = lesp.lespacios.begin(); itl != lesp.lespacios.end(); itl++)
		for (auto itl = lespacios.begin(); itl != lespacios.end(); itl++)
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
					if ((*itl2).y_ini== (*itl2).y_fin)
						itl2 = (*itl).perfiles.erase(itl2);
					else
					itl2++;
				}
			}


		}
	};
	void actualizo_espacios_iguales(Tripleta<int>& esp)
	{
		int x_anterior = 100000, x_posterior = 100000;
		list<Tripleta<int>>::iterator ptr_anterior;
		list<Tripleta<int>>::iterator ptr_posterior;
		for (auto itl = lespacios.begin(); itl != lespacios.end(); itl++)
		{
			if ((*itl).y_fin == esp.y_ini)
			{
				x_anterior = (*itl).x;
				ptr_anterior = itl;


			}
			if ((*itl).y_ini == esp.y_fin)
			{
				x_posterior = (*itl).x;
				ptr_posterior = itl;

				break;
			}

		}
		
		if (x_anterior == esp.x)
		{
			esp.x = x_anterior;
			esp.y_ini = (*ptr_anterior).y_ini;
			esp.perfiles.insert(esp.perfiles.begin(), (*ptr_anterior).perfiles.begin(),  (*ptr_anterior).perfiles.end());
			lespacios.erase(ptr_anterior);
		}
		if (x_posterior == esp.x)
		{
			esp.x = x_posterior;
			esp.y_fin = (*ptr_posterior).y_fin;
			esp.perfiles.insert(esp.perfiles.end(), (*ptr_posterior).perfiles.begin(), (*ptr_posterior).perfiles.end());
			lespacios.erase(ptr_posterior);
		}
	};
	void actualizo_espacios(Tripleta<int>& esp)
	{
//		if (esp.x == 100000)
//			int kk = 9;
		int x_anterior=100000, x_posterior=100000;
		list<Tripleta<int>>::iterator ptr_anterior;
		list<Tripleta<int>>::iterator ptr_posterior;
		for (auto itl = lespacios.begin(); itl != lespacios.end(); itl++)
		{
			if ((*itl).y_fin == esp.y_ini )
			{
				x_anterior = (*itl).x;
				ptr_anterior = itl;
			}
			if ((*itl).y_ini == esp.y_fin)
			{
				x_posterior = (*itl).x;
				ptr_posterior = itl;
				break;
			}
			
		}
		//modificamos esp y eliminamos anterior o posterior
		
		if (min(x_anterior, x_posterior) >= esp.x)
		{
			if (x_anterior < x_posterior)
			{
				esp.x = x_anterior;
				esp.y_ini = (*ptr_anterior).y_ini;
				esp.perfiles.insert(esp.perfiles.begin(),(*ptr_anterior).perfiles.begin(),(*ptr_anterior).perfiles.end());
				lespacios.erase(ptr_anterior);
			}
			else
			{	
				//if (x_posterior == 100000)
				//	int kk = 9;
				esp.x = x_posterior;
				esp.y_fin = (*ptr_posterior).y_fin;
				esp.perfiles.insert(esp.perfiles.begin(), (*ptr_posterior).perfiles.begin(), (*ptr_posterior).perfiles.end());
				lespacios.erase(ptr_posterior);			
			}
		}
		else
		{
			if (x_anterior >= esp.x && x_anterior!=100000)
			{
				esp.x = x_anterior;
				esp.y_ini = (*ptr_anterior).y_ini;
				lespacios.erase(ptr_anterior);
			}
			if (x_posterior >= esp.x && x_posterior!=100000)
			{
				esp.x = x_posterior;
				esp.y_fin = (*ptr_posterior).y_fin;
				lespacios.erase(ptr_posterior);
			}
		}
				
	};
};
#endif