add_executable(display_trie display_trie.c)
target_link_libraries(display_trie stralg)
set_target_properties(
    display_trie PROPERTIES FOLDER Tools/display_trie
)