#library(tidyverse)
library(dplyr)
library(readr)
library(ggplot2)
library(ggridges)

performance <- read_table2("bwt_search.txt",
                           col_names = c("Algorithm", "PatternLength", "PatternDistance",
                                         "Edits", "Time"),
                           col_types = "ccncn")
performance$PatternLength <- factor(performance$PatternLength, levels = c("50", "100", "150"))

ggplot(performance,
       aes(x = PatternLength, y = Time, color = Algorithm)) +
    facet_grid(~ Edits) +
    geom_boxplot() +
    xlab("Pattern length") +
    ylab("Time [seconds]") +
    scale_color_grey(start = 0.5, end = 0.05) +
    scale_fill_grey(start = 0.5, end = 0.05) +
    theme_minimal()
ggsave("Approx matching comparison.pdf", width = 7, height = 7)

withoutD <- performance %>% filter(Algorithm == "BWT-without-D")
withD <- performance %>% filter(Algorithm == "BWT-with-D")

comparison <- inner_join(withoutD, withD, by = "PatternLength",
                         suffix = c(".withoutD", ".withD"))

comparison %>% ggplot(aes(x = Edits.withD, y = Time.withD / Time.withoutD,
                          fill = as.factor(PatternDistance.withD))) +
    geom_boxplot() +
    scale_y_log10() +
    xlab("Pattern length") +
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
