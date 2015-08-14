#include <boost/filesystem.hpp>
#include "FenJiA.h"
#include "FenJiABase.h"

FJABase::FJABase( std::map<std::string, std::string> ValueMap, FJASimulator* FSP)
  :id(ValueMap["iid"]),
   NAV_m_init( std::stod( ValueMap[ "NAV_m" ]) ),
   NAV_A_init( std::stod( ValueMap[ "NAV_A" ]) ),
   fee( std::stod( ValueMap[ "fee" ] ) ),
   redemption_fee( std::stod( ValueMap[ "redemption_fee" ] ) ),
   leverage_ratio( std::stod( ValueMap[ "leverage_ratio" ])),
   fix_profit( std::stod( ValueMap[ "fix_profit" ] ) ) {
  if ( leverage_ratio < 1 ) {
    throw("leverage_ratio should be greater than 1.");
  }
  Simulator = FSP;
  TrackingIndex = FSP -> IndexMap[ ValueMap["index" ] ];
  sigma2 = FSP -> IndexArray[ TrackingIndex ].sigma2;
  updatem_fix = exp( (- fee - 0.5 * sigma2 ) * Simulator -> TimeDelta );
  // std::cout <<"update m:" << updatem_fix 
  // 	    <<"sigma2:" << sigma2
  // 	    <<"TimeDelta:" << Simulator -> TimeDelta
  // 	    << std::endl;
}

FJABase::~FJABase() {
}


int FJABase::Initialize() {

  End = false;
  data_t.clear();

  NAV_m = NAV_m_init;
  NAV_A = NAV_A_init;
  NUM_A = 1;
  NAV_A_vec.clear(); NAV_A_vec.push_back( NAV_A );
  NAV_m_vec.clear(); NAV_m_vec.push_back( NAV_m );
  Am2B();
  
  return 0;
}

int FJABase::Terminate() {
  End = true;
  data_set.push_back( data_t );
  return 0;
}

void FJABase::updatem() {

  NAV_m = NAV_m * updatem_fix 
    * exp( Simulator -> sqrtTimeDelta 
	   * Simulator -> NormalE[ TrackingIndex ] );
}

int FJABase::StepOn() {
  if ( End ) return End;
  
  updatem();  updateA();  Am2B();
  
  if ( up_condition() ) up_conversion();
  if ( down_condition() ) down_conversion();
  if ( Simulator -> SimulateTime - floor(  Simulator -> SimulateTime) 
       < Simulator -> TimeDelta ) fix_conversion();
  NAV_m_vec.push_back( NAV_m );
  NAV_A_vec.push_back( NAV_A );
  
  if ( terminate_condition() ) Terminate();
  
  return 0;
}

int FJABase::Stats() {
  return 0;
}

int FJABase::OutputPath() {
  char filename[100];
  sprintf(filename, "data/%s", Simulator -> Tag.c_str());
  boost::filesystem::create_directories(filename);
  sprintf(filename, "data/%s/path-%s-%02lu.csv", 
	  Simulator -> Tag.c_str(), id.c_str(),
	  Simulator -> Count);
  std::ofstream fp(filename);
  fp << "NAV_A,";
  for ( std::vector<double>::iterator it = NAV_A_vec.begin();
	it != NAV_A_vec.end(); it ++ ) {
    fp << (*it);
    if (it != NAV_A_vec.end() - 1) fp << ",";
    else fp << "\n";
  }

  fp << "NAV_m,";
  for ( std::vector<double>::iterator it = NAV_m_vec.begin();
	it != NAV_m_vec.end(); it ++ ) {
    fp << (*it);
    if (it != NAV_m_vec.end() - 1) fp << ",";
    else fp << "\n";
  }
  fp.close();
  return 0;
}

int FJABase::OutputDataSet() {
  char filename[100];
  sprintf(filename, "data/%s", Simulator -> Tag.c_str());
  boost::filesystem::create_directories(filename);
  sprintf(filename, "data/%s/DataSet-%s.csv", 
	  Simulator -> Tag.c_str(), id.c_str());
  ofstream fp(filename); size_t c = 0;
  for ( std::vector<FJAData>::iterator it = data_set.begin();
	it != data_set.end(); it ++ ) { c ++;
    fp << c <<"\n";

    fp << "up dates,";
    for ( std::vector<double>::iterator Dit = (*it).up_dates.begin();
	  Dit != (*it).up_dates.end(); Dit ++ ) {
      fp << *Dit;
      if ( Dit != (*it).up_dates.end() -1 ) fp << ",";
    }
    fp << "\n";
    
    fp << "down dates,";
    for ( std::vector<double>::iterator Dit = (*it).down_dates.begin();
	  Dit != (*it).down_dates.end(); Dit ++ ) {
      fp << *Dit;
      if ( Dit != (*it).down_dates.end() -1 ) fp << ",";
    }
    fp << "\n";
    fp << "currency dates,";
    for ( std::vector<std::pair<double, double> >::iterator Dit = (*it).currency.begin();
	  Dit != (*it).currency.end(); Dit ++ ) {
      fp << (*Dit).first ;
      if ( Dit != (*it).currency.end() -1 ) fp << ",";
    }
    fp << "\n";
    fp << "currency value,";
    for ( std::vector<std::pair<double, double> >::iterator Dit = (*it).currency.begin();
	  Dit != (*it).currency.end(); Dit ++ ) {
      fp << (*Dit).second;
      if ( Dit != (*it).currency.end() -1 ) fp << ",";
    }
    fp << "\n";
  }
  fp.close();
  return 0;
}

CommonA::CommonA( std::map<std::string, std::string> ValueMap, 
		  FJASimulator*  FSP):FJABase(ValueMap, FSP) {
  ;
}

void CommonA::updateA() {
  NAV_A = NAV_A * pow(( 1 + fix_profit), Simulator -> TimeDelta);
}

int CommonA::up_condition() {
  return ( NAV_m >= 2.0 ); 
}

int CommonA::up_conversion() {
  std::pair<double, double> cur( Simulator-> SimulateTime, 
				 NUM_A * (NAV_A - 1) 
				 * (1-redemption_fee ) );
  data_t.currency.push_back( cur );
  data_t.up_dates.push_back( Simulator -> SimulateTime );
  NAV_A = 1;
  NAV_B = 1;
  NAV_m = 1;
  return 0;
}

int CommonA::down_condition() {
  return ( NAV_B <= 0.25 );
}

int CommonA::down_conversion() {
  std::pair<double, double> cur( Simulator-> SimulateTime, 
				 NUM_A * (NAV_A - NAV_B)
				 * (1 - redemption_fee ) );
  data_t.currency.push_back( cur );
  data_t.down_dates.push_back( Simulator -> SimulateTime );
  NUM_A = NUM_A * NAV_B;
  NAV_A = 1;
  NAV_B = 1;
  NAV_m = 1;
  return 0;
}

int CommonA::fix_conversion() {
  std::pair<double, double> cur( Simulator-> SimulateTime, 
				 NUM_A * (NAV_A - 1));
  data_t.currency.push_back( cur );
  NAV_A = 1;
  AB2m();
  return 0;
}

int CommonA::terminate_condition() {
  if ( Simulator -> SimulateTime > 
       Simulator -> SimulationLength ) return 1;
  if ( NUM_A < Simulator -> StopRatio ) {
    std::pair<double, double> cur( Simulator-> SimulateTime, 
				 NUM_A * (NAV_A - 1));
    data_t.currency.push_back( cur );
    return 1;
  }
  
  return 0;
}
