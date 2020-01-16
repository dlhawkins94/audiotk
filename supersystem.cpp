#include "supersystem.hpp"

node::node(systm *sys) : sys(sys), t(0), deps(0), wait(0) {
  for (auto &dim : sys->get_dims())
    state.push_back(frame(dim[0], dim[1]));
}

node::~node() { delete sys; }

void node::append_link(node *n, int i) {
  n->deps++;
  n->wait++;
  inputs.push_back(n);
  indices.push_back(i);
}

// when wait == 0, all dependencies have gotten this node's state.
// thus it's ready to to update.
void node::tick() {
  if (!deps || !(--wait)) {
    bus in;
    for (int j = 0; j < inputs.size(); j++) {
      // Assemble input bus from each input node j.
      // Then let that node know we got its state.
      in.push_back(inputs[j]->state[indices[j]]);
      inputs[j]->tick();
    }

    state = sys->apply(in);
    wait = deps;
    t++;
  }
}

supersystem::supersystem() : systm("supersystem", {}), t(0) {}

supersystem::~supersystem() {
  for (auto &n : nodes) delete n;
}

bus supersystem::apply(bus b) {
  for (int i = 0; i < in.size(); i++)
    in[i]->buffer(b[i]);

  for (auto &name : sinks)
    node_map[name]->tick();

  bus b_;
  for (int j = 0; j < out.size(); j++)
    b_.push_back(out[j]->unbuffer()[0]);
    
  t++;
  return b_;
}

void supersystem::append_sys(string name, systm *sys) {
  node *n = new node(sys);
  nodes.push_back(n);
  node_map[name] = n;
}

void supersystem::append_inport(string name, inport *inp) {
  in.push_back(inp);
  node *n = new node((systm*) inp);
  nodes.push_back(n);
  node_map[name] = n;
}

void supersystem::append_outport(string name, outport *outp) {
  out.push_back(outp);
  node *n = new node((systm*) outp);
  nodes.push_back(n);
  node_map[name] = n;

  this->add_dims(outp->get_dims()[0]);
}

void supersystem::append_sink(string name) {
  sinks.push_back(name);
}

void supersystem::link_node(string to, string from, int i) {
  node_map[to]->append_link(node_map[from], i);
}
