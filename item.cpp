#include "item.h"
double item::Volumen(int widhtTruck, int min_ancho)
{

	
	if (forced_orientation_int == 0)
	{
		if (min(length, width) < min_ancho)
		{
			return ((double)widhtTruck/1000) * ((double)min(length, width)/1000)  *((double)height/1000);
		}
		else
		{
			return volumen;
		}
	}
	//lengthwise
	if (forced_orientation_int == 1)
	{
		if (widhtTruck - width < min_ancho)
		{
			return ((double)widhtTruck / 1000) * ((double)length / 1000)  *((double)height / 1000);
		}
		else
		{
			return volumen;
		}
	}
	//widthwise
	if (forced_orientation_int == 2)
	{
		if (widhtTruck - length < min_ancho)
		{
			return ((double)widhtTruck / 1000) * ((double)width / 1000) * ((double)height / 1000);
		}
		else
		{
			return volumen;
		}
	}
	return volumen;
}