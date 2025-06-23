
#include "encabezados.h"
#include <random>


struct Facility {
    int index;
    long long int opening_day;
    double fixed_cost;
    double assigned_weight;
    double assigned_volume;
    double total_cost;
    int max_customers; // Límite de clientes que pueden ser asignados a la instalación
    int max_weight; // Clientes asignados actualmente
    int max_volume; // Clientes asignados actualmente
    vector<int> posibles_clientes; //Posibles clientes de esta instalacion
    double available_weight_capacity() const {
        return max_weight - assigned_weight;
    };

    double available_volume_capacity() const {
        return max_volume - assigned_volume;
    };
};

struct Customer {
    int index;
    long long int start_day;
    long long int end_day;
    double weight;
    double volume;
    double total_cost;
    int num_items;
    int num_items_iniciales;
    vector<int> posibles_instalaciones;//posibles instalaciones de este cliente
    vector<int> assignment_costs;
    vector<vector<int>> assignment;
};

bool is_feasible_partial(const vector<Customer>& solution, const vector<vector<Facility>>& facilities_matrix, int c_id, int f_id, int f_copy) {
    const Facility& facility_instance = facilities_matrix[f_id][f_copy];
    return (facility_instance.available_weight_capacity() > solution[c_id].weight) &&
        (facility_instance.available_volume_capacity() > solution[c_id].volume);
}

int is_feasible_partial(const vector<Customer>& solution, const vector<vector<Facility>>& facilities_matrix, int customer_id, int facility) {
  




    for (int j = 0; j < facilities_matrix[facility].size(); ++j) 
    {
        const Facility& facility_instance = facilities_matrix[facility][j];

        if ((facility_instance.available_weight_capacity() > solution[customer_id].weight) &&
            (facility_instance.available_volume_capacity() > solution[customer_id].volume))
            return j;

    }

    return -1;
}

double solution_cost(  vector<vector<Facility>>& facilities_matrix) {
    double cost1 = 0.0;
    double cost2 = 0.0;

    double facility_cost = 0.0;
    for (const auto& facility_copies : facilities_matrix) {
        for (const auto& facility_instance : facility_copies) {
            if (facility_instance.assigned_weight > 0) 
            {
                cost1 += facility_instance.fixed_cost;
                cost2 += facility_instance.total_cost;
            }
        }
    }
    return cost1+cost2;
}
vector<Customer> generate_initial_solution(vector<Customer>& customers, vector<vector<Facility>>& facilities_matrix,  int random_chance) {
    vector<Customer> initial_solution(customers);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 100);

    for (Customer& customer : initial_solution) {
        if (customer.num_items <= 0) continue;
        bool assigned = false;
        //if (customer.index == 671)
        //    int kk = 9;
        for (auto iti = customer.posibles_instalaciones.begin(); iti != customer.posibles_instalaciones.end() && !assigned; ++iti) {
  

            int copia = is_feasible_partial(initial_solution, facilities_matrix,  customer.index, (*iti));
            if (copia >= 0) {
                Facility& facility_instance = facilities_matrix[(*iti)][copia];
                int cuantos = std::min({ (facility_instance.max_weight - facility_instance.assigned_weight) / customer.weight,
                                        (facility_instance.max_volume - facility_instance.assigned_volume) / customer.volume,
                                        static_cast<double>(customer.num_items) });
                facility_instance.assigned_weight += cuantos * customer.weight;
                facility_instance.assigned_volume += cuantos * customer.volume;
                facility_instance.total_cost += cuantos * customer.assignment_costs[(*iti)]; // Aquí actualizamos el total_cost
                vector<int> p= { (*iti), copia, cuantos };
                customer.assignment.push_back(p);
                customer.num_items -= cuantos;
                assigned = true;

                for (auto it = facilities_matrix[(*iti)][0].posibles_clientes.begin(); it != facilities_matrix[(*iti)][0].posibles_clientes.end();++it) {
                    if (customers[(*it)].num_items > 0 && dist(rng) >= random_chance) {
                        int other_copy = is_feasible_partial(initial_solution, facilities_matrix, customers[(*it)].index, (*iti));
                        if (other_copy == copia) {
                            Facility& other_facility_instance = facilities_matrix[(*iti)][other_copy];
                            cuantos = std::min({ (other_facility_instance.max_weight - other_facility_instance.assigned_weight) / customers[(*it)].weight,
                            (other_facility_instance.max_volume - other_facility_instance.assigned_volume) / customers[(*it)].volume,
                            static_cast<double>(customers[(*it)].num_items) });
                            other_facility_instance.assigned_weight += cuantos * customers[(*it)].weight;
                            other_facility_instance.assigned_volume += cuantos * customers[(*it)].volume;
                            other_facility_instance.total_cost += cuantos * customer.assignment_costs[(*iti)];
                            vector<int> p= { (*it), copia, cuantos };
                            customers[(*it)].assignment.push_back(p);
                            customers[(*it)].num_items -= cuantos;
                        }
                    }
                }
            }
        }
        if (!assigned) 
        {
           throw runtime_error("No se pudo encontrar una solución inicial factible para el cliente.");
        }
    }

    return initial_solution;
}
void CleanAsignacion(vector<Customer>& customers,vector<vector<Facility>>& facilities_matrix)
{
    for (int i = 0; i < customers.size(); ++i)
    {
        customers[i].assignment.clear();
        customers[i].num_items=customers[i].num_items_iniciales;
    }
    for (int i = 0; i < facilities_matrix.size(); ++i) {
        for (int j = 0; j < facilities_matrix[i].size(); ++j) {


            facilities_matrix[i][j].assigned_weight = 0;
            facilities_matrix[i][j].assigned_volume = 0;
            facilities_matrix[i][j].total_cost = 0;
        }
    }

}
vector<Customer> generate_multiple_solution(int MAX_NUM, vector<Customer>& customers, vector<vector<Facility>>& facilities_matrix,  int random_chance) {
    double min_cost = numeric_limits<double>::max();
    vector<Customer> best_solution;
        for (int i = 0; i < MAX_NUM; ++i) {
            try {

                vector<Customer> solution = generate_initial_solution(customers, facilities_matrix, random_chance);
                double cost = solution_cost(facilities_matrix);

                if (cost < min_cost) {
                    min_cost = cost;
                    best_solution = solution;
                }
                CleanAsignacion(customers, facilities_matrix);
            }
            catch (const runtime_error& e) {
                cerr << "Error al generar solución " << i << ": " << e.what() << endl;
            }
        }

    return best_solution;
}








int TabuAdaptativo(string dir, parameters param, Subproblema* Sub)
{
    int num_facilities = Sub->lista_camiones.size();
    int m = Sub->lista_items.size();

    vector<Customer> customers;
    vector<vector<Facility>> facilities_matrix(num_facilities, vector<Facility>(1));
    vector<vector<bool>> feasibility;
    vector<int> initial_assignment;


    customers.resize(m);
    feasibility.resize(m, vector<bool>(num_facilities));
    // ... (Leer datos, generar solución inicial, etc.)
    for (auto i = 0; i < m; i++)
    {
        customers[i].assignment_costs.resize(num_facilities);
    }
    //Customers
    for (auto it = Sub->lista_items.begin(); it != Sub->lista_items.end(); it++)
    {
        for (auto ic = (*it)->camiones.begin(); ic != (*it)->camiones.end(); ic++)
        {
            int days = Days_0001((*it)->latest_arrival_tm_ptr.tm_year, (*it)->latest_arrival_tm_ptr.tm_mon + 1, (*it)->latest_arrival_tm_ptr.tm_mday) -
                Days_0001((*ic)->arrival_time_tm_ptr.tm_year, (*ic)->arrival_time_tm_ptr.tm_mon + 1, (*ic)->arrival_time_tm_ptr.tm_mday);

            customers[(*it)->CodIdCC].posibles_instalaciones.push_back((*ic)->CodIdCC);
            customers[(*it)->CodIdCC].assignment_costs[(*ic)->CodIdCC]=(*it)->inventory_cost* param.inventory_cost* days;
        }
        customers[(*it)->CodIdCC].index = (*it)->CodIdCC;
        customers[(*it)->CodIdCC].start_day = (*it)->earliest_arrival;
        customers[(*it)->CodIdCC].end_day = (*it)->latest_arrival;
        customers[(*it)->CodIdCC].volume = (*it)->volumen;
        customers[(*it)->CodIdCC].weight = (*it)->weight;
        customers[(*it)->CodIdCC].num_items = (*it)->num_items;
        customers[(*it)->CodIdCC].num_items_iniciales = (*it)->num_items;
    }
    for (auto it = Sub->lista_camiones.begin(); it != Sub->lista_camiones.end(); it++)
    {
        facilities_matrix[(*it)->CodIdCC].resize(((*it)->cota_extra) + 4);
        for (auto it2 = (*it)->items.begin(); it2 != (*it)->items.end(); it2++)
        {
            facilities_matrix[(*it)->CodIdCC][0].posibles_clientes.push_back((*it2)->CodIdCC);
        }

        for (int k = 0; k < (((*it)->cota_extra) + 4); k++)
        {
            facilities_matrix[(*it)->CodIdCC][k].index = (*it)->CodIdCC;
            if (k == 0)
                facilities_matrix[(*it)->CodIdCC][k].fixed_cost = (*it)->cost * param.transportation_cost;
            else
                facilities_matrix[(*it)->CodIdCC][k].fixed_cost = (*it)->cost * param.transportation_cost * (1 + param.extra_truck_cost);
            facilities_matrix[(*it)->CodIdCC][k].max_volume = (*it)->volumen_kp;
            facilities_matrix[(*it)->CodIdCC][k].max_weight = (*it)->peso_inicial;
            facilities_matrix[(*it)->CodIdCC][k].opening_day = (*it)->arrival_time;
        }

    }

 

    vector<Customer> best_initial_solution = generate_multiple_solution(100, customers, facilities_matrix,15);
 //   vector<Customer> initial_solution = generate_initial_solution_optimized_parallel(facilities_matrix, customers, feasibility, num_threads, num_iterations);

    // ... (Mostrar resultados, etc.)
//    double best_cost = solution_cost(best_initial_solution);

 //   cout << "Costo de la solución: " << best_cost << endl;
 
    return 0;
}


