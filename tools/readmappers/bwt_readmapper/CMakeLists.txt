add_executable(bwt_readmapper bwt_readmapper.c)
target_link_libraries(bwt_readmapper stralg stralg_bioinf)

set_target_properties(
    bwt_readmapper PROPERTIES FOLDER Tools/bwt_readmapper
)

