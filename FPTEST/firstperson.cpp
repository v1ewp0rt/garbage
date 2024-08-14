#include <iostream>
#include <SDL2/SDL.h>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <algorithm>

#define PI 3.141592653589

SDL_Window *WINDOW;
SDL_Renderer* RENDERER;
SDL_Event EVENT;

int RENDERSCALE=3;
int SCREEN_WIDTH=2880/RENDERSCALE;
int SCREEN_HEIGHT=1800/RENDERSCALE;

std::vector<std::array<float, 3>> OBJ1_VERT_DATA;
std::vector<std::array<int, 3>> OBJ1_FACE_DATA;
std::vector<std::array<float, 3>> OBJ2_VERT_DATA;
std::vector<std::array<int, 3>> OBJ2_FACE_DATA;
std::array<float, 3> GROT={0, 0, 0};
std::array<float, 3> GPOS={0, -3, 0};

void STRING_TO_LIST(std::string STRING, std::vector<std::string> &LIST, char DELIMITER=' ') {
  STRING+=DELIMITER;
  std::vector<int> INDEXES = {0};
  for (int C=0; C<STRING.length(); C++) { 
    if (STRING[C]==DELIMITER) { INDEXES.push_back(C); }
  }
  for (int I=0; I<INDEXES.size()-1; I++) {
    std::string ELEMENT=STRING.substr(INDEXES[I], INDEXES[I+1]-INDEXES[I]);
    ELEMENT.erase(std::remove(ELEMENT.begin(), ELEMENT.end(), DELIMITER), ELEMENT.end());
    LIST.push_back(ELEMENT);
  }
}

void MAIN_WINDOW(int W=SCREEN_WIDTH, int H=SCREEN_HEIGHT) {
  SDL_Init(SDL_INIT_VIDEO);
  WINDOW=SDL_CreateWindow("MAIN", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W*RENDERSCALE, H*RENDERSCALE, SDL_WINDOW_SHOWN);
  RENDERER=SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);
  SDL_RenderSetScale(RENDERER, RENDERSCALE, RENDERSCALE);
}

void PIXEL(int X, int Y, int R=0, int G=255, int B=255) {
  SDL_SetRenderDrawColor(RENDERER, R, G, B, 255);
  SDL_RenderDrawPoint(RENDERER, X, Y);
  SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 255);
}

void LINE(int X0, int Y0, int X1, int Y1, int R=0, int G=255, int B=255) { 
  int DX=X1-X0; 
  int DY=Y1-Y0; 
  int STEPS=abs(DX)>abs(DY)?abs(DX):abs(DY); 
  float XINC=DX/(float)STEPS; 
  float YINC=DY/(float)STEPS; 
  float X=X0; 
  float Y=Y0; 
    
  for (int I=0; I<=STEPS; I++) { 
    PIXEL(round(X), round(Y), R, G, B);
    X+=XINC; Y+=YINC;
  } 
} 

void LOAD_OBJECT(std::string FILENAME, std::vector<std::array<float, 3>> &OBJ_V_DATA, std::vector<std::array<int, 3>> &OBJ_F_DATA) {
  std::fstream OBJECTFILE;
  OBJECTFILE.open(FILENAME, std::ios::in);
  if (OBJECTFILE.is_open()) {
    std::string LINE;
    while(getline(OBJECTFILE, LINE)) { 
      if (LINE[0]=='v') { 
        std::vector<std::string> DATA;
        STRING_TO_LIST(LINE.substr(2, LINE.length()-2), DATA);
        for (int I=0; I<DATA.size(); I+=3) {
          OBJ_V_DATA.push_back({std::stof(DATA[I]), 
                                std::stof(DATA[I+1]),
                                std::stof(DATA[I+2])});
        }
      }
      if (LINE[0]=='f') {
        std::vector<std::string> DATA;
        STRING_TO_LIST(LINE.substr(2, LINE.length()-2), DATA);
        for (int I=0; I<DATA.size(); I+=3) {
          OBJ_F_DATA.push_back({std::stoi(DATA[I]),
                                std::stoi(DATA[I+1]),
                                std::stoi(DATA[I+2])});
        }
      }
    }
    OBJECTFILE.close();
    std::cout << "VERTICES: " << OBJ_V_DATA.size() << std::endl;
    std::cout << "FACES: " << OBJ_F_DATA.size() << std::endl;
  }
}

void DRAW_OBJECT(float XPOS, float YPOS, float ZPOS, float XROT, float YROT, float ZROT, int SIZE, std::vector<std::array<float, 3>> &OBJ_V_DATA, std::vector<std::array<int, 3>> &OBJ_F_DATA, int R=0, int G=255, int B=255) {
  XROT*=float(PI/180); YROT*=float(PI/180); ZROT*=float(PI/180);
  for (int F=0; F<OBJ_F_DATA.size(); F++) {
    float X0=OBJ_V_DATA[OBJ_F_DATA[F][0]-1][0]+XPOS;
    float Y0=OBJ_V_DATA[OBJ_F_DATA[F][0]-1][1]+YPOS;
    float Z0=OBJ_V_DATA[OBJ_F_DATA[F][0]-1][2]+ZPOS;

    float RXY0=Y0*cos(XROT)+Z0*-sin(XROT);
    float RXZ0=Y0*sin(XROT)+Z0*cos(XROT);
    float RYX0=X0*cos(YROT)+RXZ0*sin(YROT);
    float RYZ0=X0*-sin(YROT)+RXZ0*cos(YROT);
    float RZX0=RYX0*cos(ZROT)+RXY0*-sin(ZROT);
    float RZY0=RYX0*sin(ZROT)+RXY0*cos(ZROT);
    
    float PZ0=(RYZ0+(ZPOS/SIZE))/8;
    float PX0=RZX0/PZ0;
    float PY0=RZY0/PZ0;
    
    float X1=OBJ_V_DATA[OBJ_F_DATA[F][1]-1][0]+XPOS;
    float Y1=OBJ_V_DATA[OBJ_F_DATA[F][1]-1][1]+YPOS;
    float Z1=OBJ_V_DATA[OBJ_F_DATA[F][1]-1][2]+ZPOS;

    float RXY1=Y1*cos(XROT)+Z1*-sin(XROT);
    float RXZ1=Y1*sin(XROT)+Z1*cos(XROT);
    float RYX1=X1*cos(YROT)+RXZ1*sin(YROT);
    float RYZ1=X1*-sin(YROT)+RXZ1*cos(YROT);
    float RZX1=RYX1*cos(ZROT)+RXY1*-sin(ZROT);
    float RZY1=RYX1*sin(ZROT)+RXY1*cos(ZROT);
    
    float PZ1=(RYZ1+(ZPOS/SIZE))/8;
    float PX1=RZX1/PZ1;
    float PY1=RZY1/PZ1;

    if (PZ0>0.3 && PZ1>0.3) {
      LINE((SCREEN_WIDTH/2)+SIZE*PX0, (SCREEN_HEIGHT/2)+SIZE*PY0, (SCREEN_WIDTH/2)+SIZE*PX1, (SCREEN_HEIGHT/2)+SIZE*PY1, R, G, B);
    }

    float X2=OBJ_V_DATA[OBJ_F_DATA[F][2]-1][0]+XPOS;
    float Y2=OBJ_V_DATA[OBJ_F_DATA[F][2]-1][1]+YPOS;
    float Z2=OBJ_V_DATA[OBJ_F_DATA[F][2]-1][2]+ZPOS;

    float RXY2=Y2*cos(XROT)+Z2*-sin(XROT);
    float RXZ2=Y2*sin(XROT)+Z2*cos(XROT);
    float RYX2=X2*cos(YROT)+RXZ2*sin(YROT);
    float RYZ2=X2*-sin(YROT)+RXZ2*cos(YROT);
    float RZX2=RYX2*cos(ZROT)+RXY2*-sin(ZROT);
    float RZY2=RYX2*sin(ZROT)+RXY2*cos(ZROT);
    
    float PZ2=(RYZ2+(ZPOS/SIZE))/8;
    float PX2=RZX2/PZ2;
    float PY2=RZY2/PZ2;

    if (PZ1>0.3 && PZ2>0.3) {
      LINE((SCREEN_WIDTH/2)+SIZE*PX1, (SCREEN_HEIGHT/2)+SIZE*PY1, (SCREEN_WIDTH/2)+SIZE*PX2, (SCREEN_HEIGHT/2)+SIZE*PY2, R, G, B);
      LINE((SCREEN_WIDTH/2)+SIZE*PX2, (SCREEN_HEIGHT/2)+SIZE*PY2, (SCREEN_WIDTH/2)+SIZE*PX0, (SCREEN_HEIGHT/2)+SIZE*PY0, R, G, B);
    }
  }
}

void DRAW() {
  SDL_RenderClear(RENDERER);
  DRAW_OBJECT(GPOS[0], GPOS[1]+5, GPOS[2], GROT[0], GROT[1], GROT[2], 100, OBJ1_VERT_DATA, OBJ1_FACE_DATA, 0, 255, 255);
  DRAW_OBJECT(GPOS[0], GPOS[1], GPOS[2], GROT[0], GROT[1], GROT[2], 100, OBJ2_VERT_DATA, OBJ2_FACE_DATA, 200, 0, 100);
  SDL_RenderPresent(RENDERER);
}

void HANDLE_EVENTS() {
  const Uint8* KEY = SDL_GetKeyboardState(NULL);
  while (SDL_PollEvent(&EVENT)) {
    if (EVENT.type == SDL_QUIT) { SDL_Quit(); exit(0); }
  }
  if (KEY[SDL_SCANCODE_W]) { 
    GPOS[2]-=cos(GROT[1]*PI/180)*0.1; 
    GPOS[0]+=sin(GROT[1]*PI/180)*0.1; 
  }
  if (KEY[SDL_SCANCODE_A]) {
    GPOS[2]+=sin(GROT[1]*PI/180)*0.1; 
    GPOS[0]+=cos(GROT[1]*PI/180)*0.1; 
  }
  if (KEY[SDL_SCANCODE_S]) {
    GPOS[2]+=cos(GROT[1]*PI/180)*0.1; 
    GPOS[0]-=sin(GROT[1]*PI/180)*0.1; 
  }
  if (KEY[SDL_SCANCODE_D]) {
    GPOS[2]-=sin(GROT[1]*PI/180)*0.1; 
    GPOS[0]-=cos(GROT[1]*PI/180)*0.1; 
  }
  if (KEY[SDL_SCANCODE_J]) {
    GROT[1]++;
  }
  if (KEY[SDL_SCANCODE_L]) {
    GROT[1]--;
  }
}

int main() {
  LOAD_OBJECT("./object1.obj", OBJ1_VERT_DATA, OBJ1_FACE_DATA);
  LOAD_OBJECT("./object2.obj", OBJ2_VERT_DATA, OBJ2_FACE_DATA);

  MAIN_WINDOW();
  while(true) { 
    HANDLE_EVENTS();
    DRAW();
  }
}