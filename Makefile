FLAGS = -std=c++11 -O3 -Wall -Werror
OGL = -lGL -lGLU -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lglfw3
LIBS = src/glad.c

run: build
	@./bin/out
build:
	g++ $(FLAGS) $(LIBS) src/main.cpp -o bin/out $(OGL)
t:
	g++ $(FLAGS) $(LIBS) test/test.cpp -o bin/tout $(OGL) && ./bin/tout