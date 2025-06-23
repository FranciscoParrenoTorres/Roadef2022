#include "encabezados.h"


extern  "C" int  bouknap(int n, int* p, int* w, int* m, int* x, int c);
//Si el peso es el mismo que el ancho el knapsack es trivial
int knapsack_bouknap(set<int> valores,int max)
{

	//Se crean los vectores de enteros, profit es lo que ganamos
	//weight es el peso 
	//m cuántos de esa pieza
	//x sería la solución

	int* profit = (int*)calloc(valores.size(), sizeof(int));
	int* weight = (int*)calloc(valores.size(), sizeof(int));
	int* m = (int*)calloc(valores.size(), sizeof(int));
	int* x = (int*)calloc(valores.size(), sizeof(int));

	int sum_posible = 0;
	int val_posible = 0;
	int i = 0;
	for (auto it=valores.begin(); it!=valores.end(); it++,i++)
	{
		
		profit[i] = (*it);
		weight[i] = (*it);
		//en este caso todos sería de 1
		m[i] = max/(*it);
		val_posible += profit[i]*m[i];
		sum_posible += weight[i] * m[i];

	}

	int maximo_knapsack = 0;
	if (sum_posible <= max)
		return sum_posible;
	else maximo_knapsack = bouknap((int)valores.size(), profit, weight, m, x, max);
	return  maximo_knapsack;
//	printf("Máximo Tiras: Tiras=%d\t Pilas=%d \tPiezas=%d\n", maxTiras, maxPilas, maxPiezasPila);
}