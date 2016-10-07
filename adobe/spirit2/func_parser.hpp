#ifndef ADOBE_SPIRIT2_FUNC_PARSER_HPP
#define ADOBE_SPIRIT2_FUNC_PARSER_HPP

#include <adobe/spirit2/stmt_parser.hpp>
#include <adobe/adam_function.hpp>


namespace adobe { namespace spirit2 {

struct function_parser_rules_t
{
    explicit function_parser_rules_t(const expression_parser_rules_t& expression_parser);

    typedef boost::spirit::qi::rule<
        token_iterator_t,
        any_regular_t(),
        boost::spirit::qi::locals<
            std::vector<name_t>,
            std::vector<array_t>
        >,
        skipper_type_t
    > lambda_rule_t;

    lambda_rule_t lambda;

    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(adam_function_map_t&),
        boost::spirit::qi::locals<
            name_t,
            std::vector<name_t>,
            std::vector<array_t>
        >,
        skipper_type_t
    > function_rule_t;

    function_rule_t function;

    typedef boost::spirit::qi::rule<
        token_iterator_t,
        void(std::vector<name_t>&, std::vector<array_t>&),
        boost::spirit::qi::locals<>,
        skipper_type_t
    > function_specifier_rule_t;

    function_specifier_rule_t function_specifier;

    statement_parser_rules_t statement_parser;
};

bool parse_functions(const std::string& functions,
                     const std::string& filename,
                     adam_function_map_t&);

} }

#endif
