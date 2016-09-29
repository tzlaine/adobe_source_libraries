#include <adobe/spirit2/expr_parser.hpp>

#include <adobe/adam_parser.hpp>
#include <adobe/algorithm/lower_bound.hpp>
#include <adobe/algorithm/sort.hpp>
#include <adobe/implementation/adam_parser_impl.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "spirit2_test_utils.hpp"


char const * const g_input_file = "test_expressions";


namespace {
    using namespace adobe;
    static_name_t const input_k      = "input"_name;
    static_name_t const output_k     = "output"_name;
    static_name_t const interface_k  = "interface"_name;
    static_name_t const logic_k      = "logic"_name;
    static_name_t const constant_k   = "constant"_name;
    static_name_t const invariant_k  = "invariant"_name;
    static_name_t const sheet_k      = "sheet"_name;
    static_name_t const unlink_k     = "unlink"_name;
    static_name_t const when_k       = "when"_name;
    static_name_t const relate_k     = "relate"_name;
    static_name_t const layout_k     = "layout"_name;
    static_name_t const view_k       = "view"_name;
}

namespace adobe {

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
    static const adobe::name_t s_keywords[] = {
        input_k,
        output_k,
        interface_k,
        logic_k,
        constant_k,
        invariant_k,
        sheet_k,
        unlink_k,
        when_k,
        relate_k
    };
    const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

    adobe::spirit2::lexer_t tok(s_keywords, s_keywords + s_num_keywords);

    using boost::spirit::qi::token;
    using boost::spirit::qi::_1;
    using boost::spirit::qi::_val;

    assert(tok.keywords.size() == 10u);
    const boost::spirit::lex::token_def<adobe::name_t>& input = tok.keywords[input_k];
    const boost::spirit::lex::token_def<adobe::name_t>& output = tok.keywords[output_k];
    const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
    const boost::spirit::lex::token_def<adobe::name_t>& logic = tok.keywords[logic_k];
    const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
    const boost::spirit::lex::token_def<adobe::name_t>& invariant = tok.keywords[invariant_k];
    const boost::spirit::lex::token_def<adobe::name_t>& sheet = tok.keywords[sheet_k];
    const boost::spirit::lex::token_def<adobe::name_t>& unlink = tok.keywords[unlink_k];
    const boost::spirit::lex::token_def<adobe::name_t>& when = tok.keywords[when_k];
    const boost::spirit::lex::token_def<adobe::name_t>& relate = tok.keywords[relate_k];
    assert(tok.keywords.size() == 10u);

    static adobe::spirit2::expression_parser_rules_t::keyword_rule_t adam_keywords =
        input[_val = _1]
        | output[_val = _1]
        | interface[_val = _1]
        | logic[_val = _1]
        | constant[_val = _1]
        | invariant[_val = _1]
        | sheet[_val = _1]
        | unlink[_val = _1]
        | when[_val = _1]
        | relate[_val = _1]
        ;
    adam_keywords.name("keyword");

    adobe::array_t stack;
    adobe::spirit2::expression_parser_rules_t parser_rules(tok, adam_keywords);

    std::string expressions_file_contents = read_file(g_input_file);
    std::vector<std::string> expressions;
    using boost::algorithm::split;
    using boost::algorithm::is_any_of;
    split(expressions, expressions_file_contents, is_any_of("\n"));

    std::size_t passes = 0;
    std::size_t failures = 0;
    for (std::size_t i = 0; i < expressions.size(); ++i) {
        if (!expressions[i].empty()) {
            bool success =
                test_expression(tok, parser_rules, stack, &adobe::parse_adam_expression, expressions[i]);
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
    static const adobe::name_t s_keywords[] = {
        interface_k,
        constant_k,
        layout_k,
        view_k
    };
    const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

    adobe::spirit2::lexer_t tok(s_keywords, s_keywords + s_num_keywords);

    using boost::spirit::qi::token;
    using boost::spirit::qi::_1;
    using boost::spirit::qi::_val;

    assert(tok.keywords.size() == 4u);
    const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
    const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
    const boost::spirit::lex::token_def<adobe::name_t>& layout = tok.keywords[layout_k];
    const boost::spirit::lex::token_def<adobe::name_t>& view = tok.keywords[view_k];
    assert(tok.keywords.size() == 4u);

    static adobe::spirit2::expression_parser_rules_t::keyword_rule_t eve_keywords =
        interface[_val = _1]
        | constant[_val = _1]
        | layout[_val = _1]
        | view[_val = _1]
        ;
    eve_keywords.name("keyword");

    adobe::array_t stack;
    adobe::spirit2::expression_parser_rules_t parser_rules(tok, eve_keywords);

    std::string expressions_file_contents = read_file(g_input_file);
    std::vector<std::string> expressions;
    using boost::algorithm::split;
    using boost::algorithm::is_any_of;
    split(expressions, expressions_file_contents, is_any_of("\n"));

    std::size_t passes = 0;
    std::size_t failures = 0;
    for (std::size_t i = 0; i < expressions.size(); ++i) {
        if (!expressions[i].empty()) {
            bool success =
                test_expression(tok, parser_rules, stack, &adobe::parse_eve_expression, expressions[i]);
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
