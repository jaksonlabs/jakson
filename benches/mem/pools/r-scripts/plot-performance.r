library('ggplot2')

this.dir <- dirname(parent.frame(2)$ofile)
setwd(this.dir)

data = read.csv(file="bench-result-clib-allocator.csv",head=TRUE,sep=";")