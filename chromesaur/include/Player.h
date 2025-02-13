#ifndef PLAYER_H
#define PLAYER_H

#include "EventHandler.h"
#include "Level.h"

class Player {
private:
    MainWindow *gameWindow; // Ventana de juego
    EventHandler *gameEvent; // Objeto de gestión de eventos
    Music jumpSound; // Sonido de salto
    unordered_map<string, SpriteSheet*> *gameSheets; // Lista de todas las plantillas del juego
    float xPos, yPos, xVel, yVel; // Posición y velocidad del jugador
    float jumpPower; // Fuerza de salto
    float jetPower; // Fuerza del jetpack
    float gravityForce; // Fuerza de la gravedad
	int collisionMs; // Tiempo transcurrido hasta que se colisionó con un obstáculo mortal
    int jetpackMs; // Tiempo transcurrido hasta que se obtuvo un jetpack
    int jetTime; // Tiempo de duración del jetpack
    int coinsValue; // Valor de las monedas en puntaje
    bool isAlive; // Verdadero si el jugador sige vivo
    bool isGrounded; // Verdadero si el jugador está en el suelo
    bool isCrouched; // Verdadero si el jugador está agachado
    bool jetpack; // Verdadero si el jugador tiene un jetpack activo
    bool godMode; // Modo invencibilidad
public:
    Player(MainWindow *window, EventHandler *event, Setting *configFile, unordered_map<string, SpriteSheet*> *sheets);
    void update_state(Level *currentLevel, bool sound=true);
    void handle_collisions(Level *currentLevel);
    void render();
    void forward();
    void back();
    void reset();
    bool is_alive();
    Box get_box();
};

#endif
