
add_executable(display_match display_match.c)
target_link_libraries(display_match stralg stralg_bioinf)
set_target_properties(
    display_match PROPERTIES FOLDER Tools/display_match
)