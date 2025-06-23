#pragma once
#ifndef PILAS_CARGADAS_H
#define PILAS_CARGADAS_H
class pilas_cargadas
{
public:
	stack_sol id_pila;
	list<item_sol> items;
	
	pilas_cargadas(stack_sol id_pila1, list<item_sol> items1)
	{
		id_pila = id_pila1;
		items = items1;
	};
	~pilas_cargadas()
	{


	}

};
#endif
