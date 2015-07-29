/**
 * @file   Random.h
 * @author YeShiwei <yeshiwei.math@gmail.com>
 * @date   Tue Jul 14 16:11:37 2015
 * 
 * @brief  Random number generators wrapped from the C++ 11 std class.
 *    
 * 
 */

#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <iostream>
#include <random>
#include <sys/time.h>

class NormalDistribution{	/**< A Normal Distribution Generator. */
public:

  NormalDistribution():
    distribution(0, 1) {
    ;
  }
  
  NormalDistribution(double mean, double stddev):
    distribution( mean, stddev) {
    
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    unsigned Seed = tv_now.tv_usec; //tv_now.tv_sec
    generator.seed( Seed );
    
  }

  /** 
   * If object is built with no paramter, set these parameter with this function.
   * 
   * @param mean 
   * @param stddev 
   * @param Seed 
   * 
   * @return 
   */
  int SetSeed(unsigned Seed) {
    generator.seed( Seed);
    return 0;
  }
  
  /** 
   *  Get the next random number.
   * 
   * 
   * @return the next random number.
   */
  inline double number() {
    return distribution(generator);
  }
  
private:
  /// random number generator.
  std::default_random_engine generator;
  /// Normal distribution, mean and stddev will be given in the Constructor.
  std::normal_distribution<double> distribution;
};

#endif
