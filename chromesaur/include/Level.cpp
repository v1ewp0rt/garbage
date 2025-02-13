#include "Game.h"

// Constructor que inicializa las variables y carga los obstáculos desde el archivo del nivel
// Argumentos:
//    - Número del nivel (n)
//    - Ventana del juego (window)
//    - Lista de todas las plantillas (sheets)
Level::Level(int n, MainWindow *window, unordered_map<string, SpriteSheet*> *sheets) {
    // Asignación de las variables a utilizar y sus valores iniciales
    gameWindow = window;
    gameSheets = sheets;
    number = n;
    loaded = false;
    finished = false;
    background = {0, 0, 0, 255};
    restart();
    string path = "game/level_"+to_string(number)+".lvl";
    // Carga de obstáculos del archivo de nivel (game/level_n.lvl)
    ifstream file(path);
    if (!file.is_open()) { throw runtime_error("Error al abrir archivo de nivel "+path); }
    string line;
    while(getline(file, line)) {
        if (line[0]=='{') { 
            Obstacle newObstacle;
            newObstacle.type = line.substr(1, line.find(',')-1);
            line.erase(0, line.find(',')+1);
            newObstacle.variant = stoi(line.substr(0, line.find(',')));
            line.erase(0, line.find(',')+1);
            newObstacle.xPos = stoi(line.substr(0, line.find(',')));
            line.erase(0, line.find(',')+1);
            newObstacle.yPos = stoi(line.substr(0, line.find(',')));
            line.erase(0, line.find(',')+1);
            newObstacle.xVel = stof(line.substr(0, line.find('}')));
            levelObstacles.push_back(newObstacle);
            dynamicObstacles.push_back(newObstacle);
        } else if (line[0]=='[') {
            background.r = stoi(line.substr(1, line.find(",")-1));
            line.erase(0, line.find(",")+1);
            background.g = stoi(line.substr(0, line.find(",")));
            line.erase(0, line.find(",")+1);
            background.b = stoi(line.substr(0, line.length()-1));
        }
    } file.close();
} 
// Método para comprobar si el jugador colisiona con cierto tipo de obstáculo
// Argumentos:
//    - Tipo del obstáculo (type)
//    - Hitbox del jugador (player)
//    - Indicador que determina si el obstáculo debe desaparecer al colisionar (destroy)
//    - Margen de recorte lateral de la hitbox del obstáculo (gap)
bool Level::collide(string type, Box player, bool destroy, int gap) {
    int groundLevel = gameWindow->get_game_size().y-40-(*gameSheets)[type]->get_sprite_size().y;
	// Comprueba cada obstáculo del tipo especificado y compara las hitbox con la del jugador
    for(int i=0; i<dynamicObstacles.size(); i++) {
        Obstacle &o = dynamicObstacles[i];
        if (o.type == type) {
            Box obstacle = {int(o.xPos-obstacleOffset)+gap, int(groundLevel-o.yPos), (*gameSheets)[type]->get_sprite_size().x-2*gap, (*gameSheets)[type]->get_sprite_size().y};
            if (player.x+player.w >= obstacle.x && player.x <= obstacle.x+obstacle.w) {
                if (player.y+player.h >= obstacle.y && player.y <= obstacle.y+obstacle.h) {
                    if (destroy) { dynamicObstacles.erase(dynamicObstacles.begin()+i); }
                    return true; 
                }
            }
        }
	} return false;
} 
// Método para sumar puntos extra
// Argumento: Número de puntos a sumar (score)
void Level::add_score(int score) { extraScore+=score; }
// Método para cambiar la velocidad del nivel
// Argumento: Velocidad en píxeles por frame (speed)
void Level::set_speed(int speed) { levelSpeed = speed; }
// Método para renderizar los obstáculos
void Level::render_obstacles() {
    finished = true;
    finishMs = levelClock.getElapsedTime().asMilliseconds();
    // Dibuja cada obstáculo en caso de estar dentro de la pantalla
    // Si no hay mas obstáculos en la pantalla determina que el nivel finalizó
    for (Obstacle &o : dynamicObstacles) {
        (*gameSheets)[o.type]->set_frame(o.variant);
        if (levelSpeed!=0) { o.xPos-=o.xVel; }
        int groundLevel = gameWindow->get_game_size().y-40-(*gameSheets)[o.type]->get_sprite_size().y;
        float xPos = o.xPos-obstacleOffset;
        float yPos = groundLevel-o.yPos;
        if (xPos>0-(*gameSheets)[o.type]->get_sprite_size().x && xPos<gameWindow->get_game_size().x) {
            finished = false;
            finishMs = 0;
            if (o.type == "coin" || o.type == "missile") { (*gameSheets)[o.type]->update_animation(); }
            (*gameSheets)[o.type]->render(gameWindow, {xPos, yPos});
        } // Los obstáculos se mueven hacia la izquierda
    } obstacleOffset+=levelSpeed;
} 
// Método para renderizar el fondo
void Level::render_background() {
    int width = (*gameSheets)["background_"+to_string(number)]->get_sprite_size().x;
    for (int i=0; i<=gameWindow->get_game_size().x/width; i++) {
        backgroundOffset = backgroundOffset>gameWindow->get_game_size().x+width?0:backgroundOffset;
        float xPos = i*width-int(backgroundOffset);
        xPos = xPos<-width?xPos+gameWindow->get_game_size().x+width:xPos;
        (*gameSheets)["background_"+to_string(number)]->render(gameWindow, {xPos, float(gameWindow->get_game_size().y-(*gameSheets)["background_"+to_string(number)]->get_sprite_size().y)});
    } backgroundOffset+=levelSpeed/2;
} 
// Método para renderizar lo que se debe mostrar frente al jugador (máscara de los agujeros por donde cae el dinosaurio)
void Level::render_front() {
    for (Obstacle &o : levelObstacles) {
        if (o.type == "hole_"+to_string(number)) {
            float xPos = o.xPos-obstacleOffset;
            float yPos = gameWindow->get_game_size().y-40-(*gameSheets)[o.type]->get_sprite_size().y-o.yPos;
            (*gameSheets)["hole_mask_"+to_string(number)]->render(gameWindow, {xPos, yPos});
        }
    }
}
// Método para renderizar el suelo
void Level::render_ground() {
    int width = (*gameSheets)["ground_"+to_string(number)]->get_sprite_size().x;
    for (int i=0; i<=gameWindow->get_game_size().x/width; i++) {
        groundOffset = groundOffset>gameWindow->get_game_size().x?0:groundOffset;
        float xPos = i*width-int(groundOffset);
        xPos = xPos<-width?xPos+gameWindow->get_game_size().x+width:xPos;
        (*gameSheets)["ground_"+to_string(number)]->render(gameWindow, {xPos, float(gameWindow->get_game_size().y-50)});
    } groundOffset+=levelSpeed;
} 
// Método para renderizar las nubes
void Level::render_clouds() {
    for (int i=0; i<3; i++) {
        float xPos = gameWindow->get_game_size().x-int(i*150+cloudOffset)%(gameWindow->get_game_size().x+53);
        (*gameSheets)["cloud_"+to_string(number)]->set_frame(i);
        (*gameSheets)["cloud_"+to_string(number)]->render(gameWindow, {xPos, float(gameWindow->get_game_size().y/8)});
    } cloudOffset+=levelSpeed/4;
} 
// Método para actualizar el puntaje en base al reloj
void Level::update_score() { score = ( levelClock.getElapsedTime().asMilliseconds()/1000)+extraScore; }
// Método para volver a los valores iniciales 
void Level::restart() {
    dynamicObstacles = levelObstacles;
	obstacleOffset = 0;
	backgroundOffset = 0;
	groundOffset = 0;
	cloudOffset = 0;
	levelSpeed = 2;
	score = 0;
    extraScore = 0;
    finished = false;
	levelClock.restart();	
} 
// Método para comprobar si el nivel fue finalizado
bool Level::is_finished() { return finished; }
// Método para obtener la puntuación total actual
int Level::get_score() { return score+extraScore; } 
// Método para obtener el número del nivel
int Level::get_number() { return number; }
// Método para obtener el tiempo transcurrido desde que comenzó el nivel
int Level::get_level_time() { return levelClock.getElapsedTime().asMilliseconds(); }
// Método para obtener el tiempo transcurrido hasta que se finalizó el nivel
int Level::get_finish_time() { return finishMs; }
