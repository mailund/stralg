file(GLOB sources *.c)
foreach(test_c ${sources})
    get_filename_component(testname ${test_c} NAME_WE)
    add_executable(${testname} ${test_c})
    target_link_libraries(${testname} stralg)
    set_target_properties(
        ${testname} PROPERTIES FOLDER Performance
    )
endforeach(test_c)

