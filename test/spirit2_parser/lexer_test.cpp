// #define BOOST_SPIRIT_LEXERTL_DEBUG

#include <adobe/spirit2/adam_parser.hpp>
#include <adobe/spirit2/eve_parser.hpp>
#include <adobe/implementation/token.hpp>

#include <boost/spirit/home/qi/action.hpp>
#include <boost/spirit/home/qi/char.hpp>
#include <boost/spirit/home/qi/numeric.hpp>
#include <boost/spirit/home/qi/operator.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "spirit2_test_utils.hpp"


#define GENERATE_TEST_RESULTS 0

char const * const g_input_file = "test_expressions";
#if ADAM_TEST
char const * const g_expected_output_file = "adam_test_expressions_tokens";
#else
char const * const g_expected_output_file = "eve_test_expressions_tokens";
#endif

namespace adobe { namespace spirit2 {

    static_name_t const layout_k = "layout"_name;
    static_name_t const view_k = "view"_name;
    static_name_t const input_k = "input"_name;
    static_name_t const output_k = "output"_name;
    static_name_t const interface_k = "interface"_name;
    static_name_t const logic_k = "logic"_name;
    static_name_t const constant_k = "constant"_name;
    static_name_t const invariant_k = "invariant"_name;
    static_name_t const external_k = "external"_name;
    static_name_t const sheet_k = "sheet"_name;
    static_name_t const unlink_k = "unlink"_name;
    static_name_t const when_k = "when"_name;
    static_name_t const relate_k = "relate"_name;
    static_name_t const if_k = "if"_name;
    static_name_t const else_k = "else"_name;
    static_name_t const for_k = "for"_name;
    static_name_t const return_keyword_k = "return"_name;
    static_name_t const break_keyword_k = "breakd"_name;
    static_name_t const continue_keyword_k = "continue"_name;

#define DUMP_TOK(x) tok.x[print(boost::phoenix::ref(stream), val(#x" -- "), _1, val('\n'))]
#define DUMP_LIT(x) lit(x)[print(boost::phoenix::ref(stream), val("'"), val(x), val("'\n"))]
#define DUMP_UNATTRIBUTED(x) tok.x[print(boost::phoenix::ref(stream), val(#x"\n"))]
#define DUMP_KEYWORD_TOK(x) x[print(boost::phoenix::ref(stream), val("keyword -- "), _1, val('\n'))]
#define DUMP_CPP_KEYWORD_TOK(x) x##__[print(boost::phoenix::ref(stream), val("keyword -- "), _1, val('\n'))]

    struct print_t
    {
        template <typename Arg1, typename Arg2, typename Arg3 = void, typename Arg4 = void>
        struct result
        { typedef void type; };

        template < typename Arg2>
        void operator()(std::stringstream& stream, Arg2 arg2) const
            { stream << arg2; }

        template <typename Arg2, typename Arg3>
        void operator()(std::stringstream& stream, Arg2 arg2, Arg3 arg3) const
            { stream << arg2 << arg3; }

        template <typename Arg2, typename Arg3, typename Arg4>
        void operator()(std::stringstream& stream, Arg2 arg2, Arg3  arg3, Arg4 arg4) const
            { stream << arg2 << arg3 << arg4; }
    };

    const boost::phoenix::function<print_t> print;

#if ADAM_TEST
    struct adam_lexer_test_grammar_t :
        boost::spirit::qi::grammar<token_iterator_t, skipper_type_t>
    {
        using grammar_t = boost::spirit::qi::grammar<token_iterator_t, skipper_type_t>;

        adam_lexer_test_grammar_t(const lexer_t& lexer, std::stringstream& stream_) :
            grammar_t(start),
            stream(stream_)
            {
                using boost::spirit::qi::_1;
                using boost::spirit::qi::lit;
                using boost::phoenix::val;

                lexer_t & tok = const_cast<lexer_t &>(lexer);
                auto const initial_size = tok.keywords.size();
                (void)initial_size;
                const boost::spirit::lex::token_def<name_t>& input = tok.keywords[input_k];
                const boost::spirit::lex::token_def<name_t>& output = tok.keywords[output_k];
                const boost::spirit::lex::token_def<name_t>& interface = tok.keywords[interface_k];
                const boost::spirit::lex::token_def<name_t>& logic = tok.keywords[logic_k];
                const boost::spirit::lex::token_def<name_t>& constant = tok.keywords[constant_k];
                const boost::spirit::lex::token_def<name_t>& invariant = tok.keywords[invariant_k];
                const boost::spirit::lex::token_def<name_t>& sheet = tok.keywords[sheet_k];
                const boost::spirit::lex::token_def<name_t>& unlink = tok.keywords[unlink_k];
                const boost::spirit::lex::token_def<name_t>& when = tok.keywords[when_k];
                const boost::spirit::lex::token_def<name_t>& relate = tok.keywords[relate_k];
                const boost::spirit::lex::token_def<name_t>& if__ = tok.keywords[if_k];
                const boost::spirit::lex::token_def<name_t>& else__ = tok.keywords[else_k];
                const boost::spirit::lex::token_def<name_t>& for__ = tok.keywords[for_k];
                const boost::spirit::lex::token_def<name_t>& continue__ = tok.keywords[continue_keyword_k];
                const boost::spirit::lex::token_def<name_t>& break__ = tok.keywords[break_keyword_k];
                const boost::spirit::lex::token_def<name_t>& return__ = tok.keywords[return_keyword_k];
                assert(tok.keywords.size() == initial_size);

                start =
                    +(
                        DUMP_KEYWORD_TOK(input)
                      | DUMP_KEYWORD_TOK(output)
                      | DUMP_KEYWORD_TOK(interface)
                      | DUMP_KEYWORD_TOK(logic)
                      | DUMP_KEYWORD_TOK(constant)
                      | DUMP_KEYWORD_TOK(invariant)
                      | DUMP_KEYWORD_TOK(sheet)
                      | DUMP_KEYWORD_TOK(unlink)
                      | DUMP_KEYWORD_TOK(when)
                      | DUMP_KEYWORD_TOK(relate)
                      | DUMP_CPP_KEYWORD_TOK(if)
                      | DUMP_CPP_KEYWORD_TOK(else)
                      | DUMP_CPP_KEYWORD_TOK(for)
                      | DUMP_CPP_KEYWORD_TOK(continue)
                      | DUMP_CPP_KEYWORD_TOK(break)
                      | DUMP_CPP_KEYWORD_TOK(return)
                      | DUMP_TOK(identifier)
                      | DUMP_TOK(lead_comment)
                      | DUMP_TOK(trail_comment)
                      | DUMP_TOK(quoted_string)
                      | DUMP_TOK(number)
                      | DUMP_TOK(bitshift_op)
                      | DUMP_TOK(eq_op)
                      | DUMP_TOK(rel_op)
                      | DUMP_TOK(mul_op)
                      | DUMP_TOK(keyword_true_false)
                      | DUMP_UNATTRIBUTED(keyword_empty)
                      | DUMP_UNATTRIBUTED(define)
                      | DUMP_UNATTRIBUTED(or_)
                      | DUMP_UNATTRIBUTED(and_)
                      | DUMP_LIT('=')
                      | DUMP_LIT('+')
                      | DUMP_LIT('-')
                      | DUMP_LIT('|')
                      | DUMP_LIT('&')
                      | DUMP_LIT('^')
                      | DUMP_LIT('~')
                      | DUMP_LIT('!')
                      | DUMP_LIT('?')
                      | DUMP_LIT(':')
                      | DUMP_LIT('.')
                      | DUMP_LIT(',')
                      | DUMP_LIT('(')
                      | DUMP_LIT(')')
                      | DUMP_LIT('[')
                      | DUMP_LIT(']')
                      | DUMP_LIT('{')
                      | DUMP_LIT('}')
                      | DUMP_LIT('@')
                      | DUMP_LIT(';')
                    )
                    ;
            }

        boost::spirit::qi::rule<token_iterator_t, skipper_type_t> start;

        std::stringstream& stream;
    };
#endif

#if !ADAM_TEST
    struct eve_lexer_test_grammar_t :
        boost::spirit::qi::grammar<token_iterator_t, skipper_type_t>
    {
        eve_lexer_test_grammar_t(const lexer_t& lexer, std::stringstream& stream_) :
            base_type(start),
            stream(stream_)
            {
                using boost::spirit::qi::_1;
                using boost::spirit::qi::lit;
                using boost::phoenix::val;

                lexer_t & tok = const_cast<lexer_t &>(lexer);
                auto const initial_size = tok.keywords.size();
                (void)initial_size;
                const boost::spirit::lex::token_def<name_t>& interface = tok.keywords[interface_k];
                const boost::spirit::lex::token_def<name_t>& constant = tok.keywords[constant_k];
                const boost::spirit::lex::token_def<name_t>& layout = tok.keywords[layout_k];
                const boost::spirit::lex::token_def<name_t>& logic = tok.keywords[logic_k];
                const boost::spirit::lex::token_def<name_t>& relate = tok.keywords[relate_k];
                const boost::spirit::lex::token_def<name_t>& unlink = tok.keywords[unlink_k];
                const boost::spirit::lex::token_def<name_t>& view = tok.keywords[view_k];
                const boost::spirit::lex::token_def<name_t>& when = tok.keywords[when_k];
                assert(tok.keywords.size() == initial_size);

                start =
                    +(
                        DUMP_KEYWORD_TOK(interface)
                      | DUMP_KEYWORD_TOK(constant)
                      | DUMP_KEYWORD_TOK(layout)
                      | DUMP_KEYWORD_TOK(logic)
                      | DUMP_KEYWORD_TOK(relate)
                      | DUMP_KEYWORD_TOK(unlink)
                      | DUMP_KEYWORD_TOK(view)
                      | DUMP_KEYWORD_TOK(when)
                      | DUMP_TOK(identifier)
                      | DUMP_TOK(lead_comment)
                      | DUMP_TOK(trail_comment)
                      | DUMP_TOK(quoted_string)
                      | DUMP_TOK(number)
                      | DUMP_TOK(bitshift_op)
                      | DUMP_TOK(eq_op)
                      | DUMP_TOK(rel_op)
                      | DUMP_TOK(mul_op)
                      | DUMP_TOK(keyword_true_false)
                      | DUMP_UNATTRIBUTED(keyword_empty)
                      | DUMP_UNATTRIBUTED(define)
                      | DUMP_UNATTRIBUTED(or_)
                      | DUMP_UNATTRIBUTED(and_)
                      | DUMP_LIT('=')
                      | DUMP_LIT('+')
                      | DUMP_LIT('-')
                      | DUMP_LIT('|')
                      | DUMP_LIT('&')
                      | DUMP_LIT('^')
                      | DUMP_LIT('~')
                      | DUMP_LIT('!')
                      | DUMP_LIT('?')
                      | DUMP_LIT(':')
                      | DUMP_LIT('.')
                      | DUMP_LIT(',')
                      | DUMP_LIT('(')
                      | DUMP_LIT(')')
                      | DUMP_LIT('[')
                      | DUMP_LIT(']')
                      | DUMP_LIT('{')
                      | DUMP_LIT('}')
                      | DUMP_LIT('@')
                      | DUMP_LIT(';')
                    )
                    ;
            }

        boost::spirit::qi::rule<token_iterator_t, skipper_type_t> start;

        std::stringstream& stream;
    };
#endif

#undef DUMP_TOK
#undef DUMP_LIT
#undef DUMP_UNATTRIBUTED
#undef DUMP_KEYWORD_TOK

} }

#if ADAM_TEST
BOOST_AUTO_TEST_CASE( adam_lex )
{
    const adobe::spirit2::lexer_t & lexer = adobe::spirit2::adam_lexer();
    std::stringstream stream;
    adobe::spirit2::adam_lexer_test_grammar_t test_grammar(lexer, stream);

    const std::string str = read_file(g_input_file);

    adobe::spirit2::text_iterator_t it(str.begin());
    adobe::spirit2::token_iterator_t iter = lexer.begin(it, adobe::spirit2::text_iterator_t(str.end()));
    adobe::spirit2::token_iterator_t end = lexer.end();

    boost::spirit::qi::phrase_parse(iter,
                                    end,
                                    test_grammar,
                                    boost::spirit::qi::in_state("WS")[lexer.self]);

    std::cout << stream.str();

#if GENERATE_TEST_RESULTS
    std::ofstream ofs(g_expected_output_file);
    ofs << stream.str();
#else
    std::string expected_results = read_file(g_expected_output_file);
    BOOST_CHECK_EQUAL(stream.str(), expected_results);
#endif
}
#endif

#if !ADAM_TEST
BOOST_AUTO_TEST_CASE( eve_lex )
{
    const adobe::spirit2::lexer_t & lexer = adobe::spirit2::eve_lexer();
    std::stringstream stream;
    adobe::spirit2::eve_lexer_test_grammar_t test_grammar(lexer, stream);

    const std::string str = read_file(g_input_file);

    adobe::spirit2::text_iterator_t it(str.begin());
    adobe::spirit2::token_iterator_t iter = lexer.begin(it, adobe::spirit2::text_iterator_t(str.end()));
    adobe::spirit2::token_iterator_t end = lexer.end();

    boost::spirit::qi::phrase_parse(iter,
                                    end,
                                    test_grammar,
                                    boost::spirit::qi::in_state("WS")[lexer.self]);

    std::cout << stream.str();

#if GENERATE_TEST_RESULTS
    std::ofstream ofs(g_expected_output_file);
    ofs << stream.str();
#else
    std::string expected_results = read_file(g_expected_output_file);
    BOOST_CHECK_EQUAL(stream.str(), expected_results);
#endif
}
#endif
