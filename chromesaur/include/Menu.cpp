#include "Menu.h"

// Constructor que inicializa las variables
// Argumentos:
//    - Ventana de juego (window)
//    - Objeto de gestión de eventos (event)
//    - Archivo de configuración (cF)
//    - Lista de plantillas del juego (sheets)
//    - Lista de niveles del juego (levels)
Menu::Menu(MainWindow *window, EventHandler *event, Setting *cF, unordered_map<string, SpriteSheet*> *sheets, vector<Level*> *levels) {
    // Asignación de objetos necesarios y valores iniciales
    gameWindow = window;
    gameEvent = event;
    configFile = cF;
    gameSheets = sheets;
    gameLevels = levels;
    currentLevel = 1;
    selected = 0;
    inMain = true;
    inSettings = false;
    inLevels = false;
    // Carga de configuraciones
    resolutionScale = configFile->get_value("Resolution Scale");
    soundState = configFile->get_value("Sound");
    // Carga de archivos de sonido y musica
    if (!menuSong.openFromFile("./res/menu.ogg")) { throw runtime_error("Error al cargar ./res/menu.ogg"); }
    if (!selectSound.openFromFile("./res/select.ogg")) { throw runtime_error("Error al cargar ./res/select.ogg"); }
    if (!enterSound.openFromFile("./res/enter.ogg")) { throw runtime_error("Error al cargar ./res/enter.ogg"); }
    selectSound.setVolume(soundState*100);
    enterSound.setVolume(soundState*100);
    menuSong.setVolume(soundState*100);
    menuSong.play();
}
// Método para renderizar texto en la pantalla
// Argumentos:
//    - Texto a renderizar (text)
//    - Posición en pantalla (pos)
//    - Color del texto (color)
void Menu::render_text(string text, Vector2f pos, Color color) {
    string characters = "abcdefghijklmnopqrstuvwxyz0123456789,.";
    for (int a=0; a<text.length(); a++) {
        for (int b=0; b<characters.length(); b++) {
            if (text[a]==characters[b]) { 
                (*gameSheets)["font"]->set_frame(b);
                (*gameSheets)["font"]->render(gameWindow, {pos.x+a*7, pos.y}, color);
            }
        }
    }
} 
// Método para cargar una plantilla nueva a la lista de plantillas
// Argumento: Plantilla a cargar (newSheet)
void Menu::load_sheet(SpriteSheet *newSheet) {
    // Utiliza el nombre del archivo sin la extensión como clave para la lista 
    string fN = newSheet->get_filename();
    string key = fN.substr(fN.find('/')+1, fN.length()-fN.find('/')-5);
    (*gameSheets)[key] = newSheet; 
}
// Método para cambiar el nivel actual
// Argumento: Número del nivel (number)
void Menu::set_level(int number) { 
    // Si no estaba cargado el nivel entonces lo carga
    if (!(*gameLevels)[number-1]->loaded) { load_level_assets(number); }
    currentLevel = number; 
    (*gameLevels)[currentLevel-1]->restart();
}
// Método para cargar las plantillas necesarias para un nivel
// Argumento: Número del nivel (number)
void Menu::load_level_assets(int number) {
    load_sheet(new SpriteSheet("res/ground_"+to_string(number)+".bmp"));
    load_sheet(new SpriteSheet("res/background_"+to_string(number)+".bmp"));
    load_sheet(new SpriteSheet("res/cloud_"+to_string(number)+".bmp", 53, 14));
    load_sheet(new SpriteSheet("res/ground_obstacle_"+to_string(number)+".bmp", 11, 16));
    load_sheet(new SpriteSheet("res/hole_"+to_string(number)+".bmp", 60, 10));
    load_sheet(new SpriteSheet("res/hole_mask_"+to_string(number)+".bmp", 60, 50));
    (*gameLevels)[number-1]->loaded = true; // Indica que el nivel fue cargado
}
// Método para mostrar y actualizar el menú
void Menu::show() {
    vector<string> options;
    currentMs = menuClock.getElapsedTime().asMilliseconds();
    // Asigna las opciones para el menú principal
    if (inMain) { options.assign({"nuevo juego", "niveles", "configuracion", "salir"}); }
    // Asigna las opciones para el menú de niveles
    if (inLevels) {
        for (int i=0; i<configFile->get_value("Levels"); i++) { options.push_back("nivel "+to_string(i+1)); }
        options.push_back("volver");
    } // Asigna las opciones para el menú de configuración
    if (inSettings) { 
        string state = soundState?"activado":"desactivado";
        options.push_back("resolucion "+to_string(resolutionScale*gameWindow->get_game_size().x)+"x"+to_string(resolutionScale*gameWindow->get_game_size().y)); 
        options.push_back("sonido "+state); 
        options.push_back("aplicar"); 
        options.push_back("volver"); 
    } // Renderiza la imagen de fondo y las opciones
    (*gameSheets)["splash"]->render(gameWindow, {0, 0});
    for (int i=0; i<options.size(); i++) { render_text(options[i], {21, 40+i*20}, i==selected?Color::Cyan:Color::White); } 
    // Las flechas direccionales se utilizan para seleccionar las opciones
    if (gameEvent->key_press(Keyboard::Up) && currentMs-selectionMs>100 && selected>0) { 
        selected--; 
        selectSound.play();
        selectionMs = currentMs;
    } if (gameEvent->key_press(Keyboard::Down) && currentMs-selectionMs>100 && selected<options.size()-1) { 
        selected++; 
        selectSound.play();
        selectionMs = currentMs;
    } // Lo que haga el enter depende de el menú en el que esté el jugador y la opcion que seleccione
    if (gameEvent->key_press(Keyboard::Enter) && currentMs-selectionMs>250) { 
        if (inMain) {
            if (selected==0) { 
                set_level(1);
                menuSong.stop(); 
            } inMain = false; 
            inLevels = selected==1;
            inSettings = selected==2;
            if (selected==3) { gameWindow->get_window()->close(); exit(0); }
            selected = 0;
        } else if (inLevels) {
            if (selected==options.size()-1) { inMain = true; }
            else { 
                set_level(selected+1); 
                menuSong.stop();
            } inLevels = false;
            selected = 0;
        } else if (inSettings) {
            if (selected==0) { 
                resolutionScale++;
                resolutionScale>6?resolutionScale=3:resolutionScale;
            } else if (selected==1) { soundState = !soundState; }
            else if (selected==2) {
                // Se aplican las configuraciones en el momento
                configFile->set_value("Resolution Scale", resolutionScale);
                configFile->set_value("Sound", soundState);
                gameWindow->get_window()->setSize({gameWindow->get_game_size().x*resolutionScale, gameWindow->get_game_size().y*resolutionScale});
                selectSound.setVolume(soundState*100);
                enterSound.setVolume(soundState*100);
                menuSong.setVolume(soundState*100);
            } else if (selected==3) { 
                inMain = true; 
                inSettings = false; 
                selected = 0;
            }
        } enterSound.play();
        selectionMs = currentMs;
    }
}
// Método para llamar al menú principal
void Menu::call() { 
    inMain = true; 
    selectionMs = menuClock.getElapsedTime().asMilliseconds();
    menuSong.play();
}
// Método para comprobar si el jugador está en el menú
bool Menu::in_menu() { return inMain || inSettings || inLevels; }
// Método para obtener el número de nivel actual
int Menu::get_current_level() { return currentLevel; }
// Método para obtener el estado actual del sonido
int Menu::get_sound_state() { return soundState; }