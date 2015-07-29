sample = load('sample.txt');
m_sample = mean(sample);
S = size(sample);
L = S(1);
for i = L
    for j = 1:10
        sample(i, j) = sample(i, j) - m_sample(j);
    end
end

for i = 1:10
    for j = 1:10
        cov(i, j) = 0;
        for k = 1:L
            cov(i, j) = cov(i, j) + sample(k, i) * sample(k, j);
        end
        cov(i, j) = cov(i, j)/L;
    end
end
cov;
cov_b = load('orth_mat.txt');
cov - cov_b
