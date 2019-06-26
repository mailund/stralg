library(tidyverse)

performance <- read_table2("suffix_array_construction.txt",
                           col_names = c("Algorithm", "String", "Size", "Time"))

ggplot(performance,
       aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_point() +
    geom_smooth() +
    theme_minimal()
