#include <adobe/spirit2/func_parser.hpp>

#include <adobe/spirit2/adam_parser.hpp>
#include <adobe/implementation/token.hpp>


namespace adobe { namespace spirit2 {

namespace {

    struct add_function_t
    {
        template <typename ...T>
        struct result
        { typedef void type; };

        void operator()(
            adam_function_map_t& arg1,
            name_t arg2,
            const std::vector<name_t>& arg3,
            const std::vector<array_t>& arg4
        ) const
        { arg1[arg2] = adam_function_t(arg2, arg3, arg4); }
    };

    const boost::phoenix::function<add_function_t> add_function;

}

function_parser_rules_t::function_parser_rules_t(
    const lexer_t& tok,
    const expression_parser_rules_t& expression_parser
) :
    statement_parser(tok, expression_parser)
{
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    using phoenix::push_back;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using qi::_r1;
    using qi::lit;

    function
        =     tok.identifier [_a = _1]
        >     '('
        >>  - ((tok.identifier [push_back(_b, _1)]) % ',')
        >     ')'
        >     '{'
        >   * statement_parser.statement [push_back(_c, _1)]
        >     lit('}')
              [
                  add_function(_r1, _a, _b, _c)
              ]
        ;


    // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
    NAME(function);
#undef NAME

    qi::on_error<qi::fail>(function, report_error(_1, _2, _3, _4));
}

bool parse_functions(const std::string& functions,
                     const std::string& filename,
                     adam_function_map_t& retval)
{
    using boost::spirit::qi::phrase_parse;
    text_iterator_t it(functions.begin());
    detail::s_text_it = &it;
    detail::s_begin = it;
    detail::s_end = text_iterator_t(functions.end());
    detail::s_filename = filename.c_str();
    token_iterator_t iter = adam_lexer().begin(it, detail::s_end);
    token_iterator_t end = adam_lexer().end();
    function_parser_rules_t rules(adam_lexer(), adam_expression_parser());
    bool result =
        phrase_parse(iter,
                     end,
                     *rules.function(boost::phoenix::ref(retval)),
                     boost::spirit::qi::in_state("WS")[adam_lexer().self]);
    return result && iter == end;
}

} }
