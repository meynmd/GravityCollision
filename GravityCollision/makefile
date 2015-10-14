all: particles

particles: particles.cpp libfreeglut.a libglew32.a
	g++ -O3 -std=c++0x -g -o particles particles.cpp libfreeglut.a -lGL -lGLU -lglut -lGLEW -lm -fopenmp
