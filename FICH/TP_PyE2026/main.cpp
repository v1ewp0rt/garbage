/* Lucas Gutierrez - Ingeniería en Inteligencia Artificial
Probabilidad y Estadística 2026
"La predictibilidad de nuestras contraseñas"

Este script genera archivos .dat pensados para ser graficados
con gnuplot. Además de algunas tablas .csv con valores calculados */

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <array>
#include <unordered_map>
#include <cmath>
#include <algorithm>

using namespace std;

const uint8_t charCount = 94; // Número de caracteres contemplados
const uint32_t lineCount = 100000; // Cada archivo del dataset tiene 100.000 lineas
uint32_t totalValues;
uint16_t totalFiles, valuesPerFile;
string srcPath, dstPath;
unordered_map<char, uint8_t> charIndex;

// Carga las contraseñas equidistantemente desde un archivo
void add_entries(ifstream* s_file, vector<string>* d_vec) {
    string line;
    int counter = 0;
    uint32_t padding = lineCount/valuesPerFile;

    while (getline(*s_file, line)) {
        if (counter%padding!=0) { counter++; continue; }
        int index = line.find(':');
        if (index==string::npos) { continue; }
        string pass = line.substr(index+1, line.length()-index-2);
        if (pass.length()<2) { continue; }

        bool hashed = pass.length()==32;
        if (hashed) {
            for (char c : pass) { 
                if ((c<'a' || c>'f') && (c<'0' || c>'9')) { hashed = 0; break; }
            } if (hashed) { continue; }
        } d_vec->push_back(pass);
        counter++;
    }
}
// Calcula la probabilidad para cada secuencia de 2 caracteres
void calculate_probabilities(vector<string>* s_entry, array<array<float, charCount>, charCount>* d_arr) {
    for (uint32_t j=0; j<totalValues; j++) {
        string pass = "|"+(*s_entry)[j];
        if (pass.length()<2) { continue; }
        for (uint8_t i=0; i<pass.length()-1; i++) {
            uint8_t aIndex = charIndex[pass[i]];
            uint8_t bIndex = charIndex[pass[i+1]];
            if (aIndex>=charCount || bIndex>=charCount) { continue; }
            (*d_arr)[bIndex][aIndex] += 1.0f;
        }
    } for (uint32_t i=0; i<charCount; i++) {
        float sum = 0;
        for (uint32_t j=0; j<charCount; j++) { sum += (*d_arr)[j][i]; }
        for (uint32_t j=0; j<charCount; j++) { 
            float value = (*d_arr)[j][i];
            (*d_arr)[j][i] = sum==0?0:value/sum;
        } 
    }
}
// Calcula el IPCP para una contraseña
float get_acsi(string s_key, array<array<float, charCount>, charCount>* s_arr) {
    string key = "|"+s_key;
    float score = 0;
    for (int i=0; i<key.size()-1; i++) {
        char current = key[i];
        char next = key[i+1];
        float p = (*s_arr)[charIndex[next]][charIndex[current]];
        if (p>0) { score += -log2(p); }
    } score /= (key.size()-1);
    return score; 
}
// Escribe archivos para el mapa de calor y probabilidades secuenciales
void write_heatmap_file(array<array<float, charCount>, charCount>* d_arr) {
    cout << "mapa-de-calor.dat" << endl;
    ofstream destination("mapa-de-calor.dat");
    for (uint8_t j=0; j<charCount; j++) {
        for (uint8_t i=0; i<charCount; i++) { 
            float value = (*d_arr)[i][j];
            destination << to_string(log10(value+1e-9f)) << " ";
        } destination << endl; 
    } destination.close();

    cout << "matriz-secuencias.csv" << endl;
    destination.open("matriz-secuencias.dat");
    for (uint8_t j=0; j<charCount; j++) {
        for (uint8_t i=0; i<charCount; i++) { 
            destination << to_string((*d_arr)[i][j]) << " ";
        } destination << endl; 
    } destination.close();
}
// Escribe todos los demás archivos (Diagramas, tablas, etc...)
void write_descriptive_files(vector<string>* s_entry, array<array<float, charCount>, charCount>* s_arr) {
    string upper = "QWERTYUIOPASDFGHJKLÑZXCVBNM";
    string lower = "qwertyuiopasdfghjklñzxcvbnm";
    string number = "1234567890";
    string symbol = "!@-_#.,$%&?/\\=({[]})+*^;:<>~`\"\'";
    array<string, 6> variable = {"longitud", "mayus", "minus", "num", "sim", "ipcp"};
    array<vector<uint32_t>, 6> freq; // freq[característica][cantidad] = numero de contraseñas
    vector<array<uint64_t, 5>> value(totalValues); //value[numero de entrada][característica] = cantidad
    vector<pair<string, double>> correlation;
    vector<double> acsi(totalValues);
    ofstream destination;

    double acsiMean = 0;
    double acsiExpectedSquared = 0;
    for (uint32_t j=0; j<totalValues; j++) {
        string& key = (*s_entry)[j];
        array<uint8_t, 5> counter{};
        counter[0] = key.length();
        acsi[j] = get_acsi(key, s_arr);
        acsiMean += acsi[j]; acsiExpectedSquared += acsi[j]*acsi[j];
        for (char c : key) { 
            bool found = 0;
            for (char u : upper) { if (c==u) { counter[1]++; found = 1; break; } }
            if (found) { continue; }
            for (char l : lower) { if (c==l) { counter[2]++; found = 1; break; } } 
            if (found) { continue; }
            for (char n : number) { if (c==n) { counter[3]++; found = 1; break; } }
            if (found) { continue; }
            for (char s : symbol) { if (c==s) { counter[4]++; break; } }
        } for (uint8_t i=0; i<5; i++) {
            value[j][i] = counter[i];
            if (counter[i]>=freq[i].size()) { freq[i].resize(counter[i]+1); }
            freq[i][counter[i]]++;
        }
    } acsiMean /= (double)totalValues;
    acsiExpectedSquared /= (double)totalValues;

    cout << "dataset.csv" << endl;
    destination.open("dataset.csv");
    destination << "Longitud,Cant. Mayús,Cant. Minús,Cant. Num,Cant. Sím,IPCP" << endl;
    for (uint32_t j=0; j<totalValues; j++) {
        for (uint8_t i=0; i<5; i++) { destination << (int)value[j][i] << ","; }
        destination << acsi[j] << endl;
    } destination.close();
    
    for (uint8_t j=0; j<5; j++) {
        string path = variable[j]+"-frecuencia.dat";
        cout << path << endl;
        destination.open(path);
        for (uint8_t i=0; i<freq[j].size(); i++) { destination << (int)i << " " << freq[j][i] << endl; } 
        destination.close();
    }

    for (uint8_t j=0; j<6; j++) {
        string path = variable[j]+"-muestra.dat";
        cout << path << endl;
        destination.open(path);
        for (uint32_t i=0; i<totalValues; i++) {
            if (j==5) { destination << (double)acsi[i] << endl; continue; }
            destination << (int)value[i][j] << endl;
        } destination.close();
    } 
    
    for (uint8_t k=0; k<5; k++) {
        string path = variable[k]+"-x-ipcp.dat";
        cout << path << endl;
        destination.open(path);
        double sumX = 0; double sumY = 0;
        double sumXX = 0; double sumYY = 0;
        double sumXY = 0;

        for (uint32_t i=0; i<totalValues; i++) {
            double x = value[i][k];
            double y = acsi[i];
            sumX += x; sumY += y;
            sumXX += x*x; sumYY += y*y;
            sumXY += x*y;
            destination << value[i][k] << " " << acsi[i] << endl;
        } destination.close();

        double num = totalValues*sumXY-sumX*sumY;
        double den = sqrt((totalValues*sumXX-sumX*sumX)*(totalValues*sumYY-sumY*sumY));
        double r = num/den;
        correlation.push_back({variable[k]+":ipcp", r});
    }

    cout << "correlacion.csv" << endl;
    destination.open("correlacion.csv");
    for (uint8_t i=0; i<correlation.size(); i++) {
        destination << correlation[i].first << "," << correlation[i].second << endl;
    } destination.close();
    
    for (uint8_t j=0; j<5; j++) {
        float mean = 0;
        float expectedSquared = 0;
        bool medianFound = 0;
        uint8_t median = 0;
        uint8_t mode = 0;
        uint64_t sum = 0;
        for (uint32_t i=0; i<freq[j].size(); i++) { 
            sum += freq[j][i];
            mean += i*freq[j][i];
            expectedSquared += i*i*freq[j][i];
            if (!medianFound && sum>=totalValues/2) { median = i; medianFound = 1; }
            if (freq[j][i]>freq[j][mode]) { mode = i; }
        } mean /= (float)totalValues;
        expectedSquared /= (float)totalValues;
        float variance = expectedSquared-mean*mean;
        float cv = (sqrt(variance)/mean)*100;

        string path = variable[j]+"-tabla.csv";
        cout << path << endl;
        destination.open(path);
        destination << "Media," << mean << ",\n";
        destination << "Mediana," << (int)median << ",\n";
        destination << "Moda," << (int)mode << ",\n";
        destination << "Variabilidad," << cv << "%,\n";
        destination.close();
    } 

    sort(acsi.begin(), acsi.end());
    float acsiMedian = totalValues%2==0?(acsi[totalValues/2-1]+acsi[totalValues/2])/2.0f:acsi[totalValues/2];
    float acsiVariance = acsiExpectedSquared-acsiMean*acsiMean;
    float acsiCv = (sqrt(acsiVariance)/acsiMean)*100;

    string path = variable[5]+"-tabla.csv";
    cout << path << endl;
    destination.open(path);
    destination << "Media," << acsiMean << ",\n";
    destination << "Mediana," << acsiMedian << ",\n";
    destination << "Variabilidad," << acsiCv << "%,\n";
    destination << "Mínimo," << acsi[0] << ",\n";
    destination << "Máximo," << acsi[totalValues-1] << ",\n";
    destination.close();
}
// Escribe en un archivo la lista de todas las contraseñas analizadas
void write_selected(vector<string>* s_entry) {
    ofstream destination("seleccionados.txt");
    cout << "seleccionados.txt" << endl;
    for (string key : *s_entry) { destination << key << endl; }
    destination.close();
}
// Funcion principal
int main() {
    totalValues = 230000;
    totalFiles = 230;
    valuesPerFile = totalValues/totalFiles;
    string srcPath = "./dataset/";
    string characters = "|QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890!@-_#.,$%&?/\\=({[]})+*^;:<>~`\"\'";
    vector<string> srcEntry;

    for (uint8_t i=0; i<characters.length(); i++) { charIndex.emplace(characters[i], i); }
    // Carga de contraseñas desde los 230 archivos del dataset
    for (int i=0; i<totalFiles; i++) {
        string srcFile = srcPath+to_string(i)+".txt";
        cout << "Cargando contraseñas de "+srcFile << endl;
        ifstream source(srcFile);
        if (source.is_open()) { add_entries(&source, &srcEntry);  }
        else { cerr << srcFile+": Archivo no encontrado" << endl; }
        source.close();
    } 

    array<array<float, charCount>, charCount> probability{};
    for (array<float, charCount>& row : probability) { row.fill(1.0f); }
    cout << "Calculando probabilidades secuenciales..." << endl;
    calculate_probabilities(&srcEntry, &probability);
    
    // Comenta estas lineas para omitir la generacion de archivos
    cout << "Escribiendo archivos..." << endl;
    write_descriptive_files(&srcEntry, &probability);
    write_heatmap_file(&probability);
    write_selected(&srcEntry);
    cout << "Listo." << endl;

    while (1) {
        string key;
        cout << "Ingrese una contraseña: "; cin >> key;
        cout << "IPCP: " << get_acsi(key, &probability) << endl;
    } return 0;
}