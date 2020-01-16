all: .lib/libaudiotk.a .bin/test

CPPC = g++
LIBS = eigen3 fftw3 gl glew glfw3 pangocairo libpulse-simple
CFLAGS = -O3 -g -std=c++11 `pkg-config --cflags $(LIBS)` -I..
LFLAGS = -lm `pkg-config --libs $(LIBS)` -L../videotk/.lib -lvideotk

OBJS = .bld/dsp_core.o .bld/ops.o .bld/scope.o .bld/sink.o .bld/source.o .bld/supersystem.o

.bin/test: .lib/libaudiotk.a test.cpp
	$(CPPC) -o .bin/test $(CFLAGS) test.cpp -L .lib -laudiotk $(LFLAGS)

.lib/libaudiotk.a: $(OBJS)
	ar rcs .lib/libaudiotk.a ../videotk/.lib/libvideotk.a $(OBJS) 

.bld/dsp_core.o: dsp_core.cpp dsp_core.hpp
	$(CPPC) -c $(CFLAGS) dsp_core.cpp -o .bld/dsp_core.o $(LFLAGS)

.bld/ops.o: ops.cpp ops.hpp
	$(CPPC) -c $(CFLAGS) ops.cpp -o .bld/ops.o $(LFLAGS)

.bld/scope.o: scope.cpp scope.hpp
	$(CPPC) -c $(CFLAGS) scope.cpp -o .bld/scope.o $(LFLAGS)

.bld/sink.o: sink.cpp sink.hpp
	$(CPPC) -c $(CFLAGS) sink.cpp -o .bld/sink.o $(LFLAGS)

.bld/source.o: source.cpp source.hpp
	$(CPPC) -c $(CFLAGS) source.cpp -o .bld/source.o $(LFLAGS)

.bld/supersystem.o: supersystem.cpp supersystem.hpp
	$(CPPC) -c $(CFLAGS) supersystem.cpp -o .bld/supersystem.o $(LFLAGS)

clean:
	rm .bin/* .bld/* .lib/*

