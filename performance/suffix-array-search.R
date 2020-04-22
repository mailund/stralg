library(tidyverse)

performance <- read_table2("suffix-array-search-tiny.txt",
                           col_names = c("Algorithm", "n", "m", "Time"))

performance %>%
#    filter(n < 10000) %>%
    filter(m %in% c(10, 30, 50)) %>%
    ggplot(aes(x = n, y = Time, color = factor(m))) +
    geom_jitter(alpha = 0.4) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("m", start = 0.5, end = 0.05) +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90))

performance %>%
#    filter(n %in% c(1e6, 2e6, 3e6, 4e6)) %>%
    filter(n %in% c(2e2, 4e2, 6e2)) %>%
    ggplot(aes(x = m, y = Time, color = factor(n))) +
    geom_jitter(alpha = 0.3) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("n", start = 0.5, end = 0.05) +
    theme_minimal()


performance <- read_table2("suffix-array-search-small.txt",
                           col_names = c("Algorithm", "n", "m", "Time"))

performance %>%
    #    filter(n < 10000) %>%
    filter(m %in% c(100, 300, 500)) %>%
    ggplot(aes(x = n, y = Time, color = factor(m))) +
    geom_jitter(alpha = 0.4) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("m", start = 0.5, end = 0.05) +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90))

performance %>%
    #    filter(n %in% c(1e6, 2e6, 3e6, 4e6)) %>%
    filter(n %in% c(2e3, 4e3, 6e3)) %>%
    ggplot(aes(x = m, y = Time, color = factor(n))) +
    geom_jitter(alpha = 0.3) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("n", start = 0.5, end = 0.05) +
    theme_minimal()


performance <- read_table2("suffix-array-search-medium.txt",
                           col_names = c("Algorithm", "n", "m", "Time"))

performance %>%
    #    filter(n < 10000) %>%
    filter(m %in% c(100, 300, 500)) %>%
    ggplot(aes(x = n, y = Time, color = factor(m))) +
    geom_jitter(alpha = 0.3) +
#    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("m", start = 0.5, end = 0.05) +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90))

performance %>%
    #    filter(n %in% c(1e6, 2e6, 3e6, 4e6)) %>%
    filter(n %in% c(2e4, 4e4, 1e5, 2e5)) %>%
    ggplot(aes(x = m, y = Time, color = factor(n))) +
    geom_jitter(alpha = 0.6) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("n", start = 0.5, end = 0.05) +
    theme_minimal()



performance <- read_table2("suffix-array-search-large.txt",
                           col_names = c("Algorithm", "n", "m", "Time"))

performance %>%
    filter(n > 3.5e6 & n < 4.5e6) %>%
    filter(m %in% c(100, 300, 500)) %>%
    ggplot(aes(x = n, y = Time, color = factor(m))) +
    geom_jitter(alpha = 0.3) +
    #    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("m", start = 0.5, end = 0.05) +
    theme_minimal() +
    theme(axis.text.x = element_text(angle = 90))

performance %>%
    #    filter(n %in% c(1e6, 2e6, 3e6, 4e6)) %>%
    filter(n %in% c(1e6, 2e6, 4e6)) %>%
    ggplot(aes(x = m, y = Time, color = factor(n))) +
    geom_jitter(alpha = 0.6) +
    geom_smooth(se = FALSE, method = "lm") +
    facet_grid(~ Algorithm) +
    scale_color_grey("n", start = 0.5, end = 0.05) +
    theme_minimal()


