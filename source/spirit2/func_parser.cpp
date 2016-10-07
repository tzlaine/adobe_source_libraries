#include <adobe/spirit2/func_parser.hpp>

#include <adobe/spirit2/adam_parser.hpp>
#include <adobe/implementation/token.hpp>


namespace adobe { namespace spirit2 {

namespace {

    struct make_function_t
    {
        template <typename ...T>
        struct result
        { typedef void type; };

        adam_function_t operator()(
            name_t arg1,
            const std::vector<name_t>& arg2,
            const std::vector<array_t>& arg3
        ) const
        { return adam_function_t(arg1, arg2, arg3); }
    };

    const boost::phoenix::function<make_function_t> make_function;

    struct add_function_t
    {
        template <typename ...T>
        struct result
        { typedef void type; };

        void operator()(
            adam_function_map_t & arg1,
            name_t arg2,
            adam_function_t const & arg3
        ) const
        { arg1[arg2] = arg3; }
    };

    const boost::phoenix::function<add_function_t> add_function;

}

function_parser_rules_t::function_parser_rules_t(
    const expression_parser_rules_t& expression_parser
) :
    statement_parser(expression_parser)
{
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    using phoenix::construct;
    using phoenix::push_back;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using qi::_r1;
    using qi::_r2;
    using qi::_val;

    const lexer_t& tok = adam_lexer();

    lambda
        =     '\\'
        >>    function_specifier(_a, _b)
              [
                  _val = construct<any_regular_t>(make_function("<lambda>"_name, _a, _b))
              ]
        ;

    function
        =     tok.identifier [_a = _1]
        >     function_specifier(_b, _c)
              [
                  add_function(_r1, _a, make_function(_a, _b, _c))
              ]
        ;

    function_specifier
        =     '('
        >>  - ((tok.identifier [push_back(_r1, _1)]) % ',')
        >     ')'
        >     '{'
        >   * statement_parser.statement [push_back(_r2, _1)]
        >     '}'
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
    function_parser_rules_t rules(adam_expression_parser());
    bool result =
        phrase_parse(iter,
                     end,
                     *rules.function(boost::phoenix::ref(retval)),
                     boost::spirit::qi::in_state("WS")[adam_lexer().self]);
    return result && iter == end;
}

} }
