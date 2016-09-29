macro (make_test name)
    set(sources ${ARGN})
    if (NOT sources)
        set(sources ${name}.cpp)
    endif ()
    add_executable (${name} ${sources})
    target_link_libraries(${name} PRIVATE asl)
    target_link_boost_test(${name})
endmacro ()

macro (add_test_and_data name)
    get_target_property(bin_location ${name} RUNTIME_OUTPUT_DIRECTORY)
    foreach (arg ${ARGN})
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${arg})
            add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/${arg}"
                "$<TARGET_FILE_DIR:${name}>/${arg}"
            )
        else ()
            get_filename_component(filename_without_path ${arg} NAME)
            add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${filename_without_path}"
                $<TARGET_FILE_DIR:${name}>
            )
        endif ()
    endforeach ()
    add_test(NAME ${name}_ COMMAND ${name} WORKING_DIRECTORY $<TARGET_FILE_DIR:${name})
endmacro ()

make_test(lexer_test_adam lexer_test.cpp)
target_compile_definitions(
    lexer_test_adam
    PRIVATE
    -DADAM_TEST=1
)

make_test(lexer_test_eve lexer_test.cpp)
target_compile_definitions(
    lexer_test_eve
    PRIVATE
    -DADAM_TEST=0
)

make_test(expr_parser_test_adam expr_parser_test.cpp)
target_compile_definitions(
    expr_parser_test_adam
    PRIVATE
    -DADAM_TEST=1
)

make_test(expr_parser_test_eve expr_parser_test.cpp)
target_compile_definitions(
    expr_parser_test_eve
    PRIVATE
    -DADAM_TEST=0
)

make_test(adam_parser_test adam_parser_test.cpp)

add_test_and_data(lexer_test_adam test_expressions adam_test_expressions_tokens)
add_test_and_data(lexer_test_eve test_expressions eve_test_expressions_tokens)
add_test_and_data(expr_parser_test_adam test_expressions)
add_test_and_data(expr_parser_test_eve test_expressions)
add_test_and_data(adam_parser_test examples)
