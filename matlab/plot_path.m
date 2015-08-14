function path=plot_path(path_file, leverage_ratio)
    if nargin == 1
        leverage_ratio = 2;
    end
    path=csvread(path_file, 0, 1);
    NAV_A = path(1,:);
    NAV_m = path(2,:);
    NAV_B = get_NAV_B;
    clf;
    plot(NAV_A, 'b-');
    hold on;
    plot(NAV_B, 'r-');
    plot(NAV_m, 'k-');
    
    legend({'NAV_A','NAV_B','NAV_m'});
    function NAV_B=get_NAV_B
        NAV_B = leverage_ratio * NAV_m - ( leverage_ratio - 1) * NAV_A;
    end
end
