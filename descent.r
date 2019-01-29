descent <- read.csv("statistics.csv")

pdf(file="thread_number.pdf")
plot(descent[,1], type="s", xlab="Chunk Number", ylab="Number of Threads")
dev.off()
