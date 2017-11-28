
add_executable(match_readmap match_readmap.c)
target_link_libraries(match_readmap stralg)

add_executable(ac_readmap ac_readmap.c)
target_link_libraries(ac_readmap stralg)

add_test(Read-mapping read_map_comparison_test.sh)
