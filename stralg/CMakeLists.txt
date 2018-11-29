
set(CMAKE_MACOSX_RPATH 1)
add_library(stralg SHARED
	stralg.h
	match.h match.c
	trie.h trie.c
	queue.h queue.c
	aho_corasick.h aho_corasick.c
	edit_distance_generator.h edit_distance_generator.c
	cigar.h cigar.c
	strings.h strings.c
	string_vector.h string_vector.c
	size_vector.h size_vector.c
	fasta.h fasta.c
	fastq.h fastq.c
	sam.h sam.c
	pair_stack.h pair_stack.c
	suffix_array.h suffix_array.c
	border-arrays.c
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
install(FILES stralg.h buffer.h match.h trie.h queue.h DESTINATION include)
