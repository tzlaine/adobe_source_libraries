#ifndef ADOBE_SPIRIT2_EVE_PARSER_HPP
#define ADOBE_SPIRIT2_EVE_PARSER_HPP

#include <adobe/spirit2/expr_parser.hpp>
#include <adobe/array_fwd.hpp>

#include <string>


namespace boost {
    class any;
}

namespace adobe {

    struct eve_callback_suite_t;

    namespace spirit2 {

        const lexer_t& eve_lexer();

        const expression_parser_rules_t& eve_expression_parser();

        bool parse(const std::string& sheet,
                   const std::string& filename,
                   const boost::any& parent,
                   const eve_callback_suite_t& callbacks);

    }

}

#endif
