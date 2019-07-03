library(tidyverse)

performance1 <- read_table2("suffix_array_construction_v1.txt",
                           col_names = c("Algorithm", "String", "Size", "Time"))
performance3 <- read_table2("suffix_array_construction_v3.txt",
                            col_names = c("Algorithm", "String", "Size", "Time"))
performance4 <- read_table2("suffix_array_construction_v4.txt",
                            col_names = c("Algorithm", "String", "Size", "Time"))
performance5 <- read_table2("suffix_array_construction_v5.txt",
                            col_names = c("Algorithm", "String", "Size", "Time"))
performance <- rbind(performance1, performance3, performance4, performance5)

ggplot(performance,
       aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_point() +
    geom_smooth() +
    theme_minimal()
