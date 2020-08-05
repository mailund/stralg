library(tidyverse)

performance <- read_table2("match_search.txt",
                           col_names = c("Algorithm", "String", "n", "m", "Time"))
performance <- performance %>%
    mutate(String = ifelse(String == "ASCII", "8-bit", String))

performance$m <- factor(performance$m)
performance$Algorithm <- factor(performance$Algorithm, levels = c("Naive", "BMH", "BM", "Border", "KMP"))
performance$String <- factor(performance$String, levels = c("EQUAL", "DNA", "8-bit"))

# Overall
performance %>%
    filter(m == 100) %>%
    ggplot(
        aes(x = n, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    #geom_jitter() +
    geom_smooth(se = FALSE) +
    scale_color_grey() +
    ylab("Time [seconds]") +
    theme_minimal()
ggsave("Match search overall.pdf", width = 7, height = 7)

# dependency of m
performance %>%
#    filter(String == "ASCII") %>%
    ggplot(
        aes(x = n, y = Time, color = m)) +
    facet_grid(String ~ Algorithm, scales = "free_y") +
    #geom_jitter() +
    geom_smooth(se = FALSE) +
    scale_color_grey() +
    theme_minimal()+
    ylab("Time [seconds]") +
    theme(axis.text.x = element_text(angle = 90, hjust = 1))
ggsave("Match search dependency on m.pdf", width = 7, height = 7)

performance %>%
    filter(Algorithm %in% c("Border", "KMP")) %>%
    ggplot(
        aes(x = n, y = Time, color = m)) +
    facet_grid(String ~ Algorithm, scales = "free_y") +
    #geom_jitter() +
    geom_smooth(se = FALSE) +
    scale_color_grey() +
    ylab("Time [seconds]") +
    theme_minimal()+
    theme(axis.text.x = element_text(angle = 90, hjust = 1))
ggsave("m dependency linear algorithms.pdf", width = 7, height = 7)


performance %>%
    filter(!(Algorithm %in% c("Border", "KMP"))) %>%
    ggplot(
        aes(x = n, y = Time, color = m)) +
    facet_grid(String ~ Algorithm, scales = "free_y") +
    geom_smooth(se = FALSE) +
    scale_color_grey() +
    ylab("Time [seconds]") +
    theme_minimal()+
    theme(axis.text.x = element_text(angle = 90, hjust = 1))
ggsave("m dependency quadratic algorithms.pdf", width = 7, height = 7)



