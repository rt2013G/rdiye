compiler_flags = -std=c++11 -O0 -Wall -g
linkers_flags = -lGL -lGLU -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lglfw3 -lassimp
files = src/thirdparty/glad.c src/thirdparty/stb/stb_image.cpp
build_flags = -DDEBUG_BUILD=1

all: build run
run:
	@./bin/out
build:
	g++ $(compiler_flags) src/rdiye.cpp $(files) $(build_flags) -o bin/out $(linkers_flags)