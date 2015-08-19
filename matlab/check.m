Omega = csvread('config/cov_20150727.csv', 1, 1);
X = csvread('config/expo.csv',1,1);
Sigma =diag( csvread('config/srisk.csv', 1,1));
W = csvread('config/indexWgt.csv', 1,1)';
%sum(W')
cov_s = W*Sigma*W'
cov_b = W * (X * Omega * X' + Sigma) *W'

sample = load('sample.txt');
cov_s = cov( sample );
hist(sample, 50);
norm(cov_b - cov_s)/norm(cov_b)


