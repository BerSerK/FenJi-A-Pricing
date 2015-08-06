#include "FenJiA.h"
#include "cholesky.h"
#include "Random.h"

#include <fstream>
#include <string>

FJASimulator::FJASimulator(std::string config_file) : SimulationCount( 0 ),
						      StockNumber( 0 ),
						      FactorNumber( 0 ),
						      FJALength( 0 ) {
  int status = Config( config_file );
  if ( status ) exit(1);
  SetRandomNumberGenerator();
}

int FJASimulator::Config(std::string config_file ) {
  std::ifstream Cfile( config_file, std::ios::in );
  std::string feature, value;
  if ( ! Cfile.is_open() ) {
    std::cerr << "Can't open config file: " << config_file << std::endl;
    return 0;
  }
  while ( Cfile >> feature ) {
    
    if ( feature == "SimulationCount" ) {
      Cfile >> SimulationCount;
    } else if ( feature == "StockNumber" ) {
      Cfile >> StockNumber;
    } else if ( feature == "FactorNumber" ) {
      Cfile >> FactorNumber;
    } else if ( feature == "Tag" ) {
      Cfile >> Tag; 
    } else if ( feature == "FJA_file" ) { 
      Cfile >> value;
      int status = ReadFJA( value );
      if ( status ) return status + 10;
    } else if ( feature == "IndexWeight" ) {
      Cfile >> value;
      int status = ReadIndexWeight( value );
      if ( status ) return status + 20;
    } else if ( feature == "FactorExposure" ) {
      Cfile >> value;
      int status = ReadFactorExposure( value );
      if ( status ) return status + 30;
    } else if ( feature == "Sigma" ) {
      Cfile >> value;
      int status = ReadSigma( value );
      if ( status ) return status + 40;
    } else if ( feature == "Omega") {
      Cfile >> value; 
      int status = ReadOmega( value );
      if ( status ) return status + 50;
    } else {
      std::cerr << "No such feature name: " << feature << std::endl;
      return 60;
    }
  }
  std::cout << "Finished Reading config." << std::endl;
  return 0;
}

int FJASimulator::SetRandomNumberGenerator() {
  ND.resize( FJALength );
  vec.resize( FJALength );
  NormalE.resize( FJALength );
  
  // 用时间和斐波那契数列设置伪随机数生成器的种子.
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  unsigned Seed = tv_now.tv_usec; //tv_now.tv_sec
  unsigned Seed_b = Seed * 2, tmp;
  for (int i = 0; i < FJALength; i ++) {
    ND[i].SetSeed( Seed );
    tmp = Seed;
    Seed = Seed + Seed_b;
    Seed_b = tmp;
  }

  // \f$ cov = W(X \Omega X^T + \Sigma)W^T \f$
  MatDoub cov(FJALength, FJALength);

  // \f$ B = WX \f$
  MatDoub B(FJALength, FactorNumber);
  for ( int i = 0; i < FJALength; i ++ ) {
    for ( int j = 0; j < FactorNumber; j ++ ) {
      B[ i ][ j ] = 0;
      for ( int k = 0; k < StockNumber; k ++ ) {
	B[ i ][ j ] += ( IndexWeight[ i ][ k ]
			 * FactorExposure[ k ][ j ]);
      }
    }
  }
  // \f$ C =  W X \Omega \f$
  MatDoub C(FJALength, FactorNumber);
  for ( int i = 0; i < FJALength; i ++ ) {
    for ( int j = 0; j < FactorNumber; j ++ ) {
      C[ i ][ j ]= 0;
      for ( int k = 0; k < StockNumber; k ++ ) {
	C[ i ][ j ] += (B[ i ][ k ] * Omega[ k ][ j ]);
      }
    }
  }
  // \f$ D = W X \Omega X^T W^T
  MatDoub D(FJALength, FJALength);
  for ( int i = 0; i < FJALength; i ++ ) {
    for ( int j = 0; j < FJALength; j ++ ) {
      D[ i ][ j ] = 0;
      for ( int k = 0; k < FactorNumber; k ++ ) {
	D[ i ][ j ] += ( C[ i ][ k ] * B[ j ][ k ]);
      }
    }
  }
  // \f$ E = W \Sigma \f$
  MatDoub E(FJALength, StockNumber);
  for ( int i = 0; i < FJALength; i ++ ) {
    for ( int j = 0; j < StockNumber; j ++ ) {
      E[ i ][ j ] = IndexWeight[ i ][ j ] * Sigma[ j ];
    }
  }
  // \F$ F = W \Sigma W^T \f$
  MatDoub F(FJALength, FJALength);
  for ( int i = 0; i < FJALength; i ++ ) {
    for ( int j = 0; j < FJALength; j ++ ) {
      F[ i ][ j ]  = 0;
      for ( int k = 0; k < StockNumber; k ++ ) {
	F[ i ][ j ] += ( E[ i ][ k ] * IndexWeight[ j ][ k ] ); 
      }
    }
  }
  
  for ( int i = 0; i < FJALength; i ++ ) {
    for ( int j = 0; j < FJALength; j ++ ) {
      cov[ i ][ j ] = D[ i ][ j ] + F[ i ][ j ];
    }
    FJAarray[ i ] -> sigma2 = cov[ i ][ i ];
  }
  chol = new Cholesky( cov );

  return 0;
}

int FJASimulator::ReadFJA( std::string FJA_file ) {
  std::ifstream INfile( FJA_file, std::ios::in );
  std::string buf;
  std::string feature, value;
  if ( ! INfile.is_open() ) {
    std::cerr << "Can't open FJA file: " << FJA_file << std::endl;
    return 0;
  }

  while ( INfile >> buf ) {
    if ( buf == "<==") { // "<==" indicates a new one.
      FJABase * FJAbaseP;
      while ( INfile >> feature ) {
	if ( feature == "==>" ) break; // A FJA finished.
	INfile >> value;
	if ( feature == "AType" ) {
	  FJAbaseP = NewFJA( value );
	  if ( FJAbaseP == NULL ) return 1;
	} else if ( feature == "NAV_m" ) {
	  FJAbaseP -> NAV_m_init = atof( value.c_str() );
	} else if ( feature == "fee" ) {
	  FJAbaseP -> fee = atof( value.c_str() );
	} else if ( feature == "leverage_ratio" ) {
	  FJAbaseP -> leverage_ratio = atof( value.c_str() );
	} else if ( feature == "fix_profit" ) {
	  FJAbaseP -> fix_profit = atof( value.c_str() );
	} else {
	  std::cerr << "Wrong feature name: " << feature << std::endl;
	  return 2;
	}
      }
      FJAarray.push_back( FJAbaseP ); /// finished read one FJA.
    }
  }
  FJALength = FJAarray.size( );
  return 0;
}

int FJASimulator::ReadIndexWeight( std::string IW_file ) {
  std::cout << "Start Reading Index Weight Matrix.\n";
  if ( StockNumber == 0 ) {
    std::cerr << "Please put StockNumber before IndexWeight.\n";
    return 1;
  }
  if ( FJALength == 0 ) {
    std::cerr << "Please put FJA before IndexWeight.\n";
    return 2;
  }
  ifstream InFile( IW_file );
  for ( int i = 0; i < FJALength; i ++ ) {
    std::vector< double > buf( StockNumber );
    IndexWeight.push_back(buf);
  }
  for ( int i = 0; i < FJALength; i ++ ) {
    for ( int j = 0; j < StockNumber; j ++ ) {
      InFile >> IndexWeight[ i ][ j ];
    }
  }
  std::cout << "Reading Index Weight Matrix Finished.\n";
  return 0;
}

int FJASimulator::ReadFactorExposure( std::string FE_file ) {
  std::cout << "Start Reading Factor Exposure Matrix.\n";
  if ( StockNumber == 0 ) {
    std::cerr << "Please put StockNumber before FactorExposure.\n";
    return 1;
  }
  if ( FactorNumber == 0 ) {
    std::cerr << "Please put FactorNumber before FactorExposure.\n";
    return 2;
  }
  ifstream InFile( FE_file );
  for ( int i = 0; i < StockNumber; i ++ ) {
    std::vector< double > buf( FactorNumber );
    FactorExposure.push_back(buf);
  }
  for ( int i = 0; i < StockNumber; i ++ ) {
    for ( int j = 0; j < FactorNumber; j ++ ) {
      InFile >> FactorExposure[ i ][ j ];
    }
  }
  std::cout << "Reading Factor Exposure Matrix Finished.\n";
  return 0;
}

int FJASimulator::ReadSigma( std::string Sigma_file ) {
  std::cout << "Start Reading Sigma.\n";
  if ( StockNumber == 0 ) {
    std::cerr << "Please put StockNumber before FactorExposure.\n";
    return 1;
  }
  ifstream InFile( Sigma_file );
  Sigma.resize( StockNumber );
  for ( int i = 0; i < StockNumber; i ++ ) {
    InFile >> Sigma[i];
  }
  std::cout << "Reading Sigma Finished.\n";
  return 0;
}

int FJASimulator::ReadOmega( std::string Omega_file ) {
  std::cout << "Reading Omega, covariance of factors." << std::endl;
  ifstream InFile( Omega_file );
  if ( FactorNumber == 0 ) {
    std::cerr << "Please put Factor number before Omega (covariance of factors)" << std::endl;
    return 1;
  }
  for ( int i = 0; i < FactorNumber; i ++ ) {
    std::vector< double > buf( FactorNumber );
    Omega.push_back( buf );
  }
  for ( int i = 0; i < FactorNumber; i ++ ) {
    for ( int j = 0; j < FactorNumber; j ++ ) {
      InFile >> Omega[i][j];
    }
  }
  std::cout << "Finishing Reading Omega.\n";
  return 0;
}

FJABase* FJASimulator::NewFJA( std::string FJA_name ) {
  if ( FJA_name == "common" ) {
    return new CommonA;
  } else {
    std::cerr << "No such Fen Ji A with name: " << FJA_name << std::endl;
    return NULL;
  }
}

int FJASimulator::GenerateRandomNumber() {
  for ( int i = 0; i < FJALength; i ++ ) {
    vec[ i ] = ND[i].number();
  }
  chol -> elmult(vec, NormalE);

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
