#library(tidyverse)
library(readr)
library(ggplot2)


performance6 <- read_table2("suffix_array_construction_v6.txt",
                            col_names = c("Algorithm", "String", "Size", "Time"))
performance7 <- read_table2("suffix_array_construction_v7.txt",
                            col_names = c("Algorithm", "String", "Size", "Time"))
performance8 <- read_table2("suffix_array_construction_v8.txt",
                            col_names = c("Algorithm", "String", "Size", "Time"))


performance <- read_table2("suffix_array_construction.txt",
                            col_names = c("Algorithm", "String", "Size", "Time"))
performance$String <- factor(performance$String, levels = c("Equal", "DNA", "ASCII"))


ggplot(performance,
       aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_jitter() +
    geom_smooth(method = "loess", se = FALSE) +
    theme_minimal()
