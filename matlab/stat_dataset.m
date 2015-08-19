function stat_dataset(filename)
    data = csvread(filename, 1, 0);
    
    fp = fopen(filename);    l = fgetl(fp);    fclose(fp);
    Keys = strsplit(l,',');
    Values = 1:length(Keys);
    M = containers.Map(Keys, Values);
    duration = data(:, M('duration'));
    irr = data(:, M('IRR'));
    npv = data(:, M('NPV'));
    down1 = data(:, M('down1'));
    up1 = data(:, M('up1'));
    
    figure(1); hist(down1, 50);
    down1mean =mean( down1 );
    disp( sprintf('mean of 1st down=%f', down1mean));
    trimmean10=trimmean( down1, 10);
    quantile75=quantile( down1, 0.7);
    
    disp( sprintf('mean of duration =%f', mean(duration)));
    disp( sprintf('trimmean of duration =%f', trimmean(duration,10)));
    disp( sprintf('mean of irr =%f', mean(irr)));
    disp( sprintf('trimmean of irr =%f', trimmean(irr, 10)));
    disp( sprintf('mean of npv =%f', mean(npv)));
    
end