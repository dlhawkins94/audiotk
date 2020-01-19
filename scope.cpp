#include "scope.hpp"

string to_str_fix(double val, int N) {
  ostringstream out;
  out.precision(N);
  out << std::fixed << val;
  return out.str();
}

axis::axis(rect base, double min, double max, int orient)
  : obj2(base), min(min), max(max), orient(orient)
{
  float pad = 2.0;
  N = 5;

  Vector2f major_pos, major_dim, tick_dim, tick_pos, tick_pos_inc;
  if (orient) {
    major_pos << w() - pad, 0.0;
    major_dim << 0.0, h();
    tick_dim << 5.0, 0.0;
    tick_pos = major_pos - tick_dim;
    tick_pos_inc << 0.0, (double) h() / (double) N;
  }
  else {
    major_pos << 0.0, pad;
    major_dim << w(), 0.0;
    tick_dim << 0.0, 5.0;
    tick_pos = major_pos;
    tick_pos_inc << (double) w() / (double) N, 0.0;
  }

  major = new line({major_pos, major_dim}, 1.0);
  
  for (int n = 0; n < N + 1; n++) {
    line *tick = new line({tick_pos, tick_dim}, 1.0);
    ticks.push_back(tick);

    sprite *text = print_text(to_str_fix(min + n * (max-min) / (double) N, 2));
    if (orient) {
      text->set_x(tick->x() - (pad + text->w()));
      text->set_y(tick->y());
    }
    else {
      text->set_x(tick->x());
      text->set_y(tick->y() + pad + tick->h());
    }
    labels.push_back(text);

    tick_pos += tick_pos_inc;
  }
}

axis::~axis() {
  delete major;
  for (auto &ln : ticks) delete ln;
  for (auto &lbl : labels) delete lbl;
}

void axis::render() {
  major->render_me();
  for (auto &ln : ticks) ln->render_me();
  for (auto &lbl: labels) lbl->render_me();
}

waterfall::waterfall(int M, int N, int w, int h, double min, double max)
  : scope("speech_recognition", w, h), M(M), N(N), min(min), max(max)
{
  int lmgn = 64;
  int rmgn = 64;
  int bmgn = 24;
  
  grid = new point_grid({Vector2f(lmgn, 0),
			 Vector2f(w - (lmgn + rmgn), h - bmgn)}, M, N);
  win.push_back(grid);

  hor = new axis({Vector2f(lmgn, h - bmgn),
		  Vector2f(w - (lmgn + rmgn), bmgn)}, 0, N-1, 0);
  win.push_back(hor);
  
  ver = new axis({Vector2f(0, 0), Vector2f(lmgn, h - bmgn)}, 0, M-1, 1);
  win.push_back(ver);
}

waterfall::~waterfall() {
  delete grid;
  delete hor;
  delete ver;
}


bus waterfall::apply(bus b) {
  VectorXf vec = b[0].samples.real().cast<float>();
  vec = (vec - min * VectorXf::Ones(N)) / (max - min);
  grid->buffer_vec(vec);
  win.draw();
  return bus({});
}

vector_scope::vector_scope(int w, int h, int N, double min, double max)
  : scope("speech_recognition", w, h), min(min), max(max)
{
  int lmgn = 64;
  int bmgn = 24;
  
  graph = new line_graph({Vector2f(lmgn, 0), Vector2f(w - lmgn, h - bmgn)}, N);
  win.push_back(graph);

  hor = new axis({Vector2f(lmgn, h-bmgn), Vector2f(w - lmgn, bmgn)}, 0, N-1, 0);
  win.push_back(hor);
}

vector_scope::~vector_scope() {
  delete graph;
  delete hor;
}

bus vector_scope::apply(bus b) {
  VectorXf vec = b[0].samples.real().cast<float>();
  graph->update(vec, min, max);
  win.draw();
  return bus({});
}

multiscope::multiscope(int w, int h,
		       vector<int> Ns,
		       vector<double> mins,
		       vector<double> maxs)
  : scope("speech_recognition", w, h), mins(mins), maxs(maxs)
{
  int lmgn = 64;
  int bmgn = 24;

  double y_ = 0;
  double dy = (h - bmgn) / Ns.size();

  for (auto &N: Ns) {
    line_graph *graph = new line_graph({Vector2f(lmgn, y_),
					Vector2f(w - lmgn, dy)}, N);
    graphs.push_back(graph);
    win.push_back(graph);
    
    y_ += dy;
  }

  hor = new axis({Vector2f(lmgn, h-bmgn), Vector2f(w-lmgn, bmgn)}, 0, Ns[0], 0);
  win.push_back(hor);
}

multiscope::~multiscope() {
  for (auto &graph : graphs) delete graph;
  for (auto &ver : vers) delete ver;
  delete hor;
}

bus multiscope::apply(bus b) {
  for (int m = 0; m < b.size(); m++) {
    VectorXf vec = b[m].samples.real().cast<float>();
    graphs[m]->update(vec, mins[m], maxs[m]);
  }
  win.draw();
  return bus({});
}
