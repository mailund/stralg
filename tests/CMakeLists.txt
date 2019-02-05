
add_executable(
    queue_test
    queue_test.c
)
target_link_libraries(
    queue_test
    stralg
)
add_test(
    Queue
    queue_test
)
add_executable(
    vector_test
    vector_test.c
)
target_link_libraries(
    vector_test
    stralg
)
add_test(
    Vector
    vector_test
)

add_executable(
    match_test
    match_test.c
)
target_link_libraries(
    match_test
    stralg
)
add_test(
    NAME Match[modest-proposal]
    COMMAND
    sh -c "$<TARGET_FILE:match_test> the test-data/modest-proposal.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME "Match[rep]"
    COMMAND
    sh -c "$<TARGET_FILE:match_test> ababaaba test-data/repetitive-string.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)

add_executable(
    edit_distance_generator_test
    edit_distance_generator_test.c
)
target_link_libraries(
    edit_distance_generator_test
    stralg
)
add_test(
    NAME Edit-distance-generator-run
    COMMAND
    sh -c "$<TARGET_FILE:edit_distance_generator_test> > test-data/edit_distance_generator_test_observed.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Edit-distance-generator-compare-output
    COMMAND
    cmp edit_distance_generator_test_observed.txt edit_distance_generator_test_expected.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/test-data"
)

add_executable(
    trie_test
    trie_test.c
)
target_link_libraries(
    trie_test
    stralg
)
add_test(
    Trie
    trie_test
)

add_executable(
    aho_corasick_test
    aho_corasick_test.c
)
target_link_libraries(
    aho_corasick_test
    stralg
)
add_test(
    Aho-Corasick
    aho_corasick_test
)

add_executable(
    remap_test
    remap_test.c
)
target_link_libraries(
    remap_test
    stralg
)
add_test(
    Remap
    remap_test
)

add_executable(
    bwt_test
    bwt_test.c
)
target_link_libraries(
    bwt_test
    stralg
)
add_test(
    BWT
    bwt_test
)


add_executable(
    suffix_array_test
    suffix_array_test.c
)
target_link_libraries(
    suffix_array_test
    stralg
)
add_test(
    Suffix-array
    suffix_array_test
)

# BIOINF ----------------------------------------------
add_executable(
    fastq_iterator_test
    fastq_iterator_test.c
)
target_link_libraries(
    fastq_iterator_test
    stralg stralg_bioinf
)
add_test(
    NAME FASTQ-iterator-run
    COMMAND
    sh -c "$<TARGET_FILE:fastq_iterator_test> test-data/test.fq > test-data/fastq_iterator_test_current.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME FASTQ-iterator-compare
    COMMAND
    cmp fastq_iterator_test_current.txt fastq_iterator_test_expected.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/test-data"
)

add_executable(
    fasta_test
    fasta_test.c
)
target_link_libraries(
    fasta_test stralg
    stralg_bioinf
)
add_test(
    NAME FASTA-run
    COMMAND
    sh -c "$<TARGET_FILE:fasta_test> test-data/ref.fa test-data/malformed.fa > test-data/fasta-test-observed.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME FASTA-compare-output
    COMMAND
    cmp test-data/fasta-test-observed.txt test-data/fasta-test-expected.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)

add_executable(
    suffix_tree_test
    suffix_tree_test.c
)
target_link_libraries(
    suffix_tree_test stralg
)
add_test(
    NAME Suffix-tree
    COMMAND suffix_tree_test
)

add_executable(
    suffix_tree_match_test
    suffix_tree_match_test.c
)
target_link_libraries(
    suffix_tree_match_test stralg
)
add_test(
    NAME Suffix-tree[Match/modest-proposal]
    COMMAND
    sh -c "$<TARGET_FILE:suffix_tree_match_test> the test-data/modest-proposal.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Suffix-tree[Match/rep]
    COMMAND
    sh -c "$<TARGET_FILE:suffix_tree_match_test> ababaaba test-data/repetitive-string.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)

add_executable(
    approx_match_test
    approx_match_test.c
)
target_link_libraries(
    approx_match_test stralg
)
add_test(
    NAME Approximative-matching
    COMMAND approx_match_test
)
