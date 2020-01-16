#pragma once

#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <pulse/simple.h>

#include "dsp_core.hpp"

using namespace std;

// Generic source. read_frame yields one new frame.
class source : public systm {
public:
  source() : systm("source", {{1, FRAME_N}}) {}
  source(int M, int N) : systm("source", {{M,N}}) {}
  bus apply(bus b) { return bus({frame(FRAME_N)}); }
};

// Pulseaudio source, default device
class pa_source : public source {
  pa_simple *pulse;
  pa_sample_spec spec;
  
public:
  pa_source();
  ~pa_source();
  bus apply(bus b);
};

// Wav file source, reads frames from a wav file
class file_source : public source {
  string path;
  ifstream file;
  
public:
  file_source(string path);
  string get_path() { return path; }
  bus apply(bus b);
};

