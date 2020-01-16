#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "dsp_core.hpp"
#include "sink.hpp"
#include "source.hpp"

/*
 * The supersystem is basically a big digraph of systems whose outputs and
 * inputs are all connected.
 *
 * The input for each node is describ2ed by two side-by-side vectors: the first
 * points to a node, and the second tells which channel of that nodes's state
 * to use; iow, the vectors tell of a certain frame in a certain node. These
 * frames are appended together to make the input bus for the current node.
 * 
 * Each node keeps track of when other nodes, which depend on its state, have
 * accessed that state. When all such nodes have gotten the state, the node will
 * update itself, getting the states of its own dependencies and applying its
 * system to that input.
 *
 * The supersystem is thus lazily evaluated due to the recurrent nature of its
 * nodes. It has a vector of pointers to certain nodes which are sinks; as each
 * sink node is ticked, it will cause the necessary part of the supersystm
 * graph to update. If some part of the graph is not eventually connected
 * to a sink it will never be ticked.
 *
 * This design makes feedback fairly straightforward, but a major flaw is that
 * there can be no direct sample rate conversion; any variation in sample rate
 * will have to be handled inside each systm. I.e., a non-running average would
 * need to repeat its output N times if the average is N-wide. This also means
 * you can't upsample beyond the given sample rate.
 */

struct node {
  systm *sys;
  bus state;
  int t;
  int deps; // number of places this node is used as input.
  int wait; // number of deps waiting to get state.

  // vectors are indexed along the input bus to this systm.
  // so, for bus slot 0, inputs[0] is the input node and indices[0] is the
  // channel of the output state for that node.
  vector<node*> inputs;
  vector<int> indices;

  node(systm *sys);
  ~node();
  void append_link(node *n, int i);
  void tick(); // Only tick AFTER retrieving state!
};

/*
 * inport and outport are special a special pair of source & sink that allow
 * input and output to the supersystem.
 */

class inport : public source {
  frame in;
  
public:
  inport(int M, int N) : source(M, N), in(M, N) {}
  bus apply(bus b) { return bus({in}); }
  void buffer(frame f) { in = f; }
};

class outport : public sink {
  frame out;

public:
  outport(int M, int N) : out(M, N) {}
  bus apply(bus b) { out = b[0]; return bus({}); }
  bus unbuffer() { return bus({out}); }
};

class supersystem : public systm {
  int t;

  vector<inport*> in; 
  vector<node*> nodes; // all nodes besides ports
  vector<outport*> out;
  
  map<string, node*> node_map;
  vector<string> sinks;
  
public:
  supersystem();
  ~supersystem();
  bus apply(bus b);

  void append_sys(string name, systm *sys);
  void append_inport(string name, inport *inp);
  void append_outport(string name, outport *outp);
  void append_sink(string name);
  void link_node(string to, string from, int i);
};
