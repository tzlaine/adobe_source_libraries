#include <adobe/spirit2/expr_parser.hpp>

#include <adobe/implementation/token.hpp>

#include <boost/algorithm/string/replace.hpp>


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
    };

    const boost::phoenix::function<array_t_push_back_t> push;

    struct process_strings_t
    {
        template <typename ...T>
        struct result
        { typedef std::string type; };

        std::string operator()(std::string const & quoted_str) const
        {
            std::string str = quoted_str.substr(1, quoted_str.size() - 2);
            boost::algorithm::replace_all(str, "\\n", "\n");
            return str;
        }
    };

    const boost::phoenix::function<process_strings_t> process_strings;

}

expression_parser_rules_t::expression_parser_rules_t(const lexer_t& tok, const keyword_rule_t& keyword_) :
    keyword(keyword_)
{
    namespace ascii = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    using ascii::char_;
    using phoenix::clear;
    using phoenix::construct;
    using phoenix::if_;
    using phoenix::static_cast_;
    using phoenix::val;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_r1;
    using qi::_val;
    using qi::alpha;
    using qi::bool_;
    using qi::digit;
    using qi::double_;
    using qi::eol;
    using qi::eps;
    using qi::lexeme;
    using qi::lit;

    expression
        =     or_expression(_r1)
        >>  - (
                   "?"
                >  expression(_a)
                >  ":"
                >  expression(_b)
              )
              [
                  push(_r1, _a, _b, ifelse_k)
              ]
        ;

    or_expression
        =     and_expression(_r1)
        >>  * (
                   tok.or_
                >  and_expression(_a)
              )
              [
                  push(_r1, _a, or_k),
                  clear(_a)
              ]
        ;

    and_expression
        =     bitwise_or_expression(_r1)
        >>  * (
                   tok.and_
                >  bitwise_or_expression(_a)
              )
              [
                  push(_r1, _a, and_k),
                  clear(_a)
              ]
        ;

    bitwise_or_expression
        =     bitwise_xor_expression(_r1)
        >>  * (
                   '|'
                >  bitwise_xor_expression(_a)
              )
              [
                  push(_r1, _a, bitwise_or_k),
                  clear(_a)
              ]
        ;

    bitwise_xor_expression
        =     bitwise_and_expression(_r1)
        >>  * (
                   '^'
                >  bitwise_and_expression(_a)
              )
              [
                  push(_r1, _a, bitwise_xor_k),
                  clear(_a)
              ]
        ;

    bitwise_and_expression
        =     equality_expression(_r1)
        >>  * (
                   '&'
                >  equality_expression(_a)
              )
              [
                  push(_r1, _a, bitwise_and_k),
                  clear(_a)
              ]
        ;

    equality_expression
        =     relational_expression(_r1)
        >>  * (
                   tok.eq_op [_a = _1]
                >  relational_expression(_r1)
              )
              [
                  push(_r1, _a)
              ]
        ;

    relational_expression
        =     bitshift_expression(_r1)
        >>  * (
                   tok.rel_op [_a = _1]
                >  bitshift_expression(_r1)
              )
              [
                  push(_r1, _a)
              ]
        ;

    bitshift_expression
        =     additive_expression(_r1)
        >>  * (
                   tok.bitshift_op [_a = _1]
                >  additive_expression(_r1)
              )
              [
                  push(_r1, _a)
              ]
        ;

    additive_expression
        =     multiplicative_expression(_r1)
        >>  * (
                   (
                        lit('+') [_a = add_k]
                     |  lit('-') [_a = subtract_k]
                   )
                >  multiplicative_expression(_r1)
              )
              [
                  push(_r1, _a)
              ]
        ;

    multiplicative_expression
        =     unary_expression(_r1)
        >>  * (
                   tok.mul_op [_a = _1]
                >  unary_expression(_r1)
              )
              [
                  push(_r1, _a)
              ]
        ;

    unary_expression
        =     postfix_expression(_r1)
        |     (
                   (
                        lit('+')
                     |  lit('-') [_a = unary_negate_k]
                     |  lit('!') [_a = not_k]
                     |  lit('~') [_a = bitwise_negate_k]
                   )
                >  unary_expression(_r1)
              )
              [
                  if_(_a) [push(_r1, _a)]
              ]
        ;

    // omitting unary_operator

    postfix_expression
        =     primary_expression(_r1)
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

    primary_expression
        =     name(_r1)
        |     tok.number
              [
                  push(_r1, _1)
              ]
        |     boolean(_r1)
        |     string(_r1)
        |     tok.keyword_empty
              [
                  push(_r1, any_regular_t())
              ]
        |     array(_r1)
        |     dictionary(_r1)
        |     variable_or_function(_r1)
        |     (
                   '('
                >  expression(_r1)
                >  ')'
              )
        ;

    variable_or_function
        =     (
                   tok.identifier [_a = _1]
                >> (
                        "("
                     >> (
                             argument_expression_list(_r1)
                          |  eps [push(_r1, array_t())]
                        )
                     >  ")"
                   )
              )
              [
                  push(_r1, _a, function_k)
              ]
        |     tok.identifier
              [
                  push(_r1, _1, variable_k)
              ]
        ;

    array
        =     '['
        >>    (
                   argument_list(_r1)
                |  eps [push(_r1, array_t())]
              )
        >     ']'
        ;

    dictionary
        =     '{'
        >>    (
                   named_argument_list(_r1)
                |  eps [push(_r1, dictionary_t())]
              )
        >     '}'
        ;

    argument_expression_list
        =     named_argument_list(_r1)
        |     argument_list(_r1)
        ;

    argument_list
        =     (
                    expression(_r1) [_a = 1]
               >> * (
                         ','
                      >  expression(_r1) [++_a]
                    )
              )
              [
                  push(_r1, static_cast_<double>(_a), array_k)
              ]
        ;

    named_argument_list
        =     (
                    named_argument(_r1) [_a = 1]
               >> * (
                         ','
                      >  named_argument(_r1) [++_a]
                    )
              )
              [
                  push(_r1, static_cast_<double>(_a), dictionary_k)
              ]
        ;

    named_argument
        =     tok.identifier [_a = _1]
        >>    lit(':') [push(_r1, _a)]
        >     expression(_r1)
        ;

    name
        =     '@'
        >     (
                   keyword [push(_r1, _1)]
                |  tok.identifier [push(_r1, _1)]
              )
        ;

    boolean = tok.keyword_true_false [push(_r1, _1)] ;


    // lexical grammar not covered by lexer

    string
        =     (
                     tok.quoted_string [_a = process_strings(_1)]
                >> * tok.quoted_string [_a += process_strings(_1)]
              )
              [
                  push(_r1, _a)
              ]
        ;

    keyword
        =     tok.keyword_true_false
              [
                  _val = if_else(_1, true_k, false_k)
              ]
        |     tok.keyword_empty
              [
                  _val = empty_k
              ]
        |     keyword_
              [
                  _val = _1
              ]
        ;


    // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
    NAME(expression);
    NAME(or_expression);
    NAME(and_expression);
    NAME(bitwise_or_expression);
    NAME(bitwise_xor_expression);
    NAME(bitwise_and_expression);
    NAME(equality_expression);
    NAME(relational_expression);
    NAME(bitshift_expression);
    NAME(additive_expression);
    NAME(multiplicative_expression);
    NAME(unary_expression);
    NAME(postfix_expression);
    NAME(primary_expression);
    NAME(variable_or_function);
    NAME(array);
    NAME(dictionary);
    NAME(argument_expression_list);
    NAME(argument_list);
    NAME(named_argument_list);
    NAME(named_argument);
    NAME(name);
    NAME(boolean);
    NAME(string);
#undef NAME

    qi::on_error<qi::fail>(expression, report_error(_1, _2, _3, _4));
}
