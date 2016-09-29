#include <adobe/spirit2/adam_eve_common_parser.hpp>

#include <adobe/spirit2/expr_parser.hpp>
#include <adobe/adam_parser.hpp>
#include <adobe/eve_parser.hpp>
#include <adobe/implementation/token.hpp>


// requirements of spirit::qi::debug()
namespace adobe {

    inline std::ostream& operator<<(std::ostream& os, const adam_callback_suite_t::relation_t& relation)
    {
        os << "( [ ";
        for (auto const name : relation.name_set_m) {
            os << name << " ";
        }
        os << "] ";
        return os << relation.position_m << " "
                  << relation.expression_m << " "
                  << relation.detailed_m << " "
                  << relation.brief_m << " "
                  << ")";
    }

    inline std::ostream& operator<<(std::ostream& os, const std::vector<adam_callback_suite_t::relation_t>& relation_set)
    {
        return os; // TODO
    }

namespace spirit2 {

namespace {

    static_name_t interface_k  = "interface"_name;
    static_name_t logic_k      = "logic"_name;
    static_name_t constant_k   = "constant"_name;
    static_name_t unlink_k     = "unlink"_name;
    static_name_t when_k       = "when"_name;
    static_name_t relate_k     = "relate"_name;

#define GET_REF(type_, name)                                            \
    struct get_##name##_t                                               \
    {                                                                   \
        template <typename ...T>                                        \
        struct result                                                   \
        { typedef type_ type; };                                        \
                                                                        \
        template <typename Arg1>                                        \
        type_ operator()(Arg1 & relation) const                         \
        { return relation.name##_m; }                                   \
    };                                                                  \
    const boost::phoenix::function<get_##name##_t> get_##name

    GET_REF(std::vector<name_t>&, name_set);
    GET_REF(line_position_t&, position);
    GET_REF(array_t&, expression);
    GET_REF(std::string&, detailed);
    GET_REF(std::string&, brief);

#undef GET_REF

    struct add_cell_t
    {
        template <typename ...T>
        struct result
        { typedef void type; };

        template <typename T0, typename ...T>
        void operator()(T0 & callbacks, T &&... args) const
        { callbacks.add_cell_proc_m(std::forward<T>(args)...); }
    };

    const boost::phoenix::function<add_cell_t> add_cell;

    struct add_relation_t
    {
        template <typename ...T>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
        void operator()(Arg1 & callbacks, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6) const
        { callbacks.add_relation_proc_m(arg2, arg3, &arg4.front(), &arg4.front() + arg4.size(), arg5, arg6); }
    };

    const boost::phoenix::function<add_relation_t> add_relation;

    struct add_interface_t
    {
        template <typename ...T>
        struct result
        { typedef void type; };

        template <typename T0, typename ...T>
        void operator()(T0 & callbacks, T &&... args) const
        { callbacks.add_interface_proc_m(std::forward<T>(args)...); }
    };

    const boost::phoenix::function<add_interface_t> add_interface;

    struct strip_c_comment_t
    {
        template <typename ...T>
        struct result
        { typedef std::string type; };

        std::string operator()(const std::string& str) const
        { return str.substr(2, str.size() - 4); }
    };

    const boost::phoenix::function<strip_c_comment_t> strip_c_comment;

    struct strip_cpp_comment_t
    {
        template <typename ...T>
        struct result
        { typedef std::string type; };

        std::string operator()(const std::string& str) const
        { return str.substr(2, str.size() - 2); }
    };

    const boost::phoenix::function<strip_cpp_comment_t> strip_cpp_comment;

}

template <typename Callbacks>
adam_eve_common_parser_rules_t<Callbacks>::adam_eve_common_parser_rules_t(
    lexer_t const & lexer_,
    expression_parser_rules_t const & expression_parser_,
    Callbacks const & callbacks_
) :
    lexer(lexer_),
    expression_parser(expression_parser_),
    callbacks(callbacks_)
{
    namespace ascii = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    using ascii::char_;
    using phoenix::clear;
    using phoenix::construct;
    using phoenix::cref;
    using phoenix::if_;
    using phoenix::static_cast_;
    using phoenix::push_back;
    using phoenix::val;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using qi::_d;
    using qi::_e;
    using qi::_f;
    using qi::_g;
    using qi::_r1;
    using qi::_r2;
    using qi::_r3;
    using qi::_r4;
    using qi::_val;
    using qi::alpha;
    using qi::bool_;
    using qi::digit;
    using qi::double_;
    using qi::eol;
    using qi::eps;
    using qi::lexeme;
    using qi::lit;

    lexer_t& tok = const_cast<lexer_t&>(lexer);
    auto const initial_size = tok.keywords.size();
    (void)initial_size;
    const boost::spirit::lex::token_def<name_t>& interface = tok.keywords[interface_k];
    const boost::spirit::lex::token_def<name_t>& logic = tok.keywords[logic_k];
    const boost::spirit::lex::token_def<name_t>& constant = tok.keywords[constant_k];
    const boost::spirit::lex::token_def<name_t>& unlink = tok.keywords[unlink_k];
    const boost::spirit::lex::token_def<name_t>& when = tok.keywords[when_k];
    const boost::spirit::lex::token_def<name_t>& relate = tok.keywords[relate_k];
    assert(tok.keywords.size() == initial_size);

    using adobe::spirit2::detail::next_pos;

    auto const & expression = expression_parser.expression;

#define SET_DECL(name)                          \
    name##_set_decl                                             \
        =      name                                             \
        >      ':'                                      \
        >>   * (                                        \
                    (                                   \
                        lead_comment [_a = _1]          \
                      | eps [clear(_a)]                 \
                    )                                   \
                 >> name##_cell_decl(_a)                \
               )

    SET_DECL(interface);
    SET_DECL(constant);
    SET_DECL(logic);

#undef SET_DECL

    interface_cell_decl
        =
        (
            (
                  tok.identifier
                  [
                      _a = _1,
                      _b = val(true)
                  ]
             |    (
                       unlink [_b = val(false)]
                   >   tok.identifier [_a = _1]
                  )
            )
            >>  - initializer(_e, _c)
            >>  - define_expression(_f, _d)
            >     end_statement(_g)
        )
        [
            add_interface(callbacks, _a, _b, _e, _c, _f, _d, _g, _r1)
        ]
        ;

    constant_cell_decl
        =     tok.identifier [_a = _1]
        >     initializer(_c, _b)
        >     end_statement(_d)
              [
                  add_cell(callbacks, Callbacks::constant_k, _a, _c, _b, _d, _r1)
              ]
        ;

    logic_cell_decl
        =     named_decl(_a, _c, _b, _d)
              [
                  add_cell(callbacks, Callbacks::logic_k, _a, _c, _b, _d, _r1)
              ]
        |     relate_decl(_c, _b, _e, _d)
              [
                  add_relation(callbacks, _c, _b, _e, _d, _r1)
              ]
        ;

    relate_decl
        =    (
                   (relate >> next_pos [_r1 = _1])
              |    (conditional(_r1, _r2) > relate)
             )
        >    '{'
        >    relate_expression(_a)
        >    relate_expression(_b)
             [
                 push_back(_r3, _a),
                 push_back(_r3, _b),
                 clear(get_expression(_a))
             ]
        >> * relate_expression(_a)
             [
                 push_back(_r3, _a),
                 clear(get_expression(_a))
             ]
        >    '}'
        >> - trail_comment [_r4 = _1]
        ;

    relate_expression
        =    - lead_comment [get_detailed(_r1) = _1]
        >>     (tok.identifier % ',') [get_name_set(_r1) = _1]
        >      define_expression(get_position(_r1), get_expression(_r1))
        >      end_statement(get_brief(_r1))
        ;

    named_decl
        =     tok.identifier [_r1 = _1]
        >     define_expression(_r2, _r3)
        >     end_statement(_r4)
        ;

    initializer
        =    ':'
        >>   next_pos [_r1 = _1]
        >    expression(_r2)
        ;

    define_expression
        =     tok.define
        >>    next_pos [_r1 = _1]
        >     expression(_r2)
        ;

    conditional
        =     when
        >     '('
        >>    next_pos [_r1 = _1]
        >     expression(_r2)
        >     ')'
        ;

    end_statement = ';' >> - trail_comment [_r1 = _1] ;

    lead_comment = tok.lead_comment [_val = strip_c_comment(_1)] ;
    trail_comment = tok.trail_comment [_val = strip_cpp_comment(_1)] ;

    // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
    NAME(interface_set_decl);
    NAME(constant_set_decl);
    NAME(logic_set_decl);
    NAME(interface_cell_decl);
    NAME(constant_cell_decl);
    NAME(logic_cell_decl);
    NAME(relate_decl);
    NAME(relate_expression);
    NAME(named_decl);
    NAME(initializer);
    NAME(define_expression);
    NAME(conditional);
    NAME(end_statement);
    NAME(lead_comment);
    NAME(trail_comment);
#undef NAME
}

template struct adam_eve_common_parser_rules_t<adam_callback_suite_t>;
template struct adam_eve_common_parser_rules_t<eve_callback_suite_t>;

} }
