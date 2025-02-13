#ifndef MENU_H
#define MENU_H

#include "EventHandler.h"
#include "Level.h"
#include <SFML/Audio.hpp>

class Menu {
private:
    MainWindow *gameWindow; // Ventana del juego
    EventHandler *gameEvent; // Objeto de gestión de eventos
    Setting *configFile; // Archivo de configurración
    unordered_map<string, SpriteSheet*> *gameSheets; // Lista de todas las plantillas
    vector<Level*> *gameLevels; // Lista de todos los niveles
    Clock menuClock; // Reloj del menú
    Music selectSound; // Sonido de selección
    Music enterSound; // Sonido de confirmación
    Music menuSong; // Música del menú
    int currentLevel; // Número de nivel actual
    int resolutionScale; // Coeficiente de resolución actual
    int selected; // Opción seleccionada
    int currentMs; // Tiempo transcurrido en el menú (milisegundos)
    int selectionMs; // Tiempo transcurrido hasta un cambio de selección en el menú (milisegundos)
    bool soundState; // Verdadero si el sonido está activado
    bool inMain; // Verdadero si el jugador está en el menú principal
    bool inSettings; // Verdadero si el jugador está en configuración
    bool inLevels; // Verdadero si el jugador está en la selección de nivel
public:
    Menu(MainWindow *window, EventHandler *event, Setting *cF, unordered_map<string, SpriteSheet*> *sheets, vector<Level*> *levels);
    void render_text(string text, Vector2f pos, Color color=Color::White);
    void load_sheet(SpriteSheet *newSheet);
    void set_level(int number);
    void load_level_assets(int number);
    void show();
    void call();
    bool in_menu();
    int get_current_level();
    int get_sound_state();
};

#endif
