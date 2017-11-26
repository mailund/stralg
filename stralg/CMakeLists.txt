
set(CMAKE_MACOSX_RPATH 1)
add_library(stralg SHARED 
	stralg.h
	buffer.h buffer.c 
	match.h match.c
	trie.h trie.c
	queue.h queue.c
	aho_corasick.h aho_corasick.c
	border-arrays.c 
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
