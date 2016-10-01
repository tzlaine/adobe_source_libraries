#ifndef ADOBE_SPIRIT2_FUNC_PARSER_HPP
#define ADOBE_SPIRIT2_FUNC_PARSER_HPP

#include <adobe/spirit2/stmt_parser.hpp>
#include <adobe/adam_function.hpp>


namespace adobe { namespace spirit2 {

struct function_parser_rules_t
{
    function_parser_rules_t(
        const lexer_t& tok,
        const expression_parser_rules_t& expression_parser
    );

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

    statement_parser_rules_t statement_parser;
};

bool parse_functions(const std::string& functions,
                     const std::string& filename,
                     adam_function_map_t&);

} }

#endif
