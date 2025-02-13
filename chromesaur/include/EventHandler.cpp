#include "EventHandler.h"
#include "MainWindow.h"

// Método para procesar la cola de eventos (se ejecuta a cada frame del juego)
// Argumento: Ventana del juego (window)
void EventHandler::update(MainWindow *window) {
    keyPressed = false;
    Event event = *window->get_event();
    while (window->get_window()->pollEvent(event)) {
        if (event.type == Event::Closed) { window->get_window()->close(); exit(0); }
    }
} 
// Método para obtener el estado de una tecla específica (presionada o no)
// Argumento: Tecla a comprobar (Scancode)
bool EventHandler::key_press(Keyboard::Key scancode) { return Keyboard::isKeyPressed(scancode); }