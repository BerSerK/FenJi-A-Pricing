function A=gen_orth(N, filename)
    %生成一个随机的正定矩阵，并输出
    if nargin < 2
        filename = 'orth_mat.txt';
    end
    X = diag( rand(N,1));
    U = orth( rand(N,N));
    A = 0.3*U' * X * U;
    dlmwrite(filename, A, 'delimiter', '\t', 'precision', '%.16f');
    %dlmwrite('tst.txt', chol(A), 'delimiter', '\t', 'precision', '%.16f');    
end