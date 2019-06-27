library(tidyverse)

performance1 <- read_table2("suffix_tree_construction_v1.txt",
                           col_names = c("Algorithm", "String", "Size", "Time"))
performance2 <- read_table2("suffix_tree_construction.txt",
                           col_names = c("Algorithm", "String", "Size", "Time"))

performance <- rbind(performance1, performance2)

ggplot(performance,
       aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_point() +
    geom_smooth() +
    theme_minimal()

performance %>% filter(Algorithm != "naive") %>%
    ggplot(aes(x = Size, y = Time, color = Algorithm)) +
    facet_grid(String ~ ., scales = "free_y") +
    geom_point() +
    geom_smooth() +
    theme_minimal()

comparison_data <- inner_join(
    performance1,
    filter(performance2, Algorithm != "naive"),
    by = c("String", "Size")) %>%
    rename(`First McCreight` = Time.x,
           `Node pool McCreight` = Time.y) %>%
    select(-starts_with("Algorithm."))

comparison_data %>%
    ggplot(aes(x = Size, y = `Node pool McCreight` / `First McCreight`, color = String)) +
    geom_jitter() +
    geom_smooth(method = "lm") +
    theme_minimal()
