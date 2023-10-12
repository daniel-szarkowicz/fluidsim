#!/bin/sh
set -e

mkdir -p build
g++ -o build/fluidsim src/main.cpp -lglfw -lGL -lGLEW
./build/fluidsim
