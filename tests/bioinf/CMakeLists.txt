

enable_testing()

file(GLOB test_sources *_test.c)
foreach(test_c ${test_sources})
    get_filename_component(testname ${test_c} NAME_WE)
    add_executable(${testname} ${test_c})
    target_link_libraries(${testname} stralg stralg_bioinf)
    add_test(
        NAME ${testname}
        COMMAND ${testname}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/bioinf"
    )
    set_target_properties(
        ${testname} PROPERTIES FOLDER Tests/Bioinf
    )
endforeach(test_c)

# -- TESTING ------------------------
add_test(
    NAME FASTQ-iterator-compare
    COMMAND
    cmp fastq_iterator_test_current.txt fastq_iterator_test_expected.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/bioinf/test-data"
)
add_test(
    NAME FASTA-compare-output
    COMMAND
    cmp test-data/fasta-test-observed.txt test-data/fasta-test-expected.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/bioinf"
)
add_test(
    NAME FASTA-io-output
    COMMAND
    cmp test-data/ref.fa test-data/ref.out.fa
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/bioinf"
)
