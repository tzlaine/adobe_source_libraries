macro (make_test name)
    set(sources ${ARGN})
    if (NOT sources)
        set(sources ${name}.cpp)
    endif ()
    add_executable (${name} ${sources})
    target_link_libraries(${name} PRIVATE asl double-conversion)
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
                "${CMAKE_CURRENT_BINARY_DIR}/${arg}"
            )
        else ()
            get_filename_component(filename_without_path ${arg} NAME)
            add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${filename_without_path}"
                ${CMAKE_CURRENT_BINARY_DIR}
            )
        endif ()
    endforeach ()
    add_test(NAME ${name}_ COMMAND ${name})
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

make_test(adam_parser_test)
make_test(eve_parser_test)
make_test(func_parser_test)
make_test(func_test)
make_test(builtin_functions_test)

add_test_and_data(lexer_test_adam test_expressions adam_test_expressions_tokens)
add_test_and_data(lexer_test_eve test_expressions eve_test_expressions_tokens)
add_test_and_data(expr_parser_test_adam test_expressions)
add_test_and_data(expr_parser_test_eve test_expressions)
add_test_and_data(adam_parser_test examples)
add_test_and_data(eve_parser_test examples)
add_test_and_data(func_parser_test function_parser_test.fn)
add_test_and_data(func_test function_parser_test.fn)
add_test_and_data(builtin_functions_test function_parser_test.fn)
