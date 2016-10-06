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

any_regular_t increment(dictionary_t const & parameters)
{
    double retval;
    get_value(parameters, "value"_name, retval);
    ++retval;
    return any_regular_t(retval);
}

any_regular_t push_back(dictionary_t const & parameters)
{
    any_regular_t state;
    get_value(parameters, "state"_name, state);
    array_t& array = state.cast<array_t>();
    any_regular_t value;
    get_value(parameters, "value"_name, value);
    array.push_back(value);
    return state;
}

any_regular_t push_back_key(dictionary_t const & parameters)
{
    any_regular_t state;
    get_value(parameters, "state"_name, state);
    array_t& array = state.cast<array_t>();
    any_regular_t key;
    get_value(parameters, "key"_name, key);
    array.push_back(key);
    return state;
}

struct test_t
{
    test_t() {}
    test_t(char const* expression,
           any_regular_t const & expected_result,
           bool expect_exception = false) :
        expression_m(expression),
        expected_result_m(expected_result),
        expect_exception_m(expect_exception)
        {}
    std::string expression_m;
    any_regular_t expected_result_m;
    bool expect_exception_m;
};

const std::vector<test_t>& tests()
{
    static std::vector<test_t> retval;
    if (retval.empty()) {
        {
            dictionary_t result;
            result["one"_name] = any_regular_t(2.0);
            result["two"_name] = any_regular_t(3.0);
            retval.push_back(test_t("transform({one: 1, two: 2}, @increment)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(4.0));
            result.push_back(any_regular_t(5.0));
            retval.push_back(test_t("transform([3, 4], @increment)", any_regular_t(result)));
        }
        retval.push_back(test_t("transform(5, @increment)", any_regular_t(6.0)));

        {
            array_t result;
            result.push_back(any_regular_t("one"_name));
            result.push_back(any_regular_t("two"_name));
            result.push_back(any_regular_t("three"_name));
            retval.push_back(test_t("fold({one: 1, two: 2, three: 3}, [], @push_back_key)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(4.0));
            result.push_back(any_regular_t(5.0));
            result.push_back(any_regular_t(6.0));
            retval.push_back(test_t("fold([4, 5, 6], [], @push_back)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(7.0));
            retval.push_back(test_t("fold(7, [], @push_back)", any_regular_t(result)));
        }

        {
            array_t result;
            result.push_back(any_regular_t(3.0));
            result.push_back(any_regular_t(2.0));
            result.push_back(any_regular_t(1.0));
            retval.push_back(test_t("foldr({one: 1, two: 2, three: 3}, [], @push_back)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(6.0));
            result.push_back(any_regular_t(5.0));
            result.push_back(any_regular_t(4.0));
            retval.push_back(test_t("foldr([4, 5, 6], [], @push_back)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(7.0));
            retval.push_back(test_t("foldr(7, [], @push_back)", any_regular_t(result)));
        }

        retval.push_back(test_t("append()", any_regular_t(), true));
        {
            array_t result;
            retval.push_back(test_t("append([])", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(7.0));
            retval.push_back(test_t("append([], 7)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(7.0));
            result.push_back(any_regular_t(std::string("8")));
            retval.push_back(test_t("append([7], '8')", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(7.0));
            result.push_back(any_regular_t(std::string("8")));
            result.push_back(any_regular_t("nine"_name));
            retval.push_back(test_t("append([7], '8', @nine)", any_regular_t(result)));
        }

        retval.push_back(test_t("prepend()", any_regular_t(), true));
        {
            array_t result;
            retval.push_back(test_t("prepend([])", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(7.0));
            retval.push_back(test_t("prepend([], 7)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(7.0));
            result.push_back(any_regular_t(std::string("8")));
            retval.push_back(test_t("prepend(['8'], 7)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(7.0));
            result.push_back(any_regular_t(std::string("8")));
            result.push_back(any_regular_t("nine"_name));
            retval.push_back(test_t("prepend([@nine], 7, '8')", any_regular_t(result)));
        }

        retval.push_back(test_t("insert()", any_regular_t(), true));

        // insert() on arrays
        retval.push_back(test_t("insert([])", any_regular_t(), true));
        retval.push_back(test_t("insert([], 'foo')", any_regular_t(), true));
        retval.push_back(test_t("insert([], 1)", any_regular_t(), true));
        retval.push_back(test_t("insert([], -1)", any_regular_t(), true));
        {
            array_t result;
            retval.push_back(test_t("insert([], 0)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(std::string("one")));
            retval.push_back(test_t("insert([], 0, 'one')", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(std::string("one")));
            result.push_back(any_regular_t(2));
            retval.push_back(test_t("insert([], 0, 'one', 2)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(std::string("one")));
            retval.push_back(test_t("insert(['one'], 0)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(std::string("one")));
            result.push_back(any_regular_t(2));
            retval.push_back(test_t("insert([2], 0, 'one')", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(std::string("one")));
            result.push_back(any_regular_t(2));
            result.push_back(any_regular_t("three"_name));
            retval.push_back(test_t("insert([@three], 0, 'one', 2)", any_regular_t(result)));
        }

        // insert() on dictionaries
        retval.push_back(test_t("insert({})", any_regular_t(), true));
        retval.push_back(test_t("insert({}, @foo)", any_regular_t(), true));
        retval.push_back(test_t("insert({}, [1])", any_regular_t(), true));
        {
            dictionary_t result;
            result["foo"_name] = any_regular_t(std::string("bar"));
            retval.push_back(test_t("insert({}, @foo, 'bar')", any_regular_t(result)));
        }
        {
            dictionary_t result;
            result["foo"_name] = any_regular_t(std::string("baz"));
            retval.push_back(test_t("insert({foo: 'bar'}, @foo, 'baz')", any_regular_t(result)));
        }
        {
            dictionary_t result;
            result["foo"_name] = any_regular_t(std::string("bar"));
            retval.push_back(test_t("insert({foo: 'bar'}, {})", any_regular_t(result)));
        }
        {
            dictionary_t result;
            result["foo"_name] = any_regular_t(std::string("bar"));
            result["baz"_name] = any_regular_t(1);
            retval.push_back(test_t("insert({foo: 'bar'}, {baz: 1})", any_regular_t(result)));
        }

        retval.push_back(test_t("erase()", any_regular_t(), true));

        // erase() on arrays
        retval.push_back(test_t("erase([])", any_regular_t(), true));
        retval.push_back(test_t("erase([], 'foo')", any_regular_t(), true));
        retval.push_back(test_t("erase([], 1)", any_regular_t(), true));
        retval.push_back(test_t("erase([], 0)", any_regular_t(), true));
        retval.push_back(test_t("erase([], -1)", any_regular_t(), true));
        {
            array_t result;
            retval.push_back(test_t("erase([@foo], 0)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(2));
            result.push_back(any_regular_t("three"_name));
            retval.push_back(test_t("erase(['one', 2, @three], 0)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(std::string("one")));
            result.push_back(any_regular_t("three"_name));
            retval.push_back(test_t("erase(['one', 2, @three], 1)", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(std::string("one")));
            result.push_back(any_regular_t(2));
            retval.push_back(test_t("erase(['one', 2, @three], 2)", any_regular_t(result)));
        }

        // erase() on dictionaries
        retval.push_back(test_t("erase({})", any_regular_t(), true));
        retval.push_back(test_t("erase({}, [1])", any_regular_t(), true));
        retval.push_back(test_t("erase({foo: 'bar'}, @foo, 1)", any_regular_t(), true));
        {
            dictionary_t result;
            retval.push_back(test_t("erase({one: 2, two: 3}, @one, @two)", any_regular_t(result)));
        }

#if 0 // TODO eval() is currently broken.
        retval.push_back(test_t("parse()", any_regular_t(), true));
        retval.push_back(test_t("parse(@one)", any_regular_t(), true));
        {
            array_t result;
            result.push_back(any_regular_t("one"_name));
            result.push_back(any_regular_t(2));
            result.push_back(any_regular_t(2));
            result.push_back(any_regular_t(".array"_name));
            retval.push_back(test_t("parse('[@one, 2]')", any_regular_t(result)));
        }
        retval.push_back(test_t("parse('')", any_regular_t(array_t())));
        retval.push_back(test_t("parse('[')", any_regular_t(array_t())));

        retval.push_back(test_t("eval()", any_regular_t(), true));
        retval.push_back(test_t("eval(@one)", any_regular_t(), true));
        {
            array_t result;
            result.push_back(any_regular_t("one"_name));
            result.push_back(any_regular_t(2));
            retval.push_back(test_t("eval(parse('[@one, 2]'))", any_regular_t(result)));
        }
#endif

        retval.push_back(test_t("size()", any_regular_t(), true));
        retval.push_back(test_t("size(1)", any_regular_t(), true));
        retval.push_back(test_t("size(@one)", any_regular_t(), true));
        retval.push_back(test_t("size([])", any_regular_t(0)));
        retval.push_back(test_t("size([1, @two])", any_regular_t(2)));
        retval.push_back(test_t("size({})", any_regular_t(0)));
        retval.push_back(test_t("size({one: 1, two: @two, three: '3'})", any_regular_t(3)));

        retval.push_back(test_t("split()", any_regular_t(), true));
        retval.push_back(test_t("split('a', 'b', 'c')", any_regular_t(), true));
        retval.push_back(test_t("split(1, 'a')", any_regular_t(), true));
        retval.push_back(test_t("split('a', 1)", any_regular_t(), true));
        retval.push_back(test_t("split('a', '')", any_regular_t(), true));
        {
            array_t result;
            result.push_back(any_regular_t(std::string("")));
            retval.push_back(test_t("split('', '\n')", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(std::string("foo")));
            result.push_back(any_regular_t(std::string("bar")));
            retval.push_back(test_t("split('foo\nbar', '\n')", any_regular_t(result)));
        }
        {
            array_t result;
            result.push_back(any_regular_t(std::string("foo")));
            result.push_back(any_regular_t(std::string("")));
            result.push_back(any_regular_t(std::string("bar")));
            result.push_back(any_regular_t(std::string("")));
            retval.push_back(test_t("split('foo\n\nbar\n', '\n')", any_regular_t(result)));
        }

        retval.push_back(test_t("join()", any_regular_t(), true));
        retval.push_back(test_t("join(1)", any_regular_t(), true));
        retval.push_back(test_t("join(['a'], 1)", any_regular_t(), true));
        retval.push_back(test_t("join(['a', 1])", any_regular_t(), true));
        retval.push_back(test_t("join(['a', 'b'])", any_regular_t(std::string("ab"))));
        retval.push_back(test_t("join(['a', 'b'], ' ')", any_regular_t(std::string("a b"))));
        retval.push_back(test_t("join(split('foo\n\nbar\n', '\n'), '\n')", any_regular_t(std::string("foo\n\nbar\n"))));

        retval.push_back(test_t("assert()", any_regular_t(), true));
        retval.push_back(test_t("assert(true)", any_regular_t(), true));
        retval.push_back(test_t("assert(true, false)", any_regular_t(), true));
        retval.push_back(test_t("assert(true, 'message')", any_regular_t()));
        retval.push_back(test_t("assert(false, 'message')", any_regular_t(), true));
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

    spirit2::dictionary_function_map_t dictionary_function_map;
    dictionary_function_map["increment"_name] = &::increment;
    dictionary_function_map["push_back"_name] = &::push_back;
    dictionary_function_map["push_back_key"_name] = &::push_back_key;
    virtual_machine_t::dictionary_function_lookup_t dictionary_lookup =
        [&](name_t name, dictionary_t const & arguments) -> any_regular_t {
            auto const it = dictionary_function_map.find(name);
            if (it == dictionary_function_map.end())
                throw std::runtime_error(make_string("DFunction ", name.c_str(), " not found."));
            else
                return it->second(arguments);
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
    BOOST_CHECK(result == test.expected_result_m);

    if (result != test.expected_result_m) {
        std::cout << "result(=" << result.type_info()
                  << ") is not test.expected_result_m(=" << test.expected_result_m.type_info()
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
