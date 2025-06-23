#include "encabezados.h"
int parameters::tqueda()
{
	struct timeb tiempo_ahora;

	ftime(&tiempo_ahora);
	int tactual = ((double)((tiempo_ahora.time - tini.time) * 1000 + tiempo_ahora.millitm - tini.millitm)) / 1000;

	return time_limit-tactual;

}
int parameters::tllevo()
{
	struct timeb tiempo_ahora;

	ftime(&tiempo_ahora);
	int tactual = ((double)((tiempo_ahora.time - tini.time) * 1000 + tiempo_ahora.millitm - tini.millitm)) / 1000;

	return tactual;

}
