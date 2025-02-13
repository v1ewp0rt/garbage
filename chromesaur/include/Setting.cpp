#include "Setting.h"

// Constructor que carga los parámetros de configuración desde un archivo de texto hacia la lista
// Argumento: Ubicación y nombre del archivo (path)
Setting::Setting(string path) {
    ifstream file(path);
    if (!file.is_open()) { throw runtime_error("Error al leer archivo de configuración"); }
    string line;
    while (getline(file, line)) {
        if (line[0]=='#' || line[0]==' ') { continue; }
        string tag = line.substr(0, line.find(": "));
        values[tag] = stoi(line.substr(line.find(": ")+2));
    } file.close();
} 
// Método para cambiar uno de los datos dentro de un par utilizando la etiqueta
// Argumentos:
//    - Etiqueta del par (tag)
//    - Valor a establecer (value)
void Setting::set_value(string tag, int value) {
    values[tag] = value;
    ofstream file("./game/settings.cfg", ios::trunc);
    for (const auto& value : values) { file << value.first << ": " << value.second << '\n'; }
    file.close();
}
// Método para obtener un parámetro específico 
// Argumento: Etiqueta del parámetro (tag)
int Setting::get_value(string tag) { return values[tag]; } 
