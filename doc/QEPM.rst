.. index:: QEPM (Quantitative Equity Portfolio Management)

QEPM模型
==============================
我们通过QEPM模型来确定分级A模型中需要用到的被跟踪 :math:`N` 个指数的预期收益率 :math:`R_i` 以及指数之间的相关矩阵 :math:`\Sigma_i` . 理论细节可以从书 `Quantitative Equity Portfolio Management (证券组合定量管理) <http://book.douban.com/subject/2799221/>`_ 中获得.  这里我给出我们用到的那一小部分. 

我们用向量 :math:`\vec{R}_i \in \mathcal{R}^N` 表示我们需要跟踪的指数, 指数包含了 :math:`m` 只个股以权重矩阵 :math:`W\in \mathcal{R}^{m\times N}` 组合而成. 
则 :math:`R_i=W^T \vec{R}` . 这里 :math:`\vec{R}\in \mathcal{R}^m` 为各个股的预期收益. 

我们假设各个股的预期收益 :math:`\vec{R}` 被 :math:`n` 个因子所决定. :math:`\vec{R}=\vec{x}^T\beta + \vec{\epsilon}` 得到. 
这里 :math:`\beta\in \mathcal{R^{n\times m}}` 表示因子矩阵, 而 :math:`\vec{x}\in \mathcal{R}^n` 表示相应的因子向量, :math:`\vec{\epsilon}` 表示一个服从正态分布的一个 :math:`N` 维随机变量.  :math:`\vec{\epsilon} \in N(0, \Sigma)` . :math:`\vec{x} \in N(\vec{\mu}, \Omega)` . 
:math:`\vec{\epsilon}` 与 :math:`\vec{x}` 之间相互独立. 

于是有 :math:`\Sigma_i=cov(R_i,R_i)=W^T\beta^T \Omega \beta W + W^T\Sigma W`.

于是, 我们的QEPM模型可以以 :math:`W, \beta, \Sigma, \Omega, \vec{\mu}` 为输入, 得到输出 :math:`R_i` 与 :math:`\Sigma_i` . 


