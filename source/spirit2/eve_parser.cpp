#include <adobe/spirit2/eve_parser.hpp>

#include <adobe/spirit2/expr_parser.hpp>
#include <adobe/array.hpp>
#include <adobe/dictionary.hpp>
#include <adobe/eve_parser.hpp>
#include <adobe/implementation/token.hpp>


namespace adobe { namespace spirit2 {

namespace {

    static_name_t interface_k  = "interface"_name;
    static_name_t constant_k   = "constant"_name;
    static_name_t layout_k     = "layout"_name;
    static_name_t logic_k      = "logic"_name;
    static_name_t relate_k     = "relate"_name;
    static_name_t unlink_k     = "unlink"_name;
    static_name_t view_k       = "view"_name;
    static_name_t when_k       = "when"_name;

    const lexer_t& eve_lexer()
    {
        static const name_t s_keywords[] = {
            interface_k,
            constant_k,
            layout_k,
            logic_k,
            relate_k,
            unlink_k,
            view_k,
            when_k
        };
        static const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

        static lexer_t s_lexer(s_keywords, s_keywords + s_num_keywords);

        return s_lexer;    
    }

    const expression_parser_rules_t& eve_expression_parser_rules()
    {
        using boost::spirit::qi::token;
        using boost::spirit::qi::_1;
        using boost::spirit::qi::_val;

        lexer_t& tok = const_cast<lexer_t&>(eve_lexer());
        assert(tok.keywords.size() == 8u);
        const boost::spirit::lex::token_def<name_t>& interface = tok.keywords[interface_k];
        const boost::spirit::lex::token_def<name_t>& constant = tok.keywords[constant_k];
        const boost::spirit::lex::token_def<name_t>& layout = tok.keywords[layout_k];
        const boost::spirit::lex::token_def<name_t>& logic = tok.keywords[logic_k];
        const boost::spirit::lex::token_def<name_t>& relate = tok.keywords[relate_k];
        const boost::spirit::lex::token_def<name_t>& unlink = tok.keywords[unlink_k];
        const boost::spirit::lex::token_def<name_t>& view = tok.keywords[view_k];
        const boost::spirit::lex::token_def<name_t>& when = tok.keywords[when_k];
        assert(tok.keywords.size() == 8u);

        static expression_parser_rules_t::keyword_rule_t eve_keywords =
              interface[_val = _1]
            | constant[_val = _1]
            | layout[_val = _1]
            | logic[_val = _1]
            | relate[_val = _1]
            | unlink[_val = _1]
            | view[_val = _1]
            | when[_val = _1]
            ;
        eve_keywords.name("keyword");

        static const expression_parser_rules_t s_parser(eve_lexer(), eve_keywords);

        return s_parser;
    }

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

    struct add_view_t
    {
        template <typename ...T>
        struct result
        { typedef boost::any type; };

        template <typename T0, typename ...T>
        boost::any operator()(T0 & callbacks, T &&... args) const
        { return callbacks.add_view_proc_m(std::forward<T>(args)...); }
    };

    const boost::phoenix::function<add_view_t> add_view;

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

    /*
        TODO: Make common rules for:
        interface_set_decl      = "interface"   ":" { [lead_comment] interface_cell_decl }.
        constant_set_decl       = "constant"    ":" { [lead_comment] constant_cell_decl }.
        logic_set_decl          = "logic"       ":" { [lead_comment] logic_cell_decl }.
        interface_cell_decl     = ["unlink"] identifier [initializer] [define_expression] end_statement.
        constant_cell_decl      = identifier initializer end_statement.
        logic_cell_decl         = named_decl | relate_decl.
        relate_decl             = [conditional] "relate" "{" relate_expression relate_expression
                                      { relate_expression } "}" [trail_comment].

        ... to be shared between adam and eve parsers.
    */

    struct eve_parser_rules_t
    {
        eve_parser_rules_t(const eve_callback_suite_t& callbacks_) :
            callbacks(callbacks_)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;
            using phoenix::push_back;
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
            using qi::_r1;
            using qi::_r2;
            using qi::_val;
            using qi::eps;

            lexer_t& tok = const_cast<lexer_t&>(eve_lexer());
            assert(tok.keywords.size() == 8u);
            const boost::spirit::lex::token_def<name_t>& interface = tok.keywords[interface_k];
            const boost::spirit::lex::token_def<name_t>& constant = tok.keywords[constant_k];
            const boost::spirit::lex::token_def<name_t>& layout = tok.keywords[layout_k];
            const boost::spirit::lex::token_def<name_t>& logic = tok.keywords[logic_k];
            const boost::spirit::lex::token_def<name_t>& relate = tok.keywords[relate_k];
            const boost::spirit::lex::token_def<name_t>& unlink = tok.keywords[unlink_k];
            const boost::spirit::lex::token_def<name_t>& view = tok.keywords[view_k];
            const boost::spirit::lex::token_def<name_t>& when = tok.keywords[when_k];
            assert(tok.keywords.size() == 8u);

            const expression_parser_rules_t::expression_rule_t& expression = eve_expression_parser_rules().expression;
            const expression_parser_rules_t::local_size_rule_t& named_argument_list = eve_expression_parser_rules().named_argument_list;

            using adobe::spirit2::detail::next_pos;

            // Note that the comments and layout name are currently ignored.
            layout_specifier
                =  - lead_comment
                >>   layout
                >    tok.identifier
                >    '{'
                >> * qualified_cell_decl // TODO: if (assembler_m.finalize_sheet_proc_m) assembler_m.finalize_sheet_proc_m();
                >    view
                >    view_definition(_r1)
                >    '}'
                >> - trail_comment
                ;

            qualified_cell_decl
                =    interface_set_decl
                |    constant_set_decl
//                |    logic_set_decl
                ;

            // TODO: Common cell parsing...
            // interface_set_decl
            // constant_set_decl
            // logic_set_decl
            // interface_cell_decl
            // constant_cell_decl
            // logic_cell_decl
            // relate_decl
            // relate_expression_decl
            // named_decl
            // define_expression
            // conditional

            interface_set_decl
                =    interface
                >    ':'
                >  * cell_decl(eve_callback_suite_t::interface_k)
                ;

            constant_set_decl
                =    constant
                >    ':'
                >  * cell_decl(eve_callback_suite_t::constant_k)
                ;

            // TODO: Remove.
            cell_decl
                =    (
                          - lead_comment [_a = _1]
                       >>   tok.identifier [_c = _1]
                       >    initializer(_d, _e)
                       >    end_statement(_b)
                     )
                     [
                         add_cell(callbacks, _r1, _c, _d, _e, _b, _a)
                     ]
                ;

            initializer
                =    ':'
                >>   next_pos [_r1 = _1]
                >    expression(_r2)
                ;

            view_definition
                =  - lead_comment [_a = _1]
                >>   next_pos [_e = _1]
                >>   view_class_decl(_c, _d)
                >    (
                          end_statement(_b)
                          [
                              _f = add_view(callbacks, _r1, _e, _c, _d, _b, _a) // TODO try/catch?
                          ]
                       |  (
                              - trail_comment [_b = _1]
                            >>  eps
                                [
                                    _f = add_view(callbacks, _r1, _e, _c, _d, _b, _a) // TODO try/catch?
                                ]
                            >   view_statment_list(_f)
                          )
                     )
                ;

            view_statment_sequence
                =  * view_definition(_r1)
                ;

            view_class_decl
                =     tok.identifier [_r1 = _1]
                >     '('
                >>    (
                           named_argument_list(_r2)
                        |  eps [push_back(_r2, any_regular_t(dictionary_t()))]
                      )
                >     ')'
                ;

            view_statment_list
                =     '{'
                >>    view_statment_sequence(_r1)
                >     '}'
                ;

            end_statement = ';' >> - trail_comment [_r1 = _1] ;

            // convenience rules
            lead_comment = tok.lead_comment [_val = strip_c_comment(_1)] ;
            trail_comment = tok.trail_comment [_val = strip_cpp_comment(_1)] ;

            // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
            NAME(layout_specifier);
            NAME(qualified_cell_decl);
            NAME(interface_set_decl);
            NAME(constant_set_decl);
            NAME(cell_decl);
            NAME(initializer);
            NAME(view_definition);
            NAME(view_statment_sequence);
            NAME(view_class_decl);
            NAME(view_statment_list);
            NAME(end_statement);
#undef NAME

            qi::on_error<qi::fail>(layout_specifier, report_error(_1, _2, _3, _4));
        }

        typedef eve_callback_suite_t::cell_type_t cell_type_t;

        typedef boost::spirit::qi::rule<token_iterator_t, void(), skipper_type_t> void_rule_t;

        typedef boost::spirit::qi::rule<
            token_iterator_t,
            void(const boost::any&),
            skipper_type_t
        > position_rule_t;

        position_rule_t layout_specifier;

        void_rule_t qualified_cell_decl;
        void_rule_t interface_set_decl;
        void_rule_t constant_set_decl;

        boost::spirit::qi::rule<
            token_iterator_t,
            void(cell_type_t),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                name_t,
                line_position_t,
                array_t
            >,
            skipper_type_t
        > cell_decl;

        boost::spirit::qi::rule<
            token_iterator_t,
            void(line_position_t&, array_t&),
            skipper_type_t
        > initializer;

        boost::spirit::qi::rule<
            token_iterator_t,
            void(const boost::any&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                name_t,
                array_t,
                line_position_t,
                boost::any
            >,
            skipper_type_t
        > view_definition;

        position_rule_t view_statment_sequence;

        boost::spirit::qi::rule<
            token_iterator_t,
            void(name_t&, array_t&),
            skipper_type_t
        > view_class_decl;

        position_rule_t view_statment_list;

        boost::spirit::qi::rule<token_iterator_t, void(std::string&), skipper_type_t> end_statement;

        boost::spirit::qi::rule<token_iterator_t, std::string(), skipper_type_t> lead_comment;
        boost::spirit::qi::rule<token_iterator_t, std::string(), skipper_type_t> trail_comment;

        const eve_callback_suite_t& callbacks;
    };

}

bool parse(const std::string& layout,
           const std::string& filename,
           const boost::any& parent,
           const eve_callback_suite_t& callbacks)
{
    using boost::spirit::qi::phrase_parse;
    text_iterator_t it(layout.begin());
    detail::s_text_it = &it;
    detail::s_begin = it;
    detail::s_end = text_iterator_t(layout.end());
    detail::s_filename = filename.c_str();
    token_iterator_t iter = eve_lexer().begin(it, detail::s_end);
    token_iterator_t end = eve_lexer().end();
    eve_parser_rules_t eve_rules(callbacks);
    return phrase_parse(iter,
                        end,
                        eve_rules.layout_specifier(boost::phoenix::cref(parent)),
                        boost::spirit::qi::in_state("WS")[eve_lexer().self]);
}

} }
