#include <boost/filesystem.hpp>
#include <boost/progress.hpp>
#include <ql/math/solvers1d/newton.hpp>
#include <algorithm>

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
	  redemption_fee( 0.01 * 0.5 ),
	  position(0.95),
	  expiry(1e10) {

  if ( leverage_ratio < 1 ) {
    throw("leverage_ratio should be greater than 1.");
  }
  try {
    redemption_fee = 0.01 * std::stod( ValueMap[ "redemption_fee" ] ) ;
  } catch (const std::invalid_argument& ia) {
    ;
  }
  Simulator = FSP;
  TrackingIndex = FSP -> IndexMap[ ValueMap[ "symbolIndex" ] ];
  sigma2 = FSP -> IndexArray[ TrackingIndex ].sigma2;
  updatem_fix = exp( (- fee - 0.5 * sigma2 ) * Simulator -> TimeDelta );
  //updatem_fix = exp( (- fee ) * Simulator -> TimeDelta );
  if ( ifrateFixed == 0 ) {
    fix_profit += FSP -> FixRate;
    //std::cout << id <<":"<< fix_profit<< std::endl;
  }

  try {
    boost::gregorian::date d( boost::gregorian::from_string( ValueMap["expiry"] ) );
    boost::gregorian::date_period dp(FSP -> startDate, d);
    expiry = (dp.length().days()/365.);
    //std::cout << d <<"\t"<<dp<<"\t"<<dp.length()<<"\t"<<expiry<< std::endl;
  } catch (const boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::bad_lexical_cast> >& e) {
    ;
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
  Am2B();
  NAV_A_vec.clear(); NAV_A_vec.push_back( NAV_A );
  NAV_m_vec.clear(); NAV_m_vec.push_back( NAV_m );
  NAV_B_vec.clear(); NAV_B_vec.push_back( NAV_B );
  
  return 0;
}

int FJABase::Terminate() {
  End = true;
  data_t.add_currency( Simulator -> TimeFromStart(),
		       NUM_A * NAV_A );
  data_set.push_back( data_t );
  return 0;
}

void FJABase::updatem() {
  double step_ratio = updatem_fix 
    * exp( Simulator -> sqrtTimeDelta 
	   * Simulator -> NormalE[ TrackingIndex ] );
  if (step_ratio > up_limit) step_ratio = up_limit;
  if (step_ratio < low_limit) step_ratio = low_limit;
  
  NAV_m = NAV_m * ( position * step_ratio + 1 - position);
}

int FJABase::StepOn() {
  if ( End ) return 1;
  
  updatem();  updateA();  Am2B();
  
  if ( up_condition() ) up_conversion();
  if ( down_condition() ) down_conversion();
  if ( fix_condition() ) fix_conversion();

  NAV_m_vec.push_back( NAV_m );
  NAV_A_vec.push_back( NAV_A );
  NAV_B_vec.push_back( NAV_B );
  
  if ( terminate_condition() ) Terminate();
  
  return 0;
}

int FJABase::Stats() {
  std::vector <double> NPV, duration, IRR;
  for ( std::vector<FJAData>::iterator it = data_set.begin();
	it != data_set.end(); it ++ ) {
    it -> NPV = it -> get_NPV( Simulator -> DiscountRate );
    NPV.push_back( it -> NPV);
    it -> getDuration(Simulator -> DiscountRate );
    duration.push_back( it -> duration );
  }

  QuantLib::Real xAcu = 1e-8;
  QuantLib::Newton solver;
  solver.setMaxEvaluations(1e3);

  for ( std::vector<FJAData>::iterator it = data_set.begin();
	it != data_set.end(); it ++ ) {
    try {
      it -> IRR = solver.solve((*it), xAcu, 0.1, -0.99, 1e8);
    } catch (const QuantLib::Error& E) {
      std::cout << E.what() << std::endl;
      std::cout << id <<":" << price <<  std::endl;
      for ( size_t i = 0; i < it -> currency.size(); i ++ ) {
	std::cout << it -> currency[i].first << ",";
      }
      std::cout << "\n";
      for ( size_t i = 0; i < it -> currency.size(); i ++ ) {
	std::cout << it -> currency[i].second << ",";
      }
      std::cout << "\n" << price << "\n";
    }
    // std::cout << "IRR:" << it -> IRR << std::endl;
    IRR.push_back( it -> IRR);
  }

  sort(NPV.begin(), NPV.end());
  sort(duration.begin(), duration.end());
  sort(IRR.begin(), IRR.end());
  int mid = NPV.size() / 2;

  median.NPV = NPV[ mid ];
  median.duration = duration[mid];
  median.IRR = IRR[ mid ];

  return 0;
}

int FJABase::OutputPath() {
  char filename[100];
  sprintf(filename, "data/%s/path", Simulator -> Tag.c_str());
  boost::filesystem::create_directories(filename);
  sprintf(filename, "data/%s/path/%s-%02lu.csv", 
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

  fp << "NAV_B,";
  for ( std::vector<double>::iterator it = NAV_B_vec.begin();
	it != NAV_B_vec.end(); it ++ ) {
    fp << (*it);
    if (it != NAV_B_vec.end() - 1) fp << ",";
    else fp << "\n";
  }

  fp << data_t;
  fp.close();
  return 0;
}

int FJABase::OutputDataSet() {
  char filename[100];
  sprintf(filename, "data/%s/DataSet/%s.csv", 
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

int FJABase::terminate_condition() {
  if ( Simulator -> TimeFromStart() > expiry ) {
    /// 到达到期日.
    return 1;
  }

  if ( NUM_A < Simulator -> StopRatio ) {
    /// 多次下折之后大部分资金已经赎回, 假设剩余资金一次赎回, 终止模拟.
    return 1;
  }
  
  return 0;
}


CommonA::CommonA( std::map<std::string, std::string> ValueMap, 
		  FJASimulator*  FSP):FJABase(ValueMap, FSP) {
  ;
}

int CommonA::StepOn() {
  return FJABase::StepOn();
}

void CommonA::updateA() {
  if ( rateType == 1 ) {
    NAV_A = NAV_A * pow(( 1 + fix_profit), Simulator -> TimeDelta);
  } else {
    NAV_A = NAV_A + fix_profit * Simulator -> TimeDelta;
  }
}

int CommonA::Initialize() {
  return FJABase::Initialize();
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

int CommonA::fix_condition() {
  if (Simulator -> SimulateTime - floor(  Simulator -> SimulateTime) 
      < Simulator -> TimeDelta) return 1;
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
  return FJABase::terminate_condition();
}

FJA2::FJA2( std::map<std::string, std::string> ValueMap, 
	    FJASimulator*  FSP):FJABase(ValueMap, FSP),
				last_fix_conversion_init(0),
				fix_triger(0.15),
				conversion_period(3.0) {
  std::map<std::string, std::string>::iterator it;
  it = ValueMap.find("fix_triger");
  if (it != ValueMap.end() && (it->second).size() != 0 ) {
    fix_triger = std::stod( ValueMap["fix_triger"] );
  }
  if ( down_triger > 0) fix_triger = down_triger;
  
  it = ValueMap.find("lastConv");
  if (it != ValueMap.end() && (it->second).size() != 0 ) {
    boost::gregorian::date d( boost::gregorian::from_string( it -> second ) );
    boost::gregorian::date_period dp(FSP -> startDate, d);
    last_fix_conversion_init = (dp.length().days()/365.);
  }

  it = ValueMap.find("maxConvPer");
  if (it != ValueMap.end() && (it->second).size() != 0 ) {
    conversion_period = std::stod( it -> second );
  }
  //std::cout << id <<"\t"<< conversion_period <<"\t"<<last_fix_conversion_init<<"\t"<<fix_triger << std::endl;
}

int FJA2::StepOn() {
  return FJABase::StepOn();
}

int FJA2::Initialize() {
  FJABase::Initialize();
  last_fix_conversion = last_fix_conversion_init;
  //std::cout << last_fix_conversion << std::endl;
  return 0;
}

void FJA2::updateA() {
  if ( rateType == 1 ) {
    NAV_A = NAV_A * pow(( 1 + fix_profit), Simulator -> TimeDelta);
  } else {
    NAV_A = NAV_A + fix_profit * Simulator -> TimeDelta;
  }
}

int FJA2::up_condition() { return 0; }
int FJA2::up_conversion() { return 0; }
int FJA2::down_condition() { return 0; }
int FJA2::down_conversion() { return 0; }

int FJA2::fix_condition() {
  if ( NAV_B <= fix_triger) return 1;
  if ( Simulator -> TimeFromStart() - last_fix_conversion 
       >= conversion_period - EPS ) {
    return 1;
  }
  return 0;
}

int FJA2::fix_conversion() {
  
  if ( NAV_B >= 1 ) {
    data_t.add_currency( Simulator-> TimeFromStart(), 
			 NUM_A * (NAV_A - 1) 
			 * (1-redemption_fee ) );
    data_t.add_up_date( Simulator -> TimeFromStart());
  } else {
    data_t.add_currency( Simulator-> TimeFromStart(), 
			 NUM_A * (NAV_A - NAV_B)
			 * (1 - redemption_fee ) );
    //if (NAV_B <0) 
    data_t.add_down_date( Simulator -> TimeFromStart() );
    NUM_A = NUM_A * NAV_B;
  }

  NAV_A = 1;
  NAV_B = 1;
  NAV_m = 1;

  last_fix_conversion = Simulator -> TimeFromStart();
  return 0;
}

int FJA2::terminate_condition() {
  return FJABase::terminate_condition();
}

FJA3::FJA3( std::map<std::string, std::string> ValueMap,
	    FJASimulator* FSP):FJABase(ValueMap, FSP) {
}

int FJA3::Initialize() {
  FJABase::Initialize();
  up_count = 0;
  down_trigered = 0;
  down_NAV_A = 0;
  
  return 0;
}

int FJA3::StepOn() {
  if ( End ) return 1;
  
  if ( down_condition() ) {	/**< 极端情况被触发. */
    updateA();
    down_conversion();
  } else {
    updatem(); updateA();  Am2B();
    if ( up_condition() ) up_conversion();
  }
  
  if ( fix_condition() ) fix_conversion();

  NAV_m_vec.push_back( NAV_m );
  NAV_A_vec.push_back( NAV_A );
  NAV_B_vec.push_back( NAV_B );
  
  if ( terminate_condition() ) Terminate();

  return 0;
}

int FJA3::up_condition() {
  if ( NAV_m > up_triger ) {
    up_count ++;
  } else {
    up_count = 0;
  }

  if ( up_count == up_triger_day ) {
    up_count = 0;
    return 1;
  }

  return 0;
}

int FJA3::up_conversion() {
  data_t.add_currency( Simulator-> TimeFromStart(), 
		       NUM_A * (NAV_A - 1) 
		       * (1-redemption_fee ) );
  data_t.add_up_date( Simulator -> TimeFromStart());
  NAV_A = 1;
  NAV_B = 1;
  NAV_m = 1;
  return 0;
}

int FJA3::down_condition() {
  if ( NAV_B < down_triger) {
    if ( down_trigered == 0 ) {
      down_NAV_A = NAV_A;
      //std::cout <<"down trigered:" << down_NAV_A << std::endl;
    }
    down_trigered = 1;
    return 1;
  }
  if ( down_trigered ) return 1;
  return 0;
}

int FJA3::down_conversion() {
  double NAV_m_old = NAV_m;
  updatem(); 
  NAV_A *= (NAV_m/NAV_m_old);
  NAV_B *= (NAV_m/NAV_m_old);

  if ( NAV_B >= down_triger ) {
    NAV_A = NAV_A + (NAV_B - down_triger) /(leverage_ratio - 1);
    NAV_B = down_triger;
  }
  
  if ( NAV_A >= down_NAV_A ) {	/**< A份补足  多余部分留给B*/
    NAV_B += ( (NAV_A - down_NAV_A )
	       * (leverage_ratio - 1) );
    NAV_A = down_NAV_A;
  }
  
  if ( NAV_B >= down_triger ) down_trigered = 0;
  
  return 0;
}

int FJA3::terminate_condition() {
  return FJABase::terminate_condition();
}

void FJA3::updateA() {
  if ( down_trigered ) {
    if ( rateType == 1 ) {
      down_NAV_A = down_NAV_A * pow(( 1 + fix_profit), Simulator -> TimeDelta);
    } else {
      down_NAV_A = down_NAV_A + fix_profit * Simulator -> TimeDelta;
    }
  } else {
    if ( rateType == 1 ) {
      NAV_A = NAV_A * pow(( 1 + fix_profit), Simulator -> TimeDelta);
    } else {
      NAV_A = NAV_A + fix_profit * Simulator -> TimeDelta;
    }
  }
}

int FJA3::fix_condition() {
  if (Simulator -> SimulateTime - floor(  Simulator -> SimulateTime) 
      < Simulator -> TimeDelta
      && NAV_A > 1 ) return 1;
  return 0;
}

int FJA3::fix_conversion() {
  data_t.add_currency( Simulator-> TimeFromStart(), 
		       NUM_A * (NAV_A - 1) 
		       * (1 - redemption_fee ) );
  NAV_A = 1;
  AB2m();
  return 0;
}
