function tst
    for i =1:100
        t(i) = foo(i);
    end
    plot(t)
    
    function res = foo(r)
        a=[0.02,0.3,0.44,0.64]
        b=[0.980021,0.0453828,4.70776e-05,0.0114228]
        c=0.848;
        res=sum(b./(1+r).^a)-c;
    end
end