


file(GLOB test_sources *_test.c *.cpp)
foreach(test_c ${test_sources})
    get_filename_component(testname ${test_c} NAME_WE)
    add_executable(${testname} ${test_c})
    target_link_libraries(${testname} stralg)
    add_test(
        ${testname} ${testname}
        #COMMAND "$<TARGET_FILE:${testname}>"
        #WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/stralg"
    )
    set_target_properties(
        ${testname} PROPERTIES FOLDER Tests/StrAlg
    )
endforeach(test_c)

add_test(
	NAME my_test
    COMMAND 
	#sh -c "$<TARGET_FILE:my_suffix_array_test> test-data/clusters_VGR.txt test-data/text_patterns.txt"
	sh -c "$<TARGET_FILE:my_suffix_array_test> test-data/text_data.txt test-data/text_data_patterns.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/stralg"
)
add_test(
    NAME Match[modest-proposal]
    COMMAND
    sh -c "$<TARGET_FILE:match_test> the test-data/modest-proposal.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/stralg"
)
add_test(
    NAME "Match[rep]"
    COMMAND
    sh -c "$<TARGET_FILE:match_test> ababaaba test-data/repetitive-string.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/stralg"
)

add_test(
    NAME Edit-distance-generator-run
    COMMAND
    sh -c "$<TARGET_FILE:edit_distance_generator_test> > test-data/edit_distance_generator_test_observed.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/stralg"
)
add_test(
    NAME Edit-distance-generator-compare-output
    COMMAND
    cmp edit_distance_generator_test_observed.txt edit_distance_generator_test_expected.txt
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/stralg/test-data"
)

add_test(
    NAME Suffix-tree[Match/modest-proposal]
    COMMAND
    sh -c "$<TARGET_FILE:suffix_tree_match_test> the test-data/modest-proposal.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/stralg"
)
add_test(
    NAME Suffix-tree[Match/rep]
    COMMAND
    sh -c "$<TARGET_FILE:suffix_tree_match_test> ababaaba test-data/repetitive-string.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/stralg"
)

add_test(
    NAME Approximative[Edits]
    COMMAND
    sh -c "$<TARGET_FILE:approx_match_test> approx"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/stralg"
)


