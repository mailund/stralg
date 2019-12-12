library(tidyverse)

performance <- read_table2("match_search.txt",
                           col_names = c("Algorithm", "String", "n", "m", "Time"))

performance %>%
    ggplot(
        aes(x = n, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_jitter() +
    geom_smooth(se = FALSE) +
    scale_color_grey() +
    theme_minimal()
