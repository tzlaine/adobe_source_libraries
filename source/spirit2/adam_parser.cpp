#include <adobe/spirit2/adam_parser.hpp>

#include <adobe/spirit2/expr_parser.hpp>
#include <adobe/spirit2/adam_eve_common_parser.hpp>
#include <adobe/array.hpp>
#include <adobe/dictionary.hpp>
#include <adobe/adam_parser.hpp>
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

    struct adam_parser_rules_t
    {
        adam_parser_rules_t(const adam_callback_suite_t& callbacks_) :
            callbacks(callbacks_),
            common_rules(adam_lexer(), adam_expression_parser(), callbacks_)
        {
            namespace ascii = boost::spirit::ascii;
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;
            using phoenix::clear;
            using qi::_1;
            using qi::_2;
            using qi::_3;
            using qi::_4;
            using qi::_a;
            using qi::_b;
            using qi::_c;
            using qi::_d;
            using qi::_r1;
            using qi::eps;

            lexer_t& tok = const_cast<lexer_t&>(adam_lexer());
            auto const initial_size = tok.keywords.size();
            (void)initial_size;
            const boost::spirit::lex::token_def<name_t>& input = tok.keywords[input_k];
            const boost::spirit::lex::token_def<name_t>& output = tok.keywords[output_k];
            const boost::spirit::lex::token_def<name_t>& invariant = tok.keywords[invariant_k];
            const boost::spirit::lex::token_def<name_t>& external = tok.keywords[external_k];
            const boost::spirit::lex::token_def<name_t>& sheet = tok.keywords[sheet_k];
            assert(tok.keywords.size() == initial_size);

            auto const & interface_set_decl = common_rules.interface_set_decl;
            auto const & constant_set_decl = common_rules.constant_set_decl;
            auto const & logic_set_decl = common_rules.logic_set_decl;
            auto const & named_decl = common_rules.named_decl;
            auto const & initializer = common_rules.initializer;
            auto const & end_statement = common_rules.end_statement;
            auto const & lead_comment = common_rules.lead_comment;
            auto const & trail_comment = common_rules.trail_comment;

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

            SET_DECL(input);
            SET_DECL(output);
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

            invariant_cell_decl
                =     named_decl(_a, _c, _b, _d)
                      [
                          add_cell(callbacks, adam_callback_suite_t::invariant_k, _a, _c, _b, _d, _r1)
                      ]
                ;

            // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
            NAME(sheet_specifier);
            NAME(qualified_cell_decl);
            NAME(input_set_decl);
            NAME(output_set_decl);
            NAME(invariant_set_decl);
            NAME(external_set_decl);
            NAME(input_cell_decl);
            NAME(output_cell_decl);
            NAME(invariant_cell_decl);
#undef NAME

            qi::on_error<qi::fail>(sheet_specifier, report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<token_iterator_t, void(), skipper_type_t> void_rule_t;
        void_rule_t sheet_specifier;
        void_rule_t qualified_cell_decl;

        typedef adam_eve_common_parser_rules_t<adam_callback_suite_t>::cell_set_rule_t cell_set_rule_t;

        cell_set_rule_t input_set_decl;
        cell_set_rule_t output_set_decl;
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

        typedef adam_eve_common_parser_rules_t<adam_callback_suite_t>::cell_decl_rule_t cell_decl_rule_t;

        cell_decl_rule_t input_cell_decl;
        cell_decl_rule_t output_cell_decl;
        cell_decl_rule_t invariant_cell_decl;

        const adam_callback_suite_t& callbacks;
        adam_eve_common_parser_rules_t<adam_callback_suite_t> common_rules;
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

const expression_parser_rules_t& adam_expression_parser()
{
    using boost::spirit::qi::token;
    using boost::spirit::qi::_1;
    using boost::spirit::qi::_val;

    auto & tok = const_cast<lexer_t&>(adam_lexer());
    auto const initial_size = tok.keywords.size();
    (void)initial_size;
    static expression_parser_rules_t::keyword_rule_t adam_keywords =
        tok.keywords[input_k][_val = _1]
        | tok.keywords[output_k][_val = _1]
        | tok.keywords[interface_k][_val = _1]
        | tok.keywords[logic_k][_val = _1]
        | tok.keywords[constant_k][_val = _1]
        | tok.keywords[invariant_k][_val = _1]
        | tok.keywords[external_k][_val = _1]
        | tok.keywords[sheet_k][_val = _1]
        | tok.keywords[unlink_k][_val = _1]
        | tok.keywords[when_k][_val = _1]
        | tok.keywords[relate_k][_val = _1]
        | tok.keywords[if_k][_val = _1]
        | tok.keywords[else_k][_val = _1]
        | tok.keywords[for_k][_val = _1]
        | tok.keywords[continue_k][_val = _1]
        | tok.keywords[break_k][_val = _1]
        | tok.keywords[return_k][_val = _1]
        ;
    assert(tok.keywords.size() == initial_size);
    adam_keywords.name("keyword");

    static const expression_parser_rules_t s_parser(adam_lexer(), adam_keywords);

    return s_parser;
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
    const expression_parser_rules_t& expression_parser = adam_expression_parser();
    phrase_parse(iter,
                 end,
                 expression_parser.expression(boost::phoenix::ref(retval)),
                 boost::spirit::qi::in_state("WS")[adam_lexer().self]);
    return retval;
}

} }
