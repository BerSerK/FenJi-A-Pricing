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

#include "nr3.h"

class FJASimulator;

class FJAData{
public:
  void clear() {
    up_dates.clear();
    down_dates.clear();
    currency.clear();
  }
  std::vector<double> up_dates; /**< 不定期上折日期数列. 以年为单位. */
  std::vector<double> down_dates; /**< 不定期下折日期数列. 以年为单位. */
  std::vector<std::pair<double, double> > currency; /**< 现金流. */
};

class Stock{
public:
  Stock(std::string idd, double sk, size_t a_id):id( idd ),
						    srisk( sk ),
						    array_id( a_id ) {;}
  std::string id;
  double srisk;
  size_t array_id;
  std::vector<double>* FactorExposure;
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
   * 构造函数.
   * 
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

  std::string id;		/**< 基金编号 */
  size_t TrackingIndex;		/**< 母基金所跟踪的指数在指数数组中的编号. */
  double NAV_m_init;		/**< 母基金初始净值. */
  double NAV_A_init;		/**< 母基金初始净值. */
  double fee;			/**< 费用. */
  double redemption_fee;	/**< 母基金赎回费用比例. */
  double leverage_ratio;	/**< 进取份额的杠杆倍数. */
  double fix_profit;		/**< A份额的约定收益率. */

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
