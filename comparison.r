descent <- read.csv("statistics.csv")

pdf(file="comparison.pdf", height=8, width=16)
plot(descent[,4], type="l", xlab="Chunk Number", ylab="Total Time in seconds")
dev.off()
