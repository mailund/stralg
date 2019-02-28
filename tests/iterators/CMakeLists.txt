
file(GLOB test_sources *_test.c)
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
        ${testname} PROPERTIES FOLDER Tests/Iterators
    )
endforeach(test_c)

# add_test(
#     NAME Match[modest-proposal]
#     COMMAND
#     sh -c "$<TARGET_FILE:match_test> the test-data/modest-proposal.txt"
#     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/iterators"
# )
# add_test(
#     NAME "Match[rep]"
#     COMMAND
#     sh -c "$<TARGET_FILE:match_test> ababaaba test-data/repetitive-string.txt"
#     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/iterators"
# )
