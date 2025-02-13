#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Setting.h"

class MainWindow {
private:
    RenderWindow *window; // Objeto ventana de SFML
    View *renderer; // Objeto vista de SFML (representa un sector de la ventana)
    Event event; // Objeto evento de SFML (para procesar las entradas de teclado y otros)
    string title; // Nombre de la ventana
    int width, height, gameWidth, gameHeight, renderScale; // Medidas de la ventana y el sector de juego
public:
    MainWindow(Setting *configFile);
    void clear(Color color);
    void render();
    Vector2i get_game_size();
    RenderWindow* get_window();
    View* get_renderer();
    Event* get_event();
    ~MainWindow();
};

#endif