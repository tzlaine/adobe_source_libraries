#ifndef ADOBE_SPIRIT2_STMT_PARSER_HPP
#define ADOBE_SPIRIT2_STMT_PARSER_HPP

#include <adobe/spirit2/lexer.hpp>
#include <adobe/array_fwd.hpp>
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

struct expression_parser_rules_t;

struct statement_parser_rules_t
{
    explicit statement_parser_rules_t(const expression_parser_rules_t& expression_parser);

    typedef boost::spirit::qi::rule<
        token_iterator_t,
        array_t(),
        boost::spirit::qi::locals<
            name_t,
            array_t,
            array_t,
            array_t,
            array_t
        >,
        skipper_type_t
    > statement_rule_t;
    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(array_t&),
        boost::spirit::qi::locals<
            name_t,
            array_t,
            array_t,
            array_t
        >,
        skipper_type_t
    > statement_rule_2_t;
    typedef boost::spirit::qi::rule<
        token_iterator_t,
        array_t(),
        boost::spirit::qi::locals<
            name_t,
            name_t,
            array_t,
            array_t
        >,
        skipper_type_t
    > simple_for_rule_t;

    // statement grammar
    statement_rule_t statement;

    statement_rule_t const_declaration;
    statement_rule_t declaration;
    statement_rule_2_t lvalue_expression;
    statement_rule_2_t postfix_lvalue_expression;
    statement_rule_2_t primary_lvalue_expression;
    statement_rule_2_t lvalue_variable;
    statement_rule_2_t assignment_prefix;
    statement_rule_t assignment;
    statement_rule_2_t block;
    statement_rule_2_t substatements;
    statement_rule_t if_;
    simple_for_rule_t simple_for;
    statement_rule_2_t for_decl;
    statement_rule_t complex_for;
    statement_rule_t continue_;
    statement_rule_t break_;
    statement_rule_t return_;
};

} }

#endif
