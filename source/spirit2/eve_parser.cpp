#include <adobe/spirit2/eve_parser.hpp>

#include <adobe/spirit2/expr_parser.hpp>
#include <adobe/spirit2/adam_eve_common_parser.hpp>
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

    const expression_parser_rules_t& eve_expression_parser()
    {
        using boost::spirit::qi::token;
        using boost::spirit::qi::_1;
        using boost::spirit::qi::_val;

        lexer_t& tok = const_cast<lexer_t&>(eve_lexer());
        auto const initial_size = tok.keywords.size();
        (void)initial_size;
        static expression_parser_rules_t::keyword_rule_t eve_keywords =
              tok.keywords[interface_k][_val = _1]
            | tok.keywords[constant_k][_val = _1]
            | tok.keywords[layout_k][_val = _1]
            | tok.keywords[logic_k][_val = _1]
            | tok.keywords[relate_k][_val = _1]
            | tok.keywords[unlink_k][_val = _1]
            | tok.keywords[view_k][_val = _1]
            | tok.keywords[when_k][_val = _1]
            ;
        assert(tok.keywords.size() == initial_size);
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
        {
            boost::any retval;
            try {
                retval = callbacks.add_view_proc_m(std::forward<T>(args)...);
            } catch (std::exception const &) {}
            return retval;
        }
    };

    const boost::phoenix::function<add_view_t> add_view;

    struct nonempty_t
    {
        template <typename ...T>
        struct result
        { typedef bool type; };

        bool operator()(boost::any const & position) const
        { return !position.empty(); }
    };

    const boost::phoenix::function<nonempty_t> nonempty;

    struct finalize_sheet_t
    {
        template <typename ...T>
        struct result
        { typedef void type; };

        template <typename T>
        void operator()(T & callbacks) const
        {
            if (callbacks.finalize_sheet_proc_m)
                callbacks.finalize_sheet_proc_m();
        }
    };

    const boost::phoenix::function<finalize_sheet_t> finalize_sheet;

    struct eve_parser_rules_t
    {
        eve_parser_rules_t(const eve_callback_suite_t& callbacks_) :
            callbacks(callbacks_),
            common_rules(eve_lexer(), eve_expression_parser(), callbacks_)
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
            using qi::eps;

            lexer_t& tok = const_cast<lexer_t&>(eve_lexer());
            auto const initial_size = tok.keywords.size();
            (void)initial_size;
            const boost::spirit::lex::token_def<name_t>& layout = tok.keywords[layout_k];
            const boost::spirit::lex::token_def<name_t>& view = tok.keywords[view_k];
            assert(tok.keywords.size() == initial_size);

            auto const & interface_set_decl = common_rules.interface_set_decl;
            auto const & constant_set_decl = common_rules.constant_set_decl;
            auto const & logic_set_decl = common_rules.logic_set_decl;
            auto const & end_statement = common_rules.end_statement;
            auto const & lead_comment = common_rules.lead_comment;
            auto const & trail_comment = common_rules.trail_comment;

            const expression_parser_rules_t::local_size_rule_t& named_argument_list =
                eve_expression_parser().named_argument_list;

            using adobe::spirit2::detail::next_pos;

            // Note that the comments and layout name are currently ignored.
            layout_specifier
                =  - lead_comment
                >>   layout
                >    tok.identifier
                >    '{'
                >> * qualified_cell_decl
                >>   eps [finalize_sheet(callbacks)]
                >    view
                >    view_definition(_r1)
                >    '}'
                >> - trail_comment
                ;

            qualified_cell_decl
                =    interface_set_decl
                |    constant_set_decl
                |    logic_set_decl
                ;

            view_definition
                =  - lead_comment [_a = _1]
                >>   next_pos [_e = _1]
                >>   view_class_decl(_c, _d)
                >    (
                          end_statement(_b)
                          [
                              _f = add_view(callbacks, _r1, _e, _c, _d, _b, _a)
                          ]
                       |  (
                              - trail_comment [_b = _1]
                            >>  eps
                                [
                                    _f = add_view(callbacks, _r1, _e, _c, _d, _b, _a)
                                ]
                            >   view_statment_list(_f)
                          )
                     )
                >    eps(nonempty(_f))
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


            // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
            NAME(layout_specifier);
            NAME(qualified_cell_decl);
            NAME(view_definition);
            NAME(view_statment_sequence);
            NAME(view_class_decl);
            NAME(view_statment_list);
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

        const eve_callback_suite_t& callbacks;
        adam_eve_common_parser_rules_t<eve_callback_suite_t> common_rules;
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
