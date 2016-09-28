#ifndef ADOBE_SPIRIT2_REPORT_PARSE_ERROR_HPP
#define ADOBE_SPIRIT2_REPORT_PARSE_ERROR_HPP

#include <adobe/spirit2/lexer_fwd.hpp>

#include <boost/algorithm/string/replace.hpp>
#include <boost/tuple/tuple.hpp>


namespace adobe { namespace spirit2 {

    namespace detail {

        inline void default_send_error_string(const std::string& str)
        { std::cerr << str; }

        extern const char* s_filename;
        extern text_iterator_t* s_text_it;
        extern text_iterator_t s_begin;
        extern text_iterator_t s_end;

    }

    template <typename TokenType>
    struct report_error_t
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        void operator()(Arg1 first, Arg2, Arg3 it, Arg4 rule_name) const
            {
                std::string error_string;
                generate_error_string(first, it, rule_name, error_string);
                send_error_string(error_string);
            }

        static boost::function<void (const std::string&)> send_error_string;

    private:
        std::pair<text_iterator_t, unsigned int> line_start_and_line_number(text_iterator_t error_position) const
            {
                unsigned int line = 1;
                text_iterator_t it = detail::s_begin;
                text_iterator_t line_start = detail::s_begin;
                while (it != error_position) {
                    bool eol = false;
                    if (it != error_position && *it == '\r') {
                        eol = true;
                        line_start = ++it;
                    }
                    if (it != error_position && *it == '\n') {
                        eol = true;
                        line_start = ++it;
                    }
                    if (eol)
                        ++line;
                    else
                        ++it;
                }
                return std::pair<text_iterator_t, unsigned int>(line_start, line);
            }

        std::string get_line(text_iterator_t line_start) const
            {
                text_iterator_t line_end = line_start;
                while (line_end != detail::s_end && *line_end != '\r' && *line_end != '\n') {
                    ++line_end;
                }
                return std::string(line_start, line_end);
            }

        template <typename TokenIter>
        void generate_error_string(const TokenIter& first,
                                   const TokenIter& it,
                                   const boost::spirit::info& rule_name,
                                   std::string& str) const
            {
                std::stringstream is;

                text_iterator_t line_start;
                unsigned int line_number;
                text_iterator_t text_it = it->matched().begin();
                if (it->matched().begin() == it->matched().end()) {
                    text_it = *detail::s_text_it;
                    if (text_it != detail::s_end)
                        ++text_it;
                }
                boost::tie(line_start, line_number) = line_start_and_line_number(text_it);
                std::size_t column_number = std::distance(line_start, text_it);

                is << detail::s_filename << ":" << line_number << ":" << column_number << ": "
                   << "Parse error: expected " << rule_name;

                if (text_it == detail::s_end) {
                    is << " before end of input.\n";
                } else {
                    is << " here:\n"
                       << "  " << get_line(line_start) << "\n"
                       << "  " << std::string(column_number, ' ') << '^' << std::endl;
                }

                str = is.str();
            }
    };

    template <typename TokenType>
    boost::function<void (const std::string&)> report_error_t<TokenType>::send_error_string =
        &detail::default_send_error_string;

} }

#endif
