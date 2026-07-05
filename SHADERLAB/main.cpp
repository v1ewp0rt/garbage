#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

using namespace std;

string loadFile(const string& path) {
    ifstream file(path);
    stringstream ss;
    ss << file.rdbuf();
    ss << "void main() { mainImage(FragColor, gl_FragCoord.xy); }";
    return ss.str();
}
GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        cerr << log << endl;
    } return s;
}
GLuint createProgram(string vSrc, string fSrc) {
    GLuint vs = compile(GL_VERTEX_SHADER, vSrc.c_str());
    GLuint fs = compile(GL_FRAGMENT_SHADER, fSrc.c_str());

    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}
int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* win = SDL_CreateWindow("SHADERLAB", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_OPENGL);
    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    SDL_GL_SetSwapInterval(1);
    glewExperimental = GL_TRUE;
    glewInit();
    GLuint vao;
    glGenVertexArrays(1, &vao);
    
    const char* vertexSrc = R"(
        #version 330 core
        const vec2 verts[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2( 3.0, -1.0),
            vec2(-1.0,  3.0)
        );
        void main() {
            gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
        }
    )";
    string fragmentSrc = loadFile("shader.glsl");
    glBindVertexArray(vao);
    GLuint program = createProgram(vertexSrc, fragmentSrc);
    glUseProgram(program);
    GLint timeLoc = glGetUniformLocation(program, "uTime");
    GLint resLoc  = glGetUniformLocation(program, "uResolution");
    Uint64 last = SDL_GetPerformanceCounter();
    float time = 0.0f;
    bool running = true;
    int fd = inotify_init1(IN_NONBLOCK);
    int wd = inotify_add_watch(fd, "shader.glsl", IN_CLOSE_WRITE);

    while (running) {
        SDL_Event e;
        char buffer[4096];
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_QUIT) { running = false; }
        }

        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((now-last)/(double)SDL_GetPerformanceFrequency());
        last = now;
        time += dt;

        ssize_t len;
        while ((len = read(fd, buffer, sizeof(buffer)))>0) {
            fragmentSrc = loadFile("shader.glsl");
            GLuint newProgram = createProgram(vertexSrc, fragmentSrc);
            if (newProgram!=0) {
                glDeleteProgram(program);
                program = newProgram;
                glUseProgram(program);
                time -= time;
            }
        }

        int w, h;
        SDL_GetWindowSize(win, &w, &h);
        glUniform1f(timeLoc, time);
        glUniform2f(resLoc, (float)w, (float)h);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        SDL_GL_SwapWindow(win);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}