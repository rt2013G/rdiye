#!/bin/sh

mkdir src
mkdir src/include
mkdir src/lib
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