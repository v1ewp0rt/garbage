/* Lucas Gutierrez - Ingeniería en Inteligencia Artificial
Probabilidad y Estadística 2026
"La predictibilidad de nuestras contraseñas"

Este script genera 1000 contraseñas a partir de la tabla de 
frecuencias relativas condicionales generada por main.cpp */

#include <iostream>
#include <fstream>
#include <array>
#include <cstdint>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <ctime>

using namespace std;

const uint8_t charCount = 94;
unordered_map<char, uint8_t> charIndex;
string characters = "|QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890!@-_#.,$%&?/\\=({[]})+*^;:<>~`\"\'";

// Carga la matriz-secuencias.dat generada por main.cpp
void load_matrix(array<array<float, charCount>, charCount>* d_arr) {
    ifstream source("matriz-secuencias.dat");
    if (!source.is_open()) { throw runtime_error("matriz-secuencias.dat No encontrado"); }
    for (uint8_t j=0; j<charCount; j++) {
        for (uint8_t i=0; i<charCount; i++) { source >> (*d_arr)[j][i]; }
    }
}
// Predice el siguiente caracter dado un caracter inicial
char getch(char s_char, array<array<float, charCount>, charCount>* s_arr, bool s_max=0) {
    float dice = float(rand()%1000000)/1000000.0f;
    float counter = 0;
    float max = 0;
    uint8_t maxIndex;

    for (uint8_t i=0; i<charCount; i++) {
        float p = (*s_arr)[charIndex[s_char]][i];
        if (s_max) { 
            if (p>max) { max = p; maxIndex = i; }
            continue; 
        } counter += p;
        if (!s_max && dice<counter) { return characters[i]; }
    } if (s_max) { return characters[maxIndex]; } 
    return ' ';
}
// Calcula la IPCP de una contraseña
float get_acsi(string s_key, array<array<float, charCount>, charCount>* s_arr) {
    float score = 0;
    for (int i=0; i<s_key.size()-1; i++) {
        char current = s_key[i];
        char next = s_key[i+1];
        float p = (*s_arr)[charIndex[next]][charIndex[current]];
        if (p>0) { score += -log2(p); }
    } score /= (s_key.size()-1);
    return score; 
}
// Función principal
int main() {
    srand(time(nullptr));
    for (uint8_t i=0; i<characters.length(); i++) { charIndex.emplace(characters[i], i); }
    array<array<float, charCount>, charCount> probability;
    load_matrix(&probability);

    string upper = "QWERTYUIOPASDFGHJKLÑZXCVBNM";
    string lower = "qwertyuiopasdfghjklñzxcvbnm";
    string number = "1234567890";
    string symbol = "!@-_#.,$%&?/\\=({[]})+*^;:<>~`\"\'";
    array<string, 4> variable = {"Mayúsculas", "Minúsculas", "Números", "Símbolos"};
    uint32_t counter = 0;
    array<uint64_t, 4> subCounter{};
    while (counter<1000) { 
        string key = "|";
        for (uint8_t i=0; i<=8; i++) { key += getch(key[key.length()-1], &probability, 0); }
        float acsi = get_acsi(key, &probability);
        key = key.substr(1, key.length()-2);
        if (acsi<4.3f) { // Este es el valor máximo para la IPCP
            for (char c : key) { 
                bool found = 0;
                for (char u : upper) { if (c==u) { subCounter[0]++; found = 1; break; } }
                if (found) { continue; }
                for (char l : lower) { if (c==l) { subCounter[1]++; found = 1; break; } } 
                if (found) { continue; }
                for (char n : number) { if (c==n) { subCounter[2]++; found = 1; break; } }
                if (found) { continue; }
                for (char s : symbol) { if (c==s) { subCounter[3]++; break; } }
            } cout << key << endl;
            counter++;
        }
    } 

    ofstream destination("sub-dataset-ipcp-bajo.csv");
    for (uint8_t i=0; i<4; i++) {
        destination << variable[i] << "," << subCounter[i] << endl;
    } destination.close();
}