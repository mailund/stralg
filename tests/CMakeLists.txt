
add_executable(match_test match_test.c)
target_link_libraries(match_test stralg)
add_test(Match match_test)

add_executable(queue_test queue_test.c)
target_link_libraries(queue_test stralg)
add_test(Queue queue_test)

add_executable(trie_test trie_test.c)
target_link_libraries(trie_test stralg)
add_test(Trie trie_test)

