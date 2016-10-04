#include <adobe/spirit2/func_parser.hpp>
#include <adobe/spirit2/adam_parser.hpp>
#include <adobe/spirit2/builtin_functions.hpp>
#include <adobe/adam.hpp>
#include <adobe/adam_evaluate.hpp>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

#include <fstream>
#include <iostream>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "spirit2_test_utils.hpp"


const char* g_input_file = "function_parser_test.fn";

using namespace adobe;

struct test_t
{
    test_t() {}
    test_t(const char* expression,
           const any_regular_t& expected_result,
           bool expect_exception = false) :
        expression_m(expression),
        expected_result_m(expected_result),
        expect_exception_m(expect_exception)
        {}
    test_t(const char* expression,
           const any_regular_t& expected_result,
           const any_regular_t& alternate_expected_result,
           bool expect_exception) :
        expression_m(expression),
        expected_result_m(expected_result),
        alternate_expected_result_m(alternate_expected_result),
        expect_exception_m(expect_exception)
        {}
    std::string expression_m;
    any_regular_t expected_result_m;
    any_regular_t alternate_expected_result_m;
    bool expect_exception_m;
};

const std::vector<test_t>& tests()
{
    static std::vector<test_t> retval;
    if (retval.empty()) {
        retval.push_back(test_t("empty_fn()", any_regular_t()));
        retval.push_back(test_t("simple_number_fn()", any_regular_t(1)));
        retval.push_back(test_t("decl_fn()", any_regular_t("none"_name)));
        retval.push_back(test_t("const_decl_fn()", any_regular_t("none"_name)));
        retval.push_back(test_t("shadowed_param_1_fn()", any_regular_t()));
        retval.push_back(test_t("shadowed_param_1_fn(empty)", any_regular_t()));
        retval.push_back(test_t("shadowed_param_1_fn(1)", any_regular_t(1)));
        retval.push_back(test_t("shadowed_param_2_fn()", any_regular_t(array_t())));
        retval.push_back(test_t("shadowed_param_3_fn()", any_regular_t(array_t())));
        retval.push_back(test_t("assignment_fn()", any_regular_t(dictionary_t())));
        retval.push_back(test_t("simple_if_1_fn(true)", any_regular_t(1)));
        retval.push_back(test_t("simple_if_1_fn(false)", any_regular_t()));
        retval.push_back(test_t("simple_if_2_fn(true)", any_regular_t(1)));
        retval.push_back(test_t("simple_if_2_fn(false)", any_regular_t(0)));
        retval.push_back(test_t("nested_ifs_fn(true, true, true)", any_regular_t(0)));
        retval.push_back(test_t("nested_ifs_fn(true, true, false)", any_regular_t(1)));
        retval.push_back(test_t("nested_ifs_fn(true, false, true)", any_regular_t(2)));
        retval.push_back(test_t("nested_ifs_fn(true, false, false)", any_regular_t(3)));
        retval.push_back(test_t("nested_ifs_fn(false, true, true)", any_regular_t(4)));
        retval.push_back(test_t("nested_ifs_fn(false, true, false)", any_regular_t(5)));
        retval.push_back(test_t("nested_ifs_fn(false, false, true)", any_regular_t(6)));
        retval.push_back(test_t("nested_ifs_fn(false, false, false)", any_regular_t(7)));
        retval.push_back(test_t("chained_ifs_fn(true, true, true)", any_regular_t(0)));
        retval.push_back(test_t("chained_ifs_fn(false, true, true)", any_regular_t(1)));
        retval.push_back(test_t("chained_ifs_fn(false, false, true)", any_regular_t(2)));
        retval.push_back(test_t("chained_ifs_fn(false, false, false)", any_regular_t(3)));

        retval.push_back(test_t("scoped_decl_if_test_1_fn()", any_regular_t(), true));
        retval.push_back(test_t("scoped_decl_if_test_2_fn()", any_regular_t(), true));
        {
            array_t result;
            result.push_back(any_regular_t(true));
            result.push_back(any_regular_t(true));
            retval.push_back(test_t("scoped_decl_if_test_3_fn()", any_regular_t(result)));
        }

        retval.push_back(test_t("scoped_decl_simple_for_test_1_fn()", any_regular_t(), true));
        {
            array_t result;
            result.push_back(any_regular_t(0));
            result.push_back(any_regular_t(1));
            result.push_back(any_regular_t(0));
            result.push_back(any_regular_t(1));

            retval.push_back(test_t("scoped_decl_simple_for_test_2_fn()", any_regular_t(result)));

            {
                array_t alternate_result;
                alternate_result.push_back(any_regular_t(1));
                alternate_result.push_back(any_regular_t(0));
                alternate_result.push_back(any_regular_t(1));
                alternate_result.push_back(any_regular_t(0));
                retval.push_back(test_t("scoped_decl_simple_for_test_3_fn()", any_regular_t(result), any_regular_t(alternate_result), false));
            }

            retval.push_back(test_t("scoped_decl_complex_for_test_1_fn()", any_regular_t(result)));
        }

        retval.push_back(test_t("scoped_decl_simple_for_test_4_fn()", any_regular_t(3)));
        retval.push_back(test_t("scoped_decl_simple_for_test_5_fn()", any_regular_t(3)));
        retval.push_back(test_t("scoped_decl_complex_for_test_2_fn()", any_regular_t(3)));

        retval.push_back(test_t("slow_size([])", any_regular_t(0)));
        retval.push_back(test_t("slow_size([0])", any_regular_t(1)));
        retval.push_back(test_t("slow_size([0, @two])", any_regular_t(2)));
        retval.push_back(test_t("slow_size({})", any_regular_t(0)));
        retval.push_back(test_t("slow_size({one: 0})", any_regular_t(1)));
        retval.push_back(test_t("slow_size({one: 0, two: @two})", any_regular_t(2)));

        retval.push_back(test_t("simple_for_1({})", any_regular_t(true)));
        retval.push_back(test_t("simple_for_1({one: 0})", any_regular_t(true)));
        retval.push_back(test_t("simple_for_1({one: 0, two: @two})", any_regular_t(true)));

        retval.push_back(test_t("simple_for_2({})", any_regular_t(true)));
        retval.push_back(test_t("simple_for_2({one: 0})", any_regular_t(true)));
        retval.push_back(test_t("simple_for_2({one: 0, two: @two})", any_regular_t(true)));

        retval.push_back(test_t("complex_for_1([])", any_regular_t(true)));
        retval.push_back(test_t("complex_for_1([0])", any_regular_t(true)));
        retval.push_back(test_t("complex_for_1([0, @two])", any_regular_t(true)));

        retval.push_back(test_t("complex_for_2([])", any_regular_t(array_t())));
        retval.push_back(test_t("complex_for_2([0])", any_regular_t(array_t())));

        {
            array_t result;
            result.push_back(any_regular_t(0));
            result.push_back(any_regular_t("two"_name));
            retval.push_back(test_t("complex_for_2([0, @two])", any_regular_t(result)));
            result.push_back(any_regular_t("two"_name));
            result.push_back(any_regular_t(3));
            result.push_back(any_regular_t(3));
            result.push_back(any_regular_t(std::string("4")));
            retval.push_back(test_t("complex_for_2([0, @two, 3, '4'])", any_regular_t(result)));
        }

        {
            array_t result;
            array_t element(2);
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(1);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(2);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(1);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(1);
            element[1] = any_regular_t(1);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(1);
            element[1] = any_regular_t(2);
            result.push_back(any_regular_t(element));
            retval.push_back(test_t("complex_for_3(2, 3)", any_regular_t(result)));
        }

        {
            array_t result;
            array_t element(2);
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(1);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(1);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(1);
            element[1] = any_regular_t(1);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(2);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(2);
            element[1] = any_regular_t(1);
            result.push_back(any_regular_t(element));
            retval.push_back(test_t("complex_for_3(3, 2)", any_regular_t(result)));
        }

        {
            array_t result;
            array_t element(2);
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(1);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t("two"_name);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t("two"_name);
            element[1] = any_regular_t(1);
            result.push_back(any_regular_t(element));
            retval.push_back(test_t("complex_for_4({one: 0, two: @two}, 2)", any_regular_t(result)));
        }

        retval.push_back(test_t("simple_for_return_test_1([0])", any_regular_t(3)));
        retval.push_back(test_t("simple_for_return_test_2({one: 0})", any_regular_t(3)));
        retval.push_back(test_t("complex_for_return_test_1([0])", any_regular_t(3)));
        retval.push_back(test_t("complex_for_return_test_2(2, 4)", any_regular_t(3)));

        retval.push_back(test_t("simple_for_continue_test_1({one: 0, two: @two}, @one)", any_regular_t(dictionary_t())));
        {
            dictionary_t result;
            result["two"_name] = any_regular_t("two"_name);
            retval.push_back(test_t("simple_for_continue_test_2({one: 0, two: @two}, @one)", any_regular_t(result)));
            retval.push_back(test_t("simple_for_continue_test_3({one: 0, two: @two}, @one)", any_regular_t(result)));
        }
        {
            dictionary_t result;
            result["one"_name] = any_regular_t(0);
            retval.push_back(test_t("simple_for_continue_test_2({one: 0, two: @two}, @two)", any_regular_t(result)));
            retval.push_back(test_t("simple_for_continue_test_3({one: 0, two: @two}, @two)", any_regular_t(result)));
        }

        {
            array_t result;
            result.push_back(any_regular_t(1));
            retval.push_back(test_t("complex_for_continue_test_1([0, 1], 0)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(0));
            retval.push_back(test_t("complex_for_continue_test_1([0, 1], 1)", any_regular_t(result)));
        }

        {
            array_t result;
            array_t element(2);
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(2);
            result.push_back(any_regular_t(element));
            retval.push_back(test_t("complex_for_continue_test_2(2, 3, 1, 1)", any_regular_t(result)));
        }
        {
            array_t result;
            array_t element(2);
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(2);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            retval.push_back(test_t("complex_for_continue_test_2(3, 2, 1, 1)", any_regular_t(result)));
        }

        retval.push_back(test_t("simple_for_break_test_1({one: 0, two: @two})", any_regular_t(dictionary_t())));
        {
            dictionary_t result;
            result["one"_name] = any_regular_t(0);
            retval.push_back(test_t("simple_for_break_test_2({one: 0, two: @two})", any_regular_t(result)));
        }

        retval.push_back(test_t("simple_for_break_test_3({one: 0, two: @two}, @one)", any_regular_t(dictionary_t())));
        retval.push_back(test_t("simple_for_break_test_4({one: 0, two: @two}, @one)", any_regular_t(dictionary_t())));
        {
            dictionary_t result;
            result["one"_name] = any_regular_t(0);
            retval.push_back(test_t("simple_for_break_test_3({one: 0, two: @two}, @two)", any_regular_t(result)));
            retval.push_back(test_t("simple_for_break_test_4({one: 0, two: @two}, @two)", any_regular_t(result)));
        }

        retval.push_back(test_t("complex_for_break_test_1([0, 1], 0)", any_regular_t(array_t())));
        {
            array_t result;
            result.push_back(any_regular_t(0));
            retval.push_back(test_t("complex_for_break_test_1([0, 1], 1)", any_regular_t(result)));
        }

        {
            array_t result;
            array_t element(2);
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            retval.push_back(test_t("complex_for_break_test_2(2, 3, 1, 1)", any_regular_t(result)));
        }
        {
            array_t result;
            array_t element(2);
            element[0] = any_regular_t(0);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            element[0] = any_regular_t(1);
            element[1] = any_regular_t(0);
            result.push_back(any_regular_t(element));
            retval.push_back(test_t("complex_for_break_test_2(3, 2, 2, 1)", any_regular_t(result)));
        }

        auto const foo = "foo"_name;
        {
            dictionary_t result;
            result[foo] = any_regular_t(std::string("bar"));
            retval.push_back(test_t("lvalue_assignment_test_1({foo: 0})", any_regular_t(result)));
            retval.push_back(test_t("lvalue_assignment_test_2({foo: 0})", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(0));
            result.push_back(any_regular_t(std::string("bar")));
            retval.push_back(test_t("lvalue_assignment_test_3([0, 0])", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(0));
            dictionary_t dict;
            dict[foo] = any_regular_t(std::string("bar"));
            result.push_back(any_regular_t(dict));
            retval.push_back(test_t("lvalue_assignment_test_4([0, {foo: 0}])", any_regular_t(result)));
            retval.push_back(test_t("lvalue_assignment_test_5([0, {foo: 0}])", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(0));
            array_t array;
            array.push_back(any_regular_t(0));
            array.push_back(any_regular_t(std::string("bar")));
            result.push_back(any_regular_t(array));
            retval.push_back(test_t("lvalue_assignment_test_6([0, [0, 0]])", any_regular_t(result)));
        }
        {
            dictionary_t result;
            dictionary_t dict;
            dict[foo] = any_regular_t(std::string("bar"));
            result[foo] = any_regular_t(dict);
            retval.push_back(test_t("lvalue_assignment_test_7({foo: {foo: 0}})", any_regular_t(result)));
        }
        {
            dictionary_t result;
            array_t array;
            array.push_back(any_regular_t(0));
            array.push_back(any_regular_t(std::string("bar")));
            result[foo] = any_regular_t(array);
            retval.push_back(test_t("lvalue_assignment_test_8({foo: [0, 0]})", any_regular_t(result)));
            retval.push_back(test_t("lvalue_assignment_test_9({foo: [0, 0]})", any_regular_t(result)));
        }
    }
    return retval;
}

void run_test(test_t const & test, adam_function_map_t & functions)
{
    std::string adam_str;
    adam_str += "sheet function_test_sheet {\n";
    adam_str += "    output: result <== { value: " + test.expression_m + " };\n";
    adam_str += "}";
    sheet_t sheet;

    std::cout << "\n" << test.expression_m << "\n";

    bool parsed = true;
    try {
        parsed = spirit2::parse(adam_str, test.expression_m.c_str(), bind_to_sheet(sheet));
    } catch (stream_error_t const & e) {
        parsed = false;
        std::cout << "<parse failure>\n"
                  << test.expression_m << "\n"
                  << "FAIL\n";
    }
    BOOST_CHECK(parsed);
    if (!parsed)
        return;

    sheet.machine_m.set_variable_lookup(
        [&](name_t name) {
            return sheet.get(name);
        }
    );

    virtual_machine_t::adam_function_lookup_t adam_lookup =
        [&](name_t name) -> boost::optional<adam_function_t const &> {
            auto const it = functions.find(name);
            if (it == functions.end())
                return boost::optional<adam_function_t const &>();
            else
                return it->second;
        };
    sheet.machine_m.set_adam_function_lookup(adam_lookup);

    spirit2::array_function_map_t array_function_map;

    virtual_machine_t::array_function_lookup_t array_lookup =
        [&](name_t name, array_t const & arguments) {
            auto const it = array_function_map.find(name);
            if (it == array_function_map.end())
                throw std::runtime_error(make_string("AFunction ", name.c_str(), " not found."));
            else
                return it->second(arguments);
        };
    sheet.machine_m.set_array_function_lookup(array_lookup);

    virtual_machine_t::dictionary_function_lookup_t dictionary_lookup =
        [&](name_t name, dictionary_t const &) -> any_regular_t {
            throw std::runtime_error(make_string("DFunction ", name.c_str(), " not found."));
        };
    sheet.machine_m.set_dictionary_function_lookup(dictionary_lookup);

    spirit2::add_predefined_functions(
        array_function_map,
        array_lookup,
        dictionary_lookup,
        adam_lookup,
        sheet
    );

    try {
        sheet.update();
    } catch (std::exception const & e) {
        if (!test.expect_exception_m)
            std::cout << "Exception! what(): '" << e.what() << "'\n";
        BOOST_CHECK(test.expect_exception_m == true);
        return;
    }

    BOOST_CHECK(test.expect_exception_m == false);

    auto sheet_result = sheet.inspect(parse_adam_expression(std::string("result")));
    auto const result = sheet_result.cast<dictionary_t>()["value"_name];
    BOOST_CHECK(result == test.expected_result_m || result == test.alternate_expected_result_m);

    if (result != test.expected_result_m && result != test.alternate_expected_result_m) {
        std::cout << "result(=" << result.cast<dictionary_t>()
                  << ") is not test.expected_result_m(=" << test.expected_result_m.type_info().name()//cast<dictionary_t>()
                  << ") or test.alternate_expected_result_m(=" << test.alternate_expected_result_m.type_info().name()//cast<array_t>()
                  <<  ")\n";
    }
}


BOOST_AUTO_TEST_CASE( adam_functions )
{
    std::string const file_contents = read_file(g_input_file);

    adam_function_map_t functions;
    spirit2::parse_functions(file_contents, g_input_file, functions);

    for (auto const & test : tests()) {
        run_test(test, functions);
    }
}
