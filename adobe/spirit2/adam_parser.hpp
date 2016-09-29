#ifndef ADOBE_SPIRIT2_ADAM_PARSER_HPP
#define ADOBE_SPIRIT2_ADAM_PARSER_HPP

#include <adobe/spirit2/expr_parser.hpp>
#include <adobe/array_fwd.hpp>

#include <string>


namespace adobe {
    struct adam_callback_suite_t;

    namespace spirit2 {

        const lexer_t& adam_lexer();

        const expression_parser_rules_t::expression_rule_t& adam_expression_parser();

        bool parse(const std::string& sheet,
                   const std::string& filename,
                   const adam_callback_suite_t& callbacks);

        array_t parse_adam_expression(const std::string& expr);

    }
}

#endif
