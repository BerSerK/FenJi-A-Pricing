#define timer timer_class
#include <boost/progress.hpp>
#undef timer

#include <boost/timer/timer.hpp>

#include "FenJiA.h"

std::istream& operator>>(std::istream& str, CSVRow& data) {
  data.readNextRow(str);
  return str;
}   

FJASimulator::FJASimulator(std::string config_file) : SimulationNumber( 0 ),
						      SimulationLength( 0 ),
						      TimeDelta( 1.0/50 ),
						      StockNumber( 0 ),
						      FactorNumber( 0 ),
						      YearLength( 252. ),
						      Tag(""),
						      FJALength( 0 ) {

  int status = Config( config_file );
  if ( status ) exit(1);
  DisplayConfig();
}

int FJASimulator::Config(std::string config_file ) {
  std::ifstream Cfile( config_file, std::ios::in );
  std::string feature, value;
  if ( ! Cfile.is_open() ) {
    std::cerr << "Can't open config file: " << config_file << std::endl;
    return 1;
  }
  while ( Cfile >> feature ) {
    //throw(feature.c_str());

    if ( feature == "SimulationNumber" ) {
      Cfile >> SimulationNumber;
    } else if ( feature == "SimulationLength" ) {
      Cfile >> SimulationLength;
    } else if ( feature == "StopRatio" ) {
      Cfile >> StopRatio;
    } else if ( feature == "DiscountRate" ) {
      Cfile >> DiscountRate;
      DiscountRate *= 0.01;
    } else if ( feature == "FixRate" ) {
      Cfile >> FixRate;
      FixRate *= 0.01;
    } else if ( feature == "Tag" ) {
      Cfile >> Tag; 
    } else if ( feature == "Date" ) {
      Cfile >> value;
      boost::gregorian::date d( boost::gregorian::from_string( value ) );
      startDate = d;
      startDateofYear = d.day_of_year()/365.;
      //std::cout << startDate << std::endl;
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
      status = SetRandomNumberGenerator();
      if ( status ) return status + 50;
    } else if ( feature == "FJA_file" ) { 
      Cfile >> FJA_file;
      int status = ReadFJA( FJA_file );
      if ( status ) return status + 60;
    } else {
      std::cerr << "No such feature name: " << feature << std::endl;
      return 70;
    }
  }

  if ( SimulationNumber == 0 ) {
    std::cerr << "SimulationNumber should be set as a non-zero integer." << std::endl;
    return 80;
  } else if ( Tag == "" ) {
    std::cerr << "Tag should not be an empty string.\n";
    return 81;
  }
  // 设置
  sqrtTimeDelta = sqrt( TimeDelta );
  
  std::cout << "Finished Reading config." << std::endl;
  return 0;
}

int FJASimulator::DisplayConfig() {
  DisplayConfig( std::cout );
  return 0;
}

int FJASimulator::DisplayConfig(std::ostream& str) {
  str << "<=============================";
  str << " CONFIG ";
  str << "=============================>\n";

  str << std::setw(30) << std::left << "PID " 
	    << std::left<< getpid() << std::endl;
  
  str << std::setw(30) << std::left << "Number of Path" 
	    << std::left<< SimulationNumber << std::endl;

  str << std::setw(30) << std::left << "SimulationLength(year) " 
	    << std::left<< SimulationLength << std::endl;

  str << std::setw(30) << std::left << "StockNumber " 
	    << std::left<< StockNumber << std::endl;

  str << std::setw(30) << std::left << "FactorNumber " 
	    << std::left<< FactorNumber << std::endl;

  str << std::setw(30) << std::left << "IndexNumber " 
	    << std::left<< IndexNumber << std::endl;

  str << std::setw(30) << std::left << "FundNumber " 
	    << std::left<< FJALength << std::endl;

  str << std::setw(30) << std::left << "Tag " 
 	    << std::left<< Tag << std::endl;

  str << std::setw(30) << std::left << "StartDate " 
 	    << std::left<< startDate<<" "<<startDateofYear << std::endl;

  str << std::setw(30) << std::left << "FJA file " 
 	    << std::left<< FJA_file << std::endl;

  str << std::setw(30) << std::left << "IndexWeightFile " 
 	    << std::left<< IW_file << std::endl;

  str << std::setw(30) << std::left << "FactorExposureFile "
 	    << std::left<< FE_file << std::endl;

  str << std::setw(30) << std::left << "SigmaFile " 
 	    << std::left<< Sigma_file << std::endl;

  str << std::setw(30) << std::left << "OmegaFile " 
 	    << std::left<< Omega_file << std::endl;
  
  str << "<=============================";
  str << "   END  ";
  str << "=============================>\n";

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
  cov.resize(IndexNumber, IndexNumber);

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
  // std::cout <<"cov:"<<std::endl;
  // for ( size_t i = 0; i < IndexNumber; i ++ ) {
  //   for ( size_t j = 0; j < IndexNumber; j ++ ) {
  //     std::cout << cov[i][j];
  //     if (j == IndexNumber -1) std::cout << "\n";
  //     else std::cout << "\t";
  //   }
  // }
  // std::cout <<"chol:"<<std::endl;
  // for ( size_t i = 0; i < IndexNumber; i ++ ) {
  //   for ( size_t j = 0; j < IndexNumber; j ++ ) {
  //     std::cout << chol -> el[i][j];
  //     if (j == IndexNumber -1) std::cout << "\n";
  //     else std::cout << "\t";
  //   }
  // }

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
    double sigma = std::stod( row[1] );
    Stock s(row[0], sigma * sigma * YearLength, k);
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
    //    std::cout << k << ": " << row[0] << std::endl;
    StockArray[k].FactorExposure = &FactorExposure[ k ];
    for ( size_t i = 0; i < FactorNumber; i ++ ) {
      FactorExposure[k][i] = std::stod( row[i + 1]);
    }
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
      buf[ i ] = std::stod( row[ i + 1] ) * YearLength;
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
  //row.print();
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    std::vector< double > buf( StockNumber ); 
    IndexWeight.push_back( buf );    
    IndexMap[ row[i + 1] ] = i;
    IndexArray[ i ].IndexWeight = i;
    IndexArray[ i ].id = row[ i + 1 ];
  }

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
  std::map<std::string, std::string> ValueMap;
  //feature.print();
  while ( INfile.good() ) {
    INfile >> value;
    if (value.size() == 0) continue;
    for ( size_t i = 0; i < feature.size(); i ++ ) {
      ValueMap[ feature[i] ] = value[i];
    }
    FJABase *FJAbaseP = NewFJA( ValueMap, this);
    if (FJAbaseP == NULL) return 1;
    FJAarray.push_back( FJAbaseP ); 
  }
  FJALength = FJAarray.size( );
  if (FJALength == 0) {
    std::cerr << "Something wrong with the FJA file." << std::endl;
    return 1;
  }
  return 0;
}

FJABase* FJASimulator::NewFJA( std::map<std::string, std::string> ValueMap, FJASimulator* FSP) {
  if ( IndexMap.find( ValueMap["symbolIndex"] )
       == IndexMap.end() ) {
    std::cerr << "Wrong tracking index id:" 
	      << ValueMap["symbolIndex"] << std::endl;
    return NULL;
  }
  // for ( std::map<std::string, std::string>::iterator it = ValueMap.begin();
  // 	it != ValueMap.end(); it ++ ) {
  //   std::cout << it -> first << ":" << it -> second <<"\n";
  // }
  if ( ValueMap["type"] == "1" ) {
    return new CommonA( ValueMap, FSP);
  } else {
    std::cerr << "不存在分级A类别: " << ValueMap["type"] << std::endl;
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

int FJASimulator::Run() {
  int status;
  
  status = Simulate();
  if ( status ) return 100 + status;
  
  status = Stats();
  if ( status ) return 200 + status;
  
  status = Output();
  if ( status ) return 300 + status;

  return 0;
}

int FJASimulator::Simulate() {
  std::cout << "Simulating:";
  boost::timer::auto_cpu_timer Timer;
  std::vector< FJABase *>::iterator FJAit;
  Count = 0;
  boost::progress_display show_progress( SimulationNumber);
  while ( Count++ < SimulationNumber ) {
    ++ show_progress;
    SimulateTime = startDateofYear;
    
    for ( FJAit = FJAarray.begin(); FJAit != FJAarray.end(); FJAit ++ ) {
      (*FJAit) -> Initialize();
    }
    while ( TimeFromStart() <= SimulationLength) {
      GenerateRandomNumber();
      SimulateTime += TimeDelta;
      for ( FJAit = FJAarray.begin(); FJAit != FJAarray.end(); FJAit ++ ) {
	(*FJAit) -> StepOn();
      }
    }
    if ( Count <= 10 ) {
      for ( FJAit = FJAarray.begin(); FJAit != FJAarray.end(); FJAit ++ ) {
	(*FJAit) -> OutputPath();
      }
    }
  }
  
  return 0;
}

int FJASimulator::Stats() {
  for ( std::vector< FJABase *>::iterator FJAit = FJAarray.begin();
	FJAit != FJAarray.end(); FJAit ++ ) {
    (*FJAit) -> Stats();
  }
  return 0;
}

int FJASimulator::OutputCov() {
  char filename[100];
  sprintf(filename, "data/%s/cov.csv",
	  Tag.c_str());
  std::ofstream fp(filename);
  fp << "cov,";
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    fp << IndexArray[ i ].id;
    if ( i == IndexNumber - 1) {
      fp << '\n';
    } else {
      fp << ',';
    }
  }
  for ( size_t i = 0; i < IndexNumber; i ++ ) {
    fp << IndexArray[ i ].id << ",";
    for ( size_t j = 0; j < IndexNumber; j ++ ) {
      fp << cov[ i ][ j ];
      if ( j == IndexNumber - 1) {
	fp << '\n';
      } else {
	fp << ',';
      }
    }
  }
  fp.close();
  
  return 0;
}

int FJASimulator::OutputResults() {
  for ( std::vector< FJABase *>::iterator FJAit = FJAarray.begin();
	FJAit != FJAarray.end(); FJAit ++ ) {
    (*FJAit) -> OutputDataSet();
  }
  
  return 0;
}

int FJASimulator::Output() {
  
  std::cout << "Writing Data to disk.";

  char filename[100];
  sprintf(filename, "data/%s/conf.txt",
	  Tag.c_str());
  std::ofstream fp(filename);
  DisplayConfig(fp); fp.close();

  std::cout << ".";
  OutputCov();

  std::cout << ".";
  OutputResults();
  
  std::cout << ".\n";
  
  return 0;
}

