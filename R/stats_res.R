setwd('~/github/FenJiADingJia/')
data_dir <- '~/github/FenJiADingJia/data/'

read_dat <-function(tag){
  dir_name =  paste(data_dir, tag, "DataSet",sep="/");
  fs <- list.files( dir_name )
  dat <- list()
  for (f in fs) {
    filename= paste(dir_name,f,sep="/");
    symbolA = strsplit(f,"[.]")[[1]][1]
    dat[[symbolA]] <- read.csv(gzfile(filename))
  }
  return(dat)
}

read_compact_data <- function(tag) {
  dir_name =  paste(data_dir, tag, "DataSet",sep="/");
  fs <- list.files( dir_name )
  dat <- list()
  for (f in fs) {
    filename= paste(dir_name,f,sep="/");
    symbolA = strsplit(f,"[.]")[[1]][1]
    d <- read.csv(gzfile(filename))
    dat[[symbolA]] = d[,c('duration','IRR','NPV','down1','down2','up1')]
  }
  return(dat)
}

load_all_data <- function() {
  fs = list.files('data')
  t = list()
  Data = list()
  for (f in fs) {
    print(f)
    Data[[f]] <- read_compact_data(f);
  }
  return(Data)
}

selectAll <- function(Data) {#from the loaded compact Data.
  AllAtr = list()
  for (n in names(Data)) {
    print(n)
    AllAtr[[n]] <- selectedAtr(Data[[n]], n);
  }
  return(AllAtr)
}

selectAllfromDisk <- function(enddate = 'S20150915', startdate = 'S20140101') {
  fs = list.files('data')
  t = list()
  AllAtr = list()
  for (f in fs) {
    if ( f < enddate && f >= startdate) {
      print(f)
      dat <- read_compact_data(f);
      AllAtr[[f]] <- selectedAtr(dat, f);
    }
  }
  return(AllAtr)
}

calcPNL <- function(AllAtr, atr='dif') {
  ns = names(AllAtr);
  sort(ns)
  dlength = length(ns)
  for (i in 1:(dlength-1)) {
    symbols = rownames( AllAtr[[ns[i]]])
    next_symbols = rownames( AllAtr[[ns[i + 1]]])
    AllAtr[[ns[i]]]$pnl <- 1;
    for (symbol in symbols){
      if ( is.element(symbol, next_symbols) ) {
          #print(pnl)
        if (AllAtr[[ns[i+1]]][symbol,'navA'] <
            AllAtr[[ns[i]]][symbol,'navA']) {
          if (AllAtr[[ns[i+1]]][symbol,'navB'] > 0.8 
              &&AllAtr[[ns[i]]][symbol,'navB'] < 2 * AllAtr[[ns[i]]][symbol,'downtriger']) {
            print(sprintf('%s down conversioned at %s', symbol, ns[i]))
            triger = AllAtr[[ns[i]]][symbol,'downtriger'];
            pnl = (AllAtr[[ns[i]]][symbol,'navA'] - triger + triger *
                     AllAtr[[ns[i+1]]][symbol,]$price)/AllAtr[[ns[i]]][symbol,]$price;
          } else if (AllAtr[[ns[i]]][symbol, 'navA'] > 1) {
            pnl = (AllAtr[[ns[i]]][symbol,'navA'] - 1 + 
                     AllAtr[[ns[i+1]]][symbol,]$price)/AllAtr[[ns[i]]][symbol,]$price;
          } else {
            pnl=AllAtr[[ns[i+1]]][symbol,]$price/AllAtr[[ns[i]]][symbol,]$price          
          }
        } else {
          pnl=AllAtr[[ns[i+1]]][symbol,]$price/AllAtr[[ns[i]]][symbol,]$price          
        }
          
        if (is.infinite(pnl) ){
          AllAtr[[ns[i]]][symbol,'pnl'] <- 1;
        }else{
          AllAtr[[ns[i]]][symbol,'pnl'] <- pnl;
        }
      }        
    }
  }
  pnlgroup = list(); divid = 5;
  for (i in 1:divid) {
    pnlgroup[[i]] = list()
  }
  PNL = list()
  t = c()
  labels = list()
  for ( i in 1:(dlength-1)) {
    #t = c(t, fn2doy(ns[i]))
    t = c(t, fn2date(ns[i]))
    l = ns[i]
    labels = c(labels, substring(l,2,9))
    TradingSet = AllAtr[[ns[i]]][AllAtr[[ns[i]]]$volumeA > 0.01,]
    TradingSet = TradingSet[rownames(TradingSet)!="150022",]
    Order = order(TradingSet[atr]);
    Alength = length(TradingSet[[atr]])
    #print(Alength)
    #print(Order)
    step = ceiling(Alength/divid)
    for ( d in 1:divid ) {
      s =1 + (d-1) * step;
      e = d * step;
      if ( d == divid ) { e = Alength}
      # print(s)
      # print(e)
      pnlD = TradingSet[Order[s:e],]
      pnlgroup[[d]][i] = weighted.mean(pnlD$pnl, pnlD$unitA);
    }
    PNL[i] = weighted.mean(TradingSet$pnl, TradingSet$unitA)
  }
  mx = -10;
  mn = 10;
  PNL = cumprod(PNL)
  for (i in 1:divid){
    pnlgroup[[i]] = cumprod(pnlgroup[[i]])
   #pnlgroup[[i]] = pnlgroup[[i]] - PNL
    mx = max(pnlgroup[[i]], mx)
    mn = min(pnlgroup[[i]], mn)
  }
  #print(mx)
  opar <- par(mfrow = c(1,1), oma = c(0, 0, 1.1, 0))
  plot(t, pnlgroup[[1]], type='b',col='red', xlab = 'date', ylab='pnl', ylim=c(mn, mx), axes=FALSE)
  Colors = c('red','blue', 'green', 'grey', 'yellow','black')
  for (i in 2:divid){
    lines(t, pnlgroup[[i]], type='b', col=Colors[[i]])
  }
  lines(t, PNL, type='b', col='black')
  c = 12;
  pos = seq(1, length(t), floor(length(t)/c));
  axis(1, at=t[pos], labels=labels[pos],las = 2)
  axis(2, at=seq(-1,4,0.1), labels=seq(-1,4,0.1),las = 1)
  grid()
  legend("topleft", c("1",'2','3','4','5','all'), col=Colors,cex=0.5, pch=21, pt.cex = 1.,text.font=2)
  title(atr)
  return(AllAtr)
}

stats_var <-function(dat, var) {
  var <- data.frame( mean = unlist(lapply(dat, function(x){mean(x[[var]],na.rm=TRUE)})),
                          mean10 = unlist(lapply(dat, function(x){mean(x[[var]],0.1,na.rm=TRUE)})),
                          quantile10 = unlist(lapply(dat, function(x){quantile(x[[var]],probs=0.10,na.rm=TRUE)})),
                          quantile25 = unlist(lapply(dat, function(x){quantile(x[[var]],0.25,na.rm=TRUE)})),
                          quantile50 = unlist(lapply(dat, function(x){quantile(x[[var]],probs=0.50,na.rm=TRUE)})),
                          quantile75 = unlist(lapply(dat, function(x){quantile(x[[var]],0.75,na.rm=TRUE)})),
                          quantile90 = unlist(lapply(dat, function(x){quantile(x[[var]],probs=0.90,na.rm=TRUE)}))
  )
  return(var)
}

readFJA <- function(tag){
  config <- read.csv(gzfile(paste(data_dir, tag, 'conf.csv.gz', sep='/')))
  FJA <- read.csv(gzfile(toString( config$FJAfile)))
  return(FJA)
}

stats_all <- function(dat, tag) {
  res <- list()
  res[['FJA']] <- readFJA(tag)
  res[['IRR']] <- stats_var(dat, 'IRR')
  res[['down1']] <- stats_var(dat, 'down1')
  res[['down2']] <- stats_var(dat, 'down2')
  res[['duration']] <- stats_var(dat, 'duration')
  res[['NPV']] <- stats_var(dat, 'NPV')
  return(res)
}

ana_npv <- function(dat, tag) {
  NPV <- stats_var(dat, 'NPV')
  FJA <- readFJA(tag)
  symbols = rownames(NPV)
  #print(symbols)
  #print(lapply(FJA$symbolA, toString))
  for ( i in 1:length(symbols)) {
    for ( j in 1:length(FJA$symbolA)) {
      if ( symbols[i] == toString(FJA$symbolA[j])){
        #print(symbols[i])
        NPV$price[i] = FJA$priceA[j]
        NPV$navA[i] = FJA$navA[j]
        NPV$navM[i] = FJA$navM[j]
      }
    }
  }
  return(NPV)
}

selectedAtr <- function(dat, tag){
  var <- data.frame( npvmean10 = unlist(lapply(dat, function(x){mean(x[['NPV']],0.1,na.rm=TRUE)})),
                     npv = unlist(lapply(dat, function(x){quantile(x[['NPV']],probs=0.50,na.rm=TRUE)})),
                     irr =unlist(lapply(dat, function(x){quantile(x[['IRR']],probs=0.50,na.rm=TRUE)})),
                     duration = unlist(lapply(dat, function(x){quantile(x[['duration']],probs=0.50,na.rm=TRUE)})),
                     down1 = unlist(lapply(dat, function(x){quantile(x[['down1']],probs=0.50,na.rm=TRUE)})),
                     down2 = unlist(lapply(dat, function(x){quantile(x[['down2']],probs=0.50,na.rm=TRUE)})),
                     up1 = unlist(lapply(dat, function(x){quantile(x[['up1']],probs=0.50,na.rm=TRUE)}))
  )
  FJA <- readFJA(tag)
  symbols = rownames(var)
  for ( i in 1:length(symbols)) {
    for ( j in 1:length(FJA$symbolA)) {
      if ( symbols[i] == toString(FJA$symbolA[j])){
        #print(symbols[i])
        var$price[i] = FJA$priceA[j]
        var$navA[i] = FJA$navA[j]
        var$navM[i] = FJA$navM[j]
        var$navB[i] = FJA$leverage[j] * FJA$navM[j] - 
            (FJA$leverage[j] - 1 ) * FJA$navA[j]
        var$volumeA[i] = FJA$volumeA[j]
        var$downtriger[i] = FJA$down[j]
        var$turnA[i] = FJA$turnA[j]
        var$unitA[i] = FJA$unitA[j]
        var$premium[i] = FJA$priceA[j] * (1 - 1./FJA$leverage[j]) + FJA$priceB[j] * 1./FJA$leverage[j] - FJA$navM[j]
      }
    }
  }
  var$dif = (var$npv - var$price)/var$npv
  var$avr_price = (var$volumeA/var$turnA)
  var <- var[, c("npv", "price", "dif","irr", "premium","avr_price","volumeA","unitA","down1","down2","up1","npvmean10","navA","navM","navB","downtriger")]
  return(var)
}

fn2doy <-function(fn){
  ds = paste(substring(fn, 2,5), substring(fn,6,7),substring(fn,8,9),sep='-')
  #print(ds)
  return(strftime(ds,'%j'))
}

fn2date <-function(fn){
  ds = paste(substring(fn, 2,5), substring(fn,6,7),substring(fn,8,9),sep='-')
  #print(ds)
  return( as.Date(ds) )
}

plot_irr <- function(symbolA, AllAtr){
  #fs = list.files('data')
  fs = names(AllAtr)
  t = c()
  irr = c()
  navM = list()
  navA = list()
  price = vector()
  npv = vector()
  labels = list()
  var1 = list()
  for ( f in fs ) {
    if (nchar(f) == 9) {
      labels = c(labels, substring(f, 6, 9))
      fn = paste('data',f,'DataSet',symbolA, sep='/')
      fn = paste(fn, 'csv.gz',sep='.')
      if (file.exists(fn)) {
        t = c(t, fn2date(f));
        csv = AllAtr[[f]]
        irr = c(irr,csv[symbolA,'irr'])
        npv = c(npv,csv[symbolA,'npv'])
        FJA <- readFJA(f)
        navM = c(navM, FJA[FJA$symbolA==symbolA,]$navM)
        navA = c(navA, FJA[FJA$symbolA==symbolA,]$navA)
        price = c(price, FJA[FJA$symbolA==symbolA,]$priceA)
      }
    }
  } 
  legendpos = 'topleft'
  par(mar=c(5,4,4,5)+.1)
  opar <- par(mfrow = c(2,2), oma = c(0, 0, 1.1, 0))
  print(irr)
  print(t)
  plot(t,irr,type='b',col="red");
  par(new=TRUE)
  
  plot(t,navM,type='b',col="blue",xaxt="n", yaxt="n", xlab="", ylab="")
  legend(legendpos, c("IRR","navM"), col=c('red','blue'),cex=0.5, pch=21, pt.cex = 1.,text.font=2)
  axis(4)
  mtext("navM",side=4,line=3)
  title(symbolA)  #  print(npv)
  
  print(npv)
  plot(t, price, type='b', col='red', ylim=c(min(unlist(price), unlist(npv)),max(unlist(price), unlist(npv))))
  lines(t, npv, type='b', col='blue')
  title(symbolA)
  legend(legendpos, c("Price","npv"), col=c('red','blue'),cex=0.5, pch=21, pt.cex = 1.,text.font=2)
  
  plot(t, npv, type='b', col='blue', ylim=c(min(unlist(navA), unlist(npv)), max(unlist(navA),unlist(npv))))
  lines(t, navA, type='b', col='red')
  title(symbolA)
  legend(legendpos, c("navA","npv"), col=c('red','blue'),cex=0.5, pch=21, pt.cex = 1.,text.font=2)

  plot(t, npv, type='b', col='blue', ylim=c(min(unlist(navA), unlist(npv), unlist(price)), 
                                            max(unlist(navA),unlist(npv),unlist(price))))
  lines(t, navA, type='b', col='red')
  lines(t, price, type='b', col='black')
  title(symbolA)
  legend(legendpos, c("navA","npv","price"), col=c('red','blue','black'),cex=0.5, pch=21, pt.cex = 1.,text.font=2)
  
  lm.A <- lm(price~npv + 1)
  summary(lm.A)
}

plot_atr <- function(AllAtr, atr) {
  fs = names(AllAtr)#list.files('data')
  t = list()
  avr_irr = list()
  labels = list()
  for ( f in fs ) {
    if (nchar(f) == 9) {
      print(f)
      labels = c(labels, substring(f, 6, 9))
      t = c(t, fn2doy(f));
      #t = c(t, as.Date(f, 'S%Y%m%d'));
      Tag = f;
      #dat <- read_dat(Tag)
      #selected <- selectedAtr(dat, Tag)
      selected <- AllAtr[[Tag]]
      ss <- selected[selected$volume>Vi0,]
      ss <- ss[rownames(ss)!="150022",]
      ss <- ss[rownames(ss)!="150012",]
      avr_irr = c(avr_irr, mean(ss[[atr]], weights = ss$volume))
    }
  }
  opar <- par(mfrow = c(1,1), oma = c(0, 0, 1.1, 0))
  plot(t, avr_irr, type='b', ylab=atr, axes=FALSE)
  axis(1, at=t, labels=labels, las = 2)
  l = seq(min(unlist(avr_irr)), max(unlist(avr_irr)), (max(unlist(avr_irr))-min(unlist(avr_irr)))/10);
  axis(2, at=l, labels=format(l,digits=1), las = 1)
  grid()
}

lmA <- function(symbolA) {
  fs = list.files('data')
  t = list()
  npv = vector()
  price = vector()
  for ( f in fs ) {
    if (nchar(f) == 9) {
      fn = paste('data',f,'DataSet',symbolA, sep='/')
      fn = paste(fn, 'csv.gz',sep='.')
      csv = read.csv(gzfile(fn));
      npv = c(npv, mean(csv$NPV,na.rm=TRUE))
      FJA <- readFJA(f)
      price = c(price, FJA[FJA$symbolA==symbolA,]$priceA)
    }
  } 
  print(npv)
  print(price)
  lm.A <- lm(price~npv + 1)
  plot(lm.A)
  summary(lm.A)
  return(lm.A)
}

plot_r_squared <- function(AllAtr){
  fs = list.files('data')
  fs = names(AllAtr)
  t = list()
  r_squared = list()
  labels = list()
  for ( f in fs ) {
    if (nchar(f) == 9) {
      print(f)
      labels = c(labels, substring(f, 2, 9))
      t = c(t, fn2date(f));
      #t = c(t, as.Date(f, 'S%Y%m%d'));
      Tag = f;
      #dat <- read_dat(Tag)
      #selected <- selectedAtr(dat, Tag)
      selected <- AllAtr[[Tag]]
      ss <- selected[selected$volume>0,]
      ss <- ss[rownames(ss)!="150022",]
      ss <- ss[rownames(ss)!="150012",]
      lm.npv_price_v <- lm(ss$price~ss$npv + 1, weights = ss$volume)
      r_squared <- c( r_squared, summary(lm.npv_price_v)$r.squared );
    }
  }
  plot(t, r_squared,type='b', axes=FALSE)
  c = 12;
  pos = seq(1, length(t), floor(length(t)/c));
  axis(1, at=t[pos], labels=labels[pos],las = 2)
  axis(2, at=seq(0,1,0.1), labels=seq(0,1,0.1),las = 1)
  grid()
  return(data.frame(t=unlist(t),r_squared=unlist(r_squared)))
}

plot_var <- function() {
  fs = list.files('data')
  t = list()
  mvar = list()
  labels = list()
  for ( f in fs ) {
    if (nchar(f) == 9) {
      fn <- paste('data/',f,'cov.csv.gz', sep='/')
      #print(fn)
      t = c(t, fn2doy(f));
      cov = read.csv(gzfile(fn))
      var = list();
      for ( i in 1:length(cov)-1 ) {
        var = c(var, cov[i, i+1])
      }
      mvar = c(mvar, mean(unlist(var)))
    }
  }
  #print(t)
  #print(mvar)
  plot(t, mvar,type='l',main='variance')
  return(data.frame(t=unlist(t),mvar=unlist(mvar)))
}

utils.mdd<-function(x)
{
  mdd         <- 0;
  x[is.na(x)] <- 0;
#  for (i in 1:length(x))
#  {
#     mdd <- min(c(mdd, cumsum(x[i:length(x)]) ),na.rm=T)
#  }
  mxd  <- 1; mx = 0;
  for ( i in 1:length(x)) {
    if (x[i] > mx) {
      mx = x[i]; 
      mxd = i;
    }
    mdd <- min(c(mdd, x[i] - mx), na.rm = T)
  }
  return(mdd);
}

utils.sharpe <- function(x) {
  return ( mean(x)/sqrt(var(x)) )
}

utils.mdd1<-function(x)
{
  mdd         <- 0;
  x[is.na(x)] <- 0;
  for (i in 1:length(x))
    {
       mdd <- min(c(mdd, cumsum(x[i:length(x)]) ),na.rm=T)
    }
  return(mdd);
}