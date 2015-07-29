#include "FenJiA.h"
#include "cholesky.h"
#include "Random.h"

#include <fstream>
#include <string>

FJASimulator::FJASimulator(std::string config_file) {
  Config( config_file );
  ND = new NormalDistribution[ FJALength ];

  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  unsigned Seed = tv_now.tv_usec; //tv_now.tv_sec
  unsigned Seed_b = Seed * 2, tmp;
  for (int i = 0; i < FJALength; i ++) {
    ND[i].SetSeed(Seed);
    
    tmp = Seed;
    Seed = Seed + Seed_b;
    Seed_b = tmp;
  }
}

int FJASimulator::Config(std::string config_file ) {
  std::ifstream Cfile( config_file, std::ios::in );
  std::string buf;
  std::string feature, value;
  if ( ! Cfile.is_open() ) {
    std::cerr << "Can't open config file: " << config_file << std::endl;
    return 0;
  }
  while ( Cfile >> buf ) {
    //std::cout << buf << std::endl;
    if ( buf == "<==") { /// "<==" indicates a new one.
      //std::cout << "A new FJA" << std::endl;
      FJABase * FJAbaseP;
      while ( Cfile >> feature ) {
	if ( feature == "==>" ) break;
	Cfile >> value;
	if ( feature == "AType" ) {
	  if ( value == "common" ) {
	    FJAbaseP = new CommonA;
	  }
	} else if ( feature == "NAV_m" ) {
	  FJAbaseP -> NAV_m_init = atof( value.c_str() );
	} else if ( feature == "mu" ) {
	  FJAbaseP -> mu = atof( value.c_str() );
	} else if ( feature == "fee" ) {
	  FJAbaseP -> fee = atof( value.c_str() );
	} else if ( feature == "sigma" ) {
	  FJAbaseP -> sigma = atof( value.c_str() );
	} else if ( feature == "leverage_ratio" ) {
	  FJAbaseP -> leverage_ratio = atof( value.c_str() );
	} else if ( feature == "fix_profit" ) {
	  FJAbaseP -> fix_profit = atof( value.c_str() );
	} else {
	  std::cerr << "Wrong feature name: " << feature << std::endl;
	}
      }
      FJAarray.push_back( FJAbaseP ); /// finished read one FJA.
    } else if ( buf == "cov_file") {
      std::cout << "readin cov_file" << std::endl;
      Cfile >> value;
      ifstream cov_file( value );
      FJALength = FJAarray.size();
      cov = new double *[FJALength];
      for ( int i = 0; i < FJALength; i ++ ) {
	cov[i] = new double[FJALength];
      }
      chol= new double *[FJALength];
      for ( int i = 0; i < FJALength; i ++ ) {
	chol[i] = new double[FJALength];
      }
      for ( int i = 0; i < FJALength; i ++ ) {
	for ( int j = 0; j < FJALength; j ++ ) {
	  cov_file >> cov[ i ][ j ];
	}
      }
      MatDoub mat_a(FJALength, FJALength, cov);
      Cholesky solver(mat_a);
      for ( int i = 0; i < FJALength; i ++ ) {
	for ( int j = 0; j < FJALength; j ++ ) {
	  chol[i][j] = solver.el[i][j];
	  std::cout << chol[i][j] << "\t";
	}
	std::cout << std::endl;
      }
    } else if ( buf == "simulation_count") {
      Cfile >> SimulationCount;
    }
  }
  std::cout << "Finished Reading config." << std::endl;
  return 0;
}

int FJASimulator::GenerateRandomNumber(double *Epsilon) {
  double *vec = new double[ FJALength ];

  for ( int i = 0; i < FJALength; i ++ ) {
    vec[ i ] = ND[i].number();
    Epsilon[ i ] = 0;
  }
  
  for ( int i = 0; i < FJALength; i ++ ) {
    for ( int j = 0; j < FJALength; j ++ ) {
      Epsilon[ i ] += ( vec[ j ] * chol[ i ][ j ] );
    }
  }

  delete vec;
  return 0;
}

int CommonA::up_condition() {
  return ( NAV_m >= 2.0 ); 
}

int CommonA::up_conversion() {///TODO
  return 0;
}

int CommonA::down_condition() {
  return ( NAV_B <= 0.25 );
}

int CommonA::down_conversion() {
  return 0;
}

int CommonA::fix_conversion() {
  return 0;
}

int CommonA::terminate_condition() {
  return 0;
}
