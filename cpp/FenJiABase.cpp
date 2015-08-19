#include <boost/filesystem.hpp>
#include <boost/progress.hpp>
#include <ql/math/solvers1d/newton.hpp>

#include "FenJiA.h"
#include "FenJiABase.h"

size_t FJAData::MAX_UP_LENGTH = 0;
size_t FJAData::MAX_DOWN_LENGTH = 0;
size_t FJAData::MAX_CUR_LENGTH = 0;

std::ostream& operator<<(std::ostream& fp, FJAData&data) {
    fp << "up dates,";
    for ( std::vector<double>::iterator Dit = data.up_dates.begin();
	  Dit != data.up_dates.end(); Dit ++ ) {
      fp << *Dit;
      if ( Dit != data.up_dates.end() -1 ) fp << ",";
    }
    fp << "\n";
    
    fp << "down dates,";
    for ( std::vector<double>::iterator Dit = data.down_dates.begin();
	  Dit != data.down_dates.end(); Dit ++ ) {
      fp << *Dit;
      if ( Dit != data.down_dates.end() -1 ) fp << ",";
    }
    fp << "\n";
    fp << "currency dates,";
    for ( std::vector<std::pair<double, double> >::iterator Dit = data.currency.begin();
	  Dit != data.currency.end(); Dit ++ ) {
      fp << (*Dit).first ;
      if ( Dit != data.currency.end() -1 ) fp << ",";
    }
    fp << "\n";
    fp << "currency value,";
    for ( std::vector<std::pair<double, double> >::iterator Dit = data.currency.begin();
	  Dit != data.currency.end(); Dit ++ ) {
      fp << (*Dit).second;
      if ( Dit != data.currency.end() -1 ) fp << ",";
    }
    fp << "\n";
  return fp;
}   

FJABase::FJABase( std::map<std::string, std::string> ValueMap, FJASimulator* FSP)
  :id(ValueMap["symbolA"]),
   symbolM(ValueMap["symbolM"]),
   NAV_m_init( std::stod( ValueMap[ "navM" ]) ),
   NAV_A_init( std::stod( ValueMap[ "navA" ]) ),
   price(std::stod( ValueMap[ "priceA" ]) ),
   fix_profit( 0.01 * std::stod( ValueMap[ "rate" ] ) ),
   ifrateFixed( std::stoi( ValueMap[ "ifRateFixed" ] ) ),
   leverage_ratio( std::stod( ValueMap[ "leverage" ] ) ),
   rateType( std::stoi( ValueMap[ "rateType" ] ) ),
   fee( 0.01 * std::stod( ValueMap[ "fee" ] ) ),
   up_triger(std::stod( ValueMap[ "up" ]) ),
   down_triger(std::stod( ValueMap[ "down" ]) ),
	  redemption_fee( 0.01 * std::stod( ValueMap[ "redemption_fee" ] ) ),
	  expiry(1e10) {

  if ( leverage_ratio < 1 ) {
    throw("leverage_ratio should be greater than 1.");
  }

  Simulator = FSP;
  TrackingIndex = FSP -> IndexMap[ ValueMap[ "symbolIndex" ] ];
  sigma2 = FSP -> IndexArray[ TrackingIndex ].sigma2;
  updatem_fix = exp( (- fee - 0.5 * sigma2 ) * Simulator -> TimeDelta );
  if ( ifrateFixed == 0 ) {
    fix_profit += FSP -> FixRate;
  }

  if ( ValueMap["expiry"].size() > 0 ) {
    boost::gregorian::date d( boost::gregorian::from_string( ValueMap["expiry"] ) );
    boost::gregorian::date_period dp(FSP -> startDate, d);
    expiry = (dp.length().days()/365.);
    //std::cout << d <<"\t"<<dp<<"\t"<<dp.length()<<"\t"<<expiry<< std::endl;
  }
}

FJABase::~FJABase() {
}


int FJABase::Initialize() {

  End = false;
  data_t.clear();
  data_t.price = price;

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
  
  double step_ratio = updatem_fix 
    * exp( Simulator -> sqrtTimeDelta 
	   * Simulator -> NormalE[ TrackingIndex ] );
  if (step_ratio > up_limit) step_ratio = up_limit;
  if (step_ratio < low_limit) step_ratio = low_limit;
  
  NAV_m = NAV_m * step_ratio;
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
  for ( std::vector<FJAData>::iterator it = data_set.begin();
	it != data_set.end(); it ++ ) {
    it -> NPV = it -> get_NPV( Simulator -> DiscountRate );
    it -> getDuration(Simulator -> DiscountRate );
  }

  QuantLib::Real xAcu = 1e-8;
  QuantLib::Newton solver;
  solver.setMaxEvaluations(1e3);
  //  std::cout << id << std::endl;

  for ( std::vector<FJAData>::iterator it = data_set.begin();
	it != data_set.end(); it ++ ) {
    //std::cout << c++ << "\n";
    // for ( size_t i = 0; i < it -> currency.size(); i ++ ) {
    //   std::cout << it -> currency[i].first << ",";
    // }
    // std::cout << "\n";
    // for ( size_t i = 0; i < it -> currency.size(); i ++ ) {
    //   std::cout << it -> currency[i].second << ",";
    // }

    //    std::cout << price << "\n";
    it -> IRR = solver.solve((*it), xAcu, 0.1, -0.9, 1e8);
    //std::cout << "IRR:" << it -> IRR << std::endl;
  }
  
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
  fp << data_t;
  fp.close();
  return 0;
}

int FJABase::OutputDataSet() {
  char filename[100];
  sprintf(filename, "data/%s", Simulator -> Tag.c_str());
  boost::filesystem::create_directories(filename);
  sprintf(filename, "data/%s/DataSet-%s.csv", 
	  Simulator -> Tag.c_str(), id.c_str());
  ofstream fp(filename);
  fp << FJAData::CSVhead() << "\n";
  for ( std::vector<FJAData>::iterator it = data_set.begin();
	it != data_set.end(); it ++ ) {
    fp << it -> toCSVline();
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
  if (rateType == 1) {
    NAV_A = NAV_A * pow(( 1 + fix_profit), Simulator -> TimeDelta);
  } else {
    NAV_A = NAV_A + fix_profit * Simulator -> TimeDelta;
  }
}

int CommonA::up_condition() {
  return ( NAV_m >= up_triger ); 
}

int CommonA::up_conversion() {

  data_t.add_currency( Simulator-> TimeFromStart(), 
		       NUM_A * (NAV_A - 1) 
		       * (1-redemption_fee ) );
  
  data_t.add_up_date( Simulator -> TimeFromStart());
  NAV_A = 1;
  NAV_B = 1;
  NAV_m = 1;
  return 0;
}

int CommonA::down_condition() {
  return ( NAV_B <= down_triger );
}

int CommonA::down_conversion() {
  data_t.add_currency( Simulator-> TimeFromStart(), 
				 NUM_A * (NAV_A - NAV_B)
				 * (1 - redemption_fee ) );
  data_t.add_down_date( Simulator -> TimeFromStart() );
  NUM_A = NUM_A * NAV_B;
  NAV_A = 1;
  NAV_B = 1;
  NAV_m = 1;
  return 0;
}

int CommonA::fix_conversion() {
  data_t.add_currency( Simulator-> TimeFromStart(), 
		       NUM_A * (NAV_A - 1) 
		       * (1 - redemption_fee ) );
  NAV_A = 1;
  AB2m();
  return 0;
}

int CommonA::terminate_condition() {
  if ( Simulator -> TimeFromStart() > expiry ) {
    data_t.add_currency( Simulator -> TimeFromStart(),
			 NUM_A * NAV_A );
    return 1;
  }
  if ( Simulator -> TimeFromStart() >
       Simulator -> SimulationLength - Simulator->TimeDelta) return 1;
  if ( NUM_A < Simulator -> StopRatio ) {
    /// 多次下折之后大部分资金已经赎回, 假设剩余资金一次赎回, 终止模拟.
    data_t.currency.back().second += NUM_A * NAV_A;
    return 1;
  }
  
  return 0;
}
