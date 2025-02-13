#ifndef LEVEL_H
#define LEVEL_H

#include "SpriteSheet.h"

// Tipo de dato que representa un rectangulo (posición y medidas)
struct Box { int x, y, w, h; };
// Tipo de dato que representa un obstáculo
// Datos:
//    - Tipo de obstaculo. Determina la plantilla (type)
//    - Variante del obstaculo. Determina el sprite (variant)
//    - Posición relativa al suelo (xPos, yPos)
//    - Velocidad horizontal (xVel)
struct Obstacle {
    string type;
    int variant;
    float xPos, yPos, xVel;
};

class Level {
private:
    MainWindow *gameWindow; // Ventana de juego
    unordered_map<string, SpriteSheet*> *gameSheets; // Lista de todas las plantillas
    vector<Obstacle> levelObstacles; // Lista inicial de obstáculos del nivel (referencia para el reset)
    vector<Obstacle> dynamicObstacles; // Lista variable de obstáculos del nivel
    Clock levelClock; // Reloj del nivel
    float obstacleOffset, groundOffset, backgroundOffset, cloudOffset; // Desplazamientos horizontales de los elementos del mapa
    float levelSpeed; // Velocidad de desplazamiento horizontal
    int number; // Número del nivel
    int score; // Puntuación ajustada al reloj
    int extraScore; // Puntuación extra que se suma al score (por las monedas obtenidas)
    int finishMs; // Tiempo transcurrido hasta que se finalizó el nivel
    bool finished; // Verdadero si el nivel fue finalizado
public:
    // Estas 2 variables deben poder modificarse y leerse desde otras clases
    bool loaded; // Verdadero indica que se cargaron las plantillas para el nivel
    Color background; // Color de fondo del nivel

    Level(int n, MainWindow *window, unordered_map<string, SpriteSheet*> *sheets);
    bool collide(string type, Box player, bool destroy=false, int gap=0);
    void add_score(int score);
    void set_speed(int speed);
    void render_obstacles();
    void render_background();
    void render_front();
    void render_ground();
    void render_clouds();
    void update_score();
    void restart();
    bool is_finished();
    int get_score();
    int get_number();
    int get_level_time();
    int get_finish_time();
};

#endif
