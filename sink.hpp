#pragma once

#include <cmath>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <pulse/simple.h>

#include "dsp_core.hpp"

using namespace std;

// Generic sink. write_frame does something with frame f.
class sink : public systm {
public:
  sink() : systm("sink", {}) {}
  virtual ~sink() {}
  bus apply(bus b) { return bus({}); }
};

// Pulseaudio sink, default device
class pa_sink : public sink {
  pa_simple *pulse;
  pa_sample_spec spec;

public:
  pa_sink();
  ~pa_sink();
  bus apply(bus b);
};

// Wav file sink. File completes once sink destructs (change this lmao)
class file_sink : public sink {
  ofstream file;
  size_t data_chunk_pos;
  void write_data(int data, unsigned int size);
  
public:
  file_sink(string path);
  ~file_sink();
  bus apply(bus b);
};
