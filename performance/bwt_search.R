#library(tidyverse)
library(dplyr)
library(readr)
library(ggplot2)
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

comparison <- inner_join(withoutD, withD, by = "PatternLength",
                         suffix = c(".withoutD", ".withD"))

comparison %>% ggplot(aes(x = Edits.withD, y = Time.withD / Time.withoutD,
                          fill = as.factor(PatternDistance.withD))) +
    geom_boxplot() +
    scale_y_log10() +
    theme_minimal()

performance1 <- read_table2("bwt_search_v1.txt",
                            col_names = c("Algorithm", "PatternLength", "PatternDistance",
                                          "Edits", "Time"),
                            col_types = "ccncn")

performance2 <- read_table2("bwt_search_v2.txt",
                            col_names = c("Algorithm", "PatternLength", "PatternDistance",
                                          "Edits", "Time"),
                            col_types = "ccncn")

ggplot(rbind(performance1, performance2, performance),
       aes(x = PatternLength, y = Time, color = Algorithm)) +
    facet_grid(~ Edits) +
    geom_boxplot() +
    theme_minimal()
