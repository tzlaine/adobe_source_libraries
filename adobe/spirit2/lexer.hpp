#ifndef ADOBE_SPIRIT2_LEXER_HPP
#define ADOBE_SPIRIT2_LEXER_HPP

#include <adobe/spirit2/lexer_fwd.hpp>
#include <adobe/spirit2/report_parse_error.hpp>

#include <adobe/istream.hpp>
#include <adobe/implementation/token.hpp>


namespace adobe { namespace spirit2 {

struct lexer_t :
    boost::spirit::lex::lexer<spirit_lexer_base_type_t>
{
    lexer_t(const adobe::name_t* first_keyword,
            const adobe::name_t* last_keyword);

    boost::spirit::lex::token_def<bool> keyword_true_false;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> keyword_empty;
    boost::spirit::lex::token_def<adobe::name_t> identifier;
    boost::spirit::lex::token_def<std::string> lead_comment;
    boost::spirit::lex::token_def<std::string> trail_comment;
    boost::spirit::lex::token_def<std::string> quoted_string;
    boost::spirit::lex::token_def<double> number;
    boost::spirit::lex::token_def<adobe::name_t> eq_op;
    boost::spirit::lex::token_def<adobe::name_t> rel_op;
    boost::spirit::lex::token_def<adobe::name_t> mul_op;
    boost::spirit::lex::token_def<adobe::name_t> bitshift_op;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> define;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> or_;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> and_;
    std::map<adobe::name_t, boost::spirit::lex::token_def<adobe::name_t> > keywords;
};

typedef lexer_t::iterator_type token_iterator_t;

typedef lexer_t::lexer_def lexer_def_t;

typedef boost::spirit::qi::in_state_skipper<lexer_def_t> skipper_type_t;

extern const boost::phoenix::function<report_error_t<token_type_t> > report_error;

} }


// This code creates a new Spirit.Qi parser that does approximately what the
// Adobe lexer's next_position() function does.

namespace adobe { namespace spirit2 { namespace detail {
    BOOST_SPIRIT_TERMINAL(next_pos);
} } }

namespace boost { namespace spirit {
    template <>
    struct use_terminal<qi::domain, adobe::spirit2::detail::tag::next_pos> :
        mpl::true_
    {};
} }

namespace adobe { namespace spirit2 { namespace detail {
    struct next_pos_parser_t :
        boost::spirit::qi::primitive_parser<next_pos_parser_t>
    {
        template <typename Context, typename Iter>
        struct attribute
        { typedef adobe::line_position_t type; };

        template <typename Iter, typename Context, typename Skipper, typename Attribute>
        bool parse(Iter& first, Iter const& last, Context&, Skipper const& skipper, Attribute& attr) const
        {
            boost::spirit::qi::skip_over(first, last, skipper);
            attr = adobe::line_position_t(detail::s_filename, boost::spirit::get_line(first->matched().begin()) - 1);
            // Note that the +1's below are there to provide the user with
            // 1-based column numbers.  This is Adobe's convention.  The Adobe
            // convention is also that line numbers are 0-based.  Go figure.
            attr.line_start_m =
                std::distance(detail::s_begin,
                              boost::spirit::get_line_start(detail::s_begin, first->matched().begin())) + 2;
            attr.position_m =
                std::distance(detail::s_begin, first->matched().begin()) + 1;
            return true;
        }

        template <typename Context>
        boost::spirit::info what(Context&) const
        { return boost::spirit::info("next_pos"); }
    };
} } }

namespace boost { namespace spirit { namespace qi {
    template <typename Modifiers>
    struct make_primitive<adobe::spirit2::detail::tag::next_pos, Modifiers>
    {
        typedef adobe::spirit2::detail::next_pos_parser_t result_type;
        result_type operator()(unused_type, unused_type) const
        { return result_type(); }
    };
} } }


namespace boost { namespace spirit { namespace traits
{
    // This template specialization is required by Spirit.Lex to automatically
    // convert an iterator pair to a name_t.

    template <typename Iter>
    struct assign_to_attribute_from_iterators<adobe::name_t, Iter>
    {
        static void call(const Iter& first, const Iter& last, adobe::name_t& attr)
        { attr = adobe::name_t(std::string(first, last).c_str()); }
    };

#if 0 // TODO
    // HACK! This is only necessary because of a bug in Spirit in Boost
    // versions <= 1.45.
    template <>
    struct assign_to_attribute_from_iterators<bool, adobe::spirit2::text_iterator_t, void>
    {
        static void call(const adobe::spirit2::text_iterator_t& first, const adobe::spirit2::text_iterator_t& last, bool& attr)
        { attr = *first == 't' ? true : false; }
    };
#endif

} } }

#endif
