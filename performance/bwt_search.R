library(tidyverse)
library(ggridges)

performance <- read_table2("bwt_search.txt",
                           col_names = c("Algorithm", "PatternLength", "PatternDistance",
                                         "Edits", "Time"),
                           col_types = "ccncn")

ggplot(performance,
       aes(x = PatternLength, y = Time, color = Algorithm)) +
    facet_grid(~ Edits) +
    geom_boxplot() +
    theme_minimal()

withoutD <- performance %>% filter(Algorithm == "BWT-without-D")
withD <- performance %>% filter(Algorithm == "BWT-with-D")

comparison <- inner_join(withoutD, withD, by = "PatternLength")

comparison %>% ggplot(aes(x = Edits.x, y = Time.y / Time.x)) +
    geom_boxplot() +
    scale_y_log10() +
    theme_minimal()

