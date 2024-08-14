#include <iostream>
#include <cmath>
#include <vector>
#include <array>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#define PI 3.141592653589

int WINDOW_W=1600;
int WINDOW_H=1200;

GLFWwindow* WINDOW;
GLuint VBO;

const float VSIZE=0.05;

struct vox {
  int COLOR;
  std::array<bool, 6> FACES={true, true, true, true, true, true};
};

std::vector<float> VERTEX_DATA={-VSIZE, +VSIZE, -VSIZE, +VSIZE, +VSIZE, -VSIZE,
                                +VSIZE, -VSIZE, -VSIZE, -VSIZE, -VSIZE, -VSIZE,
                                -VSIZE, +VSIZE, -VSIZE, +VSIZE, +VSIZE, -VSIZE,
                                -VSIZE, +VSIZE, +VSIZE, +VSIZE, +VSIZE, +VSIZE,
                                +VSIZE, +VSIZE, -VSIZE, +VSIZE, +VSIZE, +VSIZE,
                                +VSIZE, -VSIZE, +VSIZE, +VSIZE, +VSIZE, -VSIZE,
                                +VSIZE, -VSIZE, -VSIZE, -VSIZE, -VSIZE, -VSIZE,
                                +VSIZE, -VSIZE, +VSIZE, -VSIZE, -VSIZE, +VSIZE,
                                -VSIZE, -VSIZE, -VSIZE, -VSIZE, +VSIZE, -VSIZE,
                                -VSIZE, +VSIZE, +VSIZE, -VSIZE, -VSIZE, -VSIZE,
                                -VSIZE, -VSIZE, +VSIZE, -VSIZE, +VSIZE, +VSIZE,
                                +VSIZE, +VSIZE, +VSIZE, +VSIZE, -VSIZE, +VSIZE,
                                -VSIZE, -VSIZE, +VSIZE};

std::vector<std::array<int, 3>> PALETTE={{24, 0, 36}, {32, 0, 32},
                                         {40, 0, 28}, {48, 0, 28},
                                         {56, 0, 24}, {64, 0, 20},
                                         {72, 0, 20}, {80, 0, 16},
                                         {112, 0, 8}, {226, 145, 145},
                                         {153, 221, 146}, {147, 216, 185},
                                         {148, 196, 211}, {148, 154, 206},
                                         {179, 148, 204}, {204, 150, 177},
                                         {204, 164, 153}, {223, 229, 146},
                                         {255, 165, 96}};

std::array<float, 3> CAMERA_POS={0, -1, 0};
float CAMERA_YAW=0;
float CAMERA_PITCH=0;
float CAMERA_SPEED=0.005;

float FOV=110.0f;                  
float ASPECT_RATIO=float(WINDOW_W)/float(WINDOW_H);
float NEAR_CLIP=0.01f; 
float FAR_CLIP=250.0f;

const int CSIZE=50;
vox CHUNK[CSIZE][CSIZE][CSIZE];

int LIGHT[5]={CSIZE/2, CSIZE/2, CSIZE/2, 9, 1};
float ANGLE=0;

void UPDATE_VIEWPORT() {
  glLoadIdentity();
  gluPerspective(FOV, ASPECT_RATIO, NEAR_CLIP, FAR_CLIP);
  glMatrixMode(GL_MODELVIEW);
  glRotatef(CAMERA_PITCH, 1.0, 0.0, 0.0);
  glRotatef(CAMERA_YAW, 0.0, 1.0, 0.0);
  glTranslatef(CAMERA_POS[0], CAMERA_POS[1], CAMERA_POS[2]);
}

void RENDER_VOXEL(int X=0, int Y=0, int Z=0, int COLOR=0, bool SHADELESS=false, std::array<bool, 6> FACES={true, true, true, true, true, true}) {
  float R=float(PALETTE[COLOR][0])/255;
  float G=float(PALETTE[COLOR][1])/255;
  float B=float(PALETTE[COLOR][2])/255;
  
  if (!SHADELESS) {
    int DX=LIGHT[0]-X; 
    int DZ=LIGHT[2]-Z;
    float LIGHT_DISTANCE=sqrt(DX*DX+DZ*DZ);
    R+=0.3+float(PALETTE[LIGHT[3]][0])/1000-LIGHT_DISTANCE/150;
    G+=0.3+float(PALETTE[LIGHT[3]][1])/1000-LIGHT_DISTANCE/150;
    B+=0.3+float(PALETTE[LIGHT[3]][2])/1000-LIGHT_DISTANCE/150;
  }

  glColor3f(R, G, B);
  glTranslatef(X*VSIZE*2, Y*VSIZE*2, Z*VSIZE*2);
  for (int T=0; T<12; T++) { 
    if (FACES[int(T/2)]==true) { glDrawArrays(GL_TRIANGLES, T*2, 3); }
  }
  glTranslatef(-X*VSIZE*2, -Y*VSIZE*2, -Z*VSIZE*2);
}

void FILL_CHUNK() {
  for (int Z=0; Z<CSIZE; Z++) {
    for (int X=0; X<CSIZE; X++) {
      int STATE=1+rand()%7;
      CHUNK[X][0][Z].COLOR=STATE; 
    }
  }

  for (int Y=1; Y<7; Y++) {
    for (int Z=0; Z<CSIZE; Z++) {
      for (int X=0; X<CSIZE; X++) {
        if (CHUNK[X][Y-1][Z].COLOR!=0) {
          int STATE=-8+rand()%16;
          if (STATE<0) { STATE=0; }
          CHUNK[X][Y][Z].COLOR=STATE; 
        }
      }
    }
  }

  for (int Z=0; Z<CSIZE; Z++) {
    for (int Y=0; Y<7; Y++) {
      for (int X=0; X<CSIZE; X++) {
        if (X!=CSIZE-1) {
          if (CHUNK[X+1][Y][Z].COLOR!=0) { CHUNK[X][Y][Z].FACES[2]=false; }
        }
        if (X!=0) {
          if (CHUNK[X-1][Y][Z].COLOR!=0) { CHUNK[X][Y][Z].FACES[4]=false; }
        }
        if (Y!=CSIZE-1) {
          if (CHUNK[X][Y+1][Z].COLOR!=0) { CHUNK[X][Y][Z].FACES[1]=false; }
        }
        if (Y!=0) {
          if (CHUNK[X][Y-1][Z].COLOR!=0) { CHUNK[X][Y][Z].FACES[3]=false; }
        }
        if (Z!=CSIZE-1) {
          if (CHUNK[X][Y][Z+1].COLOR!=0) { CHUNK[X][Y][Z].FACES[5]=false; }
        }
        if (Z!=0) {
          if (CHUNK[X][Y][Z-1].COLOR!=0) { CHUNK[X][Y][Z].FACES[0]=false; }
        }
      }
    }
  }
}

void RENDER_CHUNK() {
  for (int Z=0; Z<CSIZE; Z++) {
    for (int Y=0; Y<CSIZE; Y++) {
      for (int X=0; X<CSIZE; X++) {
        if (CHUNK[X][Y][Z].COLOR>0) {
          RENDER_VOXEL(X, Y, Z, CHUNK[X][Y][Z].COLOR, false, CHUNK[X][Y][Z].FACES);
        }
      }
    }
  }
}

void DRAW() {
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  UPDATE_VIEWPORT();

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, VERTEX_DATA.size()*sizeof(float), VERTEX_DATA.data(), GL_STATIC_DRAW);  
  
  RENDER_CHUNK();
  
  RENDER_VOXEL(LIGHT[0], LIGHT[1], LIGHT[2], LIGHT[3], true);
  
  LIGHT[0]=cos(ANGLE*PI/180)*CSIZE*1.5+CSIZE/2;
  LIGHT[2]=sin(ANGLE*PI/180)*CSIZE*1.5+CSIZE/2;
  
  ANGLE+=0.1;
  if (ANGLE>360) { ANGLE=0; }
  if (LIGHT[2]==CSIZE/2) { LIGHT[3]++; }
  if (LIGHT[3]>18) { LIGHT[3]=9; }

  glfwSwapBuffers(WINDOW);
  glfwPollEvents();
}

void HANDLE_EVENTS() {
  double MOUSEX, MOUSEY;
  glfwGetCursorPos(WINDOW, &MOUSEX, &MOUSEY);
  CAMERA_YAW-=((WINDOW_W/2)-MOUSEX)/10;
  CAMERA_PITCH-=((WINDOW_H/2)-MOUSEY)/10;

  if (abs(CAMERA_YAW)>360) { CAMERA_YAW=0; }
  if (CAMERA_PITCH>90) { CAMERA_PITCH=90; }
  if (CAMERA_PITCH<-90) { CAMERA_PITCH=-90; }

  if (glfwGetKey(WINDOW, GLFW_KEY_W)==GLFW_PRESS) { 
    CAMERA_POS[2]+=cos(CAMERA_YAW*PI/180)*CAMERA_SPEED;
    CAMERA_POS[0]-=sin(CAMERA_YAW*PI/180)*CAMERA_SPEED;
  }
  if (glfwGetKey(WINDOW, GLFW_KEY_A)==GLFW_PRESS) { 
    CAMERA_POS[2]+=sin(CAMERA_YAW*PI/180)*CAMERA_SPEED;
    CAMERA_POS[0]+=cos(CAMERA_YAW*PI/180)*CAMERA_SPEED;
  }
  if (glfwGetKey(WINDOW, GLFW_KEY_S)==GLFW_PRESS) { 
    CAMERA_POS[2]-=cos(CAMERA_YAW*PI/180)*CAMERA_SPEED;
    CAMERA_POS[0]+=sin(CAMERA_YAW*PI/180)*CAMERA_SPEED;
  }
  if (glfwGetKey(WINDOW, GLFW_KEY_D)==GLFW_PRESS) { 
    CAMERA_POS[2]-=sin(CAMERA_YAW*PI/180)*CAMERA_SPEED;
    CAMERA_POS[0]-=cos(CAMERA_YAW*PI/180)*CAMERA_SPEED;
  }
  if (glfwGetKey(WINDOW, GLFW_KEY_SPACE)==GLFW_PRESS) {
    CAMERA_POS[1]-=CAMERA_SPEED;
  }
  if (glfwGetKey(WINDOW, GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS) {
    CAMERA_POS[1]+=CAMERA_SPEED;
  }
  if (glfwGetKey(WINDOW, GLFW_KEY_I)==GLFW_PRESS) { 
    CAMERA_PITCH-=1;
  }
  if (glfwGetKey(WINDOW, GLFW_KEY_J)==GLFW_PRESS) { 
    CAMERA_YAW-=1;
  }
   if (glfwGetKey(WINDOW, GLFW_KEY_K)==GLFW_PRESS) { 
    CAMERA_PITCH+=1;
  }
  if (glfwGetKey(WINDOW, GLFW_KEY_L)==GLFW_PRESS) { 
    CAMERA_YAW+=1;
  }
  glfwSetCursorPos(WINDOW, WINDOW_W/2, WINDOW_H/2);
}

void INIT() {
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, VERTEX_DATA.size()*sizeof(float), VERTEX_DATA.data(), GL_STATIC_DRAW);  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  FILL_CHUNK();
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  WINDOW=glfwCreateWindow(WINDOW_W, WINDOW_H, "VOXELS", NULL, NULL);
  glfwMakeContextCurrent(WINDOW);
  glewInit();
  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE);
  glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  INIT();
  while (!glfwWindowShouldClose(WINDOW)) { 
    DRAW(); 
    HANDLE_EVENTS();
  }
  glfwTerminate();
}