add_subdirectory(exact_readmapper)
add_subdirectory(bwt_readmapper)

file(COPY data DESTINATION ${CMAKE_BINARY_DIR}/tools/readmappers)
file(COPY scripts DESTINATION ${CMAKE_BINARY_DIR}/tools/readmappers)

