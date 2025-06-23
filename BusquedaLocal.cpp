#include "encabezados.h"
#include "BusquedaLocal.h"

//1. - Tomar la lista de camiones de la solución, ordenada por fecha de llegada creciente, es decir, primero los que primero llegan, aunque no tengo muy claro si ese es el mejor orden(empates resueltos aleatoriamente)
//2. - Tomar el siguiente camión de la lista, le llamaremos TR1, si lleva cajas que pueden ir en camiones de días posteriores(y por tanto en TR1 tienen costes de inventario).
//3. - De la lista de camiones, tomar un segundo camión, TR2, al menos con 1 día de llegada posterior a TR1, si hay cajas actualmente cargadas en TR2 que pueden ir en TR1 y cajas actualmente cargadas en TR1 que pueden ir en TR2.
//4. - Vaciar ambos camiones
//5. - Hacer el packing del camión TR2(el de mayor fecha de llegada) con una lista de cajas en la que están todas sus cajas y aquellas actualmente en TR1 que pueden ir en TR2.El objetivo es maximizar los costes de inventario de las cajas que se empaqueten.Serán obligatorias las cajas que pueden ir en TR2 pero no pueden ir en TR1.
//6. - Hacer el packing del camión TR1 con las cajas no empaquetadas en TR2.Si se pueden empaquetar todas las cajas de los dos camiones, tenemos una nueva solución posible.Habrá que comprobar que es mejor que la inicial y, si lo es, actualizar la solución.
// 7. - Volver al paso 3.
// 8. - Si no hay más camiones en la lista, terminar.

//Le paso dos camiones y tiene que ver si hay items que se puedan intercambiar entre ellos 
bool PosibleIntercambio(truck* TR1, truck* TR2)
{
    //recorro los items del camion 2
    bool delTr2puedenirAlTr1 = false;
    bool delTr1puedenirAlTr2 = false;
    double volTr2puedenirAlTr1 = 0;
    double volTr1puedenirAlTr2 = 0;
    
    for (auto itpila : TR2->pilas_solucion)
    {
        for (auto it : itpila.items)
        {
            //si el item puede ir en el camion 1, ya tendré algo mejor
            if (PuedeIrItemCam(it.Item, TR1))
            {
                delTr2puedenirAlTr1 = true;
                volTr2puedenirAlTr1 += it.volumen;
            }
        }

    }
    if (!delTr2puedenirAlTr1)
    {
        return false;
    }
    for (auto itpila : TR1->pilas_solucion)
    {
        for (auto it : itpila.items)
        {
            //si el item puede ir en el camion 2, ya tendré algo mejor
            if (PuedeIrItemCam(it.Item, TR2))
            {
                delTr1puedenirAlTr2 = true;
                volTr1puedenirAlTr2 += it.volumen;
            }
        }
    }
    if (!delTr1puedenirAlTr2)
    {
        return false;
    }

    return true;
	
}



void BusquedaLocal(Subproblema* Sub) {
    
    //Genero una única lista
 //   auto camiones = Sub->lista_camiones;
    std::list<truck*> camiones;
    for (auto it1 : Sub->lista_camiones)
    {
        
        camiones.insert(camiones.end(), (it1)->pcamiones_extra.rbegin(), (it1)->pcamiones_extra.rend());
        camiones.push_back(it1);
    }

    // Los camiones ya están ordenados por fecha
    for (auto it1 = camiones.begin(); it1 != camiones.end(); ++it1) {
        if ((*it1)->pilas_solucion.size() == 0) continue;
        truck* TR1 = *it1;
        if (Sub->Pruebas) cout << "Camion 1" <<(*it1)->id_truck <<endl;
        for (auto it2 = std::next(it1); it2 != camiones.end(); ++it2) {
            if ((*it2)->pilas_solucion.size() == 0) continue;
            truck* TR2 = *it2;
            int total_dias = TR1->DiferenciaDiasCamCam(TR2);
            //Si son del mismo día no quiero hacer el intercambio
            if (total_dias == 0) continue;
            if (total_dias < 0)
                int kk = 9;
            // 2. Verificar si es posible el intercambio entre TR1 y TR2
            if (PosibleIntercambio(TR1, TR2)) {
                if (Sub->Pruebas) cout << "Ha mejorado" << endl;
                if (Sub->Pruebas) cout << "Camion 1" << (*it1)->id_truck << "Extras" << (*it1)->pcamiones_extra.size() << "Camion 2" << (*it2)->id_truck << "Extras2" << (*it2)->pcamiones_extra.size() << endl;
                //Si es posible 
                //Quiero cargar los dos camiones pero cambiando la forma
                ReorganizarCamiones(TR1, TR2, Sub);
                if (Sub->Pruebas) cout << "Camion 1" << (*it1)->id_truck << "Extras" << (*it1)->pcamiones_extra.size() << "Camion 2" << (*it2)->id_truck << "Extras2" << (*it2)->pcamiones_extra.size() << endl;
                //Que pasa si hay cambio?
            }
        }
    }
}
//Resto los de itemsTR1 y itemsTR2
void RestablecerItemsDeLista(Subproblema* Sub,  std::vector<item_modelo>& itemsTR1, std::vector<item_modelo>& itemsTR2) 
{
    for (auto& itm : itemsTR1) {
        itm.id_item->num_items += itm.colocado;
    }
    for (auto& itm : itemsTR2) {
        itm.id_item->num_items += itm.colocado;
    }
}
void EscribirInfoItems(truck* TR2new) {
    for (const auto& pila : TR2new->pilas_solucion) {
		cout << "Pila x_ini: " << pila.id_pila.x_ini << std::endl;
		cout << "Pila y_ini: " << pila.id_pila.y_ini << std::endl;
		cout << "Pila x_fin: " << pila.id_pila.x_fin << std::endl;
		cout << "Pila y_fin: " << pila.id_pila.y_fin << std::endl;
		cout << "Pila STack: " << pila.id_pila.stack_code << std::endl;
        for (const auto& it : pila.items) {
			std::cout << "Item: " << it.Item->Id <<  std::endl;
			cout << "Plant code "<< it.Item->plant_code_int << endl;
			cout << "Suplier code " << it.Item->suplier_code_int << endl;
			cout << "Product code " << it.Item->product_code_int << endl;
			cout << "Suplier dock " << it.Item->suplier_dock_int << endl;
			cout << "Plant dock " << it.Item->plant_dock_int << endl;
			cout << "Sup cod dock " << it.Item->sup_cod_dock_int << endl;

            
        }
    }
}
void ReorganizarCamiones(truck* TR1, truck* TR2, Subproblema* Sub) {

    
    int Coste_inventario = Sub->coste_inventario;
    if (Coste_inventario == 1699)
        int kk = 9;
    // 1. Extraer y vaciar ambos camiones
    std::vector<item_modelo> listaTotalItems;
    truck* TR1new = new truck(*TR1);
    truck* TR2new = new truck(*TR2);
    TR1new->reset2();
    TR1new->reanudar_solucion();
    TR2new->reset2();
    TR2new->reanudar_solucion();
    // Extraer items de TR1
    //coste inventario
    int cost_inventori_TR1=0;
    int cost_inventori_TR2 = 0;
    int cost_inventori_TR1tiene = 0;
    int cost_inventori_TR2tiene = 0;
    for (auto& IT : TR1->items_sol)
    {
        int difday = Days_0001(IT.id_item->latest_arrival_tm_ptr.tm_year, IT.id_item->latest_arrival_tm_ptr.tm_mon + 1, IT.id_item->latest_arrival_tm_ptr.tm_mday) -
            Days_0001(TR1->arrival_time_tm_ptr.tm_year, TR1->arrival_time_tm_ptr.tm_mon + 1, TR1->arrival_time_tm_ptr.tm_mday);
        cost_inventori_TR1 += difday * IT.colocado * IT.id_item->inventory_cost;
        cost_inventori_TR1tiene +=  IT.colocado * IT.id_item->inventory_cost;
    }
    for (auto& IT : TR2->items_sol)
    {
        int difday = Days_0001(IT.id_item->latest_arrival_tm_ptr.tm_year, IT.id_item->latest_arrival_tm_ptr.tm_mon + 1, IT.id_item->latest_arrival_tm_ptr.tm_mday) -
            Days_0001(TR2->arrival_time_tm_ptr.tm_year, TR2->arrival_time_tm_ptr.tm_mon + 1, TR2->arrival_time_tm_ptr.tm_mday);
        cost_inventori_TR2 += difday * IT.colocado * IT.id_item->inventory_cost;
        cost_inventori_TR2tiene += IT.colocado * IT.id_item->inventory_cost;
    }
    //Los items que estaban en TR1 los pongo como disponibles
    // Extraer items de TR2
    for (auto &IT :TR1->items_sol)
    {
        
        if (IT.colocado == 0) continue;
        int obli = 0;
        item_modelo itm(IT.id_item, IT.colocado, IT.orden_ruta, obli, 0, TR1->CodId);
        itm.Cod_Camion_Last = TR1->CodIdCC;
        itm.Cod_Extra_Last = TR1->kextra;
            
        listaTotalItems.push_back(itm);

    }
    
    
    //Los items que estaban en TR2 los pongo como disponibles
    // y los meto en una lista única
    // Extraer items de TR2
	for (auto& IT : TR2->items_sol)
	{
		if (IT.colocado == 0) continue;
		bool enc2 = false;
        for (auto& itm : listaTotalItems)
        {
            if (itm.id_item->CodId == IT.id_item->CodId)
            {
                
                itm.num_items += IT.colocado;
                enc2 = true;
                break;
            }
        }
        if (!enc2)
        {
            
            int obli = 0;
            item_modelo itm(IT.id_item, IT.colocado, IT.orden_ruta, obli, 0, TR2->CodId);
            itm.Cod_Camion_Last = TR2->CodIdCC;
            itm.Cod_Extra_Last = TR2->kextra;
            listaTotalItems.push_back(itm);
        }


		
	}


  
    // 3. Distribuir los ítems en TR1 y TR2
    std::vector<item_modelo> itemsTR1, itemsTR2;

    //Primero probamos a meter todo los posibles en TR1new
    for (auto& itm : listaTotalItems) {
        if (itm.id_item->CodIdCC == 24)
            int kk = 9;
        // Intentar colocarlo en TR1
        if (PuedeIrItemCam(itm.id_item, TR1new)) {
            //Quito este número
            itm.id_item->num_items += itm.num_items;
            itm.orden_ruta = TR1new->orden_ruta[itm.id_item->sup_cod_dock];
            TR1new->num_items_modelo+= itm.num_items;
            //Si no puede ir en el siguiente, es obligado      
            if (!PuedeIrItemCam(itm.id_item, TR2new))
            {
                itm.obligado = 1;
            }
            itemsTR1.push_back(itm);
        }
        // Si no puede ir en TR1, probar en TR2
        else if (PuedeIrItemCam(itm.id_item, TR2new)) {
            int anterior_orden = itm.orden_ruta;
            itm.orden_ruta = TR2new->orden_ruta[itm.id_item->sup_cod_dock];
            if (itm.orden_ruta != anterior_orden)
                int kk = 9;
            //Los pongo disponibles y pueden aparecer en el modelo este número
            itm.id_item->num_items += itm.num_items;
            TR2new->num_items_modelo += itm.num_items;
            itemsTR2.push_back(itm);
        }
    }

  



// 
// 
//         //Me quedo con los costes de inventarios
    int coste_inventario_TR1 = TR1->coste_inventario_camion;
    int coste_inventario_TR2 = TR2->coste_inventario_camion;
    int suma_anterior = coste_inventario_TR1 + coste_inventario_TR2;
    // 4. Llamar a CargoCamion para cargar TR1 y TR2
//    AddCamionSolucion(Sub, TR1new);
    truck_sol camion(TR1new->id_truck);
    camion.peso_max = TR1new->max_loading_weight;
    camion.volumen_max = TR1new->volumen;
    camion.coste = TR1new->es_extra ? TR1new->coste_extra : TR1new->cost;
    Sub->sol.listado_camiones.push_back(camion);
    
    Sub->CargoCamion(TR1new, itemsTR1, TR1->id_truck, TR1new->num_items_modelo, 0,2);
    bool parar = false;
    for (auto& itm : itemsTR1)
    {
        if (itm.num_items > 0 )
        {
            //Hay un obligado sin colocar
            //tengo que parar
            if (itm.obligado == 1)
            {
                //Parar porque ya no voy a poder meter los mismo
                parar = true;
                Sub->coste_inventario -= TR1new->coste_inventario_camion;
                DeleteCamionSolucion(Sub, TR1new,false);
                RestablecerItemsDeLista(Sub, itemsTR1,itemsTR2);

                break;
            }
            bool enc2 = false;
            for (auto& itm2 : itemsTR2)
            {
                if (itm2.id_item->CodId == itm.id_item->CodId)
                {
                    int anterior_orden= itm2.orden_ruta;
                    itm2.orden_ruta = TR2new->orden_ruta[itm.id_item->sup_cod_dock];
                    if (itm2.orden_ruta != anterior_orden)
                        int kk = 9;
                    itm2.num_items += itm.num_items;
                    enc2 = true;
                    break;
                }
            }
            if (!enc2)
            {
                int obli = 0;
                int anterior_orden = itm.orden_ruta;
                itm.orden_ruta = TR2new->orden_ruta[itm.id_item->sup_cod_dock];
                if (itm.orden_ruta != anterior_orden)
                    int kk = 9;
                item_modelo itm3(itm.id_item, itm.num_items,  TR2new->orden_ruta[itm.id_item->sup_cod_dock] , obli, 0, TR2new->CodId);
                itm3.Cod_Camion_Last = TR2->CodIdCC;
                itm3.Cod_Extra_Last = TR2->kextra;
                itm3.colocado = 0;
                TR2new->num_items_modelo += itm.num_items;
                itemsTR2.push_back(itm3);
            }



        }

    }
    if (parar == true)
    {
        if (Sub->coste_inventario > Coste_inventario)
            int kk = 89;
        return;
    }
    if (itemsTR2.size() > 0)
    {
        //Los que han sobrado de TR1new tienen que ir a TR2
     //   AddCamionSolucion(Sub, TR2new);
        truck_sol camionSol(TR2new->id_truck);
        camionSol.peso_max = TR2new->max_loading_weight;
        camionSol.volumen_max = TR2new->volumen;
        camionSol.coste = TR2new->es_extra ? TR2new->coste_extra : TR2new->cost;
        Sub->sol.listado_camiones.push_back(camionSol);

        Sub->CargoCamion(TR2new, itemsTR2, TR2->id_truck, TR2new->num_items_modelo, 0, 0);
        //    EscribirInfoItems(TR2new);

            // Extraer items de TR1new
            //coste inventario
        int cost_inventori_TR1new = 0;
        int cost_inventori_TR2new = 0;
        int cost_inventori_TR1newtiene = 0;
        int cost_inventori_TR2newtiene = 0;
        for (auto& IT : TR1new->items_sol)
        {
            int difday = Days_0001(IT.id_item->latest_arrival_tm_ptr.tm_year, IT.id_item->latest_arrival_tm_ptr.tm_mon + 1, IT.id_item->latest_arrival_tm_ptr.tm_mday) -
                Days_0001(TR1new->arrival_time_tm_ptr.tm_year, TR1new->arrival_time_tm_ptr.tm_mon + 1, TR1new->arrival_time_tm_ptr.tm_mday);
            cost_inventori_TR1new += difday * IT.colocado * IT.id_item->inventory_cost;
            cost_inventori_TR1newtiene += IT.colocado * IT.id_item->inventory_cost;
        }
        for (auto& IT : TR2new->items_sol)
        {
            int difday = Days_0001(IT.id_item->latest_arrival_tm_ptr.tm_year, IT.id_item->latest_arrival_tm_ptr.tm_mon + 1, IT.id_item->latest_arrival_tm_ptr.tm_mday) -
                Days_0001(TR2new->arrival_time_tm_ptr.tm_year, TR2new->arrival_time_tm_ptr.tm_mon + 1, TR2new->arrival_time_tm_ptr.tm_mday);
            cost_inventori_TR2new += difday * IT.colocado * IT.id_item->inventory_cost;
            cost_inventori_TR2newtiene += IT.colocado * IT.id_item->inventory_cost;
        }
    }
    else
    {
        //No queda nada por meter aquí
        int kk = 9;
    }
    //         //Me quedo con los costes de inventarios nuevos
    int coste_inventario_TR1new = TR1new->coste_inventario_camion;
    int coste_inventario_TR2new = TR2new->coste_inventario_camion;
    int suma_nueva = coste_inventario_TR1new + coste_inventario_TR2new;
    // Quiero comparar el volumen ocupado y el peso cargado 
    // de TR1new y TR1
    double volumen_ocupado_TR1 = TR1->volumen_ocupado;
    double peso_cargado_TR1 = TR1->peso_cargado;
    double volumen_ocupado_TR1new = TR1new->volumen_ocupado;
    double peso_cargado_TR1new = TR1new->peso_cargado;
    
    bool mejora1 = false;
    // Comparar los valores
    if (volumen_ocupado_TR1new > volumen_ocupado_TR1 || peso_cargado_TR1new > peso_cargado_TR1) {
        mejora1 = true;
    }
    


    // Quiero comparar el volumen ocupado y el peso cargado 
    // de TR2new y TR2
    double volumen_ocupado_TR2 = TR2->volumen_ocupado;
    double peso_cargado_TR2 = TR2->peso_cargado;
    double volumen_ocupado_TR2new = TR2new->volumen_ocupado;
    double peso_cargado_TR2new = TR2new->peso_cargado;
    bool mejora2 = false;
    double vol_antes = volumen_ocupado_TR1 + volumen_ocupado_TR2;
    double vol_new = volumen_ocupado_TR1new + volumen_ocupado_TR2new;
    double peso_antes = peso_cargado_TR1 + peso_cargado_TR2;
    double peso_new = peso_cargado_TR1new + peso_cargado_TR2new;
    
    // Comparar los valores
    if (volumen_ocupado_TR2new > volumen_ocupado_TR2 || peso_cargado_TR2new > peso_cargado_TR2) {
        mejora2 = true;
    }
    // 
    // Carga los items en el camión actual.
//    Sub->CargoCamion(itc, itc->items_modelo, itc->id_truck, itc->num_items_modelo, 0);
    // 5. Actualizar la solución
 //   Sub->actualizarSolucion();
    bool mejora = false;

   //
    bool Todos_colocados = true;
    for (auto it : itemsTR2)
    {
        if (it.num_items > 0)
        {
            Todos_colocados = false;
            break;
        }
    }

    if (Todos_colocados && (coste_inventario_TR1 + coste_inventario_TR2 > coste_inventario_TR1new + coste_inventario_TR2new)) {
        mejora = true;
    }
    else
        mejora = false;


    if (mejora)
    {
        //Si tengo algo mejor tengo que quitar los camiones anteriores
        // Vaciar ambos camiones
 /*       TR1->items_colocados.clear();
        TR1->items_modelo.clear();
        TR1->volumen_total_items_modelo = 0;
        TR1->num_items_modelo = 0;
        TR2->items_colocados.clear();
        TR2->items_modelo.clear();
        TR2->volumen_total_items_modelo = 0;
        TR2->num_items_modelo = 0;*/
        Sub->coste_inventario -= TR1->coste_inventario_camion;
        Sub->coste_inventario -= TR2->coste_inventario_camion;
        DeleteCamionSolucion(Sub, TR1,true);
        DeleteCamionSolucion(Sub, TR2,true);
        TR1new->pcamiones_extra = TR1->pcamiones_extra;
        TR2new->pcamiones_extra = TR2->pcamiones_extra;
        *TR1 = *TR1new; // Asignar TR1new a TR1
        *TR2 = *TR2new; // Asignar TR1new a TR1
        if (Sub->Pruebas) cout << "Mejora" << endl;

    }
    else
    {
        Sub->coste_inventario -= TR1new->coste_inventario_camion;
        Sub->coste_inventario -= TR2new->coste_inventario_camion;
        DeleteCamionSolucion(Sub, TR1new,false);
        DeleteCamionSolucion(Sub, TR2new,false);
        RestablecerItemsDeLista(Sub, itemsTR1, itemsTR2);

    }
    if (Sub->coste_inventario > Coste_inventario)
        int kk = 89;
    if (Sub->coste_inventario < Coste_inventario)
        int kk = 89;
}

/*

struct EvaluacionMovimiento {
    bool esBeneficioso;
    double costeInventarioActual;
    double costeInventarioNuevo;
    double beneficioPotencial;
};

int tieneCosteInventario(truck* camion) {
    for (auto& item : camion->items) {
        //Si el item solo puede ir en ese camon por fecha
        int total_dias = camion->DiferenciaDiasLastCamItem(item);
            if (total_dias> 0) {
            return true;
        }
    }
    return false;
}
EvaluacionMovimiento puedeRecibir(truck* camionReceptor, item* itemMover, truck* camionOriginal) {
    EvaluacionMovimiento resultado;

    // Verificar si el ítem puede ir en el camión receptor
    if (!PuedeIrItemCam(itemMover, camionReceptor)) {
        resultado.esBeneficioso = false;
        return resultado; // No puede recibir el ítem
    }
    int total_dias_nuevo = camionReceptor->DiferenciaDiasLastCamItem(itemMover);
    int total_dias_antes = camionOriginal->DiferenciaDiasLastCamItem(itemMover);
    // Calcular el coste de inventario actual en el camión original
    resultado.costeInventarioActual = itemMover->num_items*
        total_dias_antes *
        itemMover->inventory_cost;

    // Calcular el coste de inventario en el camión receptor
    resultado.costeInventarioActual = itemMover->num_items *
        total_dias_nuevo *
        itemMover->inventory_cost;

    // Evaluar si el movimiento reduce el coste de inventario
    resultado.beneficioPotencial = resultado.costeInventarioActual - resultado.costeInventarioNuevo;
    resultado.esBeneficioso = resultado.beneficioPotencial > 0;

    return resultado;
}

void BusquedaLocal(Subproblema* Sub) {
    auto camiones = Sub->lista_camiones;

    // 1. Ordenar los camiones por fecha de llegada
    camiones.sort([](const truck* a, const truck* b) {
        return a->arrival_time < b->arrival_time;
        });

    for (auto it1 = camiones.begin(); it1 != camiones.end(); ++it1) {
        truck* TR1 = *it1;

        // 2. Si TR1 tiene ítems con coste de inventario
        for (auto it2 = std::next(it1); it2 != camiones.end(); ++it2) {
            truck* TR2 = *it2;

            // 3. Si TR2 llega al menos un día después
            if (TR2->arrival_time > TR1->arrival_time) {
                std::vector<item*> intercambiables_TR1;
                std::vector<item*> intercambiables_TR2;

                // Verificar ítems intercambiables de TR1 hacia TR2
                for (auto& item : TR1->items) {
                    EvaluacionMovimiento evaluacion = puedeRecibir(TR2, item, TR1);
                    if (evaluacion.esBeneficioso) {
                        intercambiables_TR1.push_back(item);
                    }
                }

                // Verificar ítems intercambiables de TR2 hacia TR1
                for (auto& item : TR2->items) {
                    EvaluacionMovimiento evaluacion = puedeRecibir(TR1, item, TR2);
                    if (evaluacion.esBeneficioso) {
                        intercambiables_TR2.push_back(item);
                    }
                }

                if (!intercambiables_TR1.empty() && !intercambiables_TR2.empty()) {
                    // 4. Vaciar ambos camiones
                    TR1->vaciar();
                    TR2->vaciar();

                    // 5. Empaquetar TR2
                    std::vector<item*> nuevasItemsTR2 = intercambiables_TR1;
                    nuevasItemsTR2.insert(nuevasItemsTR2.end(), TR2->items.begin(), TR2->items.end());
                    TR2->empaquetar(nuevasItemsTR2);

                    // 6. Empaquetar TR1
                    std::vector<item*> itemsRestantesTR1 = intercambiables_TR2;
                    TR1->empaquetar(itemsRestantesTR1);

                    // Comprobar si la solución es mejor
                    if (Sub->esMejorSolucion()) {
                        Sub->actualizarSolucion();
                    }
                }
            }
        }
    }
}*/