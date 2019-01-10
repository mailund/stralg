
set(CMAKE_MACOSX_RPATH 1)
add_library(stralg SHARED
	stralg.h
	match.h match.c
	trie.h trie.c
	queue.h queue.c
	aho_corasick.h aho_corasick.c
	edit_distance_generator.h edit_distance_generator.c
	cigar.h cigar.c
	pair_stack.h pair_stack.c
	suffix_array.h suffix_array.c
	suffix_tree.h suffix_tree.c
	io.h io.c
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
