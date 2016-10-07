#include <adobe/spirit2/func_parser.hpp>

// TODO #include <adobe/spirit2/func_writer.hpp>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

#include <fstream>
#include <iostream>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "spirit2_test_utils.hpp"


const char* g_input_file = "function_parser_test.fn";

using namespace adobe;

BOOST_AUTO_TEST_CASE( function_parser )
{
    std::string const file_contents = read_file(g_input_file);

    std::cout << "functions:\"\n" << file_contents << "\n\"\n"
              << "filename: " << g_input_file << '\n';

    adam_function_map_t functions;
    bool pass = spirit2::parse_functions(file_contents, g_input_file, functions);
    std::cout << (pass ? "PASS" : "FAIL") << "\n";

    std::cout << functions.size() << " functions\n";
    for (auto f : functions) {
        std::cout << "   function \"" << f.second.name() << "\"\n";
        for (auto s : f.second.statements()) {
            std::cout << "     statment: ";
            verbose_dump(s);
            std::cout << "\n";
        }

#if 0 // TODO
        std::cout << write_function(f.second) << "\n\n";
#endif
    }

    std::cout << "\n";

    BOOST_CHECK(pass);
}
