
set(CMAKE_MACOSX_RPATH 1)
add_library(stralg_bioinf SHARED
	bioinf.h
	fasta.h fasta.c
	fastq.h fastq.c
	sam.h sam.c
)
target_link_libraries(stralg_bioinf stralg)
set_target_properties(
	stralg_bioinf PROPERTIES FOLDER Libraries/Bioinformatics
)
target_include_directories(stralg_bioinf
  PUBLIC
    # Headers used from source/build location:
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/stralg>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/bioinf>"
    # Headers used from installed location:
    "$<INSTALL_INTERFACE:include>"
)

install(TARGETS stralg_bioinf DESTINATION lib)
install(
	DIRECTORY .
	DESTINATION include/stralg_bioinf/
	FILES_MATCHING PATTERN "*.h"
)
