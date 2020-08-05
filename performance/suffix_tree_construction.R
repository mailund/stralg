#library(tidyverse)
library(readr)
library(dplyr)
library(ggplot2)

strings = c(equal = "Equal", random = "DNA", random_large = "8-bit")
algomap = c("EA-LCP" = "Array LCP", "EA-McCreight" = "Array McCreight", "EA-Naive" = "Array Naive",
            "LCP-build" = "LCP + construction", "EA-LCP-build" = "Array LCP + construction")
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
performance <- performance %>%
    mutate(String = ifelse(String == "ASCII", "8-bit", String))
performance %>%
    mutate(String = strings[String], Algorithm = algorithms(Algorithm)) %>%
    filter(Algorithm %in% c("LCP", "Naive", "McCreight")) %>%
    mutate(String = factor(String, levels = c("Equal", "DNA", "8-bit"))) %>%
    ggplot(
       aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_jitter() +
    geom_smooth(se = FALSE) +
    ylab("Time [seconds]") +
    scale_color_grey() +
    theme_minimal()
ggsave("All linked lists times.pdf", width = 7, height = 7)

performance %>%
    mutate(String = strings[String], Algorithm = algorithms(Algorithm)) %>%
    filter(Algorithm %in% c("LCP", "McCreight", "Naive")) %>%
    filter(Size <= 50000) %>%
    mutate(String = factor(String, levels = c("Equal", "DNA", "8-bit"))) %>%
    filter(!(String == "Equal" & Algorithm == "Naive")) %>%
    ggplot(
        aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_jitter() +
    ylab("Time [seconds]") +
    geom_smooth(se = FALSE) +
    scale_color_grey() +
    theme_minimal()
ggsave("Linked lists without naive equal.pdf", width = 7, height = 7)


performance %>%
    group_by(Algorithm, String, Size) %>%
    filter(Time < 2 * mean(Time)) %>% # crazy outliers.
    ungroup() %>%
    filter(!(Algorithm %in% c("EA-Naive", "Naive"))) %>%
    filter(!(Algorithm %in% c("EA-LCP-build", "LCP-build"))) %>%
    mutate(String = strings[String], Algorithm = algorithms(Algorithm)) %>%
    ggplot(
        aes(y = Time, x = Size, colour = String)
    )   +
    facet_grid(. ~ Algorithm) +
    scale_color_grey() +
    geom_jitter() +
    ylab("Time [seconds]") +
    ylim(0,0.06) +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90, hjust = 1))
ggsave("Adding arrays times.pdf", width = 7, height = 7)


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
    ylab("Time [seconds]") +
    theme_minimal()
ggsave("LCP comparisons.pdf", width = 7, height = 7)


performance %>%
    #filter(Algorithm %in% c("EA-LCP-build", "LCP", "EA-LCP", "LCP-build", "McCreight")) %>%
    filter(Algorithm %in% c("LCP", "LCP-build", "McCreight")) %>%
    mutate(String = strings[String], Algorithm = algorithms(Algorithm)) %>%
    #filter(!(String == "equal")) %>%
    ggplot(
        aes(x = Size, y = Time)
    )  +
    facet_grid(String ~ Algorithm, scales = "free_y") +
    ylab("Time [seconds]") +
    geom_jitter() +
    #geom_smooth(se = FALSE) +
    scale_color_grey() +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90, hjust = 1))
ggsave("Adding array construction.pdf", width = 9, height = 8)


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
