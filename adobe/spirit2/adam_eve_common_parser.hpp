#ifndef ADOBE_SPIRIT2_ADAM_EVE_COMMON_PARSER_HPP
#define ADOBE_SPIRIT2_ADAM_EVE_COMMON_PARSER_HPP

#include <adobe/spirit2/lexer.hpp>

#include <adobe/array.hpp>

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

template <typename Callbacks>
struct adam_eve_common_parser_rules_t
{
    adam_eve_common_parser_rules_t(lexer_t const & lexer_,
                                   expression_parser_rules_t const & expression_parser_,
                                   Callbacks const & callbacks_);

    typedef typename Callbacks::relation_t relation_t;
    typedef std::vector<relation_t> relation_set_t;

    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(),
        boost::spirit::qi::locals<std::string>,
        skipper_type_t
    > cell_set_rule_t;
    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(std::string const &),
        boost::spirit::qi::locals<
            name_t,
            array_t,
            line_position_t,
            std::string
        >,
        skipper_type_t
    > cell_decl_rule_t;

    cell_set_rule_t interface_set_decl;
    cell_set_rule_t constant_set_decl;
    cell_set_rule_t logic_set_decl;

    boost::spirit::qi::rule<
        token_iterator_t,
        void(std::string const &),
        boost::spirit::qi::locals<
            name_t,
            bool,
            array_t,
            array_t,
            line_position_t,
            line_position_t,
            std::string
        >,
        skipper_type_t
    > interface_cell_decl;

    cell_decl_rule_t constant_cell_decl;

    boost::spirit::qi::rule<
        token_iterator_t,
        void(std::string const &),
        boost::spirit::qi::locals<
            name_t,
            array_t,
            line_position_t,
            std::string,
            relation_set_t
        >,
        skipper_type_t
    > logic_cell_decl;

    boost::spirit::qi::rule<
        token_iterator_t,
        void(line_position_t&, array_t&, relation_set_t&, std::string&),
        boost::spirit::qi::locals<relation_t, relation_t>,
        skipper_type_t
    > relate_decl;

    boost::spirit::qi::rule<token_iterator_t, void(relation_t&), skipper_type_t> relate_expression;

    boost::spirit::qi::rule<
        token_iterator_t,
        void(name_t&, line_position_t&, array_t&, std::string&),
        skipper_type_t
    > named_decl;

    boost::spirit::qi::rule<
        token_iterator_t,
        void(line_position_t&, array_t&),
        skipper_type_t
    > initializer;
    boost::spirit::qi::rule<
        token_iterator_t,
        void(line_position_t&, array_t&),
        skipper_type_t
    > define_expression;
    boost::spirit::qi::rule<
        token_iterator_t,
        void(line_position_t&, array_t&),
        skipper_type_t
    > conditional;
    boost::spirit::qi::rule<token_iterator_t, void(std::string&), skipper_type_t> end_statement;

    boost::spirit::qi::rule<token_iterator_t, std::string(), skipper_type_t> lead_comment;
    boost::spirit::qi::rule<token_iterator_t, std::string(), skipper_type_t> trail_comment;

    lexer_t const & lexer;
    expression_parser_rules_t const & expression_parser;
    Callbacks const & callbacks;
};

} }

#endif
