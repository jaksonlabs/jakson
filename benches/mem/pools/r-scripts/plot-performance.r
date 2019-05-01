# Uncomment the following lines, if not yet installed
# install.packages('rstudioapi')
# install.packages('ggplot2')

library('ggplot2')
library('rstudioapi')
library('plyr')
library('scales')

data_file_clib_alloc_macos = "bench-results-clib-allocator---macos.csv"
data_file_clib_alloc_linux = "bench-results-mempool-none---macos.csv"

result_dir = paste(dirname(rstudioapi::getActiveDocumentContext()$path), sep = "", "/../results");

data_file_path_clib_alloc_macos   = paste(result_dir, sep = "/", data_file_clib_alloc_macos)
data_file_path_clib_alloc_linux = paste(result_dir, sep = "/", data_file_clib_alloc_linux)

data_clib_alloc_macos = read.csv(file=data_file_path_clib_alloc_macos, head = TRUE, sep = ",")
data_clib_alloc_linux = read.csv(file=data_file_path_clib_alloc_linux, head = TRUE, sep = ",")

data_clib_alloc_macos_agg = ddply(data_clib_alloc_macos,~alpha,summarise,mean=mean(1000/call_duration_ms),sd=sd(1000/call_duration_ms))
data_clib_alloc_linux_agg = ddply(data_clib_alloc_linux,~alpha,summarise,mean=mean(1000/call_duration_ms),sd=sd(1000/call_duration_ms))

plot(data_clib_alloc_macos_agg$mean)
plot(data_clib_alloc_linux_agg$mean)

df = data.frame(
    ops_per_sec = c(data_clib_alloc_macos_agg$mean, data_clib_alloc_linux_agg$mean),
    sd = c(data_clib_alloc_macos_agg$sd, data_clib_alloc_linux_agg$sd),
    ratio = c(data_clib_alloc_macos_agg$alpha, data_clib_alloc_linux_agg$alpha),
    label = c(rep("macOS (Darwin 18.5.0)", length(data_clib_alloc_macos_agg$mean)), rep("Ubuntu (GNU/Linux 4.15.0-46)  ", length(data_clib_alloc_linux_agg$mean)))
)

breaks = round(seq(min(df$ops_per_sec),max(df$ops_per_sec), length.out = 5))

ggplot(data=df, aes(x=ratio, y=ops_per_sec, group=label)) + 
  geom_errorbar(aes(x = ratio, ymin=ops_per_sec - sd, ymax=ops_per_sec + sd, colour = label), alpha = 0.4) + 
  geom_point(aes(shape=label, colour = label), size=2.0) +
  geom_line(aes(linetype=label, colour = label), size=1.0) +
  scale_color_brewer(palette="Paired")+
  theme_minimal() + 
  theme(plot.margin=unit(c(1,1,1.5,1.2),"cm"), plot.title = element_text(size=12), legend.position="top", legend.title=element_blank(), legend.justification = c(-1, 0),
        legend.direction = "horizontal", axis.title.y=element_blank(), axis.title.x=element_text(size=10)) + scale_y_continuous(breaks=breaks, labels = unit_format(unit = "mio ops/sec", scale = 1e-6, sep = " ", digits = 0)) +
  scale_x_continuous(breaks=c(0,0.25,0.50,0.75,1.0), labels=c("free\nonly", "75% free\n25% realloc", "50% free\n50% realloc", "25% free\n75% realloc", "realloc\nonly")) +
  ggtitle("Clib realloc/free Memory Management Performances")