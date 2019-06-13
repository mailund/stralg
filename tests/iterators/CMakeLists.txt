
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

