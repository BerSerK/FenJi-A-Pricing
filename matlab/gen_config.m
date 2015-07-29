function gen_config( cov_file )
    %
    cov = load( cov_file );
    fp = fopen( 'config.txt',  'w');
    for i = 1:length( cov)
        fprintf(fp, '<==\n');
        fprintf(fp, 'AType common\n');
        fprintf(fp, 'NAV_m %.4f\n', 0.2 + rand());
        fprintf(fp, 'mu %.4f\n', 0.0902 + 0.01*rand());
        fprintf(fp, 'fee 0.0122\n');
        fprintf(fp, 'sigma %.4f\n', cov(i, i));
        fprintf(fp, 'leverage_ratio 2\n');
        fprintf(fp, 'fix_profit 0.065\n');
        fprintf(fp, '==>\n');
    end
    fprintf(fp, 'cov_file %s\n', cov_file);
    fprintf(fp, 'simulation_count %d\n', 2000);
end