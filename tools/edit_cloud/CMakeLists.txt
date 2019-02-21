add_executable(edit_cloud edit_cloud.c)
target_link_libraries(edit_cloud stralg stralg_bioinf)
set_target_properties(
    edit_cloud PROPERTIES FOLDER Tools/edit_cloud
)