#pragma once

#include <sstream>
#include <string>
#include <vector>

#include <videotk/videotk.hpp>
#include "dsp_core.hpp"
#include "sink.hpp"

using namespace std;

class scope : public sink {
protected:
  window win;

public:
  scope(string title, int w, int h) : win(title, w, h) {}
  virtual ~scope() {}
  virtual bus apply(bus b) = 0;
};

class axis : public obj2 {
  int N;
  double min, max;
  int orient;
  
  line *major;
  vector<line*> ticks;
  vector<sprite*> labels;
  
public:
  axis(rect base, double min, double max, int orient);
  ~axis();
  void render();
};

class waterfall : public scope {
  int M, N;
  double min, max;
  point_grid *grid;
  axis *hor, *ver;
  
public:
  waterfall(int M, int N, int w, int h, double min, double max);
  ~waterfall();
  bus apply(bus b);
};

class vector_scope : public scope {
  double min, max;
  line_graph *graph;
  axis *hor;

public:
  vector_scope(int w, int h, int N, double min, double max);
  ~vector_scope();
  bus apply(bus b);
};

class multiscope : public scope {
  vector<double> mins, maxs;
  vector<line_graph*> graphs;
  vector<axis*> vers;
  axis *hor;

public:
  multiscope(int w, int h,
	     vector<int> Ns,
	     vector<double> mins,
	     vector<double> maxs);
  ~multiscope();
  bus apply(bus b);
};
