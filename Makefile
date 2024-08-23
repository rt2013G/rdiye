FLAGS = -std=c++11 -O0 -Wall -Werror -g
OGL = -lGL -lGLU -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lglfw3
LIBS = src/lib/glad.c src/lib/stb/stb_image.cpp -lassimp

run: build
	@./bin/out
build:
	g++ $(FLAGS) src/main.cpp $(LIBS) -o bin/out $(OGL)
tests:
	g++ $(FLAGS) src/tests/test.cpp $(LIBS) -o bin/test_out $(OGL) && ./bin/test_out
breakout:
	g++ $(FLAGS) src/demo/breakout.cpp $(LIBS) -o bin/breakout_out $(OGL) && ./bin/breakout_out