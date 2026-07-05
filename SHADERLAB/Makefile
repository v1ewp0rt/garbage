CXX = g++
CXXFLAGS = -g -O0 -Wall -Wextra -Iinclude -static-libstdc++ -static-libgcc
LDLIBS = -lSDL2 -lSDL2main -lGLEW -lGL
SRC = main.cpp $(wildcard include/*.cpp)
OBJ = $(SRC:.cpp=.o)
TARGET = main
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)