CC = g++
LIBS = eigen3 fftw3 gl glew glfw3 pangocairo libpulse-simple
CFG = -O3 -g -std=c++11 `pkg-config --cflags $(LIBS)` -I..
SFG = ../videotk/.lib/libvideotk.a
LFG = -lm `pkg-config --libs $(LIBS)` -L../videotk/.lib -lvideotk
