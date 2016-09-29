#ifndef ADOBE_SPIRIT2_EVE_PARSER_HPP
#define ADOBE_SPIRIT2_EVE_PARSER_HPP

#include <string>


namespace boost {
    class any;
}

namespace adobe {

    struct eve_callback_suite_t;

    namespace spirit2 {

        bool parse(const std::string& sheet,
                   const std::string& filename,
                   const boost::any& parent,
                   const eve_callback_suite_t& callbacks);

    }

}

#endif
