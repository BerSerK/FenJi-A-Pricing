Trading <- function(AllAtr, report = 0, atr='dif', Buy = 0.2, Sell = 0.5, algo = 1, total = 2e7, max_premium=1, sell_premium=1) {
  fee = 0.005;
  buyfee_ratio = 1e-3;
  sellfee_ratio = 1e-3;
  dates = names(AllAtr);
  sort(dates)
  dlength = length(dates)
  Portfolio = c()
  netvalue = c()
  money = total;
  NetValue = c();
  Percentage = c();
  #TradingAmount = 1e6;
  t = list();
  labels = list()
  minMoney = total
  openA = c();
  Aindex = c(1);
  TradeVolume = 0;
  TradeRatio = c();
  for ( d in 1:dlength) {
  #for ( d in 1:10) {
    Date <- dates[d]
    PreviousDate = dates[d - 1]    
    t = c(t, fn2date(Date))
    labels = c(labels, substring(Date,2,9))
    symbols = names(Portfolio)
    #calculate holding portfolio PNL, Aindex and conversion
    if ( d > 1 ) {
      avr_pnl = weighted.mean(unlist(AllAtr[[ PreviousDate ]]['pnl']), 
                                unlist(AllAtr[[ PreviousDate ]]['unitA']));
      Aindex = c(Aindex, avr_pnl)
      #Calculate netvalue of holding portfolio.
      if (length(symbols) != 0) {
        for ( i in 1:length(symbols) ) {
          symbol <- symbols[i];
          #print( symbol )
          #print( Date )
          if ( Portfolio[ symbol ] > 0 )  {
            price =AllAtr[[ Date ]][symbol, 'avr_price']
            preprice = AllAtr[[ PreviousDate ]][ symbol, 'avr_price']
            if (is.nan(price)) {
              price = AllAtr[[ Date ]][symbol, 'price']
            }
            if (is.nan(preprice) ) {
              preprice = AllAtr[[ PreviousDate ]][ symbol, 'price']
            }
            Size = Portfolio[symbol] / preprice;
            #print(symbol)
            #print(AllAtr[[Date]][symbol,'navA'])
            if (AllAtr[[ Date ]][symbol,'navA'] <
              AllAtr[[ PreviousDate ]][symbol,'navA']) {# conversion
              if (AllAtr[[ Date ]][symbol,'navB'] > 0.8 
                &&AllAtr[[ PreviousDate ]][symbol,'navB'] < 2 *AllAtr[[ Date ]][symbol,'downtriger']) {
                #down conversoin
                print(sprintf('%s down conversioned at %s', symbol, Date))
                triger = AllAtr[[ PreviousDate ]][symbol,'navB']
                newSize = Size * triger;
                Portfolio[symbol] = newSize * price;
                money = money + (AllAtr[[ PreviousDate ]][symbol, 'navA'] - triger) * Size * (1 - fee);
              } else {
                #other conversoin
                money = money + (AllAtr[[ PreviousDate ]][symbol, 'navA'] - 1) * Size * (1 - fee);
                Portfolio[symbol] = Size * price;
              }
            } else {
              #no conversion
              Portfolio[symbol] = Size * price;
            }
          }
        }
      }
    }
    
    #Trading
    TradingSet = TradingSet[rownames(TradingSet)!="150022",]
    TradingSet <- AllAtr[[ Date ]][AllAtr[[ Date ]]$volumeA > 0.0,]
    Order = order(TradingSet[atr], decreasing = TRUE);
    Alength = length(TradingSet[[atr]])
    openA = union(openA, rownames(TradingSet))
    TradingSet <- TradingSet[Order, ]
    TradingSymbols = rownames(TradingSet)
    Ranking = c()
    for (i in 1:Alength) {
      Ranking[TradingSymbols[i]] = i;
    }
    TargetPortfolio = Portfolio
    if (algo == 1) { # Tradde with fix amount.
      #Sell
      PortfolioSymbols = names(Portfolio)
      for ( i in 1:length(Portfolio)) {
        symbol = PortfolioSymbols[i]
        if (is.null(symbol)||is.na(symbol)) {
          
        } else {
          rank = Ranking[symbol]
          if ( !is.na(rank) && rank > Sell * Alength) {
            TargetPortfolio[ symbol ] = 0;
          }
        }
      }
      #Buy
      TradingAmount = total/(Sell*length(openA))
      for ( i in 1:(Alength*Buy) ) {
        symbol = TradingSymbols[i]
        position = TargetPortfolio[symbol]
        if ( position == 0 || is.null(position) || is.na(position)) {
          TargetPortfolio[symbol] = TradingAmount;
        }
      }
    } 
    else if (algo == 2) { # Holding a Full position with weight of unitA.
      #print(Date)
      PortfolioSymbols = names(TargetPortfolio)
      fixPositions = setdiff( PortfolioSymbols, TradingSymbols)
      adjustSymbols = setdiff( TradingSymbols[1:floor(Buy*Alength)], fixPositions);
      #print(adjustSymbols)
      #print(money)
      live_money = money + sum(TargetPortfolio) - sum(TargetPortfolio[fixPositions])
      adjustSet = TradingSet[adjustSymbols,];
      weights = adjustSet['unitA']/sum(adjustSet['unitA']);
      #print(live_money)
      #print(weights)
      for (sym in setdiff(PortfolioSymbols, fixPositions)) {
        TargetPortfolio[ sym ] = 0;
      }
      TargetPortfolio = TargetPortfolio[fixPositions]
      for (sym in adjustSymbols) {
        TargetPortfolio[sym] = live_money * weights[sym,];
      }
      #print(TargetPortfolio)
      #print(sum(TargetPortfolio))
    }
    else if (algo == 3) {
      if (d > 1) {
        OpenSet = AllAtr[[ PreviousDate ]][ AllAtr[[ PreviousDate ]]$unitA > 0.0, ]
        OpenSymbols = intersect( rownames(OpenSet), TradingSymbols)
        OpenSet = OpenSet[OpenSymbols,]
        Order = order(OpenSet[atr], decreasing = TRUE)
        OpenSymbols = OpenSymbols[Order]
        Alength = nrow(OpenSet)
        
        PortfolioSymbols = names(TargetPortfolio)
        fixPositions = setdiff( PortfolioSymbols, TradingSymbols)
        adjustSymbols = setdiff( OpenSymbols[1:floor(Buy*Alength)], fixPositions);
        print(sprintf('%s %.2f %d', Date, Buy*Alength, Alength))
        #print(adjustSymbols)
        #print(money)
        live_money = (money + sum(TargetPortfolio) - sum(TargetPortfolio[fixPositions]))*(1-5e-4)
        adjustSet = TradingSet[adjustSymbols,];
        weights = adjustSet['unitA']/sum(adjustSet['unitA']);
        #print(live_money)
        #print(weights)
        for (sym in setdiff(PortfolioSymbols, fixPositions)) {
          TargetPortfolio[ sym ] = 0;
        }
        TargetPortfolio = TargetPortfolio[fixPositions]
        for (sym in adjustSymbols) {
          TargetPortfolio[sym] = live_money * weights[sym,];
        }
      }
    }
    else if (algo == 4) {
      if (d > 1) {
        OpenSet = AllAtr[[ PreviousDate ]][ AllAtr[[ PreviousDate ]]$unitA > 0.0, ]
        OpenSymbols = intersect( rownames(OpenSet), TradingSymbols)
        OpenSet = OpenSet[OpenSymbols,]
        Order = order(OpenSet[atr], decreasing = TRUE)
        OpenSymbols = OpenSymbols[Order]
        Alength = nrow(OpenSet)
        
        PortfolioSymbols = names(TargetPortfolio)
        fixPositions = setdiff( PortfolioSymbols, TradingSymbols)
        BuyVolume = sum(OpenSet['unitA']) * Buy;
        Sum = 0; L = 1;
        for ( i in 1:Alength) {
          Sum = Sum + OpenSet[i, 'unitA'];
          if ( Sum > BuyVolume ) {
            L = i; break;
          }
        }
        #print(sprintf('%s %d %d', Date, L, Alength))
        adjustSymbols = setdiff( OpenSymbols[1:L], fixPositions);
        #print(adjustSymbols)
        #print(money)
        live_money = (money + sum(TargetPortfolio) - sum(TargetPortfolio[fixPositions]))*(1-5e-4)
        adjustSet = TradingSet[adjustSymbols,];
        weights = adjustSet['unitA']/sum(adjustSet['unitA']);
        #print(live_money)
        #print(weights)
        for (sym in setdiff(PortfolioSymbols, fixPositions)) {
          TargetPortfolio[ sym ] = 0;
        }
        TargetPortfolio = TargetPortfolio[fixPositions]
        for (sym in adjustSymbols) {
          TargetPortfolio[sym] = live_money * weights[sym,];
        }
      }
    }
    else if (algo == 5) {#filter TradingSet by premium before take the Buy part
      if (d > 1) {
        OpenSet = AllAtr[[ PreviousDate ]][ AllAtr[[ PreviousDate ]]$unitA > 0.0, ]
        OpenSymbols = intersect( rownames(OpenSet), TradingSymbols)
        OpenSet = OpenSet[OpenSymbols,]
        Order = order(OpenSet[atr], decreasing = TRUE)
        OpenSymbols = OpenSymbols[Order]
        OpenSet = OpenSet[ OpenSet$premium < max_premium,]        
        Alength = nrow(OpenSet)
        PortfolioSymbols = names(TargetPortfolio)
        fixPositions = setdiff( PortfolioSymbols, TradingSymbols)
        BuyVolume = sum(OpenSet['unitA']) * Buy;
        Sum = 0; L = 1;
        for ( i in 1:Alength) {
          Sum = Sum + OpenSet[i, 'unitA'];
          if ( Sum > BuyVolume ) {
            L = i; break;
          }
        }
        #print(sprintf('%s %d %d', Date, L, Alength))
        adjustSymbols = setdiff( OpenSymbols[1:L], fixPositions);
        live_money = (money + sum(TargetPortfolio) - sum(TargetPortfolio[fixPositions]))*(1-5e-4)
        adjustSet = TradingSet[adjustSymbols,];
        weights = adjustSet['unitA']/sum(adjustSet['unitA']);
        for (sym in setdiff(PortfolioSymbols, fixPositions)) {
          TargetPortfolio[ sym ] = 0;
        }
        TargetPortfolio = TargetPortfolio[fixPositions]
        for (sym in adjustSymbols) {
          TargetPortfolio[sym] = live_money * weights[sym,];
        }
      }
    }
    else if (algo == 6) {#filter TradingSet by premium before take the Buy part
      if (d > 1) {
        OpenSet = AllAtr[[ PreviousDate ]][ AllAtr[[ PreviousDate ]]$unitA > 0.0, ]
        OpenSymbols = intersect( rownames(OpenSet), TradingSymbols)
        OpenSet = OpenSet[OpenSymbols,]
        Order = order(OpenSet[atr], decreasing = TRUE)
        OpenSymbols = OpenSymbols[Order]
        Alength = nrow(OpenSet)
        PortfolioSymbols = names(TargetPortfolio)
        fixPositions = setdiff( PortfolioSymbols, TradingSymbols)
        BuyVolume = sum(OpenSet['unitA']) * Buy;
        Sum = 0; L = 1;
        for ( i in 1:Alength) {
          Sum = Sum + OpenSet[i, 'unitA'];
          if ( Sum > BuyVolume ) {
            L = i; break;
          }
        }
        print(sprintf('%s %d %d', Date, L, Alength))
        adjustSymbols = setdiff( OpenSymbols[1:L], fixPositions);
        OpenSet = OpenSet[adjustSymbols,]
        OpenSet = OpenSet[OpenSet$premium < max_premium,]
        adjustSymbols = rownames(OpenSet)
        live_money = (money + sum(TargetPortfolio) - sum(TargetPortfolio[fixPositions]))*(1-5e-4)
        for (sym in setdiff(PortfolioSymbols, fixPositions)) {
          TargetPortfolio[ sym ] = 0;
        }
        TargetPortfolio = TargetPortfolio[fixPositions]
        if (length(adjustSymbols) > 0 ) {
            adjustSet = TradingSet[adjustSymbols,];
            weights = adjustSet['unitA']/sum(adjustSet['unitA']);
           for (sym in adjustSymbols) {
              TargetPortfolio[sym] = live_money * weights[sym,];
          }
        }
        
      }
    }
    else if (algo == 7) {#filter TradingSet by premium after taking the Buy Size
      if (d > 1) {
        OpenSet = AllAtr[[ PreviousDate ]][ AllAtr[[ PreviousDate ]]$unitA > 0.0, ]
        OpenSymbols = intersect( rownames(OpenSet), TradingSymbols)
        OpenSet = OpenSet[OpenSymbols,]
        Order = order(OpenSet[atr], decreasing = TRUE)
        OpenSymbols = OpenSymbols[Order]
        Alength = nrow(OpenSet)
        PortfolioSymbols = names(TargetPortfolio)
        fixPositions = setdiff( PortfolioSymbols, TradingSymbols)
        BuyVolume = sum(OpenSet['unitA']) * Buy;
        Sum = 0; L = 1;
        for ( i in 1:Alength) {
          Sum = Sum + OpenSet[i, 'unitA'];
          if ( Sum > BuyVolume ) {
            L = i; break;
          }
        }
        #print(sprintf('%s %d %d', Date, L, Alength))
        adjustSymbols = setdiff( OpenSymbols[1:L], fixPositions);
        live_money = (money + sum(TargetPortfolio) - sum(TargetPortfolio[fixPositions]))*(1-5e-4)
        for (sym in setdiff(PortfolioSymbols, fixPositions)) {
          TargetPortfolio[ sym ] = 0;
        }
        TargetPortfolio = TargetPortfolio[fixPositions]
        adjustSet = TradingSet[adjustSymbols,];
        weights = adjustSet['unitA']/sum(adjustSet['unitA']);
        for (sym in adjustSymbols) {
          if ( adjustSet[sym,'premium'] <= max_premium ) {
            TargetPortfolio[sym] = live_money * weights[sym,];
          } else {
            TargetPortfolio[sym] = 0;
          }
        }
      }
    }
    else if (algo == 8) {#filter the new trade in by premium
      if (d > 1) {
        OpenSet = AllAtr[[ PreviousDate ]][ AllAtr[[ PreviousDate ]]$unitA > 0.0, ]
        OpenSymbols = intersect( rownames(OpenSet), TradingSymbols)
        OpenSet = OpenSet[OpenSymbols,]
        Order = order(OpenSet[atr], decreasing = TRUE)
        OpenSymbols = OpenSymbols[Order]
        Alength = nrow(OpenSet)
        PortfolioSymbols = names(TargetPortfolio)
        fixPositions = setdiff( PortfolioSymbols, TradingSymbols)
        BuyVolume = sum(OpenSet['unitA']) * Buy;
        Sum = 0; L = 1;
        for ( i in 1:Alength) {
          Sum = Sum + OpenSet[i, 'unitA'];
          if ( Sum > BuyVolume ) {
            L = i; break;
          }
        }
        #print(sprintf('%s %d %d', Date, L, Alength))
        adjustSymbols = setdiff( OpenSymbols[1:L], fixPositions);
        live_money = (money + sum(TargetPortfolio) - sum(TargetPortfolio[fixPositions]))*(1-5e-4)
        for (sym in setdiff(PortfolioSymbols, fixPositions)) {
          TargetPortfolio[ sym ] = 0;
        }
        TargetPortfolio = TargetPortfolio[fixPositions]
        adjustSet = TradingSet[adjustSymbols,];
        weights = adjustSet['unitA']/sum(adjustSet['unitA']);
        for (sym in adjustSymbols) {
          if ( adjustSet[sym,'premium'] > max_premium &&
               (Portfolio[sym] == 0|| is.null(Portfolio[sym])
                || is.na( Portfolio[sym]) )) {
            TargetPortfolio[sym] = 0;
          } else {
            TargetPortfolio[sym] = live_money * weights[sym,];          
          }
        }
      }
    }
    else if (algo == 9) {#filter the new trade in by premium, sell holding position by high premium
      if (d > 1) {
        OpenSet = AllAtr[[ PreviousDate ]][ AllAtr[[ PreviousDate ]]$unitA > 0.0, ]
        OpenSymbols = intersect( rownames(OpenSet), TradingSymbols)
        OpenSet = OpenSet[OpenSymbols,]
        Order = order(OpenSet[atr], decreasing = TRUE)
        OpenSymbols = OpenSymbols[Order]
        Alength = nrow(OpenSet)
        PortfolioSymbols = names(TargetPortfolio)
        fixPositions = setdiff( PortfolioSymbols, TradingSymbols)
        BuyVolume = sum(OpenSet['unitA']) * Buy;
        Sum = 0; L = 1;
        for ( i in 1:Alength) {
          Sum = Sum + OpenSet[i, 'unitA'];
          if ( Sum > BuyVolume ) {
            L = i; break;
          }
        }
        #print(sprintf('%s %d %d', Date, L, Alength))
        adjustSymbols = setdiff( OpenSymbols[1:L], fixPositions);
        live_money = (money + sum(TargetPortfolio) - sum(TargetPortfolio[fixPositions]))*(1-5e-4)
        for (sym in setdiff(PortfolioSymbols, fixPositions)) {
          TargetPortfolio[ sym ] = 0;
        }
        TargetPortfolio = TargetPortfolio[fixPositions]
        adjustSet = TradingSet[adjustSymbols,];
        weights = adjustSet['unitA']/sum(adjustSet['unitA']);
        for (sym in adjustSymbols) {
          P = Portfolio[sym]
          if ( is.null(Portfolio[sym])) { 
            P <- 0;
          } else if (is.na(Portfolio[sym])) {
            P <- 0;
          }
          if ( adjustSet[sym,'premium'] > max_premium &&
                P == 0 ) {
            TargetPortfolio[sym] = 0;
          } else if ( P > 0 && adjustSet[sym, 'premium'] > sell_premium ){
            TargetPortfolio[sym] = 0;
          } else {
            TargetPortfolio[sym] = live_money * weights[sym,];          
          }
        }
      }
    }
    else if (algo == 10) {#filter the new trade in by premium, sell holding position by high premium
      if (d > 1) {
        OpenSet = AllAtr[[ PreviousDate ]][ AllAtr[[ PreviousDate ]]$unitA > 0.0, ]
        OpenSymbols = intersect( rownames(OpenSet), TradingSymbols)
        OpenSet = OpenSet[OpenSymbols,]
        Order = order(OpenSet[atr], decreasing = TRUE)
        OpenSymbols = OpenSymbols[Order]
        Alength = nrow(OpenSet)
        PortfolioSymbols = names(TargetPortfolio)
        fixPositions = setdiff( PortfolioSymbols, TradingSymbols)
        BuyVolume =sum(OpenSet['unitA']) * Buy;
        SellVolume = sum(OpenSet['unitA']) * Sell;
        Sum = 0; L = -1; LSell = -1;
        for ( i in 1:Alength) {
          Sum = Sum + OpenSet[i, 'unitA'];
          if ( Sum > BuyVolume && L == -1) {
            L = i; 
          }
          if ( Sum > SellVolume && LSell == -1) {
            LSell = i;
          }
        }
        #print(sprintf('%s %d %d', Date, L, Alength))
        keepSymbols = intersect(names(Portfolio), OpenSymbols[1:LSell])
        adjustSymbols = setdiff( union(OpenSymbols[1:L],keepSymbols), fixPositions);
        
        live_money = (money + sum(TargetPortfolio) - sum(TargetPortfolio[fixPositions]))*(1-5e-4)
        for (sym in setdiff(PortfolioSymbols, fixPositions)) {
          TargetPortfolio[ sym ] = 0;
        }
        TargetPortfolio = TargetPortfolio[fixPositions]
        adjustSet = TradingSet[adjustSymbols,];
        weights = adjustSet['unitA']/sum(adjustSet['unitA']);
        for (sym in adjustSymbols) {
          P = Portfolio[sym]
          if ( is.null(Portfolio[sym])) { 
            P <- 0;
          } else if (is.na(Portfolio[sym])) {
            P <- 0;
          }
          if ( adjustSet[sym,'premium'] > max_premium &&
               P == 0 ) {
            TargetPortfolio[sym] = 0;
          } else if ( P > 0 && adjustSet[sym, 'premium'] > sell_premium ){
            TargetPortfolio[sym] = 0;
          } else {
            TargetPortfolio[sym] = live_money * weights[sym,];          
          }
        }
      }
    }
    
    PortfolioSymbols = union(names(TargetPortfolio), names(Portfolio))
    Trade = c()
    if ( length(PortfolioSymbols) > 0 ) {
      for ( i in 1:length(PortfolioSymbols)) {
        symbol = PortfolioSymbols[i];
        target = TargetPortfolio[symbol];
        origin = Portfolio[symbol];
        if ( is.null(origin) || is.na(origin) ) {
          origin = 0;
        }
        if ( is.null(target) || is.na(target) ) {
          target = 0;
        }
      
        if (target > origin ) {#Buy
          money = money  - (target - origin) *( 1 + buyfee_ratio);  
        } else {#Sell
          money = money - (target - origin) * ( 1 - sellfee_ratio);        
        }
        TradeVolume = TradeVolume + abs(target - origin)
        Trade[symbol] = target - origin
      }
    }
    Portfolio = TargetPortfolio;
    netvalue = sum(Portfolio) + money;
    NetValue = c(NetValue, netvalue);
    Percentage = c(Percentage, sum(Portfolio)/ ( netvalue ))
    if ( !is.null(Trade) )  {
      TradeRatio = c(TradeRatio, sum(abs(Trade))/netvalue);
    }
    if (money < minMoney) minMoney = money
    #print(Date)
    #print(Portfolio)
  }

  Aindex <- cumprod( Aindex )
  
  par(mar=c(5,4,4,5)+.1)  
  opar <- par(mfrow = c(1,2), oma = c(0, 0, 1.1, 0))
  pnl = NetValue / total;
  mx = max(pnl, Aindex);
  mn = min(pnl, Aindex);
  plot(t, pnl, type='l',col='blue', ylim=c(mn, mx))
  lines(t, Aindex, type = 'l', col='green')
  
  index1 <- read.csv('~/Downloads/index.csv')
  for ( i in 1:length(index1$BargainDate)) {
    index1$Date[i] = as.Date(index1$BargainDate[i], '%m/%d/%Y')
  }
  lines(index1$Date, index1$index/1101.717, type='l', col='yellow')
  
  par(new=TRUE)
  plot(t, Percentage, type='l', col='red',xaxt="n", yaxt="n", xlab="", ylab="", ylim = c(0, 1))
  axis(4)
  mtext("percentage",side=4,line=3)
  legend
  c = 12;
  pos = seq(1, length(t), floor(length(t)/c));
  axis(1, at=t[pos], labels=labels[pos],las = 2)
  #axis(2, at=seq(0.8,1.5,0.1), labels=seq(0.8,1.5,0.1),las = 1)
  legend('topleft', c("pnl","position percentage","index"), col=c('blue','red','green'),
         cex=0.5, pch=21, pt.cex = 1.,text.font=2)
  plot(t, pnl - Aindex, type='l')
  print(minMoney/total)  
  l = length(pnl);
  res = c()
  returns = diff(pnl)
  res['mdd'] = utils.mdd(pnl)
  res['sharpe'] = utils.sharpe(returns)
  res['alpha'] = pnl[l]-Aindex[l]
  res['index_mdd'] = utils.mdd(Aindex)
  res['index_sharpe'] = utils.sharpe(diff(Aindex))
  res['alpha_mdd'] = utils.mdd(pnl - Aindex)
  res['alpha_sharpe'] = utils.sharpe(diff(pnl-Aindex))
  res['trade_ratio'] = mean(TradeRatio)
  if (report == 1) {
    print(sprintf('Trade Volume:%.4f %.4f', TradeVolume, TradeVolume/total/l))
    Portfolio <- sort(Portfolio)
    print(Portfolio)
    print(100*Portfolio/sum(Portfolio))
    print(sprintf("Trade of last day: %.4f", sum(abs(Trade))/sum(Portfolio)))
    print(100*Trade/sum(Portfolio))
    print(mean(TradeRatio[(l-51):(l-1)]))  
  }
  return (res)
}

LoopPremium <- function(algo=7) {
  PP = seq(0,0.2,0.01);
  
  df <-data.frame(Trading(Atr2, algo = algo, max_premium = PP[1]))
  for (i in 2:length(PP)) {
    df <- data.frame(df, Trading(Atr2, algo = algo, max_premium = PP[i]))
  }
  colnames(df) <-PP
  df <- t(df)
  return (df)
}

LoopSellPremium <- function(algo=9, max_premium = 0) {
  PP = seq(0,0.2,0.01);
  
  df <-data.frame(Trading(Atr2, algo = algo, max_premium = max_premium, sell_premium = PP[1]))
  for (i in 2:length(PP)) {
    df <- data.frame(df, Trading(Atr2, algo = algo, max_premium = max_premium, sell_premium = PP[i]))
  }
  colnames(df) <-PP
  df <- t(df)
  return (df)
}

LoopBuy <- function(algo = 9) {
  Buy = seq(0.15, 0.5, 0.05);
  df <-data.frame(Trading(Atr2, algo = algo, Buy = Buy[1]))
  for (i in 2:length(Buy)) {
    df <- data.frame(df, Trading(Atr2, algo = algo, Buy = Buy[i]))
  }
  colnames(df) <- Buy
  df <- t(df)
  return (df)
}
