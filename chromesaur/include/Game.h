#ifndef GAME_H
#define GAME_H

#include "Menu.h"
#include "Player.h"

class Game {
private:
    MainWindow *window; // Ventana principal
    EventHandler event; // Objeto de gestión de eventos
    Setting *configFile; // Archivo de configuración
    Menu *menu; // Menú
    Player *dino; // Jugador
    unordered_map<string, SpriteSheet*> gameSheets; // Lista de plantillas
    vector<Level*> gameLevels; // Lista de niveles
    vector<int> highscores; // Lista de puntuaciones
public:
    Game();
    void play_level(int number);
    void update_highscore(int number);
    void render_player();
    void render_hud();
    void main_loop();
    ~Game();
};

#endif
