error_ana <-function( var ) {
  df<-read.csv(gzfile('~/github/FenJiADingJia/data/S0601/DataSet/150057.csv.gz'))
  df$down1=replace(df$down1, is.na(df$down1), 15)
  val = df[[var]];
  len = length(df[[var]])
  eva = cumsum(df[[var]])/1:len
  err = abs((eva - eva[len])/eva[len])
  num = 100:len-1
  lognum <- log10(num)
  logerror <- log10(err[num])
  plot(lognum, logerror, type='l',xlab=expression('lg(n)'), ylab='lg(err)')
  lm.sol <- lm(logerror~1+lognum)
  abline(lm.sol, lty=2)
  summary(lm.sol)
  return(lm.sol)
}

error_ana("IRR")
#error_ana("down1")
err = error_ana("NPV")
