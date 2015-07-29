#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys/time.h>
#include <iostream>

class Timer{
 public:
  void start() {
    gettimeofday(&tv_start, NULL);
  }

  double stop(int flag) {
    gettimeofday(&tv_now, NULL);
    seconds = 1.0 * tv_now.tv_sec - tv_start.tv_sec + 1e-6 * ( tv_now.tv_usec - tv_start.tv_usec);
    if (flag) {
      std::cout << seconds
		<<" seconds elapsed."<< std::endl;
    }
    return seconds;
  }
  
 private:
  struct timeval tv_start, tv_now;
  double seconds;
};
#endif // __TIMER_H__
