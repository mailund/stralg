
set(CMAKE_MACOSX_RPATH 1)
add_library(stralg SHARED
	stralg.h
	# helper data structures
	lists.h lists.c
	vectors.h vectors.c
	queues.h queues.c

	# string algorithms
	borders.c borders.h
	aho_corasick.h aho_corasick.c
	bwt.h bwt.c
	cigar.h cigar.c
	edit_distance_generator.h edit_distance_generator.c
	error.h
	match.h match.c

	io.h io.c
	remap.h remap.c
	serialise.h serialise.c
	string_utils.h string_utils.c

	suffix_array.h suffix_array.c
	suffix_array_internal.h suffix_array_internal.c
	skew.c
	sa_is.c sa_is_mem.c


	suffix_tree.h suffix_tree.c
	edge_array_suffix_tree.h edge_array_suffix_tree.c
	trie.h trie.c
)
set_target_properties(
	stralg PROPERTIES FOLDER Libraries/StrAlg
)

target_include_directories(stralg
  PUBLIC
    # Headers used from source/build location:
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/stralg>"
    # Headers used from installed location:
    "$<INSTALL_INTERFACE:include>"
)

install(TARGETS stralg DESTINATION lib)
install(
	DIRECTORY .
	DESTINATION include/stralg/
	FILES_MATCHING PATTERN "*.h"
)
