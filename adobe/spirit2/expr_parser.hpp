#ifndef ADOBE_SPIRIT2_EXPR_PARSER_HPP
#define ADOBE_SPIRIT2_EXPR_PARSER_HPP

#include <adobe/spirit2/lexer.hpp>
#include <adobe/array.hpp>
#include <adobe/dictionary.hpp>
#include <adobe/implementation/token.hpp>

#include <boost/spirit/home/qi/action.hpp>
#include <boost/spirit/home/qi/auxiliary.hpp>
#include <boost/spirit/home/qi/char.hpp>
#include <boost/spirit/home/qi/directive.hpp>
#include <boost/spirit/home/qi/nonterminal.hpp>
#include <boost/spirit/home/qi/numeric.hpp>
#include <boost/spirit/home/qi/operator.hpp>

#include <boost/spirit/include/phoenix.hpp>


namespace adobe { namespace spirit2 {

struct expression_parser_rules_t
{
    typedef boost::spirit::qi::rule<
        token_iterator_t,
        name_t(),
        skipper_type_t
    > keyword_rule_t;

    expression_parser_rules_t(const lexer_t& tok, const keyword_rule_t& keyword_);

    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(array_t&),
        boost::spirit::qi::locals<array_t, array_t>,
        skipper_type_t
    > expression_rule_t;
    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(array_t&),
        boost::spirit::qi::locals<name_t>,
        skipper_type_t
    > local_name_rule_t;
    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(array_t&),
        boost::spirit::qi::locals<std::size_t>,
        skipper_type_t
    > local_size_rule_t;
    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(array_t&),
        boost::spirit::qi::locals<array_t>,
        skipper_type_t
    > local_array_rule_t;
    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(array_t&),
        skipper_type_t
    > no_locals_rule_t;

    // expression grammar
    expression_rule_t expression;

    local_array_rule_t or_expression;
    local_array_rule_t and_expression;
    local_array_rule_t bitwise_or_expression;
    local_array_rule_t bitwise_xor_expression;
    local_array_rule_t bitwise_and_expression;
    local_name_rule_t equality_expression;
    local_name_rule_t relational_expression;
    local_name_rule_t bitshift_expression;
    local_name_rule_t additive_expression;
    local_name_rule_t multiplicative_expression;
    local_name_rule_t unary_expression;
    no_locals_rule_t postfix_expression;
    no_locals_rule_t primary_expression;
    local_name_rule_t variable_or_function;
    no_locals_rule_t array;
    no_locals_rule_t dictionary;
    no_locals_rule_t argument_expression_list;
    local_size_rule_t argument_list;
    local_size_rule_t named_argument_list;
    local_name_rule_t named_argument;
    no_locals_rule_t name;
    no_locals_rule_t boolean;

    // lexical grammar
    boost::spirit::qi::rule<
        token_iterator_t,
        void(array_t&),
        boost::spirit::qi::locals<std::string>,
        skipper_type_t
    > string;
    keyword_rule_t keyword;
};

} }

#endif
