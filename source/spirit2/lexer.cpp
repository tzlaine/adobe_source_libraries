#include <adobe/spirit2/lexer.hpp>

#include <adobe/name.hpp>


namespace adobe { namespace spirit2 {

    const boost::phoenix::function<report_error_t<token_type_t> > report_error;

    struct to_name_base_t
    {
        template <typename ...T>
        struct result
        { typedef adobe::name_t type; };
    };

    struct get_eq_op_t : to_name_base_t
    {
        template <typename Arg1, typename Arg2>
        adobe::name_t operator()(Arg1 first, Arg2) const
        { return *first == '=' ? equal_k : not_equal_k; }
    };
    const boost::phoenix::function<get_eq_op_t> get_eq_op;

    struct get_rel_op_t : to_name_base_t
    {
        template <typename Arg1, typename Arg2>
        adobe::name_t operator()(Arg1 first, Arg2 last) const
        {
            auto const dist = std::distance(first, last);
            return *first == '<' ?
                (dist == 1 ? less_k : less_equal_k) :
                (dist == 1 ? greater_k : greater_equal_k);
        }
    };
    const boost::phoenix::function<get_rel_op_t> get_rel_op;

    struct get_mul_op_t : to_name_base_t
    {
        template <typename Arg1, typename Arg2>
        adobe::name_t operator()(Arg1 first, Arg2) const
        { return *first == '*' ? multiply_k : (*first == '/' ? divide_k : modulus_k); }
    };
    const boost::phoenix::function<get_mul_op_t> get_mul_op;

    struct get_bitshift_op_t : to_name_base_t
    {
        template <typename Arg1, typename Arg2>
        adobe::name_t operator()(Arg1 first, Arg2) const
        { return *first == '<' ? bitwise_lshift_k : bitwise_rshift_k; }
    };
    const boost::phoenix::function<get_bitshift_op_t> get_bitshift_op;

} }

using namespace adobe::spirit2;

// TODO: Confirm these tokens against the newest grammar.
lexer_t::lexer_t(const adobe::name_t* first_keyword,
                 const adobe::name_t* last_keyword) :
    keyword_true_false("true|false"),
    keyword_empty("empty"),
    identifier("[a-zA-Z]\\w*"),
    lead_comment("\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/"),
    trail_comment("\\/\\/.*$"),
    quoted_string("\\\"[^\\\"]*\\\"|'[^']*'"),
    number("\\d+(\\.\\d*)?"),
    eq_op("==|!="),
    rel_op("<|>|<=|>="),
    mul_op("\\*|\\/|%"),
    bitshift_op("<<|>>"),
    define("<=="),
    or_("\"||\""),
    and_("&&")
{
    namespace lex = boost::spirit::lex;

    self
        =     keyword_true_false
        |     keyword_empty;

    while (first_keyword != last_keyword) {
        self.add(
            keywords[*first_keyword] =
            boost::spirit::lex::token_def<adobe::name_t>(first_keyword->c_str())
        );
        ++first_keyword;
    }

    using boost::spirit::lex::_start;
    using boost::spirit::lex::_end;
    using boost::spirit::lex::_val;

    self
        +=    identifier
        |     lead_comment
        |     trail_comment
        |     quoted_string
        |     number
        |     bitshift_op    [ _val = get_bitshift_op(_start, _end) ]
        |     eq_op          [ _val = get_eq_op(_start, _end) ]
        |     rel_op         [ _val = get_rel_op(_start, _end) ]
        |     mul_op         [ _val = get_mul_op(_start, _end) ]
        |     define
        |     or_
        |     and_
        |     '='
        |     '+'
        |     '-'
        |     '|'
        |     '&'
        |     '^'
        |     '~'
        |     '!'
        |     '?'
        |     ':'
        |     '.'
        |     ','
        |     '('
        |     ')'
        |     '['
        |     ']'
        |     '{'
        |     '}'
        |     '@'
        |     ';'
        ;

    self("WS") = lex::token_def<>("\\s+");
}
