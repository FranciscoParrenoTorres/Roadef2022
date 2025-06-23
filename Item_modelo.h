#pragma once
#include "encabezados.h"

#ifndef ITEM_MODELO_H
#define ITEM_MODELO_H
class item_modelo
{
public:
	int num_items;
	int orden_ruta;
	int colocado;
	item* id_item;
	int obligado;
	long long int dia;
	int Cod_Camion;
	int Cod_Camion_Last;
	int Cod_Extra_Last;

	item_modelo(item* id, int items, int orden, int obli, int di)
	{
		num_items = items;
		id_item = id;
		orden_ruta = orden;
		colocado = 0;
		obligado = obli;
		dia = id->latest_arrival;
		Cod_Camion = 0;
		Cod_Camion_Last = -1;
		Cod_Extra_Last = 0;
	};
	item_modelo(item* id, int items, int orden, int obli, int di, int cc)
	{
		num_items = items;
		id_item = id;
		orden_ruta = orden;
		colocado = 0;
		obligado = obli;
		dia = id->latest_arrival;
		Cod_Camion = cc;
	};
	item_modelo(item* id, int items, int orden, int obli, int di, int cc, int cid, int kex)
	{
		num_items = items;
		id_item = id;
		orden_ruta = orden;
		colocado = 0;
		obligado = obli;
		dia = id->latest_arrival;
		Cod_Camion = cc;
		Cod_Camion_Last = cid;
		Cod_Extra_Last = kex;
	};
	item_modelo(item* id, int items, int orden,int obli)
	{
		num_items = items;
		id_item = id;
		orden_ruta = orden;
		colocado = 0;
		obligado = obli;
		dia = id->latest_arrival;
		Cod_Camion = 0;
		Cod_Camion_Last = -1;
		Cod_Extra_Last = 0;

	};
	item_modelo()
	{
		num_items = 0;
		orden_ruta = 0;
		id_item = NULL;
		colocado = 0;
		obligado = 1;
		dia = 0;
		Cod_Camion = 0;
		Cod_Camion_Last = -1;
		Cod_Extra_Last = 0;
	};
	~item_modelo()
	{
	};
	item_modelo(const item_modelo& t1)
	{
		num_items = t1.num_items;
		id_item = t1.id_item;
		orden_ruta = t1.orden_ruta;
		colocado = t1.colocado;
		obligado = t1.obligado;
		dia = t1.dia;
		Cod_Camion = t1.Cod_Camion;
		Cod_Camion_Last = t1.Cod_Camion_Last;
		Cod_Extra_Last = t1.Cod_Extra_Last;
		
	};
	bool operator < (item_modelo& a)const
	{

		if (orden_ruta < a.orden_ruta) return true;
		else
		{
			if (orden_ruta > a.orden_ruta)
				return false;
			else
			{
				/*if (obligado < a.obligado)
				{
					return true;
				}
				else
				{
					if (a.obligado > obligado)
						return false;
					else
					{*/
						if (id_item->lista_camiones.size() == a.id_item->lista_camiones.size())
						{
							if (id_item->inventory_cost > a.id_item->inventory_cost)
								return true;
							else
							{
								if (id_item->densidad < a.id_item->densidad)
								{
									return true;

								}
								else
								{
									if (max(id_item->width, id_item->length) > max(a.id_item->width, a.id_item->length))
										return true;
									else return false;
								}
							}

						}
						else
						{
							if (id_item->lista_camiones.size() < a.id_item->lista_camiones.size())
								return true;
							else
							{
								if (id_item->inventory_cost > a.id_item->inventory_cost)
									return true;
								else
								{
									if (id_item->densidad < a.id_item->densidad)
									{
										return true;

									}
									else
									{
										if (max(id_item->width, id_item->length) > max(a.id_item->width, a.id_item->length))
											return true;
										else return false;
									}

								}
							}
						}
					/*}
				}*/
			}
			
		}
	};
	struct camiones
	{
		bool operator() (const item_modelo& a, const item_modelo& b)
		{
			if (a.orden_ruta == b.orden_ruta)
			{
				if (a.obligado == b.obligado)
				{
					if (a.dia == b.dia)
					{

						return (a.id_item->lista_camiones.size() < b.id_item->lista_camiones.size()) ? true : false;
					}
						else return(a.dia > b.dia) ? true : false;
				}
				else return (a.obligado > b.obligado) ? true : false;

			}
			else return (a.orden_ruta > b.orden_ruta) ? false : true;
			//if (a.orden_ruta < b.orden_ruta)
			//{
			//	return true;
			//}
			//else
			//{
			//	if (a.orden_ruta > b.orden_ruta)
			//	{
			//		return false;
			//	}
			//	else
			//	{
			//		/*if (a.obligado < b.obligado)
			//		{
			//			return true;
			//		}
			//		else
			//		{
			//			if (b.obligado > a.obligado)
			//				return false;
			//			else
			//			{*/

			//				if (a.id_item->lista_camiones.size() < b.id_item->lista_camiones.size())
			//				{
			//					return true;
			//				}
			//				else
			//				{
			//					return false;
			//				}
			//			/*}
			//		}*/
			//	}
			//}
		}
	};
	struct fecha
	{
		bool operator() (const item_modelo& a, const item_modelo& b)
		{
			if (a.orden_ruta == b.orden_ruta)
			{

					if (a.obligado == b.obligado)
					{
						if (a.dia == b.dia)
						{
						//dE MAYOR COSTE A MENOR COSTE
						return (a.id_item->latest_arrival > b.id_item->latest_arrival) ? true : false;
						}
						else return(a.dia > b.dia) ? true : false;
					}
					else return (a.obligado > b.obligado) ? true : false;

			}
			else return (a.orden_ruta > b.orden_ruta) ? false : true;
		}
	};
	struct inventario
	{
		bool operator() (const item_modelo& a, const item_modelo& b)
		{
			if (a.orden_ruta == b.orden_ruta)
			{
				if (a.obligado == b.obligado)
				{
					if (a.dia == b.dia)
					{

						//dE MAYOR COSTE A MENOR COSTE
						return (a.id_item->inventory_cost > b.id_item->inventory_cost) ? true : false;
					}
					else return(a.dia > b.dia) ? true : false;
				}
				else return (a.obligado > b.obligado) ? true : false;

			}
			else return (a.orden_ruta > b.orden_ruta) ? false : true;
			//	if (a.orden_ruta < b.orden_ruta)
			//		return true;
			//	else
			//	{
			//		if (a.orden_ruta > b.orden_ruta)
			//			return false;
			//		else
			//		{
			//			/*if (a.obligado < b.obligado)
			//			{
			//				return true;
			//			}
			//			else
			//			{
			//				if (b.obligado > a.obligado)
			//					return false;
			//				else
			//				{*/
			//					if (a.id_item->Aleatorizado_inventory_cost > b.id_item->Aleatorizado_inventory_cost)
			//						return true;
			//					else
			//					{
			//						return false;
			//					}
			//				/*}
			//			}*/
			//		}
			//	}


		}

	};
	struct volumen
	{
		bool operator() (const item_modelo& a, const item_modelo& b)
		{
			if (a.orden_ruta == b.orden_ruta)
			{
				if (a.obligado == b.obligado)
				{
					if (a.dia == b.dia)
					{

						//DE MAYOR VOLUMEN A MENOR
						return (a.id_item->Aleatorizado_volumen > b.id_item->Aleatorizado_volumen) ? true : false;

					}
					else return(a.dia > b.dia) ? true : false;
				}
				else return (a.obligado > b.obligado) ? true : false;
			}
			else return (a.orden_ruta > b.orden_ruta) ? false : true;
			//	if (a.orden_ruta < b.orden_ruta)
			//		return true;
			//	else
			//	{
			//		if (a.orden_ruta > b.orden_ruta)
			//			return false;
			//		else
			//		{
			//			/*if (a.obligado < b.obligado)
			//			{
			//				return true;
			//			}
			//			else
			//			{
			//				if (b.obligado > a.obligado)
			//					return false;
			//				else
			//				{*/
			//					if (a.id_item->Aleatorizado_inventory_cost > b.id_item->Aleatorizado_inventory_cost)
			//						return true;
			//					else
			//					{
			//						return false;
			//					}
			//				/*}
			//			}*/
			//		}
			//	}


		}

	};
	struct width_vol_ini
	{//Esta a medias
		bool operator() (const item_modelo& a, const item_modelo& b)
		{
			if (max(a.id_item->width, a.id_item->length) == max(b.id_item->width, b.id_item->length))
			{
				return (a.id_item->Aleatorizado_volumen > b.id_item->Aleatorizado_volumen) ? true : false;

			}
			else return (max(a.id_item->width, a.id_item->length) < max(b.id_item->width, b.id_item->length) ) ? false : true;
			//	if (a.orden_ruta < b.orden_ruta)
			//		return true;
			//	else
			//	{
			//		if (a.orden_ruta > b.orden_ruta)
			//			return false;
			//		else
			//		{
			//			/*if (a.obligado < b.obligado)
			//			{
			//				return true;
			//			}
			//			else
			//			{
			//				if (b.obligado > a.obligado)
			//					return false;
			//				else
			//				{*/
			//					if (a.id_item->Aleatorizado_inventory_cost > b.id_item->Aleatorizado_inventory_cost)
			//						return true;
			//					else
			//					{
			//						return false;
			//					}
			//				/*}
			//			}*/
			//		}
			//	}


		}

	};
	struct width
	{
		bool operator() (const item_modelo& a, const item_modelo& b)
		{
			if (a.orden_ruta == b.orden_ruta)
			{
				if (a.obligado == b.obligado)
				{
					return  (max(a.id_item->width, a.id_item->length) > max(b.id_item->width, b.id_item->length)) ? true : false;

					
				}
				else return (a.obligado > b.obligado) ? true : false;
			}
			else return (a.orden_ruta > b.orden_ruta) ? false : true;
			//	if (a.orden_ruta < b.orden_ruta)
			//		return true;
			//	else
			//	{
			//		if (a.orden_ruta > b.orden_ruta)
			//			return false;
			//		else
			//		{
			//			/*if (a.obligado < b.obligado)
			//			{
			//				return true;
			//			}
			//			else
			//			{
			//				if (b.obligado > a.obligado)
			//					return false;
			//				else
			//				{*/
			//					if (a.id_item->Aleatorizado_inventory_cost > b.id_item->Aleatorizado_inventory_cost)
			//						return true;
			//					else
			//					{
			//						return false;
			//					}
			//				/*}
			//			}*/
			//		}
			//	}


		}

	};
	struct weight
	{
		bool operator() (const item_modelo& a, const item_modelo& b)
		{
			if (a.orden_ruta == b.orden_ruta)
			{
				if (a.obligado == b.obligado)
				{
					if (a.dia == b.dia)
					{
					//dE MENOS PESADO A MÁS PESADO
						return (a.id_item->Aleatorizado_weight < b.id_item->Aleatorizado_weight) ? true : false;
					
					}
					else return(a.dia > b.dia) ? true : false;	
				}
				else return (a.obligado > b.obligado) ? true : false;

			}
			else return (a.orden_ruta > b.orden_ruta) ? false : true;

			//if (a.orden_ruta < b.orden_ruta)
			//	return true;
			//else
			//{
			//	if (a.orden_ruta > b.orden_ruta)
			//		return false;
			//	else
			//	{
			//		/*if (a.obligado < b.obligado)
			//		{
			//			return true;
			//		}
			//		else
			//		{
			//			if (b.obligado > a.obligado)
			//				return false;
			//			else
			//			{*/
			//				if (a.id_item->Aleatorizado_weight < b.id_item->Aleatorizado_weight)
			//					return true;
			//				else
			//				{
			//					return false;
			//				}
			//			/*}
			//		}*/
			//		
			//	}
			//}


		}

	};
	struct densidad
	{
		bool operator() (const item_modelo& a, const item_modelo& b)
		{

			if (a.orden_ruta == b.orden_ruta)
			{
				if (a.obligado == b.obligado)
				{
					if (a.dia == b.dia)
					{

						//dE MENOR DENSIDAD A MAYOR DENSIDAD
						return (a.id_item->Aleatorizado_densidad < b.id_item->Aleatorizado_densidad) ? true : false;
					}
					else return(a.dia > b.dia) ? true : false;
				}
				else return (a.obligado > b.obligado) ? true : false;

			}
			else return (a.orden_ruta > b.orden_ruta) ? false: true;
			//if (a.orden_ruta < b.orden_ruta)
			//	return true;
			//else
			//{
			//	if (a.orden_ruta > b.orden_ruta)
			//		return false;
			//	else
			//	{
			//		/*if (a.obligado < b.obligado)
			//		{
			//			return true;
			//		}
			//		else
			//		{
			//			if (b.obligado > a.obligado)
			//				return false;
			//			else
			//			{*/
			//				if (a.id_item->Aleatorizado_densidad < b.id_item->Aleatorizado_densidad)
			//					return true;
			//				else
			//				{
			//					return false;
			//				}
			//			/*}
			//		}	*/			
			//	}
			//}

		}

	};
	struct random
	{
		bool operator() (item_modelo& a, item_modelo& b)
		{

			if (a.orden_ruta < b.orden_ruta) return true;
			else
			{
				if (a.orden_ruta == b.orden_ruta)
				{
					/*if (a.obligado < b.obligado)
					{
						return true;
					}
					else
					{
						if (b.obligado > a.obligado)
							return false;
						else
						{*/
							if (a.id_item->lista_camiones.size() == 1 && b.id_item->lista_camiones.size() == 1)
							{

								if (a.id_item->Aleatorizado_inventory_cost > b.id_item->Aleatorizado_inventory_cost) return true;
								else
								{
									if (a.id_item->densidad < b.id_item->densidad)
									{
										return true;

									}
									else
									{
										double areaa = (0.01 * (double)max(a.id_item->width, a.id_item->length));
										double areab = (0.01 * (double)max(b.id_item->width, b.id_item->length));
										if (areaa > areab)
											return true;
										else return false;
									}
								}

							}
							else
							{

								if (a.id_item->lista_camiones.size() == 1) return true;
								else
								{
									if (b.id_item->lista_camiones.size() == 1) return false;
									else
									{
										if (a.id_item->lista_camiones.size() <= b.id_item->lista_camiones.size())
										{
											return true;
										}
										if (a.id_item->Aleatorizado_inventory_cost > b.id_item->Aleatorizado_inventory_cost)
											return true;
										else
										{
											if (a.id_item->densidad < b.id_item->densidad)
											{
												return true;

											}
											else
											{
												double areaa = (0.01 * (double)max(a.id_item->width, a.id_item->length));
												double areab = (0.01 * (double)max(b.id_item->width, b.id_item->length));
												if (areaa > areab)
													return true;
												else return false;
											}
										}
									}
								}
							}
						/*}
					}*/

					
				}
				else return false;
			}
		}

	};
	struct random_no_peso
	{
		bool operator() (item_modelo& a, item_modelo& b)
		{

			if (a.orden_ruta < b.orden_ruta) return true;
			else
			{
				if (a.orden_ruta == b.orden_ruta)
				{
					/*if (a.obligado < b.obligado)
					{
						return true;
					}
					else
					{
						if (b.obligado > a.obligado)
							return false;
						else
						{*/
							if (a.id_item->lista_camiones.size() == 1 && b.id_item->lista_camiones.size() == 1)
							{
								//						 int ainventory = 0.1 * (double)a.id_item->inventory_cost;
								//						 int binventory = 0.1 * (double)b.id_item->inventory_cost;
								if (a.id_item->Aleatorizado_weight > b.id_item->Aleatorizado_weight)
								{
									return true;

								}
								else
								{
									if (a.id_item->densidad < b.id_item->densidad)
									{
										return true;

									}
									else
									{
										double areaa = (0.01 * (double)max(a.id_item->width, a.id_item->length));
										double areab = (0.01 * (double)max(b.id_item->width, b.id_item->length));
										if (areaa > areab)
											return true;
										else return false;
									}
								}


							}
							else
							{
								if (a.id_item->lista_camiones.size() == 1) return true;
								else
								{
									if (b.id_item->lista_camiones.size() == 1) return false;
									else
									{
										if (a.id_item->lista_camiones.size() <= b.id_item->lista_camiones.size())
										{
											return true;
										}
										if (a.id_item->Aleatorizado_weight > b.id_item->Aleatorizado_weight)
										{
											return true;

										}
										else
										{
											if (a.id_item->densidad < b.id_item->densidad)
											{
												return true;

											}
											else
											{
												double areaa = (0.01 * (double)max(a.id_item->width, a.id_item->length));
												double areab = (0.01 * (double)max(b.id_item->width, b.id_item->length));
												if (areaa > areab)
													return true;
												else return false;
											}
										}

									}
								}
							}
						/*}
					}*/
					
				}
				else return false;
			}
		}

	};
	struct no_inventario
	{
		bool operator() (item_modelo& a, item_modelo& b)
		{

			if (a.orden_ruta < b.orden_ruta) return true;
			else
			{
				if (a.orden_ruta == b.orden_ruta)
				{
					/*if (a.obligado < b.obligado)
					{
						return true;
					}
					else
					{
						if (b.obligado > a.obligado)
							return false;
						else
						{*/
							if (a.id_item->lista_camiones.size() == 1 && b.id_item->lista_camiones.size() == 1)
							{

								if (a.id_item->Aleatorizado_densidad > b.id_item->Aleatorizado_densidad)
								{
									return true;

								}
								else
								{
									if (a.id_item->inventory_cost > b.id_item->inventory_cost) return true;
									else
									{
										if (max(a.id_item->width, a.id_item->length) > max(b.id_item->width, b.id_item->length))
											return true;
										else return false;
									}
								}


							}
							else
							{
								if (a.id_item->lista_camiones.size() == 1) return true;
								else
								{
									if (b.id_item->lista_camiones.size() == 1) return false;
									else
									{
										if (a.id_item->Aleatorizado_densidad > b.id_item->Aleatorizado_densidad)
										{
											return true;

										}
										else
										{
											if (a.id_item->inventory_cost > b.id_item->inventory_cost) return true;
											else
											{
												if (max(a.id_item->width, a.id_item->length) > max(b.id_item->width, b.id_item->length))
													return true;
												else return false;
											}
										}
									}
								}
							}
						/*}
					}*/
					
				}
				else return false;
			}
		}
	};
};
#endif