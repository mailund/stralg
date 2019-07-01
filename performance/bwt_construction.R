library(tidyverse)

performance <- read_table2("bwt_construction.txt",
                           col_names = c("Algorithm", "Size", "Time"))

ggplot(performance,
       aes(x = Size, y = Time, color = Algorithm)) +
    geom_point() +
    geom_smooth() +
    theme_minimal()

withoutD <- performance %>% filter(Algorithm == "BWT-no-D")
withD <- performance %>% filter(Algorithm == "BWT-with-D")

comparison <- inner_join(withoutD, withD, by = "Size")

comparison %>% ggplot(aes(x = Size, y = Time.y / Time.x)) +
    geom_point() +
    geom_smooth() +
    theme_minimal()


