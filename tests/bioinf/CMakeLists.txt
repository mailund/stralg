

enable_testing()

file(GLOB test_sources *_test.c)
foreach(test_c ${test_sources})
    get_filename_component(testname ${test_c} NAME_WE)
    add_executable(${testname} ${test_c})
    target_link_libraries(${testname} stralg stralg_bioinf)
    add_test(${testname} ${testname})
    set_target_properties(
        ${testname} PROPERTIES FOLDER Tests/Bioinf
    )
endforeach(test_c)

# add_test(
#     NAME FASTQ-iterator-run
#     COMMAND
#     sh -c "$<TARGET_FILE:fastq_iterator_test> test-data/test.fq > test-data/fastq_iterator_test_current.txt"
#     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
# )
# add_test(
#     NAME FASTQ-iterator-compare
#     COMMAND
#     cmp fastq_iterator_test_current.txt fastq_iterator_test_expected.txt
#     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/test-data"
# )

# add_executable(
#     fasta_test
#     fasta_test.c
# )
# target_link_libraries(
#     fasta_test stralg
#     stralg_bioinf
# )
# add_test(
#     NAME FASTA-run
#     COMMAND
#     sh -c "$<TARGET_FILE:fasta_test> test-data/ref.fa test-data/malformed.fa > test-data/fasta-test-observed.txt"
#     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
# )
# add_test(
#     NAME FASTA-compare-output
#     COMMAND
#     cmp test-data/fasta-test-observed.txt test-data/fasta-test-expected.txt
#     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
# )

