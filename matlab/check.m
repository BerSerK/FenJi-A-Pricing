Omega = csvread('config2/cov.csv', 1, 1);
X = csvread('config2/expo.csv',1,1);
Sigma =diag( csvread('config2/srisk.csv', 1,1));
W = csvread('config2/indexWgt.csv', 1,1)';
%sum(W')
cov_s = W*Sigma*W';
cov_b = W * (X * Omega * X' + Sigma) *W'

sample = load('sample.txt');
cov_s = cov( sample );
hist(sample, 50);
norm(cov_b - cov_s)/norm(cov_b)


