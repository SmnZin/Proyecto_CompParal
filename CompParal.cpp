#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>
#include <string>
#include <ctime>
#include <map>
#include <set>
#include <sstream>
#include <regex>
#include <unordered_map>

using namespace std;

bool is_valid_date(const string &date_str) {
    static regex date_pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d+)"); // expresión regular para fecha y hora en formato ISO 8601
    static regex date_pattern_partial(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})"); // expresión regular para fecha y hora en formato ISO 8601 sin milisegundos
    return regex_match(date_str, date_pattern) || regex_match(date_str, date_pattern_partial);
}

void process_file(const string &filename, int max_lines, map<string, set<string>> &data_by_month) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo" << endl;
        return;
    }

    vector<string> lines;
    string line;
    const size_t LINES_PER_BATCH = 1000;
    size_t total_lines_read = 0;

    // omite el encabezado
    getline(file, line);
    cout << "Encabezado: " << line << endl;

    clock_t start = clock(); // Inicio del cronómetro

    // Lee el archivo
    while (getline(file, line)) {
        lines.push_back(line);
        total_lines_read++;

        if (lines.size() >= LINES_PER_BATCH) {
            // Procesa el lote de líneas en paralelo
            #pragma omp parallel // Inicia la región paralela
            {
                map<string, set<string>> local_data_by_month; // datos locales por mes

                #pragma omp for nowait// for sin espera para dividir el trabajo entre los hilos
                for (size_t i = 0; i < lines.size(); i++) {
                    stringstream ss(lines[i]);
                    string cell;
                    vector<string> parsedRow;

                    // parsea la línea
                    while (getline(ss, cell, ';')) {
                        parsedRow.push_back(cell);
                    }

                    // Extrae y procesa la fecha
                    if (parsedRow.size() > 0) {
                        string date_str = parsedRow[0];

                        // Eliminar comillas
                        if (!date_str.empty() && date_str[0] == '"')
                            date_str = date_str.substr(1, date_str.size() - 2);

                        if (is_valid_date(date_str)) {
                            string year = date_str.substr(0, 4);
                            string month = date_str.substr(5, 2);

                            string month_year = year + "-" + month;
                            local_data_by_month[month_year].insert(lines[i]);
                        } else {
                            #pragma omp critical
                            {
                                cout << "Linea invalida: " << lines[i] << endl;
                            }
                        }
                    }
                }

                // Combina los resultados locales en el mapa global
                #pragma omp critical
                {
                    for (const auto &entry : local_data_by_month) {
                        data_by_month[entry.first].insert(entry.second.begin(), entry.second.end());
                    }
                }
            }

            lines.clear(); // limpia el vector para la siguiente lectura
        }

        // Detener la lectura si se ha alcanzado el número máximo de líneas especificado
        if (max_lines > 0 && total_lines_read >= max_lines) {
            break;
        }
    }

    // Procesa las líneas restantes en paralelo
    #pragma omp parallel
    {
        map<string, set<string>> local_data_by_month;

        #pragma omp for nowait
        for (size_t i = 0; i < lines.size(); i++) {
            stringstream ss(lines[i]);
            string cell;
            vector<string> parsedRow;

            // parsea la línea
            while (getline(ss, cell, ';')) {
                parsedRow.push_back(cell);
            }

            // Extrae y procesa la fecha
            if (parsedRow.size() > 0) {
                string date_str = parsedRow[0];

                // Eliminar comillas
                if (!date_str.empty() && date_str[0] == '"')
                    date_str = date_str.substr(1, date_str.size() - 2);

                if (is_valid_date(date_str)) {
                    string year = date_str.substr(0, 4);
                    string month = date_str.substr(5, 2);

                    string month_year = year + "-" + month;
                    local_data_by_month[month_year].insert(lines[i]);
                } else {
                    #pragma omp critical
                    {
                        cout << "Linea invalida: " << lines[i] << endl;
                    }
                }
            }
        }

        // Combina los resultados locales en el mapa global
        #pragma omp critical
        {
            for (const auto &entry : local_data_by_month) {
                data_by_month[entry.first].insert(entry.second.begin(), entry.second.end());
            }
        }
    }

    clock_t stop = clock(); // Fin del cronómetro
    double duration = double(stop - start) / CLOCKS_PER_SEC; // Calcula la duración en segundos

    file.close();

    cout << "Total de lineas (sin contar la primera): " << total_lines_read << endl;
    cout << "Tiempo de lectura y conteo en paralelo: " << duration << " segundos" << endl;
}

void display_grouped_data(const map<string, set<string>> &data_by_month) {
    for (const auto &entry : data_by_month) {
        cout << "Mes-Anho: " << entry.first << " tiene " << entry.second.size() << " entradas." << endl;
    }
}

void display_lines_for_month(const map<string, set<string>> &data_by_month, const string &month_year, int display_limit ) {
    auto it = data_by_month.find(month_year);
    if (it != data_by_month.end()) {
        cout << "Datos para " << month_year << ":" << endl;
        int count = 0;
        for (const string &line : it->second) {
            cout << line << endl;
            count++;
            if ( display_limit != -1 && count >= display_limit ) {
                break;
            }
        }
    } else {
        cout << "No se encontraron datos para el mes-anho especificado." << endl;
    }
}


void display_productos_repetidos(const map<string, set<string>> &data_by_month, const string &month_year, int display_limit) {
    auto it = data_by_month.find(month_year);// esto despues lo tendre que paralelizar para leer todos los meses
    if(it != data_by_month.end()){ // si se encontro el mes
        unordered_map<string, pair<int, vector<string>>> productos_repetidos;
        for (const string &line : it->second) {
            stringstream ss(line);
            string cell;
            vector<string> parsedRow;

            // parsea la línea
            while (getline(ss, cell, ';')) {
                parsedRow.push_back(cell);
            }
            
            if (parsedRow.size() > 6){ // si la linea tiene la columna de sku
                string sku = parsedRow[6];  // sku es la columna 7
                string precio = parsedRow[9]; 
                productos_repetidos[sku].first++;
                productos_repetidos[sku].second.push_back(precio);

            }
        }
        bool hay_repetidos = false;
        cout << "Productos repetidos para " << month_year << ":" << endl;
        int count = 0;
        for (const auto &entry : productos_repetidos) {
            if(entry.second.first > 1){
                cout << "SKU: " << entry.first << " - Repeticiones: " << entry.second.first << " - Precios: ";
                for (const string &precio : entry.second.second) {
                    cout << precio << " ";
                }
                cout << endl;
                hay_repetidos = true;
                count++;
                if ( display_limit != -1 && count >= display_limit ) {
                    break;
                }
            }
           
        }
        if(!hay_repetidos){
            cout << "No hay productos repetidos para el mes-anho especificado." << endl;
        }
        
    }
    else{
        cout << "No se encontraron datos para el mes-anho especificado." << endl;
    }
}
int main() {
    string filename = "pd.csv";
    int opcion;
    int cantidad_datos;
    map<string, set<string>> data_by_month;

    // menú de opciones
    cout << "Menu de opciones" << endl << endl;
    cout << "1. Leer una cantidad especifica de datos" << endl;
    cout << "2. Leer todos los datos" << endl << endl;
    cout << "Ingrese la opción: ";
    cin >> opcion;

    switch (opcion) {
        case 1:
            cout << "Ingrese la cantidad de datos a leer: ";
            cin >> cantidad_datos;
            process_file(filename, cantidad_datos, data_by_month);
            break;
        case 2:
            process_file(filename, -1, data_by_month); // leer todos los datos
            break;
        default:
            cout << "Opcion no valida." << endl;
            return 1;
    }

    display_grouped_data(data_by_month);

    string selected_month_year;
    cout << "Ingrese el mes-anho para ver los datos (yyyy-mm): ";
    cin >> selected_month_year;

    int display_limit;
    cout << "Ingrese el limite de lineas a mostrar (-1 para mostrar todas): ";
    cin >> display_limit;
    display_productos_repetidos(data_by_month, selected_month_year, display_limit);

    

    // display_lines_for_month(data_by_month, selected_month_year, display_limit);

    return 0;
}
