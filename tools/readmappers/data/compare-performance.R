library(tidyverse)

preprocess <- read_table2("preprocessing-report.txt",
                          col_names = c("Tool", "Reference", "Time"))


ggplot(preprocess, aes(color = Tool, x = Reference, y = Time)) +
  geom_boxplot() +
  theme_bw()


readmap1 <- read_table2("readmapping-report-1.txt",
                          col_names = c("Tool", "Reference", "Reads", "Time"))

readmap2 <- read_table2("readmapping-report.txt",
                       col_names = c("Tool", "Reference", "Reads", "Time"))

readmap <- rbind(readmap1, readmap2)

readmap$Reads <- factor(readmap$Reads,
                        levels = c("reads/reads-100-10-0.fq",
                                   "reads/reads-100-10-1.fq",
                                   "reads/reads-100-100-1.fq",
                                    "reads/reads-100-100-2.fq",
                                    "reads/reads-1000-100-1.fq",
                                    "reads/reads-1000-100-2.fq",
                                    "reads/reads-1000-200-1.fq"
                        ))


ggplot(readmap, aes(color = Tool, x = Reads, y = Time)) +
  facet_grid(~Reference) +
  geom_boxplot() +
  theme_bw() +
  theme(axis.text.x = element_text(angle = 90, hjust = 1))


