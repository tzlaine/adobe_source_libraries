#include <adobe/spirit2/stmt_parser.hpp>

#include <adobe/implementation/token.hpp>


using namespace adobe;
using namespace adobe::spirit2;

namespace {

    struct array_t_push_back_t
    {
        template <typename ...T>
        struct result
        { typedef void type; };

        template <typename Arg2>
        void operator()(array_t& array, Arg2 arg2) const
            { push_back(array, arg2); }

        template <typename Arg2, typename Arg3>
        void operator()(array_t& array, Arg2 arg2, Arg3 arg3) const
            {
                push_back(array, arg2);
                push_back(array, arg3);
            }

        template <typename Arg2, typename Arg3, typename Arg4>
        void operator()(array_t& array, Arg2 arg2, Arg3 arg3, Arg4 arg4) const
            {
                push_back(array, arg2);
                push_back(array, arg3);
                push_back(array, arg4);
            }

        template <typename Arg2, typename Arg3, typename Arg4, typename Arg5>
        void operator()(array_t& array, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5) const
            {
                push_back(array, arg2);
                push_back(array, arg3);
                push_back(array, arg4);
                push_back(array, arg5);
            }

        template <typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
        void operator()(array_t& array, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6) const
            {
                push_back(array, arg2);
                push_back(array, arg3);
                push_back(array, arg4);
                push_back(array, arg5);
                push_back(array, arg6);
            }
    };

    const boost::phoenix::function<array_t_push_back_t> push;

}

statement_parser_rules_t::statement_parser_rules_t(
    const lexer_t& lexer,
    const expression_parser_rules_t& expression_parser
) {
    namespace qi = boost::spirit::qi;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using qi::_d;
    using qi::_e;
    using qi::_r1;
    using qi::_val;
    using qi::eps;
    using qi::lit;

    lexer_t& tok = const_cast<lexer_t&>(lexer);
    auto const initial_size = tok.keywords.size();
    (void)initial_size;
    const boost::spirit::lex::token_def<name_t>& constant = tok.keywords[constant_k];
    const boost::spirit::lex::token_def<name_t>& if__ = tok.keywords[if_k];
    const boost::spirit::lex::token_def<name_t>& else__ = tok.keywords[else_k];
    const boost::spirit::lex::token_def<name_t>& for__ = tok.keywords[for_k];
    const boost::spirit::lex::token_def<name_t>& continue__ = tok.keywords[continue_k];
    const boost::spirit::lex::token_def<name_t>& break__ = tok.keywords[break_k];
    const boost::spirit::lex::token_def<name_t>& return__ = tok.keywords[return_k];
    assert(tok.keywords.size() == initial_size);

    const expression_parser_rules_t::expression_rule_t& expression = expression_parser.expression;

    const_declaration
        =     constant
        >     tok.identifier [_a = _1]
        >>  - (
                   lit(':')
                >  expression(_b)
              )
        >     lit(';')
              [
                  push(_val, _a, _b, const_decl_k)
              ]
        ;

    declaration
        =     tok.identifier [_a = _1]
        >>  - (
                   lit(':')
                >  expression(_b)
              )
        >     lit(';')
              [
                  push(_val, _a, _b, decl_k)
              ]
        ;

    lvalue_expression
        =     postfix_lvalue_expression(_r1)
        |     (
                   expression(_b)
               >   "?"
               >   postfix_lvalue_expression(_c)
               >   ":"
               >   postfix_lvalue_expression(_d)
              )
              [
                  push(_r1, _b, _c, _d, ifelse_k)
              ]
        ;

    postfix_lvalue_expression
        =     primary_lvalue_expression(_r1)
        >>  * (
                   (
                        '['
                     >  expression(_r1)
                     >  ']'
                   )
                |  (
                        '.'
                     >  tok.identifier [push(_r1, _1)]
                   )
              )
              [
                  push(_r1, index_k)
              ]
        ;

    primary_lvalue_expression
        =     lvalue_variable(_r1)
        |     (
                   '('
                >> lvalue_expression(_r1)
                >  ')'
              )
        ;

    lvalue_variable
        =     tok.identifier
              [
                  push(_r1, _1, variable_k)
              ]
        ;

    assignment_prefix
        =     lvalue_expression(_b)
        >>    lit('=') [push(_r1, _b)]
        >     expression(_r1)
              [
                  push(_r1, assign_k)
              ]
        ;

    assignment
        =     assignment_prefix(_val)
        >     lit(';')
        ;

    continue_
        =     continue__
        >     lit(';')
              [
                  push(_val, continue_k)
              ]
        ;

    break_
        =     break__
        >     lit(';')
              [
                  push(_val, break_k)
              ]
        ;

    block
        =     '{'
        >   * statement [push(_r1, _1)]
        >     '}'
        ;

    substatements
        =     block(_r1)
        |     statement [push(_r1, _1)]
        ;

    if_
        =     if__
        >     '('
        >     expression(_b)
        >     ')'
        >     substatements(_c)
        >>  - (
                   else__
               >   substatements(_d)
              )
        >     eps
              [
                  push(_val, _b, _c, _d, stmt_ifelse_k)
              ]
        ;

    simple_for
        =     for__
        >     '('
        >     tok.identifier [_a = _1]
        >>  - (
                   lit(',')
               >   tok.identifier [_b = _1]
              )
        >     lit(':')
        >     expression(_c)
        >     ')'
        >     substatements(_d)
        >     eps
              [
                  push(_val, _a, _b, _c, _d, simple_for_k)
              ]
        ;

    for_decl
        =     tok.identifier [_a = _1]
        >>    lit(':')
        >>    expression(_b)
              [
                  push(_r1, _a, _b, for_decl_k)
              ]
        ;

    complex_for
        =     for__
        >>    '('
        >>    for_decl(_b)
        >>  * (
                   lit(',')
               >   for_decl(_b)
              )
        >>    lit(';')
        >     expression(_c)
        >     lit(';')
        >     assignment_prefix(_d) % ','
        >     ')'
        >     substatements(_e)
        >     eps
              [
                  push(_val, _b, _c, _d, _e, complex_for_k)
              ]
        ;

    return_
        =     return__
        >     expression(_val)
        >     lit(';')
              [
                  push(_val, return_k)
              ]
        ;

    statement
        %=    const_declaration
        |     continue_
        |     break_
        |     return_
        |     if_
        |     assignment
        |     declaration
        |     complex_for
        |     simple_for
        ;

    // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
    NAME(const_declaration);
    NAME(declaration);
    NAME(assignment_prefix);
    NAME(assignment);
    NAME(block);
    NAME(substatements);
    NAME(if_);
    NAME(simple_for);
    NAME(for_decl);
    NAME(complex_for);
    NAME(continue_);
    NAME(break_);
    NAME(return_);
    NAME(statement);
#undef NAME

    qi::on_error<qi::fail>(statement, report_error(_1, _2, _3, _4));
}
