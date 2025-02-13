#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include "MainWindow.h"

class SpriteSheet {
private:
    Texture sheetTexture; // Plantilla completa
    Sprite currentSprite; // Recorte de la plantilla (sprite)
    Clock animationClock; // Reloj para la animación
    string filename; // Nombre y ubicación del archivo
    int spriteWidth, spriteHeight, sheetWidth, sheetHeight; // Medidas de la plantilla y el sprite
    int frameRate, totalFrames, currentFrame; // Parámetros de la animación
public:    
    SpriteSheet(string fN, int sW=0, int sH=0, int cF=0, int fR=10);
    void render(MainWindow *window, Vector2f pos, Color color=Color::White);
     void set_frame(int frame);
    void update_animation();
    string get_filename();
    Vector2i get_sheet_size();
    Vector2i get_sprite_size();
};

#endif