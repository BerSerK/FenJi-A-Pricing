/**
 * @file   FenJiABase.h
 * @author YeShiwei <yeshiwei.math@gmail.com>
 * @date   Tue Aug  4 16:22:42 2015
 * 
 * @brief  分级A抽象基类的头文件.
 * 
 * 
 */

#ifndef __FENJIABASE_H__
#define __FENJIABASE_H__

#include <vector>
#include <string>
#include <math.h>
#include <sstream>

#include "nr3.h"

class FJASimulator;

class FJAData{
public:
  FJAData() {
    ;
  }

  /** 
   * 清空数据.
   * 
   */
  void clear() {
    up_dates.clear();
    down_dates.clear();
    currency.clear();
  }

  /** 
   * 将数据转换成CSV文件的一行.
   * 
   * 
   * @return CSV 行.
   */
  std::string toCSVline() {
    std::string res;
    std::stringstream ss;
    ss << duration << ",";
    ss << IRR << ",";
    ss << NPV << ",";

    for ( size_t i = 0; i < MAX_UP_LENGTH; i ++ ) {
      if ( i < up_dates.size())
	ss << up_dates[i];
      ss << ",";
    }

    for ( size_t i = 0; i < MAX_DOWN_LENGTH; i ++ ) {
      if ( i < down_dates.size())
	ss << down_dates[i];
      ss << ",";
    }
    
    for ( size_t i = 0; i < MAX_CUR_LENGTH; i ++ ) {
      if ( i < currency.size()) {
	ss << currency[i].first <<",";
	ss << currency[i].second;
      }
      if ( i != MAX_CUR_LENGTH - 1) {
	ss << ",";
      } 
    }
    ss >> res;
    return res;
  }

  /** 
   * 根据各个数组的最大长度生成CSV文件头部的变量名一行.
   * 
   * 
   * @return CSV文件头.
   */
  static std::string CSVhead() {
    std::string res;
    std::stringstream ss;
    ss << "duration,";
    ss << "IRR,";
    ss << "NPV,";

    for ( size_t i = 0; i < MAX_UP_LENGTH; i ++ ) {
      ss << "up" << i + 1 << ",";
    }

    for ( size_t i = 0; i < MAX_DOWN_LENGTH; i ++ ) {
      ss << "down" << i + 1 << ",";
    }
    
    for ( size_t i = 0; i < MAX_CUR_LENGTH; i ++ ) {
      ss << "cur_date" << i + 1 << ","
	 << "cur_value"<< i + 1;
      if ( i != MAX_CUR_LENGTH - 1) {
	ss << ",";
      } 
    }
    ss >> res;
    return res;
  }

  /** 
   * 添加一个上折日.
   * 
   * @param date 上折日期(年)
   */
  inline void add_up_date(double date) {
    up_dates.push_back(date);
    if ( up_dates.size() > MAX_UP_LENGTH )
      MAX_UP_LENGTH = up_dates.size();
  }
  
  /** 
   *  添加一个下折日期
   * 
   * @param date 下折日期(年)
   */
  inline void add_down_date(double date) {
    down_dates.push_back(date);
    if ( down_dates.size() > MAX_DOWN_LENGTH) 
      MAX_DOWN_LENGTH = down_dates.size();
  }

  /** 
   * 添加一项现金流
   * 
   * @param date 现金流日期
   * @param value 现金流值
   */
  inline void add_currency(double date, double value) {
    std::pair<double, double> cur(date, value);
    add_currency(cur);
  }

  /** 
   * 添加一项现金流
   * 
   * @param cur 现金流对(日期, 值).
   */
  inline void add_currency(std::pair<double, double>& cur) {
    currency.push_back(cur);
    if (currency.size() > MAX_CUR_LENGTH) 
      MAX_CUR_LENGTH = currency.size();
  }
  
  /** 
   * 用于Newton法计算IRR.
   * 
   * 
   * @return 以rate为收益所计算出的现值与价格之间的差.
   */
  double operator() ( const double& rate) const {
    return get_NPV( rate ) - price;
  }

  /** 
   * 计算现值.
   * 
   * @param rate 计算现值所用的收益率.
   * 
   * @return 现值.
   */
  double get_NPV( const double& rate) const {
    double res = 0;
    double rate1 = 1 + rate;
    for (size_t i = 0; i < currency.size(); i ++ ) {
      res += currency[i].second/pow(rate1, currency[i].first);
    }
    return res;
  }

  /** 
   * 计算现值函数的导数, 用于Newton法计算IRR.
   * 
   * @param rate 计算导数用的收益率
   * 
   * @return 现值函数对收益率的导数.
   */
  double derivative( const double& rate) const {
    double res = 0;
    double rate1 = 1 + rate;
    for (size_t i = 0; i < currency.size(); i ++ ) {
      res += (- currency[i].first * currency[i].second/pow(rate1, currency[i].first + 1) );
    }
    return res;
  }
  
  /** 
   * 计算久期.
   * 
   * @param rate 计算久期所用的收益率.
   */
  void getDuration( const double& rate) {
    duration = 0;
    double T, V, rate1= 1+rate;
    NPV = get_NPV( rate );
    for ( size_t i = 0; i < currency.size(); i ++ ) {
      T = currency[i].first;
      V = currency[i].second;
      duration += ( T * V / pow(rate1, T) / NPV );
    }
  }
  
  std::vector<double> up_dates; /**< 不定期上折日期数列. 以年为单位. */
  std::vector<double> down_dates; /**< 不定期下折日期数列. 以年为单位. */
  std::vector<std::pair<double, double> > currency; /**< 现金流. */
  double NPV;			/**< 现金流总当前净值. */
  double price;			/**< 当前市场价格. */
  double IRR;			/**< Internal rate of return, 内部回报率. */
  double duration;		/**< 久期 */
  static size_t MAX_UP_LENGTH;	/**< 所有模拟中最长上折数列. */
  static size_t MAX_DOWN_LENGTH; /**< 所有模拟中最长下折数列. */
  static size_t MAX_CUR_LENGTH;	/**< 所有模拟中最长现金流数列. */
};

class Stock{
public:
  Stock(std::string idd, double sk, size_t a_id):id( idd ),
						    srisk( sk ),
						    array_id( a_id ) {;}
  std::string id;		/**< 股票代码. */
  double srisk;			/**< 波动率. */
  size_t array_id;		/**< 在控制类中的股票数列中的索引. */
  std::vector<double>* FactorExposure; /**< 对应的暴露矩阵. */
  inline void print() {
    std::cout << id <<":" << srisk << ":" << array_id << std::endl;
  }
};

class StockIndex{
public:
  std::string id;		/**< 股指代码. */
  double U;			/**< 预期收益. */
  double sigma2;		/**< 年化波动率的平方. */
  size_t IndexWeight;		/**< 指数权重在指数权重矩阵中的行号. */
};

class FJABase{
public:
  
  /** 
   * 构造函数
   * 
   * @param ValueMap 参数表.
   * @param FSP 模拟程序主控制类指针.
   */
  FJABase(std::map<std::string, std::string> ValueMap, FJASimulator* FSP);

  virtual ~FJABase();

  /** 
   * 初始化一次模拟.
   * 
   * 
   * @return 运行状态.
   */
  int Initialize();

  /** 
   * 向前迭代一步. 判断是否上下折算, 是否到了定期折算日, 并执行相关计算. 判断是否应该终止模拟
   *    
   * @param E \f$ \vec{U}=WX\vec{\mu} \f$, 
   *          \f$ \vec{E}=WX\vec{\beta_0} + W\vec{\epsilon} \f$ 
   *          \f$ R_I = \vec{U} + \vec{E} \f$,
   *          \f$ \vec{\beta} = \vec{\mu} + \vec{\beta_0} \f$,
   *
   * @return 是否应该结束本次模拟, 1 是, 0 否.
   */
  int StepOn();
  
  /** 
   * 结束一次模拟, 保存相关数据. 
   * 
   * 
   * @return 运行状态.
   */
  int Terminate();

  /** 
   * 统计模拟结果. 
   * 
   * 
   * @return 运行状态.
   */
  int Stats();

  /** 
   * 触发不定期向上折算的条件.
   * 
   * 
   * @return 是否向上折算, 1 是, 0 否.
   */
  virtual int up_condition() { return 0;};
  
  /** 
   * 不定期向上折算.
   * 
   * 
   * @return 运行状态.
   */
  virtual int up_conversion() { return 0;};
  
  /** 
   * 触发不定期向下折算的条件.
   * 
   * 
   * @return 是否向下折算, 1 是, 0 否.
   */
  virtual int down_condition() { return 0;};
  
  /** 
   * 不定期向下折算.
   * 
   * 
   * @return 运行状态.
   */
  virtual int down_conversion() { return 0;};
  
  /** 
   *  固定折算. 固定周期的折算.
   * 
   * 
   * @return 运行状态.
   */
  virtual int fix_conversion() { return 0;};

  /** 
   *  模拟终止条件. 当连续多次下折A份净值小到一定比例或者模拟时间长到一定程度之后, 我们将终止模拟.
   * 
   * 
   * @return 是否终止模拟, 1 是, 0 否.
   */
  virtual int terminate_condition() {return 0;};

  /** 
   * 根据指数变化更新母基金净值.
   * 
   */
  inline void updatem();

  /** 
   * 累加约定收益, 计算A份额净值.
   * 
   */
  virtual void updateA() {;};

  /** 
   * 由母基金净值和A份净值计算B份净值.
   * 
   */
  inline void Am2B() {
    NAV_B = leverage_ratio * NAV_m - ( leverage_ratio - 1) * NAV_A;
  }
  
  /** 
   * 由A,B份额的净值, 反推母基金的净值. 主要用于定期折算.
   * 
   */
  inline void AB2m() {
    NAV_m = NAV_B / leverage_ratio + ( 1 - 1./ leverage_ratio ) * NAV_A;
  }
  
  /** 
   * 输出当前模拟的这条路径.
   * 
   * 
   * @return 运行状态.
   */
  int OutputPath();

  /** 
   * 输出所有模拟结果, 包括上折日期, 下折日期, 现金流.
   * 
   * 
   * @return 
   */
  int OutputDataSet();

  std::string id;		/**< 基金编号*/
  std::string symbolM;		/**< 母基金编号 */
  double NAV_m_init;		/**< 母基金初始净值. */
  double NAV_A_init;		/**< 母基金初始净值. */
  double price;			/**< 市场价格. */
  double fix_profit;		/**< A份额的约定收益率. */
  int ifrateFixed;		/**< 约定收益率是否固定. 1固定, 0依赖定存利率. */
  double leverage_ratio;	/**< 进取份额的杠杆倍数. */
  int rateType;			/**< 单利还是复利. 单利取1, 复利取2. */
  double fee;			/**< 费用. */
  double up_triger;		/**< 上折触发点. */
  double down_triger;		/**< 下折触发点. */
  size_t TrackingIndex;		/**< 母基金所跟踪的指数在指数数组中的编号. */
  double redemption_fee;	/**< 母基金赎回费用比例. */

  double expiry;		/**< 到期日. */
  double sigma2;		/**< 年化波动率的平方. */
  double NAV_m;			/**< 母基金净值, 模拟过程中保存的临时变量. */
  std::vector<double> NAV_m_vec;/**< 母基金净值数列. */
  double NAV_A;			/**< A份净值. */
  std::vector<double> NAV_A_vec;/**< A份净值数列. */
  double NUM_A;			/**< A份额相对数量. */
  double NAV_B;			/**< B份净值. */
  std::vector<FJAData> data_set;/**< 各次模拟数据. */
  FJAData data_t;		/**< 本次模拟的数据. */
  bool End;			/**< 标记一条路径是否模拟结束. */
  FJASimulator* Simulator;	/**< 指向总控制器的指针, 用于调用其中的数据. */
private:
  double updatem_fix;
  static constexpr double up_limit = 1.6105;
  static constexpr double low_limit = 0.5905;
};


/**
 * 最常见的分级基金样例.
 * 
 */
class CommonA : public FJABase{ 
public:
  CommonA( std::map<std::string, std::string> ValueMap, FJASimulator* FSP);
  virtual int up_condition();
  virtual int up_conversion();
  virtual int down_condition();
  virtual int down_conversion();
  virtual int fix_conversion();
  virtual int terminate_condition();
  virtual void updateA();
};

#endif
