#include "Game.h"

// Constructor que inicializa las variables y carga los archivos necesarios
Game::Game() {
    // Construcción de los objetos necesarios (configuración, ventana, menú, jugador)
    configFile = new Setting("game/settings.cfg");
    window = new MainWindow(configFile);
    menu = new Menu(window, &event, configFile, &gameSheets, &gameLevels);
    dino = new Player(window, &event, configFile, &gameSheets);
    // Carga de plantillas de sprites al vector gameSheets
    menu->load_sheet(new SpriteSheet("res/splash.bmp"));
    menu->load_sheet(new SpriteSheet("res/death_screen.bmp"));
    menu->load_sheet(new SpriteSheet("res/finish_screen.bmp"));
    menu->load_sheet(new SpriteSheet("res/dino_run.bmp", 21, 23));
    menu->load_sheet(new SpriteSheet("res/dino_crouch.bmp", 34, 15));
    menu->load_sheet(new SpriteSheet("res/font.bmp", 7, 6));
    menu->load_sheet(new SpriteSheet("res/coin.bmp", 7, 9));
    menu->load_sheet(new SpriteSheet("res/missile.bmp", 25, 13));
    menu->load_sheet(new SpriteSheet("res/jetpack.bmp"));
    menu->load_sheet(new SpriteSheet("res/jetpack_equiped.bmp", 21, 23));
    menu->load_sheet(new SpriteSheet("res/pipe.bmp", 29, 20));
    menu->load_sheet(new SpriteSheet("res/pipe_top.bmp", 29, 12));
    // Carga de los niveles al vector gameLevels
    for (int n=1; n<=configFile->get_value("Levels"); n++) { 
        gameLevels.push_back(new Level(n, window, &gameSheets)); 
        highscores.push_back(0);
    } menu->set_level(configFile->get_value("Start Level"));
}
// Método para actualizar todos los elementos de un nivel
// Argumento: Número de nivel (number)
void Game::play_level(int number) {
    Level *currentLevel = gameLevels[number-1];
    // Gestión de la velocidad del nivel
    if (currentLevel->get_score()<3 || currentLevel->is_finished() || !dino->is_alive()) { currentLevel->set_speed(0); }
    else { currentLevel->set_speed(2); }
    // Renderización de los elementos del nivel
    window->clear(gameLevels[number-1]->background);
    gameLevels[number-1]->render_clouds();
    gameLevels[number-1]->render_background();
    gameLevels[number-1]->render_ground();
    if (!gameLevels[number-1]->is_finished()) { gameLevels[number-1]->render_obstacles(); }
    render_player();
    gameLevels[number-1]->render_front();
    render_hud();
} 
// Método para actualizar la puntuación máxima obtenida dentro del registro binario
void Game::update_highscore(int number) {
    int currentScore, lastHighscore;
    // Obtención del highscore guardado (Si no hay highscores se guardan como 0)
    fstream file("game/highscores.dat", ios::binary | ios::in | ios::out | ios::ate);
    if (!file.is_open()) { throw runtime_error("Error al abrir highscores.dat"); }
    if (file.tellg() == 0) {
        int x = 0;
        for (int i=0; i<gameLevels.size(); i++) { file.write(reinterpret_cast<char*>(&x), sizeof(int)); } 
    } file.seekg((number-1)*sizeof(int));
    file.read(reinterpret_cast<char*>(&lastHighscore), sizeof(int));
    highscores[number-1] = lastHighscore;
    // Sustitución del highscore en caso de ser mayor al anterior
    currentScore = gameLevels[number-1]->get_score();
    if (currentScore>lastHighscore) {
        file.seekp((number-1)*sizeof(int));
        file.write(reinterpret_cast<char*>(&currentScore), sizeof(int));
        highscores[number-1] = currentScore;
    } file.close();
}
// Método para renderizar al jugador
void Game::render_player() {
    // Actualización y renderización del jugador
    dino->update_state(gameLevels[menu->get_current_level()-1], menu->get_sound_state());
    dino->render();
    // Si el nivel terminó, el jugador camina hacia la derecha hasta salir de la pantalla
    if (gameLevels[menu->get_current_level()-1]->is_finished()) { dino->forward(); }
} 
// Método para renderizar la información en pantalla y los mensajes del juego
void Game::render_hud() {
    Level *currentLevel = gameLevels[menu->get_current_level()-1];
    float gameWidth = window->get_game_size().x;
    float gameHeight = window->get_game_size().y;
    // Texto que indica el nivel actual
    menu->render_text("nivel "+to_string(menu->get_current_level()), {7, 7});
    // Texto que indica la puntuación actual (Los primeros 3 puntos se utilizan como cuenta regresiva)
    if (currentLevel->get_score()<3) { menu->render_text(to_string(3-currentLevel->get_score()), {gameWidth/2-3, gameHeight/2-3}); }
    else { menu->render_text("score "+to_string(currentLevel->get_score()), {gameWidth-70, 7}); }
    // Si el jugador está muerto se muestran opciones (reintentar, volver al menú)
    if (!dino->is_alive()) {
		string scoreText = "puntaje obtenido "+to_string(currentLevel->get_score());
		menu->render_text(scoreText, {gameWidth/2-scoreText.length()*7/2, gameHeight/3});
		gameSheets["death_screen"]->render(window, {gameWidth/2-scoreText.length()*7/2, gameHeight/3+14});
	} // Si el jugador está vivo y el nivel no finalizó, la puntuación sigue incrementando
    else if (!currentLevel->is_finished()) { currentLevel->update_score(); } 
    // Si pasaron 3 segundos desde que el jugador finalizó el nivel, se muestran opciones (continuar, reintentar, volver al menú)
    if (gameLevels[menu->get_current_level()-1]->is_finished() && currentLevel->get_level_time()-currentLevel->get_finish_time()>3000) {
        update_highscore(menu->get_current_level());
        string scoreText = "puntaje obtenido "+to_string(currentLevel->get_score());
        string highscoreText = "mejor puntaje "+to_string(highscores[menu->get_current_level()-1]);
		menu->render_text(scoreText, {gameWidth/2-scoreText.length()*7/2, gameHeight/3});
        menu->render_text(highscoreText, {gameWidth/2-scoreText.length()*7/2, gameHeight/3+9});
		gameSheets["finish_screen"]->render(window, {gameWidth/2-scoreText.length()*7/2, gameHeight/3+23});
    }
} 
// Método de bucle principal donde se actualizan todos los elementos de juego constantemente
void Game::main_loop() {
    while(true) {
        // Si el menú está activo; se renderiza y actualiza
        if (menu->in_menu()) { menu->show(); }
        else {
            Level *currentLevel = gameLevels[menu->get_current_level()-1];
            // Si el jugador está vivo y el nivel finalizó se procesan las opciones
            if (dino->is_alive() && currentLevel->is_finished()) {
                int levelMs = currentLevel->get_level_time();
                int finishMs = currentLevel->get_finish_time();
                if (levelMs-finishMs>3000) {
                    // Si se presiona la flecha hacia arriba se pasa al siguiente nivel
                    if (event.key_press(Keyboard::Up)) {
                        dino->reset();
                        if (menu->get_current_level()<gameLevels.size()) { 
                            menu->set_level(menu->get_current_level()+1);
                            dino->back();
                        } // Si se presiona la flecha hacia abajo se reinicia el nivel actual
                    } else if (event.key_press(Keyboard::Down)) {
                        dino->reset();
                        dino->back();
                        currentLevel->restart();
                        // Si se presiona enter se vuelve al menú principal
                    } else if (event.key_press(Keyboard::Enter)) { 
                        dino->reset();
                        dino->back();
                        currentLevel->restart();
                        menu->call(); 
                    }
                }
            } // Si el jugador está muerto y se presiona enter, se vuelve al menú (La opción de reintentar se procesa en Player.cpp)
            else if (!dino->is_alive() && event.key_press(Keyboard::Enter)) { 
                dino->reset();
                dino->back();
                currentLevel->restart();
                menu->call();  
            } play_level(menu->get_current_level());
        } event.update(window);
        window->render();
    }
}
// Destructor para limpiar las variables almacenadas en el heap
Game::~Game() { 
    // ".. Por cada new va un delete .."
    for (const auto& pair : gameSheets) { delete pair.second; }
    for (Level *l : gameLevels) { delete l; }
    delete menu;
    delete dino;
    delete window;
    delete configFile;
}
