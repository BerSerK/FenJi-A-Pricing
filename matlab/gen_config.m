function gen_config( FJALength )
%    
    fp = fopen('config/config.txt', 'r');

    while ~feof( fp )
        feature = fscanf(fp, '%s', 1);
        if strcmp(feature, 'SimulationCount')
            SimulationCount = fscanf(fp, '%d') ;
        elseif strcmp( feature , 'StockNumber')
            StockNumber = fscanf(fp, '%d');
        elseif strcmp( feature , 'FactorNumber')
            FactorNumber = fscanf(fp, '%d');
        elseif strcmp( feature , 'Tag')
            Tag = fscanf(fp, '%s', 1);
        elseif strcmp( feature , 'FJA_file')
            FJA_file = fscanf(fp, '%s', 1);
        elseif strcmp( feature , 'IndexWeight')
            IndexWeight = fscanf(fp, '%s', 1);
        elseif strcmp( feature , 'FactorExposure')
            FactorExposure = fscanf(fp, '%s', 1);
        elseif strcmp( feature , 'Sigma')
            Sigma = fscanf(fp, '%s', 1);
        elseif strcmp( feature , 'Omega')
            Omega = fscanf(fp, '%s', 1);
        else
            continue
        end
    end
    
    fclose(fp);
    
    fp = fopen( FJA_file,  'w');
    for i = 1:FJALength
        fprintf(fp, '<==\n');
        fprintf(fp, 'AType common\n');
        fprintf(fp, 'NAV_m %.4f\n', 0.2 + rand());
        fprintf(fp, 'fee 0.0122\n');
        fprintf(fp, 'leverage_ratio 2\n');
        fprintf(fp, 'fix_profit 0.065\n');
        fprintf(fp, '==>\n');
    end
end