library(tidyverse)
library(patchwork)

performance <- read_table2("suffix-array-search-tiny.txt",
                           col_names = c("Algorithm", "n", "m", "Time"))

n <- performance %>%
#    filter(n < 10000) %>%
    filter(m %in% c(10, 30, 50)) %>%
    ggplot(aes(x = n, y = Time, color = factor(m))) +
    geom_jitter(alpha = 0.4) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("m", start = 0.5, end = 0.05) +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90))

m <- performance %>%
#    filter(n %in% c(1e6, 2e6, 3e6, 4e6)) %>%
    filter(n %in% c(2e2, 4e2, 6e2)) %>%
    ggplot(aes(x = m, y = Time, color = factor(n))) +
    geom_jitter(alpha = 0.3) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("n", start = 0.5, end = 0.05) +
    theme_minimal()

n / m
ggsave("Suffix array search tiny.pdf", width = 7, height = 10)

performance <- read_table2("suffix-array-search-small.txt",
                           col_names = c("Algorithm", "n", "m", "Time"))

n <- performance %>%
    #    filter(n < 10000) %>%
    filter(m %in% c(100, 300, 500)) %>%
    ggplot(aes(x = n, y = Time, color = factor(m))) +
    geom_jitter(alpha = 0.4) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("m", start = 0.5, end = 0.05) +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90))

m <- performance %>%
    #    filter(n %in% c(1e6, 2e6, 3e6, 4e6)) %>%
    filter(n %in% c(2e3, 4e3, 6e3)) %>%
    ggplot(aes(x = m, y = Time, color = factor(n))) +
    geom_jitter(alpha = 0.3) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("n", start = 0.5, end = 0.05) +
    theme_minimal()

n/m
ggsave("Suffix array search small.pdf", width = 7, height = 10)

performance <- read_table2("suffix-array-search-medium.txt",
                           col_names = c("Algorithm", "n", "m", "Time"))

n <- performance %>%
    #    filter(n < 10000) %>%
    filter(m %in% c(100, 300, 500)) %>%
    ggplot(aes(x = n, y = Time, color = factor(m))) +
    geom_jitter(alpha = 0.3) +
#    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("m", start = 0.5, end = 0.05) +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90))

m <- performance %>%
    #    filter(n %in% c(1e6, 2e6, 3e6, 4e6)) %>%
    filter(n %in% c(2e4, 4e4, 1e5, 2e5)) %>%
    ggplot(aes(x = m, y = Time, color = factor(n))) +
    geom_jitter(alpha = 0.6) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("n", start = 0.5, end = 0.05) +
    theme_minimal()

n/m

ggsave("Suffix array search medium.pdf", width = 7, height = 10)


# Mesurement artifacts in the first runs
performance_full <- read_table2("suffix-array-search-large-full.txt",
                           col_names = c("Algorithm", "n", "m", "Time"))
performance_range <- read_table2("suffix-array-search-large-range.txt",
                                col_names = c("Algorithm", "n", "m", "Time"))
performance_range2 <- read_table2("suffix-array-search-large-range-2.txt",
                                 col_names = c("Algorithm", "n", "m", "Time"))
# This ran on another machine and the artifacts are gone. I need to
# rescale to compensate for the different machine
performance_range3 <- read_table2("suffix-array-search-large-range-3.txt",
                                  col_names = c("Algorithm", "n", "m", "Time"))

bwt_full_mean <- performance_full %>%
    filter(n == 3.4e6 | n == 4.6e6) %>%
    filter(Algorithm == "BWT") %>%
    summarize(mean = mean(Time)) %>% unlist()
bwt_range_mean <- performance_range3 %>%
    filter(Algorithm == "BWT") %>%
    summarize(mean = mean(Time)) %>% unlist()

sa_full_mean <- performance_full %>%
    filter(n == 3.3e6 | n == 4.7e6) %>%
    filter(Algorithm == "SA") %>%
    summarize(mean = mean(Time)) %>% unlist()
sa_range_mean <- performance_range3 %>%
    filter(Algorithm == "SA") %>%
    summarize(mean = mean(Time)) %>% unlist()

st_full_mean <- performance_full %>%
    filter(n == 3.3e6 | n == 4.7e6) %>%
    filter(Algorithm == "ST") %>%
    summarize(mean = mean(Time)) %>% unlist()
st_range_mean <- performance_range3 %>%
    filter(Algorithm == "ST") %>%
    summarize(mean = mean(Time)) %>% unlist()

performance_range3 <- performance_range3 %>%
    mutate(Time = ifelse(Algorithm == "BWT", Time * bwt_full_mean / bwt_range_mean, Time)) %>%
    mutate(Time = ifelse(Algorithm == "SA", Time * sa_full_mean / sa_range_mean, Time)) %>%
    mutate(Time = ifelse(Algorithm == "ST", Time * st_full_mean / st_range_mean, Time))

performance_filtered <- performance_full %>% filter(n < range$min | n > range$max)
performance_excluded <- performance_full %>% filter(!(n < range$min | n > range$max))


performance <- rbind(performance_filtered, performance_range3) # performance_range

n <- performance %>%
    #filter(n < 4e6) %>%
    filter(m %in% c(100, 300, 500)) %>%
    ggplot(aes(x = n, y = Time, color = factor(m))) +
    geom_jitter(alpha = 0.3) +
    #    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("m", start = 0.5, end = 0.05) +
    scale_y_continuous(limits = c(0, 0.04)) +
#    geom_vline(xintercept = range$min) +
#    geom_vline(xintercept = range$max) +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90))
n



m <- performance %>%
    #    filter(n %in% c(1e6, 2e6, 3e6, 4e6)) %>%
    filter(n %in% c(1e6, 3e6, 6e6)) %>%
    ggplot(aes(x = m, y = Time, color = factor(n))) +
    geom_jitter(alpha = 0.6) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("n", start = 0.5, end = 0.05) +
    theme_minimal()

n/m

ggsave("Suffix array search large.pdf", width = 7, height = 10)

