/**
 * @file   timer.h
 * @author YeShiwei <yeshiwei.math@gmail.com>
 * @date   Tue Aug  4 15:24:49 2015
 * 
 * @brief  Timer based on gettimefoday in sys/time.h, for real running time, not for CPUTIME.
 * 
 * 
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys/time.h>
#include <iostream>

class Timer{
 public:
  
  /** 
   * Start timing.
   * 
   */
  void start() {
    gettimeofday(&tv_start, NULL);
  }

  /** 
   * Mark time.
   * 
   * @param flag Output the time or not. 1 Yes, 0 No.
   * 
   * @return 
   */
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
  struct timeval tv_start; /**< Timestamp of the start time. */
  struct timeval tv_now; /**< Timestamp of the mark time. */
  double seconds; /**< Time elpased since the start time till the mark time in seconds. */
};

#endif // __TIMER_H__
