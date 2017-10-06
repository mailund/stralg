
add_executable(match_test match_test.c)
target_link_libraries(match_test stralg)
add_test(Match match_test)
