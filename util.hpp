#pragma once

#include <iostream>

#include <sys/time.h>

using namespace std;

struct stopwatch {
  struct timeval start_time, stop_time;
  
  void start() { gettimeofday(&start_time, NULL); }
  void stop() { gettimeofday(&stop_time, NULL); }
  int dur() {
    return stop_time.tv_usec - start_time.tv_usec
      + (stop_time.tv_sec - start_time.tv_sec) * 1000000;
  }
};
