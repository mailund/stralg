library(tidyverse)
library(readr)
library(ggplot2)



performance <- read_table2("suffix_array_construction.txt",
                            col_names = c("Algorithm", "String", "Size", "Time"))

performance$String <- factor(performance$String, levels = c("Equal", "DNA", "ASCII"))



ggplot(performance,
       aes(x = Size, y = Time, color = Algorithm), shape = Algorithm) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_jitter() +
    geom_smooth(method = "loess", se = FALSE) +
    scale_color_grey() +
    theme_minimal()
ggsave("Suffix array construction.pdf", width = 7, height = 7)
