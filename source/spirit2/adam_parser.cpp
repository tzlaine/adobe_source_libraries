#include <adobe/spirit2/adam_parser.hpp>

#include <adobe/spirit2/expr_parser.hpp>
#include <adobe/array.hpp>
#include <adobe/dictionary.hpp>
#include <adobe/adam_parser.hpp>
#include <adobe/implementation/token.hpp>


// requirements of spirit::qi::debug()
namespace adobe {

    std::ostream& operator<<(std::ostream& os, const adam_callback_suite_t::relation_t& relation)
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

    std::ostream& operator<<(std::ostream& os, const std::vector<adam_callback_suite_t::relation_t>& relation_set)
    {
        return os; // TODO
    }

namespace spirit2 {

namespace {

    static_name_t input_k      = "input"_name;
    static_name_t output_k     = "output"_name;
    static_name_t interface_k  = "interface"_name;
    static_name_t logic_k      = "logic"_name;
    static_name_t constant_k   = "constant"_name;
    static_name_t invariant_k  = "invariant"_name;
    static_name_t external_k   = "external"_name;
    static_name_t sheet_k      = "sheet"_name;
    static_name_t unlink_k     = "unlink"_name;
    static_name_t when_k       = "when"_name;
    static_name_t relate_k     = "relate"_name;
    static_name_t if_k         = "if"_name;
    static_name_t else_k       = "else"_name;
    static_name_t for_k        = "for"_name;
    static_name_t continue_k   = "continue"_name;
    static_name_t break_k      = "break"_name;
    static_name_t return_k     = "return"_name;

#define GET_REF(type_, name)                                            \
    struct get_##name##_t                                               \
    {                                                                   \
        template <typename Arg1>                                        \
        struct result                                                   \
        { typedef type_ type; };                                        \
                                                                        \
        type_ operator()(adam_callback_suite_t::relation_t& relation) const \
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

    struct add_external_t
    {
        template <typename ...T>
        struct result
        { typedef void type; };

        template <typename T0, typename ...T>
        void operator()(T0 & callbacks, T &&... args) const
        { callbacks.add_external_proc_m(std::forward<T>(args)...); }
    };

    const boost::phoenix::function<add_external_t> add_external;

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

    struct adam_parser_rules_t
    {
        adam_parser_rules_t(const adam_callback_suite_t& callbacks_) :
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

            lexer_t& tok = const_cast<lexer_t&>(adam_lexer());
            assert(tok.keywords.size() == 17u);
            const boost::spirit::lex::token_def<name_t>& input = tok.keywords[input_k];
            const boost::spirit::lex::token_def<name_t>& output = tok.keywords[output_k];
            const boost::spirit::lex::token_def<name_t>& interface = tok.keywords[interface_k];
            const boost::spirit::lex::token_def<name_t>& logic = tok.keywords[logic_k];
            const boost::spirit::lex::token_def<name_t>& constant = tok.keywords[constant_k];
            const boost::spirit::lex::token_def<name_t>& invariant = tok.keywords[invariant_k];
            const boost::spirit::lex::token_def<name_t>& external = tok.keywords[external_k];
            const boost::spirit::lex::token_def<name_t>& sheet = tok.keywords[sheet_k];
            const boost::spirit::lex::token_def<name_t>& unlink = tok.keywords[unlink_k];
            const boost::spirit::lex::token_def<name_t>& when = tok.keywords[when_k];
            const boost::spirit::lex::token_def<name_t>& relate = tok.keywords[relate_k];
            assert(tok.keywords.size() == 17u);

            const expression_parser_rules_t::expression_rule_t& expression = adam_expression_parser();

            using adobe::spirit2::detail::next_pos;

            // note that the lead comment, sheet name, and trail comment are currently all ignored
            sheet_specifier
                =    - lead_comment
                >>     sheet
                >      tok.identifier
                >      '{'
                >>   * qualified_cell_decl
                >      '}'
                >>   - trail_comment
                ;

            qualified_cell_decl
                =      interface_set_decl
                |      input_set_decl
                |      output_set_decl
                |      constant_set_decl
                |      logic_set_decl
                |      invariant_set_decl
                |      external_set_decl
                ;

#define SET_DECL(name)                                          \
            name##_set_decl                                     \
                =      name                                     \
                >      ':'                                      \
                >>   * (                                        \
                            (                                   \
                                lead_comment [_a = _1]          \
                              | eps [clear(_a)]                 \
                            )                                   \
                         >> name##_cell_decl(_a)                \
                       )

            SET_DECL(interface);
            SET_DECL(input);
            SET_DECL(output);
            SET_DECL(constant);
            SET_DECL(logic);
            SET_DECL(invariant);

#undef SET_DECL

            external_set_decl
                =      external
                >      ':'
                >>   * (
                           - lead_comment [_a = _1]
                        >>   next_pos [_b = _1]
                        >>   (
                                  (
                                       tok.identifier [_c = _1]
                                   >   end_statement(_d) [add_external(callbacks, _c, _b, _d, _a)]
                                  )
                              |   eps
                             )
                       )
                ;

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

            input_cell_decl
                =     tok.identifier [_a = _1]
                >>  - initializer(_c, _b)
                >     end_statement(_d)
                      [
                          add_cell(callbacks, adam_callback_suite_t::input_k, _a, _c, _b, _d, _r1)
                      ]
                ;

            output_cell_decl
                =    named_decl(_a, _c, _b, _d)
                     [
                         add_cell(callbacks, adam_callback_suite_t::output_k, _a, _c, _b, _d, _r1)
                     ]
                ;

            constant_cell_decl
                =     tok.identifier [_a = _1]
                >     initializer(_c, _b)
                >     end_statement(_d)
                      [
                          add_cell(callbacks, adam_callback_suite_t::constant_k, _a, _c, _b, _d, _r1)
                      ]
                ;

            logic_cell_decl
                =     named_decl(_a, _c, _b, _d)
                      [
                          add_cell(callbacks, adam_callback_suite_t::logic_k, _a, _c, _b, _d, _r1)
                      ]
                |     relate_decl(_c, _b, _e, _d)
                      [
                          add_relation(callbacks, _c, _b, _e, _d, _r1)
                      ]
                ;

            invariant_cell_decl
                =     named_decl(_a, _c, _b, _d)
                      [
                          add_cell(callbacks, adam_callback_suite_t::invariant_k, _a, _c, _b, _d, _r1)
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

            // convenience rules
            lead_comment = tok.lead_comment [_val = strip_c_comment(_1)] ;
            trail_comment = tok.trail_comment [_val = strip_cpp_comment(_1)] ;

            // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
            NAME(sheet_specifier);
            NAME(qualified_cell_decl);
            NAME(interface_set_decl);
            NAME(input_set_decl);
            NAME(output_set_decl);
            NAME(constant_set_decl);
            NAME(logic_set_decl);
            NAME(invariant_set_decl);
            NAME(external_set_decl);
            NAME(interface_cell_decl);
            NAME(input_cell_decl);
            NAME(output_cell_decl);
            NAME(constant_cell_decl);
            NAME(logic_cell_decl);
            NAME(invariant_cell_decl);
            NAME(relate_decl);
            NAME(relate_expression);
            NAME(named_decl);
            NAME(initializer);
            NAME(define_expression);
            NAME(conditional);
            NAME(end_statement);
#undef NAME

            qi::on_error<qi::fail>(sheet_specifier, report_error(_1, _2, _3, _4));
        }

        typedef adam_callback_suite_t::relation_t relation_t;
        typedef std::vector<relation_t> relation_set_t;

        typedef boost::spirit::qi::rule<token_iterator_t, void(), skipper_type_t> void_rule_t;
        typedef boost::spirit::qi::rule<
            token_iterator_t,
            void(),
            boost::spirit::qi::locals<std::string>,
            skipper_type_t
        > cell_set_rule_t;
        typedef boost::spirit::qi::rule<
            token_iterator_t,
            void(const std::string&),
            boost::spirit::qi::locals<
                name_t,
                array_t,
                line_position_t,
                std::string
            >,
            skipper_type_t
        > cell_decl_rule_t;

        // Adam grammar
        void_rule_t sheet_specifier;
        void_rule_t qualified_cell_decl;

        cell_set_rule_t interface_set_decl;
        cell_set_rule_t input_set_decl;
        cell_set_rule_t output_set_decl;
        cell_set_rule_t constant_set_decl;
        cell_set_rule_t logic_set_decl;
        cell_set_rule_t invariant_set_decl;

        typedef boost::spirit::qi::rule<
            token_iterator_t,
            void(),
            boost::spirit::qi::locals<
                std::string,
                line_position_t,
                name_t,
                std::string
            >,
            skipper_type_t
        > external_set_decl_rule_t;
        external_set_decl_rule_t external_set_decl;

        boost::spirit::qi::rule<
            token_iterator_t,
            void(const std::string&),
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

        cell_decl_rule_t input_cell_decl;
        cell_decl_rule_t output_cell_decl;
        cell_decl_rule_t constant_cell_decl;

        boost::spirit::qi::rule<
            token_iterator_t,
            void(const std::string&),
            boost::spirit::qi::locals<
                name_t,
                array_t,
                line_position_t,
                std::string,
                relation_set_t
            >,
            skipper_type_t
        > logic_cell_decl;

        cell_decl_rule_t invariant_cell_decl;

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

        const adam_callback_suite_t& callbacks;
    };

}

const lexer_t& adam_lexer()
{
    static const name_t s_keywords[] = {
        input_k,
        output_k,
        interface_k,
        logic_k,
        constant_k,
        invariant_k,
        external_k,
        sheet_k,
        unlink_k,
        when_k,
        relate_k,
        if_k,
        else_k,
        for_k,
        continue_k,
        break_k,
        return_k
    };
    static const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

    static lexer_t s_lexer(s_keywords, s_keywords + s_num_keywords);

    return s_lexer;    
}

const expression_parser_rules_t::expression_rule_t& adam_expression_parser()
{
    using boost::spirit::qi::token;
    using boost::spirit::qi::_1;
    using boost::spirit::qi::_val;

    lexer_t& tok = const_cast<lexer_t&>(adam_lexer());
    assert(tok.keywords.size() == 17u);
    const boost::spirit::lex::token_def<name_t>& input = tok.keywords[input_k];
    const boost::spirit::lex::token_def<name_t>& output = tok.keywords[output_k];
    const boost::spirit::lex::token_def<name_t>& interface = tok.keywords[interface_k];
    const boost::spirit::lex::token_def<name_t>& logic = tok.keywords[logic_k];
    const boost::spirit::lex::token_def<name_t>& constant = tok.keywords[constant_k];
    const boost::spirit::lex::token_def<name_t>& invariant = tok.keywords[invariant_k];
    const boost::spirit::lex::token_def<name_t>& external = tok.keywords[external_k];
    const boost::spirit::lex::token_def<name_t>& sheet = tok.keywords[sheet_k];
    const boost::spirit::lex::token_def<name_t>& unlink = tok.keywords[unlink_k];
    const boost::spirit::lex::token_def<name_t>& when = tok.keywords[when_k];
    const boost::spirit::lex::token_def<name_t>& relate = tok.keywords[relate_k];
    const boost::spirit::lex::token_def<name_t>& if_ = tok.keywords[if_k];
    const boost::spirit::lex::token_def<name_t>& else_ = tok.keywords[else_k];
    const boost::spirit::lex::token_def<name_t>& for_ = tok.keywords[for_k];
    const boost::spirit::lex::token_def<name_t>& continue_ = tok.keywords[continue_k];
    const boost::spirit::lex::token_def<name_t>& break_ = tok.keywords[break_k];
    const boost::spirit::lex::token_def<name_t>& return_ = tok.keywords[return_k];
    assert(tok.keywords.size() == 17u);

    static expression_parser_rules_t::keyword_rule_t adam_keywords =
        input[_val = _1]
        | output[_val = _1]
        | interface[_val = _1]
        | logic[_val = _1]
        | constant[_val = _1]
        | invariant[_val = _1]
        | external[_val = _1]
        | sheet[_val = _1]
        | unlink[_val = _1]
        | when[_val = _1]
        | relate[_val = _1]
        | if_[_val = _1]
        | else_[_val = _1]
        | for_[_val = _1]
        | continue_[_val = _1]
        | break_[_val = _1]
        | return_[_val = _1]
        ;
    adam_keywords.name("keyword");

    static const expression_parser_rules_t s_parser(adam_lexer(), adam_keywords);

    return s_parser.expression;
}

bool parse(const std::string& sheet,
           const std::string& filename,
           const adam_callback_suite_t& callbacks)
{
    using boost::spirit::qi::phrase_parse;
    text_iterator_t it(sheet.begin());
    detail::s_text_it = &it;
    detail::s_begin = it;
    detail::s_end = text_iterator_t(sheet.end());
    detail::s_filename = filename.c_str();
    token_iterator_t iter = adam_lexer().begin(it, detail::s_end);
    token_iterator_t end = adam_lexer().end();
    adam_parser_rules_t adam_rules(callbacks);
    return phrase_parse(iter,
                        end,
                        adam_rules.sheet_specifier,
                        boost::spirit::qi::in_state("WS")[adam_lexer().self]);
}

array_t parse_adam_expression(const std::string& expr)
{
    array_t retval;
    using boost::spirit::qi::phrase_parse;
    text_iterator_t it(expr.begin());
    detail::s_text_it = &it;
    detail::s_begin = it;
    detail::s_end = text_iterator_t(expr.end());
    detail::s_filename = "adam expression";
    token_iterator_t iter = adam_lexer().begin(it, detail::s_end);
    token_iterator_t end = adam_lexer().end();
    const expression_parser_rules_t::expression_rule_t& expression = adam_expression_parser();
    phrase_parse(iter,
                 end,
                 expression(boost::phoenix::ref(retval)),
                 boost::spirit::qi::in_state("WS")[adam_lexer().self]);
    return retval;
}

} }
