add_executable(st_build_lcp st_build_lcp.c)
target_link_libraries(st_build_lcp stralg)

set_target_properties(
    st_build_lcp PROPERTIES FOLDER Tools/st_build_lcp
)