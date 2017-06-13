
add_executable(match_test match_test.c)
target_link_libraries(match_test stralg)
add_test(Match match_test)


add_executable(border_array_test border_array_test.c)
target_link_libraries(border_array_test stralg)
add_test(BorderArray border_array_test)
