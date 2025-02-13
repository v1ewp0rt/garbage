#ifndef SETTING_H
#define SETTING_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

using namespace std;
using namespace sf;

class Setting {
private: unordered_map<string, int> values; // Lista de parámetros numéricos etiquetados
public:
    Setting(string path);
    void set_value(string tag, int value);
    int get_value(string tag);
};

#endif
