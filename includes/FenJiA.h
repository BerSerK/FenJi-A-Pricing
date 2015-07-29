#ifndef __FENJIA_H__
#define __FENJIA_H__

#include "FenJiABase.h"
#include "nr3.h"
#include "Random.h"

#include <vector>
#include <string>

class FJASimulator{
public:
  FJASimulator( std::string config_file );
  int Config( std::string config_file );
  int Simulate();

  double **cov;
  double **chol;
  NormalDistribution *ND;

  int GenerateRandomNumber( double *Epsilon);

  std::vector<FJABase*> FJAarray;
  int FJALength; /// Length of FJAarray, just for convinience.
  int SimulationCount;
};

class CommonA : public FJABase{
public:
  virtual int up_condition();
  virtual int up_conversion();
  virtual int down_condition();
  virtual int down_conversion();
  virtual int fix_conversion();
  virtual int terminate_condition();
};

#endif
