#include <iostream>
#include <vector>
#include "LIB/SDL2/SDL.h"
#include "LIB/SDL2/SDL_image.h"
#include "LIB/PerlinNoise.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "LIB/stb_image.h"
#define PI 3.14159265358979323846

using namespace std;

SDL_Window* WINDOW;
SDL_Renderer* RENDERER;
SDL_Texture* FRAME_BUFFER;

const int W = 800;
const int H = 600;
const int PIXELSCALE = 2;
const int VPORT_W = W/PIXELSCALE;
const int VPORT_H = H/PIXELSCALE;

SDL_Rect SRECT = {0, 0, VPORT_W, VPORT_H};
SDL_Rect DRECT = {0, 0, W, H};

int VIEWPORT[VPORT_W][VPORT_H];

bool RUNNING;
int FRAME_START;
int FRAME_TIME;
int CURRENT_FPS;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

int MOUSEX, MOUSEY;

SDL_Color BG_COLOR = {55, 100, 200, 255};
SDL_Point CAMERA = {0, 0};

const siv::PerlinNoise::seed_type SEED = 123456u;
const siv::PerlinNoise PERLIN{ SEED };

float GET_RANDOM_AT(int X, int Y) {
  srand(X + Y * VPORT_W);
  return (float)rand() / (RAND_MAX + 1.0);
}

float MAPRANGE(float a1, float a2, float b1, float b2, float s) { 
  float OUT = b1 + (s-a1)*(b2-b1)/(a2-a1);
  if (OUT < 0) { OUT = 0; }
  return OUT; 
}

void MAIN_WINDOW(const char* TITLE) {
  WINDOW = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W*3, H*3, SDL_WINDOW_SHOWN);
  RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_SOFTWARE);
  SDL_SetRenderDrawColor(RENDERER, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
  SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_BLEND);
  SDL_RenderSetScale(RENDERER, 3, 3);
}

void CLEAR_VIEWPORT() {
  for (int Y=0; Y<VPORT_H; Y++) {
    for (int X=0; X<VPORT_W; X++) {
      VIEWPORT[X][Y] = 0;
    }
  }
}

SDL_Texture *LOAD_TEXTURE(string FILENAME) {
  SDL_Surface *TEMP_SURFACE = IMG_Load(FILENAME.c_str());
  SDL_SetColorKey(TEMP_SURFACE, SDL_TRUE, SDL_MapRGB(TEMP_SURFACE->format, 0, 0, 0));
  SDL_Texture *TEXTURE = SDL_CreateTextureFromSurface(RENDERER, TEMP_SURFACE);
  SDL_FreeSurface(TEMP_SURFACE);
  return TEXTURE;
}

vector <unsigned char> GET_IMAGE_DATA(string FILENAME) {
  vector<unsigned char> IMAGE;  
  int WIDTH, HEIGHT, N;
  unsigned char *DATA = stbi_load(FILENAME.c_str(), &WIDTH, &HEIGHT, &N, 4);
  if (DATA != nullptr && WIDTH != 0 && HEIGHT != 0) { IMAGE = vector<unsigned char>(DATA, DATA + WIDTH * HEIGHT * 4); }
  stbi_image_free(DATA);
  return IMAGE;  
}

void PIXEL(int X, int Y, SDL_Color COLOR={0, 0, 0, 255}) {
  SDL_SetRenderDrawColor(RENDERER, COLOR.r, COLOR.g, COLOR.b, COLOR.a);
  SDL_RenderDrawPoint(RENDERER, X, Y);
  SDL_SetRenderDrawColor(RENDERER, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
}

void RECTANGLE(SDL_Rect RECT, SDL_Color COLOR={0, 0, 0, 255}) {
  SDL_SetRenderDrawColor(RENDERER, COLOR.r, COLOR.g, COLOR.b, COLOR.a);
  SDL_RenderFillRect(RENDERER, &RECT);
  SDL_SetRenderDrawColor(RENDERER, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
} 

void IMAGE(SDL_Texture *TEXTURE, SDL_Rect SURFACE, int X, int Y, float SIZE = 1, float ANGLE = 0, SDL_RendererFlip FLIP = SDL_FLIP_NONE) {
  SDL_Rect DRECT;
  DRECT.h = SURFACE.h*SIZE; DRECT.w = SURFACE.w*SIZE;
  DRECT.x = X; DRECT.y = Y;
  SDL_RenderCopyEx(RENDERER, TEXTURE, &SURFACE, &DRECT, ANGLE, 0, FLIP);
}

void SPRITE(SDL_Texture *TEXTURE, SDL_Rect SPRITE_RECT, int START_FRAME, int END_FRAME, int ANIMATION = 0, int SPEED = 1000, float SIZE=1, float ANGLE = 0, SDL_RendererFlip FLIP = SDL_FLIP_NONE) {
  int FRAMES = (END_FRAME-START_FRAME)+1;
  int SX=SPRITE_RECT.w * (START_FRAME+((SDL_GetTicks() / SPEED) % FRAMES));
  int SY=ANIMATION*SPRITE_RECT.h;
  IMAGE(TEXTURE, {SX, SY, SPRITE_RECT.w, SPRITE_RECT.h}, SPRITE_RECT.x, SPRITE_RECT.y, SIZE, ANGLE, FLIP);
}

string CHECK_COLLISION(SDL_Rect COLLIDER, SDL_Point OFFSET={0, 0}, bool DEBUG=false) {
  string OUTPUT = "0000";
  // UP, LEFT, DOWN, RIGHT
  for (int X=COLLIDER.x+OFFSET.x; X<=COLLIDER.x+COLLIDER.w-OFFSET.x; X++) { 
    if ((X<VPORT_W && X>0) && (COLLIDER.y<VPORT_H && COLLIDER.y>0)) {
      if (VIEWPORT[X][COLLIDER.y]!=0) { OUTPUT[0]='1'; } 
      if (DEBUG) { PIXEL(X, COLLIDER.y); }
    }
  }
  for (int X=COLLIDER.x+OFFSET.x; X<=COLLIDER.x+COLLIDER.w-OFFSET.x; X++) { 
    if ((X<VPORT_W && X>0) && (COLLIDER.y+COLLIDER.h<VPORT_H && COLLIDER.y+COLLIDER.h>0)) {
      if (VIEWPORT[X][COLLIDER.y+COLLIDER.h]!=0) { OUTPUT[2]='1'; } 
      if (DEBUG) { PIXEL(X, COLLIDER.y+COLLIDER.h); }
    }
  }
  for (int Y=COLLIDER.y+OFFSET.y; Y<=COLLIDER.y+COLLIDER.h-OFFSET.y; Y++) { 
    if ((COLLIDER.x<VPORT_W && COLLIDER.x>0) && (Y<VPORT_H && Y>0)) {
      if (VIEWPORT[COLLIDER.x][Y]!=0) { OUTPUT[1]='1'; } 
      if (DEBUG) { PIXEL(COLLIDER.x, Y); }
    }
  }
  for (int Y=COLLIDER.y+OFFSET.y; Y<=COLLIDER.y+COLLIDER.h-OFFSET.y; Y++) { 
    if ((COLLIDER.x+COLLIDER.w<VPORT_W && COLLIDER.x+COLLIDER.w>0) && (Y<VPORT_H && Y>0)) {
      if (VIEWPORT[COLLIDER.x+COLLIDER.w][Y]!=0) { OUTPUT[3]='1'; } 
      if (DEBUG) { PIXEL(COLLIDER.x+COLLIDER.w, Y); }
    }
  }
  return OUTPUT;
} 