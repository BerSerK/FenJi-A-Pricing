index1 <- read.csv('~/Downloads/index.csv')
for ( i in 1:length(index1$BargainDate)) {
  index1$Date[i] = as.Date(index1$BargainDate[i], '%m/%d/%Y')
}
plot(index1$Date, index1$index, type='l')
