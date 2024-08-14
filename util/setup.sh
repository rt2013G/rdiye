#!/bin/sh

mkdir src
mdkri src/stb
touch src/main.cpp
mkdir test
touch test/test.cpp
mkdir bin
touch Makefile

git clone https://github.com/glfw/glfw.git
cd glfw
mkdir build
cmake -S . -B build
cd build
sudo make install
cd ../..
rm -rf glfw

## INSTALL GLAD: https://glad.dav1d.de/

## INSTALL GLM: https://github.com/g-truc/glm/releases

git clone https://github.com/nothings/stb
mv stb/stb_image.h src/stb
rm -rf stb
touch src/stb/stb_image.cpp
printf "#define STB_IMAGE_IMPLEMENTATION\n#include \"stb_image.h\"" >> src/stb/stb_image.cpp