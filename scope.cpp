#include "scope.hpp"

waterfall::waterfall(int N, int M, int w, int h, double min, double max)
  : N(N), M(M), win("speech_recognition", w, h),
    min(min), max(max), pc(M * N)
{
  for (int m = 0; m < M; m++) {
    for (int n = 0; n < N; n ++) {
      pc.points[m * N + n].pos[0] = (float) win.w * n / N;
      pc.points[m * N + n].pos[1] = (float) win.h * m / M;
    }
  }
}

bus waterfall::apply(bus b) {
  frame f = b[0];
  
  for (int m = M - 1; m > 0; m--) {
    for (int n = 0; n < N; n++) {
      int i = m * N + n;
      int j = i - N;
      pc.points[i].col[2] = pc.points[j].col[2];
      pc.points[i].col[0] = pc.points[i].col[1] = pc.points[i].col[2];
    }
  }
  for (int n = 0; n < N; n++) {
    pc.points[n].col[2] = real((f[n] - min) / (max - min));
    pc.points[n].col[0] = pc.points[n].col[1] = pc.points[n].col[2];
  }

  glfwMakeContextCurrent(win.win);
  
  glClear(GL_COLOR_BUFFER_BIT);
  glLoadIdentity();
  glPointSize(10.0);

  glBindBuffer(GL_ARRAY_BUFFER, pc.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(point_2d) * pc.points.size(),
	       pc.points.data(), GL_STREAM_DRAW);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glVertexPointer(2, GL_FLOAT, sizeof(point_2d),
		  (void*) offsetof(point_2d, pos));
  glColorPointer(3, GL_FLOAT, sizeof(point_2d),
		 (void*) offsetof(point_2d, col));
  glDrawArrays(GL_POINTS, 0, pc.points.size());

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
    
  glfwSwapBuffers(win.win);

  return bus({});
}
