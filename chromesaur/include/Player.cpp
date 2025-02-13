#include "Player.h"

// Constructor que inicializa las variables
// Argumentos:
//    - Ventana de juego (window)
//    - Objeto de gestión de eventos (event)
//    - Archivo de configuración del juego (configFile)
//    - Lista de todas las plantillas del juego (sheets)
Player::Player(MainWindow *window, EventHandler *event, Setting *configFile, unordered_map<string, SpriteSheet*> *sheets) {
    // Construcción de objetos a utilizar y valores iniciales
	gameWindow = window;
	gameEvent = event;
	gameSheets = sheets;
	back(); reset();
	// Carga de configuraciones y archivos
	if (!jumpSound.openFromFile("./res/jump.ogg")) { throw runtime_error("Error al cargar ./res/jump.ogg"); }
    jumpPower = float(configFile->get_value("Jump Power"))/10.f;
	jetPower = float(configFile->get_value("Jetpack Power"))/10.f;
	jetTime = float(configFile->get_value("Jetpack Time"))*1000;
    gravityForce = configFile->get_value("Gravity");
    gravityForce/=10;
	coinsValue = configFile->get_value("Coins Value");
	godMode = configFile->get_value("God Mode");
} 
// Método para procesar todas las acciones del jugador y sus animaciones
// Argumentos: 
//    - Nivel actual (currentLevel)
//    - Estado del sonido general (Si es verdadero se reproduce el sonido de salto) (sound)
void Player::update_state(Level *currentLevel, bool sound) {
	int currentMs = currentLevel->get_level_time();
	int groundLevel = gameWindow->get_game_size().y-40-(isCrouched?(*gameSheets)["dino_crouch"]:(*gameSheets)["dino_run"])->get_sprite_size().y;
	// Si sobrepasa el suelo automaticamente se corrige su posición
	if (yPos > groundLevel) { yPos = groundLevel; yVel=0; }
	if (isAlive) {
		handle_collisions(currentLevel);
		(*gameSheets)["jetpack_equiped"]->update_animation();
		// Si pasan x segundos desde que se obtuvo el jetpack, este va a desaparecer
		if (currentMs-jetpackMs >= jetTime) { jetpack = false; }
		// Volar con el jetpack
		if (gameEvent->key_press(Keyboard::Up) && jetpack) { yVel += jetPower; }
		// Si toca el suelo no es afectado por la gravedad, puede agacharse, correr y saltar
		if (groundLevel+yPos < groundLevel) {
			yVel = 0;
			(*gameSheets)["dino_run"]->update_animation();
			(*gameSheets)["dino_crouch"]->update_animation();
			if (currentLevel->get_score()<3) { 
				(*gameSheets)["dino_run"]->set_frame(1); 
				(*gameSheets)["dino_crouch"]->set_frame(1);
			} if (!currentLevel->is_finished()) {
				if (gameEvent->key_press(Keyboard::Down) && currentMs>250 && !jetpack) { isCrouched = true; }
				else { isCrouched = false; }
				if (gameEvent->key_press(Keyboard::Up) && currentMs>250) { 
					if (sound) { jumpSound.play(); } 
					yVel += jumpPower; 
					isCrouched = false; 
				} 
			} // Si no está en el suelo las animaciones se detienen y la gravedad influye
		} else { 
			if (jetpack) { (*gameSheets)["dino_run"]->set_frame(1); } 
			else { (*gameSheets)["dino_run"]->set_frame(0); }
			yVel-=jetpack?gravityForce*2:gravityForce; 
		} // Si no está vivo entonces se procesa la opción de reintentar el nivel
	} else {
		isCrouched = false;
		if (gameEvent->key_press(Keyboard::Up) && currentMs-collisionMs>500) {
			currentLevel->restart();
			currentLevel->add_score(2); // Esto es para evitar la cuenta regresiva a cada intento
			reset();
		} yVel-=gravityForce; 
	} yPos += yVel<-3?-3:yVel;
}
// Método para gestionar las colisiones
// Argumento: Nivel actual (currentLevel)
void Player::handle_collisions(Level *currentLevel) {
	int currentMs = currentLevel->get_level_time();
	// Lista de los obstáculos letales
	vector<string> killerObstacles = {
		"ground_obstacle_"+to_string(currentLevel->get_number()), 
		"pipe",
		"pipe_top",
		"missile"
	}; // Si no se está utilizando el modo de invencibilidad, el dinosaurio muere
	if (!godMode) {
		// Si toca un obstáculo de la lista, muere
		for (string o : killerObstacles) {
			if (currentLevel->collide(o, this->get_box(), false, 3)) {
				currentLevel->set_speed(0);
				collisionMs = currentMs;
				isAlive = false;
			} // Se procesa por separado los agujeros porque a su hitbox se debe acotar para que funcione bien
		} if (currentLevel->collide("hole_"+to_string(currentLevel->get_number()), this->get_box(), false, 30)) {
			currentLevel->set_speed(0);
			collisionMs = currentMs;
			isAlive = false;
		} // Si toca una moneda suma puntos (La cantidad de puntos se define en el archivo de configuración)
	} if (currentLevel->collide("coin", this->get_box(), true)) { currentLevel->add_score(coinsValue/2); }
	// Si toca un jetpack el jugador va a disponer de el durante cierta cantidad de tiempo (Determinado en el archivo de configuración)
	if (currentLevel->collide("jetpack", this->get_box(), true)) { 
		jetpackMs = currentMs;
		jetpack = true; 
	}
}
// Método para renderizar al jugador
void Player::render() { 
	int groundLevel;
	if (isCrouched) { 
		groundLevel = gameWindow->get_game_size().y-40-(*gameSheets)["dino_crouch"]->get_sprite_size().y;
		(*gameSheets)["dino_crouch"]->render(gameWindow, {xPos, groundLevel-yPos}); 
	} else { 
		groundLevel = gameWindow->get_game_size().y-40-(*gameSheets)["dino_run"]->get_sprite_size().y;
		(*gameSheets)["dino_run"]->render(gameWindow, {xPos, groundLevel-yPos}); 
		if (jetpack) { (*gameSheets)["jetpack_equiped"]->render(gameWindow, {xPos, groundLevel-yPos}); }
	} 
} 
// Método para que el jugador avance en la pantalla al terminar el nivel
void Player::forward() { xPos+=2; }
// Método para reestablecer la posición del jugador
void Player::back() { xPos = gameWindow->get_game_size().x/8; }
// Método para volver a los valores iniciales
void Player::reset() { 
	xVel=0; yPos = 0;
	yVel = 0;
	isCrouched = false;
	jetpack = false;
	collisionMs = 0;
	isAlive = true;
 }
// Método para comprobar si el jugador está vivo
bool Player::is_alive() { return isAlive; }
// Método para obtener la hitbox del jugador
Box Player::get_box() { 
	Box playerBox;
	int groundLevel = groundLevel = gameWindow->get_game_size().y-40-(isCrouched?(*gameSheets)["dino_crouch"]:(*gameSheets)["dino_run"])->get_sprite_size().y;
	playerBox.x = xPos; 
	playerBox.y = groundLevel-yPos;
	playerBox.w = (isCrouched?(*gameSheets)["dino_crouch"]:(*gameSheets)["dino_run"])->get_sprite_size().x;
	playerBox.h = (isCrouched?(*gameSheets)["dino_crouch"]:(*gameSheets)["dino_run"])->get_sprite_size().y; 
	return playerBox;
}