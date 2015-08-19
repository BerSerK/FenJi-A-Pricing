function path=plot_path(path_file, leverage_ratio, start_date)
    TimeDelta = 1/52.;
    if nargin == 1
        leverage_ratio = 2;
        start_date = 0.569863;
    end
    path = csvread(path_file, 0, 1);
    Time = 0:TimeDelta:TimeDelta* (length(path) - 1);
    NAV_A = path(1,:);
    NAV_m = path(2,:);
    NAV_B = get_NAV_B;
    clf;
    plot(Time, NAV_A, 'b-');
    hold on;
    plot(Time, NAV_B, 'r-');
    plot(Time, NAV_m, 'k-');
    
    legend({'NAV_A','NAV_B','NAV_m'});
    xlabel('Time(year)');
    ylabel('Net Value');
    function NAV_B=get_NAV_B
        NAV_B = leverage_ratio * NAV_m - ( leverage_ratio - 1) * NAV_A;
    end
    
end
