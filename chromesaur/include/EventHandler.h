#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "MainWindow.h"

class EventHandler {
private: bool keyPressed; // Verdadero si cualquier tecla está siendo presionada
public:
    void update(MainWindow *window);
    bool key_press(Keyboard::Key scancode);
}; 

#endif
