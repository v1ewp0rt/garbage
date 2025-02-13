#include "MainWindow.h"

// Constructor que inicializa las variables
// Argumento: Archivo de configuración del juego (configFile)
MainWindow::MainWindow(Setting *configFile) {
    gameWidth = 320; gameHeight = 180;
    renderScale = configFile->get_value("Resolution Scale");
    width = gameWidth*renderScale;
    height = gameHeight*renderScale;
    window = new RenderWindow(VideoMode(width, height), title, Style::Close);
    renderer = new View(FloatRect(0, 0, gameWidth, gameHeight));
    window->setFramerateLimit(configFile->get_value("Framerate"));
    window->setView(*renderer);
}
// Método para rellenar la ventana con un color específico
// Argumento: Color de relleno (color)
void MainWindow::clear(Color color = Color::Black) { window->clear(color); }
// Método para renderizar el contenido de la ventana
void MainWindow::render() { window->display(); }
// Método para obtener las medidas del sector de juego
Vector2i MainWindow::get_game_size() { return Vector2i(gameWidth, gameHeight); }
// Método para obtener el objeto ventana
RenderWindow* MainWindow::get_window() { return window; }
// Método para obtener el objeto vista
View* MainWindow::get_renderer() { return renderer; }
// Método para obtener el objeto evento
Event* MainWindow::get_event() { return &event; }
// Destructor para eliminar la variable del heap
MainWindow::~MainWindow() { delete window; }