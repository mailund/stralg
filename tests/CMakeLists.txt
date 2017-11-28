
add_executable(match_test match_test.c)
target_link_libraries(match_test stralg)
add_test(Match match_test)

add_executable(queue_test queue_test.c)
target_link_libraries(queue_test stralg)
add_test(Queue queue_test)

add_executable(trie_test trie_test.c)
target_link_libraries(trie_test stralg)
add_test(Trie trie_test)

add_executable(aho_corasick_test aho_corasick_test.c)
target_link_libraries(aho_corasick_test stralg)
add_test(Aho-Corasick aho_corasick_test)

add_executable(edit_distance_generator_test edit_distance_generator_test.c)
target_link_libraries(edit_distance_generator_test stralg)
add_test(Edit-distance-generations edit_distance_generator_test)

add_executable(string_vector_test string_vector_test.c)
target_link_libraries(string_vector_test stralg)
add_test(String-vector string_vector_test)

add_executable(suffix_array_test suffix_array_test.c)
target_link_libraries(suffix_array_test stralg)
add_test(Suffix-array suffix_array_test)
