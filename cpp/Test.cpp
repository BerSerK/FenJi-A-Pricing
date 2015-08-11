#include <iostream>
#include <random>
#include <fstream>

#include "FenJiA.h"
#include "timer.h"
#include "Random.h"
#include "nr3.h"
#include "cholesky.h"


#define N  10

int TestCov() {
// std::string sss("-0.909318451910229");
// double ddd = std::stod(sss);
// std::cout << ddd << std::endl;

  FJASimulator sim( "config/config.txt" );
  std::ofstream fp("sample.txt");
  for ( int i = 0; i < 1000000; i ++ ) {
    sim.GenerateRandomNumber();
    for ( int j = 0; j < sim.IndexNumber; j ++ ) {
      fp << sim.NormalE[j] << "\t";
    }
    fp << std::endl;
  }
  return 0;
}

int TestCholesky() {
  std::ifstream ifile("orth_mat.txt");
  double **mat_a_buf;
  mat_a_buf = new double*[N];
  for (int i = 0; i < N; i ++ ) {
    mat_a_buf[i] = new double[N];
  }

  for ( int i = 0; i < N; i ++ ) {
    for ( int j = 0; j < N; j ++ ) {
      ifile >> mat_a_buf[i][j];
    }
  }

  MatDoub mat_a(N, N, mat_a_buf);

  Cholesky solver(mat_a);
  std::ofstream ofile("rst.txt");
  for ( int i = 0; i < N; i ++ ) {
    for ( int j = 0; j < N; j ++ ) {
      ofile << solver.el[j][i] << " ";
    }
    ofile << std::endl;
  }
  
  return 0;
}

int TestMyNormalDistribution() {
  const int nrolls = 1e6;
  const int nstars = 100;
  
  int p[10] = {};

  NormalDistribution ND(5.0, 2.0);
  
  Timer T; T.start();
  for (int i = 0; i < nrolls; ++ i) {
    double number = ND.number();
    if (( number >= 0.0 ) && ( number < 10.0 )) ++p[int(number)];
  }
  T.stop( 1);

  std::cout << "normal distribution (5.0, 2.0):" << std::endl;
  
  for (int i = 0; i < 10; ++ i) {
    std::cout << i << "-" << (i + 1) << ":";
    std::cout << std::string(p[i] * nstars /nrolls, '*') << std::endl;
  }

  return 0;
}

int TestNormalDistribution() {
  const int nrolls = 1e6;
  const int nstars = 100;
  
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(5.0, 2.0);
  
  int p[10] = {};

  generator.seed(10000);

  std ::cout << distribution(generator) << std::endl;
  Timer T; T.start();
  //for (int i = 0; i < nrolls; ++ i) {
  int pause;
  for (int i = 0; i < nrolls; i += pause) {
    double number = distribution(generator);
    if (( number >= 0.0 ) && ( number < 10.0 )) ++p[int(number)];
  }
  T.stop( 1);

  std::cout << "normal distribution (5.0, 2.0):" << std::endl;
  
  for (int i = 0; i < 10; ++ i) {
    std::cout << i << "-" << (i + 1) << ":";
    std::cout << std::string(p[i] * nstars * pause /nrolls, '*') << std::endl;
  }
  for (int i = 0; i < 10; ++ i) {
    p[ i ] = 0;
  }
  for (int i = 0; i < nrolls; ++ i) {
    double number = distribution(generator);
    if (( number >= 0.0 ) && ( number < 10.0 )) ++p[int(number)];
  }
    
  std::cout << "normal distribution (5.0, 2.0):" << std::endl;
  
  for (int i = 0; i < 10; ++ i) {
    std::cout << i << "-" << (i + 1) << ":";
    std::cout << std::string(p[i] * nstars /nrolls, '*') << std::endl;
  }

  return 0;
}

int main() {
  TestCov();
  //TestCholesky();
  //TestMyNormalDistribution();
  //TestNormalDistribution();
  return 0;
}
