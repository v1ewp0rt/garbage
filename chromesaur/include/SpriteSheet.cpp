#include "SpriteSheet.h"

// Constructor que inicializa las variables
// Argumentos del constructor: 
//    - Ubicación y nombre del archivo (fN)
//    - Ancho del sprite (sW)
//    - Alto del sprite (sH)
//    - Frame actual (cF)
//    - Tasa de refresco de animación (fR)
SpriteSheet::SpriteSheet(string fN, int sW, int sH, int cF, int fR) {
    if (!sheetTexture.loadFromFile(fN)) { throw runtime_error("Error al cargar "+fN); }
    filename = fN;
    sheetWidth = sheetTexture.getSize().x;
    sheetHeight = sheetTexture.getSize().y;
    spriteWidth = sW; spriteHeight = sH; frameRate = fR; 
    currentFrame = cF;
    spriteWidth = spriteWidth==0?sheetWidth:spriteWidth;
    spriteHeight = spriteHeight==0?sheetHeight:spriteHeight;
    totalFrames = sheetWidth/spriteWidth;
    currentSprite.setTexture(sheetTexture);
    currentSprite.setTextureRect(IntRect(currentFrame*spriteWidth, 0, spriteWidth, spriteHeight));
}
// Método para renderizar un sprite de la plantilla
// Argumentos:
//    - Ventana donde renderizar
//    - Posición donde renderizar
void SpriteSheet::render(MainWindow *window, Vector2f pos, Color color) {
    currentSprite.setTextureRect(IntRect(currentFrame*spriteWidth, 0, spriteWidth, spriteHeight));
    currentSprite.setColor(Color(color));
    currentSprite.setPosition(pos);
    window->get_window()->draw(currentSprite);
}
// Método para cambiar el frame actual
// Argumento: Número de frame a establecer
void SpriteSheet::set_frame(int frame) { currentFrame = frame; }
// Método para actualizar la animación en base al reloj
void SpriteSheet::update_animation() { currentFrame = (animationClock.getElapsedTime().asMilliseconds()/(1000/frameRate))%totalFrames; }
// Método para obtener el nombre y ubicación del archivo de la plantilla
string SpriteSheet::get_filename() { return filename; }
// Método para obtener las medidas de la plantilla
Vector2i SpriteSheet::get_sheet_size() { return Vector2i(sheetWidth, sheetHeight); }
// Método para obtener las medidas del sprite
Vector2i SpriteSheet::get_sprite_size() { return Vector2i(spriteWidth, spriteHeight); }