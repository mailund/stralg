add_executable(display_suffix_tree display_suffix_tree.c)
target_link_libraries(display_suffix_tree stralg)
set_target_properties(
    display_suffix_tree PROPERTIES FOLDER Tools/display_suffix_tree
)