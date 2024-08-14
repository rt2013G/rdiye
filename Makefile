FLAGS = -std=c++11 -O3 -Wall -Werror
OGL = -lGL -lGLU -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lglfw3
LIBS = src/glad.c src/stb/stb_image.cpp

run: build
	@./bin/out
build:
	g++ $(FLAGS) src/main.cpp $(LIBS) -o bin/out $(OGL)
t:
	g++ $(FLAGS) test/test.cpp $(LIBS) -o bin/tout $(OGL) && ./bin/tout