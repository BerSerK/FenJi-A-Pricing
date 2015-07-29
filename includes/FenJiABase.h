#ifndef __FENJIABASE_H__
#define __FENJIABASE_H__

#include <vector>
#include <pair>

class FJAData{
public:
  std::vector<double> up_dates; ///不定期上折日期数列。
  std::vector<double> down_dates; ///不定期下折日期数列。
  std::vector<pay> currency; ///现金流
};

class FJABase{
public:
  FJABase();
  virtual ~FJABase();
  int step_on( double epsilon );
  int statistics();

  virtual int up_condition() { return 0;};
  virtual int up_conversion() { return 0;};
  virtual int down_condition() { return 0;};
  virtual int down_conversion() { return 0;};
  virtual int fix_conversion() { return 0;};
  virtual int terminate_condition() {return 0;};

  double NAV_m_init; ///母基金初始净值
  double mu; ///母基金预期收益
  double fee; ///费用
  double sigma; ///年华波动率
  double leverage_ratio; ///进取份额的杠杆倍数
  double fix_profit; ///A份额的约定收益率
  
  double NAV_m; /// 母基金净值, 模拟过程中保存的临时变量, 下同.
  double NAV_A; /// A份净值
  double NAV_B; /// B份净值
  std::vector<FJAData> data; ///各次模拟数据
};


#endif
