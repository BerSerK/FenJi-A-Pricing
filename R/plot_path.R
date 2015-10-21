read.tcsv <- function(file, sep=',') {
  n <- max(count.fields(file, sep), na.rm = TRUE)
  x <- readLines(file)
  .splitvar = function(x, sep, n) {
    var = unlist( strsplit(x, split = sep))
    length(var) = n;
    return(var)
  }
  
  x = do.call(cbind, lapply(x, .splitvar, sep=sep, n=n))
  x = apply(x, 1, paste, collapse=sep)
  out = read.csv(text=x,sep=sep)
  return(out)
}

plot_path <- function(file, lr = 2, down_triger=0.25) {
  p <- read.tcsv(file)
  t_end <- tail(p['currency.dates'][[1]][!is.na(p['currency.dates'][[1]])],n=1)
  l <- length( p['NAV_A'][[1]])
  t <- seq(0, t_end, length.out=l)
  plot(t, p$NAV_B,type='l', col='blue',xlim=c(0,t_end),ylim=c(0, max(p$NAV_A,p$NAV_B,p$NAV_m)))
  lines(t, p$NAV_A,type='l', col='red')
  lines(t, p$NAV_m,type='l', col='black')
  abline(h=down_triger)
  abline(h=0)
  return(p)
}
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-01.csv', 2, 0.25)
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-02.csv', 2, 0.25)
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-03.csv', 2, 0.25)
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-04.csv', 2, 0.25)
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-05.csv', 2, 0.25)
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-06.csv', 2, 0.25)
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-07.csv', 2, 0.25)
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-08.csv', 2, 0.25)
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-09.csv', 2, 0.25)
p <- plot_path('~/github/FenJiADingJia/data/S0105/path/150177-10.csv', 2, 0.25)


