#include <unistd.h>
#include <fstream>
#include <string>

#include "FenJiA.h"
#include "cholesky.h"
#include "Random.h"

std::istream& operator>>(std::istream& str,CSVRow& data)
{
    data.readNextRow(str);
    return str;
}   

FJASimulator::FJASimulator(std::string config_file) : SimulationCount( 0 ),
						      StockNumber( 0 ),
						      FactorNumber( 0 ),
						      Tag(""),
						      FJALength( 0 ) {
  int status = Config( config_file );
  if ( status ) exit(1);
  SetRandomNumberGenerator();
  DisplayConfig();
}

int FJASimulator::Config(std::string config_file ) {
  std::ifstream Cfile( config_file, std::ios::in );
  std::string feature, value;
  if ( ! Cfile.is_open() ) {
    std::cerr << "Can't open config file: " << config_file << std::endl;
    return 0;
  }
  while ( Cfile >> feature ) {
    //throw(feature.c_str());

    if ( feature == "SimulationCount" ) {
      Cfile >> SimulationCount;
    } else if ( feature == "Tag" ) {
      Cfile >> Tag; 
    } else if ( feature == "Sigma" ) {
      Cfile >> Sigma_file;
      int status = ReadSigma( Sigma_file );
      if ( status ) return status + 10;
    } else if ( feature == "FactorExposure" ) {
      Cfile >> FE_file;
      int status = ReadFactorExposure( FE_file );
      if ( status ) return status + 20;
    } else if ( feature == "Omega") {
      Cfile >> Omega_file; 
      int status = ReadOmega( Omega_file );
      if ( status ) return status + 30;
    } else if ( feature == "IndexWeight" ) {
      Cfile >> IW_file;
      int status = ReadIndexWeight( IW_file );
      if ( status ) return status + 40;
    } else if ( feature == "FJA_file" ) { 
      Cfile >> FJA_file;
      int status = ReadFJA( FJA_file );
      if ( status ) return status + 50;
    } else {
      std::cerr << "No such feature name: " << feature << std::endl;
      return 60;
    }
  }

  if ( SimulationCount == 0 ) {
    std::cerr << "SimulationCount should be set as a non-zero integer." << std::endl;
    return 70;
  } else if ( Tag == "" ) {
    std::cerr << "Tag should not be an empty string.\n";
    return 71;
  }
  
  std::cout << "Finished Reading config." << std::endl;
  return 0;
}

int FJASimulator::DisplayConfig( ) {
  std::cout << "<=============================";
  std::cout << " CONFIG ";
  std::cout << "=============================>\n";

  std::cout << std::setw(30) << std::right << "PID: " 
	    << std::left<< getpid() << std::endl;
  
  std::cout << std::setw(30) << std::right << "SimulationCount: " 
	    << std::left<< SimulationCount << std::endl;

  std::cout << std::setw(30) << std::right << "StockNumber: " 
	    << std::left<< StockNumber << std::endl;

  std::cout << std::setw(30) << std::right << "FactorNumber: " 
	    << std::left<< FactorNumber << std::endl;

  std::cout << std::setw(30) << std::right << "Tag: " 
 	    << std::left<< Tag << std::endl;

  std::cout << std::setw(30) << std::right << "FJA file: " 
 	    << std::left<< FJA_file << std::endl;

  std::cout << std::setw(30) << std::right << "IndexWeightFile: " 
 	    << std::left<< IW_file << std::endl;

  std::cout << std::setw(30) << std::right << "FactorExposureFile: "
 	    << std::left<< FE_file << std::endl;

  std::cout << std::setw(30) << std::right << "SigmaFile: " 
 	    << std::left<< Sigma_file << std::endl;

  std::cout << std::setw(30) << std::right << "OmegaFile: " 
 	    << std::left<< Omega_file << std::endl;
  
  std::cout << "<=============================";
  std::cout << "   END  ";
  std::cout << "=============================>\n";

  return 0;
}

int FJASimulator::SetRandomNumberGenerator() {
  ND.resize( IndexNumber );
  vec.resize( IndexNumber );
  NormalE.resize( IndexNumber );
  // 用时间和斐波那契数列设置伪随机数生成器的种子.
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  unsigned Seed = tv_now.tv_usec; //tv_now.tv_sec
  unsigned Seed_b = Seed * 2, tmp;
  for (size_t i = 0; i < IndexNumber; i ++) {
    ND[i].SetSeed( Seed );
    tmp = Seed;
    Seed = Seed + Seed_b;
    Seed_b = tmp;
  }

  // \f$ cov = W(X \Omega X^T + \Sigma)W^T \f$
  MatDoub cov(IndexNumber, IndexNumber);

  // \f$ B = WX \f$
  MatDoub B(IndexNumber, FactorNumber);
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    for ( size_t j = 0; j < FactorNumber; j ++ ) {
      B[ i ][ j ] = 0;
      for ( size_t k = 0; k < StockNumber; k ++ ) {
	B[ i ][ j ] += ( IndexWeight[ i ][ k ]
			 * FactorExposure[ k ][ j ]);
      }
    }
  }

  // \f$ C =  W X \Omega \f$
  MatDoub C(IndexNumber, FactorNumber);
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    for ( size_t j = 0; j < FactorNumber; j ++ ) {
      C[ i ][ j ]= 0;
      for ( size_t k = 0; k < FactorNumber; k ++ ) {
	C[ i ][ j ] += (B[ i ][ k ] * Omega[ k ][ j ]);
      }
    }
  }

  // \f$ D = W X \Omega X^T W^T
  MatDoub D(IndexNumber, IndexNumber);
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    for ( size_t j = 0; j < IndexNumber; j ++ ) {
      D[ i ][ j ] = 0;
      for ( size_t k = 0; k < FactorNumber; k ++ ) {
	D[ i ][ j ] += ( C[ i ][ k ] * B[ j ][ k ]);
      }
    }
  }

  // \f$ E = W \Sigma \f$
  MatDoub E(IndexNumber, StockNumber);
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    for ( size_t j = 0; j < StockNumber; j ++ ) {
      E[ i ][ j ] = IndexWeight[ i ][ j ] * StockArray[ j ].srisk;
    }
  }

  // \F$ F = W \Sigma W^T \f$
  MatDoub F(IndexNumber, IndexNumber);
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    for ( size_t j = 0; j < IndexNumber; j ++ ) {
      F[ i ][ j ]  = 0;
      for ( size_t k = 0; k < StockNumber; k ++ ) {
	F[ i ][ j ] += ( E[ i ][ k ] * IndexWeight[ j ][ k ] ); 
      }
    }
  }
  
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    for ( size_t j = 0; j < IndexNumber; j ++ ) {
      cov[ i ][ j ] = D[ i ][ j ] + F[ i ][ j ];
    }
    IndexArray[ i ].sigma2 = cov[ i ][ i ];
  }

  chol = new Cholesky( cov );

  return 0;
}

int FJASimulator::ReadSigma( std::string Sigma_file ) {
  std::cout << "-->Start Reading Sigma.\n";
  ifstream InFile( Sigma_file );
  CSVRow row; size_t k = 0;
  InFile >> row;
  while( InFile.good() ) {
    InFile >> row;
    if (row.size() == 0) continue;
    Stock s(row[0], std::stod( row[1] ), k);
    StockArray.push_back( s );
    StockMap[ row[0] ] = k;
    //s.print();
    k++;
  }
  StockNumber = StockArray.size();
  std::cout << "Reading Sigma Finished.\n";
  return 0;
}

int FJASimulator::ReadFactorExposure( std::string FE_file ) {
  std::cout << "-->Start Reading Factor Exposure Matrix.\n";
  if ( StockNumber == 0 ) {
    throw("Please put Stock Sigma before FactorExposure.");
    return 1;
  }
  ifstream InFile( FE_file );
  CSVRow row;
  InFile >> row;
  FactorNumber = row.size() - 1;
  for ( size_t i = 1; i < row.size(); i ++ ) {
    FactorNames.push_back( row[ i ] );
  }
  for ( size_t i = 0; i < StockNumber; i ++ ) {
    std::vector< double> buf( FactorNumber);
    FactorExposure.push_back( buf );
  }

  while ( InFile.good() ) {
    InFile >> row;
    size_t k = StockMap[ row[0] ];
    //std::cout << k << ": " << row[0] << std::endl;
    StockArray[k].FactorExposure = &FactorExposure[ k ];
    if (row.size() != FactorNumber + 1) throw("Wrong row length."); 
  }
  std::cout << "Reading Factor Exposure Matrix Finished.\n";
  return 0;
}

int FJASimulator::ReadOmega( std::string Omega_file ) {
  std::cout << "-->Start reading Omega, covariance of factors." << std::endl;
  ifstream InFile( Omega_file );

  if ( FactorNumber == 0 ) {
    std::cerr << "Please put FactorExposure before Omega.\n";
    return 1;
  }
  CSVRow row;
  InFile >> row;
  while ( InFile.good() ) {
    if (row.size() == 0) continue;
    std::vector< double > buf( FactorNumber );
    InFile >> row;
    for ( size_t i = 0; i < FactorNumber; i ++ ) {
      buf[ i ] = std::stod( row[ i + 1] );
    }
    Omega.push_back( buf );
  }
  std::cout << "Finishing Reading Omega.\n";
  return 0;
}

int FJASimulator::ReadIndexWeight( std::string IW_file ) {
  std::cout << "-->Start Reading Index Weight Matrix.\n";
  ifstream InFile( IW_file );
  CSVRow row;
  InFile >> row;
  IndexArray.resize( row.size() - 1);
  IndexNumber = IndexArray.size();

  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    std::vector< double > buf( StockNumber ); 
    IndexWeight.push_back( buf );    
    IndexMap[ row[i + 1] ] = &IndexArray[ i ];
  }

  for ( size_t i = 0; i < IndexNumber; i ++ ) 
    IndexArray[ i ].IndexWeight = &IndexWeight[ i ];

  while ( InFile.good() ) {
    InFile >> row;
    if (row.size() == 0) continue;
    size_t k = StockMap[ row[0] ];
    for ( size_t i = 0; i < row.size() - 1; i ++ ) {
      IndexWeight[ i ][ k ] = std::stod( row[i + 1] );
    }
  }

  std::cout << "Reading Index Weight Matrix Finished.\n";
  return 0;
}

int FJASimulator::ReadFJA( std::string FJA_file ) {
  std::ifstream INfile( FJA_file, std::ios::in );
  CSVRow feature, value;
  std::cout << "-->Start Reading FJA file.\n";
  if ( IndexNumber == 0 ) {
    throw("Please read Index Weight before FJA_file");
    return 1;
  }

  if ( ! INfile.is_open() ) {
    std::cerr << "Can't open FJA file: " << FJA_file << std::endl;
    return 0;
  }
  INfile >> feature;
  
  size_t id_i, AType_i, NAV_m_i, fee_i, leverage_ratio_i, fix_profit_i, index_i;
  for ( size_t i = 0; i < feature.size(); i ++ ) {
    if ( feature[ i ] == "iid" ) id_i = i;
    if ( feature[ i ] == "AType" ) AType_i = i;
    if ( feature[ i ] == "NAV_m" ) NAV_m_i = i;
    if ( feature[ i ] == "fee" ) fee_i = i;
    if ( feature[ i ] == "leverage_ratio" ) leverage_ratio_i = i;
    if ( feature[ i ] == "fix_profit" ) fix_profit_i = i;
    if ( feature[ i ] == "index" ) index_i = i;
  }
  //feature.print();
  while ( INfile.good() ) {
    INfile >> value;
    if (value.size() == 0) continue;
    FJABase *FJAbaseP = NewFJA( value[ AType_i ], value[ id_i ]);
    if (FJAbaseP == NULL) return 1;
    FJAbaseP -> NAV_m_init = std::stod( value[ NAV_m_i ] );
    FJAbaseP -> fee = std::stod( value[ fee_i ] );
    FJAbaseP -> leverage_ratio = std::stod( value[ leverage_ratio_i ] );
    FJAbaseP -> fix_profit = std::stod( value[ fix_profit_i ] );
    FJAbaseP -> TrackingIndex = IndexMap[ value[ index_i ] ];
    FJAarray.push_back( FJAbaseP ); 
  }
  FJALength = FJAarray.size( );
  return 0;
}

FJABase* FJASimulator::NewFJA( std::string FJA_name, std::string id) {
  if ( FJA_name == "common" ) {
    return new CommonA( id );
  } else {
    std::cerr << "No such Fen Ji A with name: " << FJA_name << std::endl;
    return NULL;
  }
}

int FJASimulator::GenerateRandomNumber() {
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
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


