#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <array>

using namespace std; 

// Posición x, posición y, velocidad x, velocidad y, valor beta, masa/carga
struct particle { float x, y, xvel, yvel, b, u; };
// Energía en el punto, al norte, al sur, al oeste, al este
struct ref_points { float p, n, s, w, e; };

SDL_Window *WINDOW;
SDL_Renderer* RENDERER;
SDL_Event EVENT;

int RENDERSCALE=6;
int SCREEN_WIDTH=2880/RENDERSCALE;
int SCREEN_HEIGHT=1800/RENDERSCALE;

vector<particle> PARTICLES; // Vector de partículas

float RANGE=10; // Valor k
float MASS=1; // Masa/Carga estandar
float SCALE=3; // Coeficiente de zoom
float MAX_ENERGY; // Maximo valor de energía alcanzable en el campo

bool VELOCITY_BASED_MOVEMENT=true; // Determina si las partículas utilizan velocidades o solo se mueven directamente por las fuerzas de interacción
bool INDIVIDUAL_BETA=false; // Determina si las partículas se pueden generar con un valor beta individual

// Función para obtener el nivel de energía del campo en un punto
float GET_ENERGY_VALUE(float X, float Y) {
    float VALUE=0;
    for (int P=0; P<PARTICLES.size(); P++) {
        float DX=X-PARTICLES[P].x;
        float DY=Y-PARTICLES[P].y;
        VALUE+=PARTICLES[P].u/(1+DX*DX+DY*DY);
    }
    return VALUE;
}

// Función para obtener los niveles de energía en un punto y en sus puntos de referencia
ref_points GET_ENERGY_VALUES(float X, float Y) {
    ref_points VALUES = {0, 0, 0, 0, 0};
    for (int P=0; P<PARTICLES.size(); P++) {
        float DX=X-PARTICLES[P].x;
        float DY=Y-PARTICLES[P].y;

        VALUES.p+=PARTICLES[P].u/(1+DX*DX+DY*DY);
        VALUES.n+=PARTICLES[P].u/(1+DX*DX+(DY+RANGE)*(DY+RANGE));
        VALUES.s+=PARTICLES[P].u/(1+DX*DX+(DY-RANGE)*(DY-RANGE));
        VALUES.w+=PARTICLES[P].u/(1+(DX-RANGE)*(DX-RANGE)+DY*DY);
        VALUES.e+=PARTICLES[P].u/(1+(DX+RANGE)*(DX+RANGE)+DY*DY);
    }   
    return VALUES;
}

// Función para crear la ventana principal
void MAIN_WINDOW(int W=SCREEN_WIDTH, int H=SCREEN_HEIGHT) {
    SDL_Init(SDL_INIT_VIDEO);
    WINDOW=SDL_CreateWindow("MAIN", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W*RENDERSCALE, H*RENDERSCALE, SDL_WINDOW_SHOWN);
    RENDERER=SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(RENDERER, RENDERSCALE, RENDERSCALE);
}

// Función para dibujar un pixel en pantalla
void PIXEL(int X, int Y, int R=255, int G=255, int B=200) {
    SDL_SetRenderDrawColor(RENDERER, R, G, B, 255);
    SDL_RenderDrawPoint(RENDERER, X, Y);
    SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 255);
}

//Función para generar aleatoriamente las partículas
void GENERATE_PARTICLES(int AMOUNT=500, int GEN_RANGE=10) {
    for (int P=0; P<AMOUNT; P++) {
        float X=-GEN_RANGE+float(rand()%(GEN_RANGE*20))/10;
        float Y=-GEN_RANGE+float(rand()%(GEN_RANGE*20))/10;
        float U=float(rand()%2==1?MASS:-MASS);
        float B=INDIVIDUAL_BETA?float(rand()%2==1?MASS:-MASS):U;
        PARTICLES.push_back({X, Y, 0, 0, B, U});
    }
}

// Función de dibujado y actualización de las posiciones de las partículas
void UPDATE() {
    SDL_RenderClear(RENDERER);
    for (int P=0; P<PARTICLES.size(); P++) {
        // Fuerzas de interacción
        ref_points VALUES=GET_ENERGY_VALUES(PARTICLES[P].x, PARTICLES[P].y);
        array<float, 2> FORCES = {VALUES.n-VALUES.s, VALUES.e-VALUES.w}; 
        // Dibujado
        if (PARTICLES[P].x<(SCREEN_WIDTH/2)/SCALE && PARTICLES[P].y>(-SCREEN_HEIGHT/2)/SCALE) {
            float VALUE=VALUES.p/(MASS*2);
            VALUE*=55;
            float G=(abs(VALUE)-500<0)?0:abs(VALUE)-500;
            if (VALUE>0) {
                PIXEL(PARTICLES[P].x*SCALE+SCREEN_WIDTH/2, PARTICLES[P].y*SCALE+SCREEN_HEIGHT/2, (VALUE>255)?255:VALUE, (G>255)?255:G, (VALUE>50)?50:VALUE);
            }
            else {
                PIXEL(PARTICLES[P].x*SCALE+SCREEN_WIDTH/2, PARTICLES[P].y*SCALE+SCREEN_HEIGHT/2, (-VALUE>50)?50:-VALUE, (G>255)?255:G, (-VALUE>255)?255:-VALUE);
            }
        }
        // Normalización de las fuerzas
        FORCES[0]/=MAX_ENERGY;
        FORCES[1]/=MAX_ENERGY;

        // Actualización de posiciones
        float FORCE_XVALUE=PARTICLES[P].b*FORCES[1];
        float FORCE_YVALUE=PARTICLES[P].b*FORCES[0];
        if (VELOCITY_BASED_MOVEMENT) {
            PARTICLES[P].xvel+=FORCE_XVALUE;
            PARTICLES[P].yvel+=FORCE_YVALUE;
            PARTICLES[P].x+=PARTICLES[P].xvel;
            PARTICLES[P].y+=PARTICLES[P].yvel;
        }
        else {
            PARTICLES[P].x+=FORCE_XVALUE;
            PARTICLES[P].y+=FORCE_YVALUE;
        }
    }
    SDL_RenderPresent(RENDERER);
}

// Función principal
int main() {
    GENERATE_PARTICLES();
    MAX_ENERGY=PARTICLES.size()*MASS;
    MAIN_WINDOW();
    while(true) { 
        UPDATE();
        const Uint8* KEY = SDL_GetKeyboardState(NULL);
        while (SDL_PollEvent(&EVENT)) {
            if (EVENT.type == SDL_QUIT) { SDL_Quit(); exit(0); }
        }
        if (KEY[SDL_SCANCODE_Q]) { SCALE-=0.05*SCALE; } // Tecla 'Q' = -Zoom 
        if (KEY[SDL_SCANCODE_E]) { SCALE+=0.05*SCALE; } // Tecla 'E' = +Zoom
    }
}

// LG
