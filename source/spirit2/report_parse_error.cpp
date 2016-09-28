#include <adobe/spirit2/report_parse_error.hpp>


namespace adobe { namespace spirit2 {

    namespace detail {

        const char* s_filename = 0;
        text_iterator_t* s_text_it;
        text_iterator_t s_begin;
        text_iterator_t s_end;

    }

} }
