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

#include "nr3.h"

class FJAData{
public:
  std::vector<double> up_dates; /**< 不定期上折日期数列. 以年为单位. */
  std::vector<double> down_dates; /**< 不定期下折日期数列. 以年为单位. */
  std::vector<double> currency; /**< 现金流. */
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
  std::string id;
  double U; /**< 预期收益. */
  double E; /**< 符合正态分布的随机变量, 有主控制类生成传入. */
  double sigma2; /**< 年华波动率的平方. */
  std::vector<double>* IndexWeight;
};

class FJABase{
public:
  
  /** 
   * 构造函数.
   * 
   */
  FJABase(std::string iid):id(iid) {;};

  virtual ~FJABase();

  /** 
   * 向前迭代一步. 判断是否上下折算, 是否到了定期折算日, 并执行相关计算. 判断是否应该终止模拟
   *    
   * @param E \f$ \vec{U}=WX\vec{\mu} \f$, 
   *          \f$ \vec{E}=WX\vec{\beta_0} + W\vec{\epsilon} \f$ 
   *          \f$ R_I = \vec{U} + \vec{E} \f$,
   *          \f$ \vec{\beta} = \vec{\mu} + \vec{\beta_0} \f$,
   *
   * @return 是否应该继续模拟, 1 是, 0 否.
   */
  int StepOn( double E );
  
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

  StockIndex* TrackingIndex; /**< 母基金所跟踪的指数. */
  double NAV_m_init; /**< 母基金初始净值. */
  double fee; /**< 费用. */
  double leverage_ratio; /**< 进取份额的杠杆倍数. */
  double fix_profit; /**< A份额的约定收益率. */
  
  double NAV_m;	/**< 母基金净值, 模拟过程中保存的临时变量, 下同. */
  double NAV_A;	/**< A份净值. */
  double NAV_B;	/**< B份净值. */
  std::vector<FJAData> data; /**< 各次模拟数据. */
  std::string id;		/**< 基金编号 */
};

#endif
