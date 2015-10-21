source("~/github/FenJiADingJia/R/stats_res.R",local=TRUE)
Tag='S20150914'
dat <- read_dat(Tag)
npv<-ana_npv(dat, Tag)
res <-stats_all(dat, Tag)
selected <- selectedAtr(dat, Tag)
ss <- selected[selected$price!=0,]
ss <- ss[rownames(ss)!="150022",]
ss <- ss[rownames(ss)!="150012",]
lm.npv_price_v <- lm(ss$price~ss$npv + 1, weights = ss$volume)
opar <- par(mfrow = c(2,2), oma = c(0, 0, 1.1, 0))
plot(lm.npv_price_v)
par(opar)
plot(ss$npv, lm.npv_price_v$fitted.values, type='l');
points(ss$npv, ss$price)
summary(lm.npv_price_v)
print(lm.npv_price_v$coefficients)
