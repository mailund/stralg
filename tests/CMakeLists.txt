
add_executable(
    match_test
    match_test.c
)
target_link_libraries(
    match_test
    stralg
)
add_test(
    NAME Run-naive-modest-proposal
    COMMAND
    sh -c "$<TARGET_FILE:match_test> naive the test-data/modest-proposal.txt > test-data/naive-modest-match.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Run-border-modest-proposal
    COMMAND
    sh -c "$<TARGET_FILE:match_test> border the test-data/modest-proposal.txt > test-data/border-modest-match.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Run-KMP-modest-proposal
    COMMAND
    sh -c "$<TARGET_FILE:match_test> kmp the test-data/modest-proposal.txt > test-data/kmp-modest-match.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Run-BMH-modest-proposal
    COMMAND
    sh -c "$<TARGET_FILE:match_test> bmh the test-data/modest-proposal.txt > test-data/bmh-modest-match.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Naive-border-modest-proposal-cmp
    COMMAND
    cmp test-data/naive-modest-match.txt test-data/border-modest-match.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Naive-KMP-modest-proposal-cmp
    COMMAND
    cmp test-data/naive-modest-match.txt test-data/kmp-modest-match.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Naive-BMH-modest-proposal-cmp
    COMMAND
    cmp test-data/naive-modest-match.txt test-data/bmh-modest-match.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)


add_test(
    NAME Run-naive-rep
    COMMAND
    sh -c "$<TARGET_FILE:match_test> naive ababaaba test-data/repetitive-string.txt > test-data/naive-rep-match.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Run-border-rep
    COMMAND
    sh -c "$<TARGET_FILE:match_test> border ababaaba test-data/repetitive-string.txt > test-data/border-rep-match.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Run-KMP-rep
    COMMAND
    sh -c "$<TARGET_FILE:match_test> kmp ababaaba test-data/repetitive-string.txt > test-data/kmp-rep-match.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Run-BMH-rep
    COMMAND
    sh -c "$<TARGET_FILE:match_test> bmh ababaaba test-data/repetitive-string.txt > test-data/bmh-rep-match.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
add_test(
    NAME Naive-border-rep-cmp
    COMMAND
    cmp naive-rep-match.txt border-rep-match.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/test-data"
)
add_test(
    NAME Naive-KMP-rep-cmp
    COMMAND
    cmp naive-rep-match.txt kmp-rep-match.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/test-data"
)
add_test(
    NAME Naive-BMH-rep-cmp
    COMMAND
    cmp naive-rep-match.txt bmh-rep-match.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/test-data"
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
    NAME Suffix-tree-run
    COMMAND
    sh -c "$<TARGET_FILE:suffix_tree_test> > test-data/suffix-tree-test-observed.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)
#add_test(
#    NAME Suffix-tree-compare-output
#    COMMAND
#    cmp test-data/suffix-tree-test-observed.txt test-data/suffix-tree-test-expected.txt
#    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
#)
