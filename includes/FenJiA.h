/**
 * @file   FenJiA.h
 * @author YeShiwei <yeshiwei.math@gmail.com>
 * @date   Tue Aug  4 16:21:33 2015
 * 
 * @brief  总控制类和各个分级A实现类的头文件.
 * 
 * 
 */

#ifndef __FENJIA_H__
#define __FENJIA_H__

#include <vector>
#include <map>
#include <string>
#include <unistd.h>
#include <fstream>

#include "boost/date_time/gregorian/gregorian.hpp"
#include "csvrow.h"
#include "FenJiABase.h"
#include "nr3.h"
#include "cholesky.h"
#include "Random.h"


class Stock;
class StockIndex;
class FJABase;

/**
 * 总控制类, 控制整个模拟流程.
 * 
 */
class FJASimulator{
public:
  /** 
   * 以配置文件名构造函数.
   * 
   * @param config_file 配置文件名.
   */
   FJASimulator( std::string config_file ); 

  /** 
   * 读取配置参数和输入数组. 在构造函数中被调用.
   * 
   * @param config_file 配置文件名.
   * 
   * @return 运行状况.
   */
  int Config( std::string config_file ); 

  /** 
   * 展示配置参数.
   * 
   * 
   * @return 运行状况.
   */
  int DisplayConfig();

  /** 
   * 展示配置参数.
   * 
   * 
   * @return 运行状况.
   */
  int DisplayConfig(std::ostream& str);
  
  /** 
   * 读取保存基金参数的文件, 将在Config函数中被调用.
   * 
   * @param FJA_file 保存基金信息的文件名.
   * 
   * @return 运行状态
   */
  int ReadFJA( std::string FJA_file );

  /** 
   * 从文件 IW_file 读取指数权重矩阵.
   * 
   * @param IW_file 指数权重矩阵的文件名.
   * 
   * @return 运行状态.
   */
  int ReadIndexWeight( std::string IW_file );
  
  /** 
   * 从文件 FE_file 读取暴露矩阵.
   * 
   * @param FE_file 暴露矩阵的文件名.
   * 
   * @return 运行状态.
   */
  int ReadFactorExposure( std::string FE_file );

  /** 
   * 从文件 Sigma_file 读取模型不可测部分的协方差矩阵, 是个对角矩阵.
   * 
   * @param Sigma_file 协方差矩阵.
   * 
   * @return 运行状态.
   */
  int ReadSigma( std::string Sigma_file );
  
  /** 
   * 读取因子回报之间的协方差矩阵.
   * 
   * @param Omega_file 因子回报之间的协方差矩阵的文件名.
   * 
   * @return 
   */
  int ReadOmega( std::string Omega_file );

  /** 
   * 主执行函数. 控制程序总体流程.
   * 
   * 
   * @return 运行状况.
   */
  int Run(); 

  /** 
   * 执行模拟循环的函数.
   * 
   * 
   * @return 运行状况.
   */
  int Simulate(); 

  /** 
   * 用模拟结果计算统计量.
   * 
   * 
   * @return 运行状况.
   */
  int Stats(); /// 
  
  /** 
   * 输出模拟结果和统计结果. 
   * 
   * 
   * @return 运行状况.
   */
  int Output(); 

  /** 
   * 输出指数之间的相关矩阵.
   * 
   * 
   * @return 运行状况.
   */
  int OutputCov();

  /** 
   * 输出模拟结果. 上下折算日期数列, 现金流.
   * 
   * 
   * @return 运行状态.
   */
  int OutputResults();

  /** 
   * 设置随机数生成器. 包括计算协方差矩阵的Cholesky分解.
   * 
   * 
   * @return 运行状态.
   */
  int SetRandomNumberGenerator();
  
  /** 
   * 生成下一组符合独立正态分布的数组. 保存在数组E中.
   * 
   * 
   * @return 运行状况
   */
  int GenerateRandomNumber(); 

  /** 
   * 新建一个分级A对象并返回其指针.
   * 
   * @param ValueMap 所创建分级A的属性表.
   * 
   * @return 指向新建的分级A对象的指针.
   */
  FJABase* NewFJA( std::map<std::string, std::string> ValueMap, FJASimulator* FSP);

  inline double TimeFromStart() {
    return SimulateTime - startDateofYear;
  }
  
  std::vector<Stock> StockArray; /**<  股票数组. */
  std::map<std::string, size_t> StockMap; /**< 股票代码映射到股票对象的字典. */
  std::vector<StockIndex> IndexArray;
  std::map<std::string, size_t> IndexMap;

  std::vector< std::string> FactorNames;
  std::vector< std::vector<double> > IndexWeight; /**<  分级基金所跟踪的指数的权重矩阵. */
  std::vector< std::vector<double> > FactorExposure; /**< 暴露矩阵. */
  std::vector< std::vector< double> > Omega; /**< 因子回报之间的相关矩阵. */
  Cholesky *chol;		/**< \f$\Sigma_I=W(X \Omega X^T + \Sigma)W^T \f$ 的Cholesky分解\f$L\f$, 用于生成协方差矩阵为 \f$\Sigma_I\f$的正态分布.  */
  MatDoub cov;

  std::vector< double> dSigma_I; /**< \f$\Sigma_I 的对角部分\f$ */
  std::vector< double> mu;	/**< 因子回报的预期值, 为了简单起见(暂时)取0. */
  std::vector< NormalDistribution> ND; /**< 正态分布随机数生成器. */

  size_t SimulationNumber;	/**< 模拟次数. */
  size_t Count;
  double SimulationLength;	/**< 模拟最长时间. 以年为单位.*/
  double StopRatio;		/**< A份额份数下限. 低于此数不继续模拟. */
  double DiscountRate;		/**< 折现率. */
  double FixRate;		/**< 定存利率. */
  double TimeDelta;		/**< 模拟步长, 日频1.0/252, 周频取 1.0/50 */
  double sqrtTimeDelta;		/**< \f$ \sqrt{TimeDelta} \f$ */
  boost::gregorian::date startDate; /**<  模拟开始日期. */
  double startDateofYear;	/**< 模拟开始日期, 以年为单位. */
  double SimulateTime;		/**< 模拟过程中记录时间. */
  size_t StockNumber;		/**< 个股的总数. */
  size_t FactorNumber;		/**< 因子个数. */
  size_t IndexNumber;
  double YearLength;
  std::string Tag;		/**< 本次模拟的标签, 用于输出结果文件的命名等. */
  std::vector<FJABase*> FJAarray; /**< 分级A数列. */
  int FJALength;		/**< Length of FJAarray, just for convinience. */

  VecDoub vec;			/**< 用于生成随机数的临时变量. */
  VecDoub NormalE;		/**< 符合正态分布的随机向量, 每次迭代重新生成. */
private:

  std::string FJA_file;
  std::string IW_file;
  std::string FE_file;
  std::string Sigma_file;
  std::string Omega_file;
  
  friend class FJABase;
};


#endif
