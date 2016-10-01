#ifndef ADOBE_SPIRIT2_LEXER_FWD_HPP
#define ADOBE_SPIRIT2_LEXER_FWD_HPP

#include <adobe/name.hpp>

#include <boost/spirit/home/support/iterators/line_pos_iterator.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/home/lex/lexer/lexertl/position_token.hpp>

#include <string>


namespace adobe { namespace spirit2 {

typedef boost::spirit::line_pos_iterator<std::string::const_iterator> text_iterator_t;

typedef boost::spirit::lex::lexertl::position_token<
    text_iterator_t,
    boost::mpl::vector<
        adobe::name_t,
        std::string,
        double,
        bool
    >
> token_type_t;

typedef boost::spirit::lex::lexertl::actor_lexer<token_type_t> spirit_lexer_base_type_t;

struct lexer_t;

} }

#endif
