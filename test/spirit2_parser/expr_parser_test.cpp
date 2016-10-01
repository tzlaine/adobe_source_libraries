#include <adobe/spirit2/expr_parser.hpp>
#include <adobe/spirit2/adam_parser.hpp>
#include <adobe/spirit2/eve_parser.hpp>

#include <adobe/adam_parser.hpp>
#include <adobe/algorithm/lower_bound.hpp>
#include <adobe/algorithm/sort.hpp>
#include <adobe/implementation/adam_parser_impl.hpp>
#include <adobe/implementation/token.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "spirit2_test_utils.hpp"


char const * const g_input_file = "test_expressions";


namespace adobe {

    static_name_t const layout_k = "layout"_name;
    static_name_t const view_k = "view"_name;
    static_name_t const interface_k = "interface"_name;
    static_name_t const constant_k = "constant"_name;

    bool eve_keyword_lookup(const name_t& name)
    {
        typedef boost::array<name_t, 4> keyword_table_t;

        keyword_table_t keyword_table =
        {{
            interface_k,
            constant_k,
            layout_k,
            view_k
        }};

        sort(keyword_table);

        keyword_table_t::const_iterator iter(lower_bound(keyword_table, name));

        return (iter != keyword_table.end() && *iter == name);
    }

    array_t parse_eve_expression(const std::string& str_expression)
    {
        std::stringstream expression_stream(str_expression);

        expression_parser parser(expression_stream, line_position_t("expression"));
        parser.set_keyword_extension_lookup(boost::bind(&eve_keyword_lookup, _1));

        array_t expression;
        parser.require_expression(expression);

        return expression;
    }

}

typedef adobe::array_t (*adobe_parser_t)(const std::string&);

namespace adobe { namespace spirit2 {

    bool test_expression(const lexer_t& lexer,
                         const expression_parser_rules_t& parser_rules,
                         array_t& new_parsed_expression,
                         adobe_parser_t adobe_parse,
                         const std::string& expression)
    {
        std::cout << "expression: \"" << expression << "\"\n";
        array_t original_parsed_expression;
        bool original_parse_failed = false;
        try {
            original_parsed_expression = adobe_parse(expression);
        } catch (const stream_error_t&) {
            original_parse_failed = true;
        }
        if (original_parse_failed)
            std::cout << "original: <parse failure>\n";
        else
            std::cout << "original: " << original_parsed_expression << "\n";
        using boost::spirit::qi::phrase_parse;
        text_iterator_t it(expression.begin());
        detail::s_text_it = &it;
        detail::s_begin = it;
        detail::s_end = text_iterator_t(expression.end());
        detail::s_filename = "test_expression";
        token_iterator_t iter = lexer.begin(it, detail::s_end);
        token_iterator_t end = lexer.end();
        bool new_parse_failed =
            !phrase_parse(iter,
                          end,
                          parser_rules.expression(boost::phoenix::ref(new_parsed_expression)),
                          boost::spirit::qi::in_state("WS")[lexer.self]);
        if (new_parse_failed)
            std::cout << "new:      <parse failure>\n";
        else
            std::cout << "new:      " << new_parsed_expression << "\n";
        bool pass =
            original_parse_failed && new_parse_failed ||
            new_parsed_expression == original_parsed_expression;
        std::cout << (pass ? "PASS" : "FAIL") << "\n";

        if (!pass) {
            std::cout << "original (verbose):\n";
            verbose_dump(original_parsed_expression);
            std::cout << "new (verbose):\n";
            verbose_dump(new_parsed_expression);
        }

        std::cout << "\n";
        new_parsed_expression.clear();

        return pass;
    }

} }

#if ADAM_TEST
BOOST_AUTO_TEST_CASE( adam_expression_parser )
{
    adobe::array_t stack;
    auto const & parser_rules = adobe::spirit2::adam_expression_parser();

    std::string expressions_file_contents = read_file(g_input_file);
    std::vector<std::string> expressions;
    using boost::algorithm::split;
    using boost::algorithm::is_any_of;
    split(expressions, expressions_file_contents, is_any_of("\n"));

    std::size_t passes = 0;
    std::size_t failures = 0;
    for (std::size_t i = 0; i < expressions.size(); ++i) {
        if (!expressions[i].empty()) {
            bool success = test_expression(
                adobe::spirit2::adam_lexer(),
                parser_rules,
                stack,
                &adobe::parse_adam_expression,
                expressions[i]
            );
            BOOST_CHECK(success);
            if (success)
                ++passes;
            else
                ++failures;
        }
    }

    std::cout << "Summary: " << passes << " passed, " << failures << " failed\n";
}
#endif

#if !ADAM_TEST
BOOST_AUTO_TEST_CASE( eve_expression_parser )
{
    adobe::array_t stack;
    auto const & parser_rules = adobe::spirit2::eve_expression_parser();

    std::string expressions_file_contents = read_file(g_input_file);
    std::vector<std::string> expressions;
    using boost::algorithm::split;
    using boost::algorithm::is_any_of;
    split(expressions, expressions_file_contents, is_any_of("\n"));

    std::size_t passes = 0;
    std::size_t failures = 0;
    for (std::size_t i = 0; i < expressions.size(); ++i) {
        if (!expressions[i].empty()) {
            bool success = test_expression(
                adobe::spirit2::eve_lexer(),
                parser_rules,
                stack,
                &adobe::parse_eve_expression,
                expressions[i]
            );
            BOOST_CHECK(success);
            if (success)
                ++passes;
            else
                ++failures;
        }
    }

    std::cout << "Summary: " << passes << " passed, " << failures << " failed\n";
}
#endif
