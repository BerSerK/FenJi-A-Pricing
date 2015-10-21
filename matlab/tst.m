function tst
    L=0:0.01:2;
    for i = 1:length(L)
        t(i) = dianxin( L(i ));
    end
    plot(L, t)
    grid on;
    foo(0.173842)

    fsolve(@dianxin, 0.5, optimoptions('fsolve','Display','iter'))
    fsolve(@foo, 0.5, optimoptions('fsolve','Display','iter'))
    
    function res = foo(r)
        a = [0.44,0.64,1.38,1.44,2.18,2.44,3.1,3.44,3.86,3.86];
        b = [0.0665667,0.788072,0.203588,0.00012335,0.00152131,0.000534515,0.0252399,0.000171165,0.0060926,0.0018648];
        c = 0.955;
        res=sum(b./(1+r).^a)-c;
    end
    function res = dianxin(r)
        a = 1./12:1./12:2;
        b = 55 * ones(1,24);
        c = 650;
        res=sum(b./(1+r).^a)-c;
    end

end