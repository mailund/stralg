#library(tidyverse)
library(ggplot2)
library(dplyr)
library(readr)

performance <- read_table2("bwt_construction.txt",
                           col_names = c("Algorithm", "Length", "Time"))

ggplot(performance,
       aes(x = Length, y = Time, color = Algorithm)) +
    geom_point() +
    geom_smooth(method = "lm", se = FALSE) +
    scale_color_grey(start = 0.5, end = 0.05) +
    theme_minimal()
ggsave("BWT construction time comparison.pdf", width = 7, height = 7)


withoutD <- performance %>% filter(Algorithm == "BWT-no-D")
withD <- performance %>% filter(Algorithm == "BWT-with-D")

comparison <- inner_join(withoutD, withD, by = "Length")

comparison %>% ggplot(aes(x = Size, y = Time.y / Time.x)) +
    geom_point() +
    geom_smooth(method = "lm") +
    theme_minimal()



