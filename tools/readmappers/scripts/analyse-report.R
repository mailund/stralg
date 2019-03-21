
check_package <- function(x) {
    if (!require(x, character.only = TRUE))
    {
    	install.packages(x, dep=TRUE)
    	if(!require(x,character.only = TRUE))
    		stop("Package not found")
    }
 }

check_package("tidyverse")

# args <- commandArgs(TRUE)
# if (length(args) < 1) {
# 	stop("The evaluation report needs to be provided as the first argument.")
# }
args <- "/Users/mailund/Projects/stralg/tools/readmappers/out.txt"
time_results <- read.table(args[1], header = TRUE)

normalisation <- time_results %>%
    filter(Operation == "Preprocessing", Mapper == "BWA") %>%
    summarise(mean = mean(Time)) %>%
    unlist()
preprocessing <- time_results %>%
    filter(Operation == "Preprocessing") %>%
    mutate(Time = Time / normalisation)

normalisation <- time_results %>%
    filter(Operation == "Mapping", Mapper == "BWA") %>%
    summarise(mean = mean(Time)) %>%
    unlist()
mapping <- time_results %>%
    filter(Operation == "Mapping") %>%
    mutate(Time = Time / normalisation)
mapping

preprocessing %>%
    ggplot(aes(x = Mapper, colour = as_factor(Editdist), y = Time)) +
    geom_boxplot() +
    geom_jitter(width = 0.1) +
    scale_color_discrete()

mapping %>%
    ggplot(aes(x = Mapper, colour = Editdist, y = Time)) +
    geom_boxplot() +
    geom_jitter(width = 0.1)


ordered_ <- time_results %>%
    group_by(mapper) %>%
    summarise(median_time = median(time)) %>%
    arrange(median_time)
ordered_mappers <- factor(time_results$mapper, levels = ordered$mapper)
fastest <- ordered %>% top_n(n = -1, wt = median_time) %>% as.list

normalised <- time_results %>%
    mutate(time = (time / fastest$median),
           mapper = factor(mapper, levels = ordered$mapper))
ordered_normalised <- normalised %>%
    group_by(mapper) %>%
    summarise(median_time = median(time))

ggplot(normalised, aes(x = mapper, y = time)) +
    geom_hline(
        aes(yintercept = median_time),
        color = "lightgray", linetype="dashed",
        data = ordered_normalised
    ) +
    geom_boxplot(fill = "#E7B800", alpha = 0.7) +
    geom_jitter(width = 0.01, colour = "#fc6721", size = 2) +
    scale_y_continuous(
        labels = paste(fastest$mapper, "x", round(ordered_normalised$median_time)),
        breaks = ordered_normalised$median_time
    ) +
    theme_classic() +
    xlab("Read-mapper") +
    ylab("Normalised running time")
ggsave(paste0(args[1], ".png"))
