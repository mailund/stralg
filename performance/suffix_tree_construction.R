#library(tidyverse)
library(readr)
library(dplyr)
library(ggplot2)

strings = c(equal = "Equal", random = "DNA", random_large = "ASCII")
algomap = c("EA-LCP" = "Array LCP", "EA-McCreight" = "Array McCreight", "EA-Naive" = "Array Naive")
algorithms <- Vectorize(function(alg) {
    if (alg %in% names(algomap)) algomap[alg]
    else alg
})


performance1 <- read_table2("suffix_tree_construction_v1.txt",
                           col_names = c("Algorithm", "String", "Size", "Time"))
performance2 <- read_table2("suffix_tree_construction_v2.txt",
                           col_names = c("Algorithm", "String", "Size", "Time"))
current_performance <- read_table2("suffix_tree_construction.txt",
                            col_names = c("Algorithm", "String", "Size", "Time", "Nodes"))


performance <- current_performance

performance %>%
    mutate(String = strings[String], Algorithm = algorithms(Algorithm)) %>%
    filter(Algorithm %in% c("LCP", "Naive", "McCreight")) %>%
    mutate(String = factor(String, levels = c("Equal", "DNA", "ASCII"))) %>%
    ggplot(
       aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_jitter() +
    geom_smooth(se = FALSE) +
    scale_color_grey() +
    theme_minimal()

performance %>%
    filter(!(Algorithm %in% c("EA-Naive", "Naive"))) %>%
    mutate(String = strings[String], Algorithm = algorithms(Algorithm)) %>%
    filter(Algorithm %in% c("LCP", "Naive", "McCreight")) %>%
    mutate(String = factor(String, levels = c("Equal", "DNA", "ASCII"))) %>%
    ggplot(
        aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_jitter() +
    geom_smooth(se = FALSE) +
    scale_color_grey() +
    theme_minimal()

performance %>% filter(!(Algorithm %in% c("EA-Naive", "Naive"))) %>%
    mutate(String = strings[String], Algorithm = algorithms(Algorithm)) %>%
    ggplot(
        aes(y = Time, x = Nodes, colour = String)
    )   +
    facet_grid(. ~ Algorithm) +
    scale_color_grey() +
    geom_jitter() +
    theme_minimal()


performance %>%
    filter(Algorithm %in% c("EA-LCP", "LCP")) %>%
    mutate(String = strings[String], Algorithm = algorithms(Algorithm)) %>%
    #filter(!(String == "equal")) %>%
    ggplot(
        aes(x = Size, y = Time, color = Algorithm)
    )  +
    facet_grid(String ~ ., scales = "free_y") +
    geom_jitter() +
    geom_smooth(se = FALSE) +
    scale_color_grey() +
    theme_minimal()



# performance %>% filter(Algorithm != "naive") %>%
#     ggplot(aes(x = Size, y = Time, color = Algorithm)) +
#     facet_grid(String ~ ., scales = "free_y") +
#     geom_point() +
#     geom_smooth() +
#     theme_minimal()
#
# comparison_data <- inner_join(
#     performance1,
#     filter(performance2, Algorithm != "naive"),
#     by = c("String", "Size")) %>%
#     rename(`First McCreight` = Time.x,
#            `Mempool McCreight` = Time.y) %>%
#     select(-starts_with("Algorithm."))
#
# comparison_data %>%
#     ggplot(aes(x = Size, y = `Mempool McCreight` / `First McCreight`, color = String)) +
#     geom_jitter() +
#     geom_smooth(method = "lm") +
#     theme_minimal()
#
# comparison_data <- inner_join(
#     performance2,
#     filter(current_performance, Algorithm != "naive"),
#     by = c("String", "Size")) %>%
#     rename(`Mempool McCreight` = Time.x,
#            `Current McCreight` = Time.y) %>%
#     select(-starts_with("Algorithm."))
#
# comparison_data %>%
#     filter(Size > 1000) %>% # too much noise at small sizes
#     ggplot(aes(x = Size, y = `Current McCreight` / `Mempool McCreight`, color = String)) +
#     facet_grid(String ~ .) + #, scales = "free_y") +
#     geom_jitter() +
#     geom_smooth(method = "lm") +
#     theme_minimal()
