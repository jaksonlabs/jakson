descent <- read.csv("descent_statistics.csv")
static <- read.csv("static_statistics.csv")

pdf(file="comparison.pdf", height=8, width=16)
plot(descent[,3], type="l", xlab="Chunk Number", ylab="Total Time in seconds")
lines(static[,3], col="blue")

legend("topleft", legend=c("Fixed Threads", "Adaptive Threads"), col=c("black", "blue"), lty=1:1)

print(descent[,3])

dev.off()
