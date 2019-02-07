descent <- read.csv("descent_statistics.csv")
static <- read.csv("static_statistics.csv")

primary <- "#0db464"
secondary <- "#8A2BE2"

pdf(file="comparison.pdf", height=8, width=16)
plot(descent[,3], type="l", xlab="Chunk Number", ylab="Total Time in seconds", col=primary)
lines(static[,3], col=secondary)

legend("topleft", legend=c("Adaptive Threads", "Fixed Threads"), col=c(primary, secondary), lty=1:1)

dev.off()
