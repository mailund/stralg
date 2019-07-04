library(tidyverse)

performance6 <- read_table2("suffix_array_construction_v6.txt",
                            col_names = c("Algorithm", "String", "Size", "Time"))


performance <- rbind(performance6)

ggplot(performance,
       aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_point() +
    geom_smooth() +
    theme_minimal()
