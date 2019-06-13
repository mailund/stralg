add_executable(exact_readmapper exact_readmapper.c)
target_link_libraries(exact_readmapper stralg stralg_bioinf)

set_target_properties(
    exact_readmapper PROPERTIES FOLDER Tools/exact_readmapper
)