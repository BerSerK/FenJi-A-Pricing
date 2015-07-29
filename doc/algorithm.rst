算法
==============================

生成满足正态分布的N维度随机变量
------------------------------------------------------------

算法
````````````````````
设 :math:`N` 维随机变量 :math:`Z` 的每个分量是满足 :math:`\mu=0` , :math:`\sigma=1` 的独立正态分布随机变量。
则可以由 :math:`x=R+Z*A` , :math:`A=chol(\Sigma)` 生成均值为 :math:`R`, 协方差矩阵为 :math:`\Sigma` 的N维随机变量。
这里chol为 `Cholesky分解 <https://en.wikipedia.org/wiki/Cholesky_decomposition>`_ 。

PS: 这里需要注意的一个问题是生成的随机数列 :math:`Z` 必须是独立的，这是程序实现过程中很容易犯的一个错误。

C++程序实现
````````````````````
我利用C++11 的新特性 `生成正态分布随机数列 <http://www.cplusplus.com/reference/random/normal_distribution/>`_ . 
为了使得所生成的 :math:`Z` 相互独立，我用 :math:`t F(n)` 来做随机数生成器的种子。这里 :math:`t` 是一个和时间相关的整数， 
而 :math:`F(n)` 是 `斐波那契数列(Fibonacci Number) <https://en.wikipedia.org/wiki/Fibonacci_number>`_ .
显然，为了每个种子不一样, :math:`F(n)` 必须从 :math:`1,2,3,5..` 开始，而不是从 :math:`0` 开始。


各种统计量的计算方法
----------------------------------------
Monte Carlo模拟完成之后, 我们需要计算如下统计量:

  a. 每个基金的 `平均 <https://en.wikipedia.org/wiki/Mean>`_ 第一次下折日;
  b. 第一向上折算日 `频数分布 <https://en.wikipedia.org/wiki/Frequency_distribution>`_ ;
  c. 第一向下折算日频数分布;
  d. 折算日 `截尾均值 <https://en.wikipedia.org/wiki/Truncated_mean>`_ ;
  e. 折算日25% `分位数 <https://en.wikipedia.org/wiki/Quantile>`_ , 50%分位数, 75%分位数, 与90%分位数;
  f. A基金优先份额久期分析;
  g. 计算隐含收益率;

其中前五项 a b c d e 都是常见统计量无需在这里赘述. 

基金优先份额久期(Macaulay Duration)分析
````````````````````````````````````````````````````````````

我们可以通过模拟永续分级基金优先份额现金流实现的金额与日期, 结合修正久期的定义, 得出其修正久期.

.. math::
   
   Macualay\ Duration = \frac{\Sigma_{t=1}^{n}\frac{t\times CF_t}{(1+y)^t} }{\Sigma_{t=1}^{n}\frac{CF_t}{(1+y)^t} }

计算隐含收益率
````````````````````````````````````````````````````````````


