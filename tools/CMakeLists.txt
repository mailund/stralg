

add_executable(edit_cloud edit_cloud.c options.c options.h)
target_link_libraries(edit_cloud stralg stralg_bioinf)

add_executable(display_match display_match.c)
target_link_libraries(display_match stralg stralg_bioinf)

add_executable(display_trie display_trie.c)
target_link_libraries(display_trie stralg stralg_bioinf)
