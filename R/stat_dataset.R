fja<-function(filename) {
df <- read.csv(filename)
hist(df$down1,80)
mirr <- mean(df$IRR)
}
fja('github/FenJiADingJia/data/Sim0/DataSet-150022.csv')
