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

#include "csvrow.h"
#include "FenJiABase.h"
#include "nr3.h"
#include "cholesky.h"
#include "Random.h"

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
   * @param FJA_name 所新建的分级A的类型名称.
   * 
   * @return 指向新建的分级A对象的指针.
   */
  FJABase* NewFJA( std::string FJA_name, std::string iid);
  
  std::vector<Stock> StockArray; /**<  股票数组. */
  std::map<std::string, size_t> StockMap; /**< 股票代码映射到股票对象的字典. */
  std::vector<StockIndex> IndexArray;
  std::map<std::string, StockIndex*> IndexMap;

  std::vector< std::string> FactorNames;
  std::vector< std::vector<double> > IndexWeight; /**<  分级基金所跟踪的指数的权重矩阵. */
  std::vector< std::vector<double> > FactorExposure; /**< 暴露矩阵. */
  std::vector< std::vector< double> > Omega; /**< 因子回报之间的相关矩阵. */
  Cholesky *chol;  /**< \f$\Sigma_I=W(X \Omega X^T + \Sigma)W^T \f$ 的Cholesky分解\f$L\f$, 用于生成协方差矩阵为 \f$\Sigma_I\f$的正态分布.  */

  std::vector< double> dSigma_I; /**< \f$\Sigma_I 的对角部分\f$ */
  std::vector< double> mu; /**< 因子回报的预期值, 为了简单起见(暂时)取0. */
  std::vector< NormalDistribution> ND; /**< 正态分布随机数生成器. */

  size_t SimulationCount; /**< 模拟次数. */
  size_t StockNumber; /**< 个股的总数. */
  size_t FactorNumber; /**< 因子个数. */
  size_t IndexNumber;
  std::string Tag; /**< 本次模拟的标签, 用于输出结果文件的命名等, 以示区分. */
  std::vector<FJABase*> FJAarray; /**< 分级A数列. */
  int FJALength; /**< Length of FJAarray, just for convinience. */

  VecDoub vec;	/**< 用于生成随机数的临时变量. */
  VecDoub NormalE;	/**< 符合正态分布的随机向量, 每次迭代重新生成. */
private:

  std::string FJA_file;
  std::string IW_file;
  std::string FE_file;
  std::string Sigma_file;
  std::string Omega_file;
};

/**
 * 最常见的分级基金样例.
 * 
 */
class CommonA : public FJABase{ 
public:
  CommonA(std::string id):FJABase(id) {;}
  virtual int up_condition();
  virtual int up_conversion();
  virtual int down_condition();
  virtual int down_conversion();
  virtual int fix_conversion();
  virtual int terminate_condition();
};

#endif
