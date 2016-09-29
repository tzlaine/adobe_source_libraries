#include <adobe/spirit2/eve_parser.hpp>

#include <adobe/eve_parser.hpp>
#include <adobe/array.hpp>
#include <adobe/dictionary.hpp>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

#include <fstream>
#include <iostream>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "spirit2_test_utils.hpp"


const char* g_examples_dir = "examples";

bool instrument_positions = false;

using namespace adobe;

struct store_add_view_params_t
{
    store_add_view_params_t(array_t& array, const std::string& str) :
        m_array(array),
        m_str(str)
    {}

    boost::any operator()(const boost::any& parent,
                          const line_position_t& parse_location,
                          name_t name,
                          const array_t& parameters,
                          const std::string& brief,
                          const std::string& detailed)
    {
        // Note that we are forced to ignore parent

        if (instrument_positions) {
            std::cerr << parse_location.stream_name() << ":"
                      << parse_location.line_number_m << ":"
                      << parse_location.line_start_m << ":"
                      << parse_location.position_m << ":";
            if (parse_location.line_start_m) {
                std::cerr << " \"" << std::string(m_str.begin() + parse_location.line_start_m,
                                                  m_str.begin() + parse_location.position_m)
                          << "\"";
            }
            std::cerr << "\n";
        } else {
            push_back(m_array, parse_location.stream_name());
            push_back(m_array, parse_location.line_number_m);
            push_back(m_array, std::size_t(parse_location.line_start_m));
            push_back(m_array, std::size_t(parse_location.position_m));
        }
        push_back(m_array, name);
        push_back(m_array, parameters);
        push_back(m_array, brief);
        push_back(m_array, detailed);

        return boost::any(1);
    }

    array_t& m_array;
    const std::string& m_str;
};

struct store_add_cell_params_t
{
    store_add_cell_params_t(array_t& array, const std::string& str) :
        m_array(array),
        m_str(str)
    {}

    void operator()(eve_callback_suite_t::cell_type_t type,
                    name_t name,
                    const line_position_t& position,
                    const array_t& initializer,
                    const std::string& brief,
                    const std::string& detailed)
    {
        std::string type_str;
        switch (type)
        {
        case eve_callback_suite_t::constant_k: type_str = "constant_k";
        case eve_callback_suite_t::interface_k: type_str = "interface_k";
        case eve_callback_suite_t::logic_k: type_str = "logic_k";
        }
        push_back(m_array, type_str);
        push_back(m_array, name);
        if (instrument_positions) {
            std::cerr << position.stream_name() << ":"
                      << position.line_number_m << ":"
                      << position.line_start_m << ":"
                      << position.position_m << ":";
            if (position.line_start_m) {
                std::cerr << " \"" << std::string(m_str.begin() + position.line_start_m,
                                                  m_str.begin() + position.position_m)
                          << "\"";
            }
            std::cerr << "\n";
        } else {
            push_back(m_array, position.stream_name());
            push_back(m_array, position.line_number_m);
            push_back(m_array, std::size_t(position.line_start_m));
            push_back(m_array, std::size_t(position.position_m));
        }
        push_back(m_array, initializer);
        push_back(m_array, brief);
        push_back(m_array, detailed);
    }

    array_t& m_array;
    const std::string& m_str;
};

struct store_add_relation_params_t
{
    store_add_relation_params_t(array_t& array, const std::string& str) :
        m_array(array),
        m_str(str)
    {}

    void operator()(const line_position_t& position,
                    const array_t& conditional,
                    const eve_callback_suite_t::relation_t* first,
                    const eve_callback_suite_t::relation_t* last,
                    const std::string& brief,
                    const std::string& detailed)
    {
        if (instrument_positions) {
            std::cerr << position.stream_name() << ":"
                      << position.line_number_m << ":"
                      << position.line_start_m << ":"
                      << position.position_m << ":";
            if (position.line_start_m) {
                std::cerr << " \"" << std::string(m_str.begin() + position.line_start_m,
                                                  m_str.begin() + position.position_m)
                          << "\"";
            }
            std::cerr << "\n";
        } else {
            push_back(m_array, position.stream_name());
            push_back(m_array, position.line_number_m);
            push_back(m_array, std::size_t(position.line_start_m));
            push_back(m_array, std::size_t(position.position_m));
        }
        push_back(m_array, conditional);
        while (first != last) {
            for (auto const & name : first->name_set_m) {
                push_back(m_array, name);
            }
            if (instrument_positions) {
                std::cerr << first->position_m.stream_name() << ":"
                          << first->position_m.line_number_m << ":"
                          << first->position_m.line_start_m << ":"
                          << first->position_m.position_m << ":";
                if (position.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + first->position_m.line_start_m,
                                                      m_str.begin() + first->position_m.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
                push_back(m_array, first->position_m.stream_name());
                push_back(m_array, first->position_m.line_number_m);
                push_back(m_array, std::size_t(first->position_m.line_start_m));
                push_back(m_array, std::size_t(first->position_m.position_m));
            }
            push_back(m_array, first->expression_m);
            push_back(m_array, first->detailed_m);
            push_back(m_array, first->brief_m);
            ++first;
        }
        push_back(m_array, brief);
        push_back(m_array, detailed);
    }

    array_t& m_array;
    const std::string& m_str;
};

struct store_add_interface_params_t
{
    store_add_interface_params_t(array_t& array, const std::string& str) :
        m_array(array),
        m_str(str)
    {}

    void operator()(name_t cell_name,
                    bool linked,
                    const line_position_t& position1,
                    const array_t& initializer,
                    const line_position_t& position2,
                    const array_t& expression,
                    const std::string& brief,
                    const std::string& detailed)
    {
        push_back(m_array, cell_name);
        push_back(m_array, linked);
        if (instrument_positions) {
            std::cerr << position1.stream_name() << ":"
                      << position1.line_number_m << ":"
                      << position1.line_start_m << ":"
                      << position1.position_m << ":";
            if (position1.line_start_m) {
                std::cerr << " \"" << std::string(m_str.begin() + position1.line_start_m,
                                                  m_str.begin() + position1.position_m)
                          << "\"";
            }
            std::cerr << "\n";
        } else {
            push_back(m_array, position1.stream_name());
            push_back(m_array, position1.line_number_m);
            push_back(m_array, std::size_t(position1.line_start_m));
            push_back(m_array, std::size_t(position1.position_m));
        }
        push_back(m_array, initializer);
        if (instrument_positions) {
            std::cerr << position2.stream_name() << ":"
                      << position2.line_number_m << ":"
                      << position2.line_start_m << ":"
                      << position2.position_m << ":";
            if (position2.line_start_m) {
                std::cerr << " \"" << std::string(m_str.begin() + position2.line_start_m,
                                                  m_str.begin() + position2.position_m)
                          << "\"";
            }
            std::cerr << "\n";
        } else {
            push_back(m_array, position2.stream_name());
            push_back(m_array, position2.line_number_m);
            push_back(m_array, std::size_t(position2.line_start_m));
            push_back(m_array, std::size_t(position2.position_m));
        }
        push_back(m_array, expression);
        push_back(m_array, brief);
        push_back(m_array, detailed);
    }

    array_t& m_array;
    const std::string& m_str;
};

struct store_finalize_sheet_called_t
{
    store_finalize_sheet_called_t(array_t& array) :
        m_array(array)
    {}

    void operator()()
    {
        push_back(m_array, "sheet_finalized"_name);
    }

    array_t& m_array;
};


BOOST_AUTO_TEST_CASE( eve_parser )
{
    bfs::path dir(g_examples_dir);
    bfs::directory_iterator const end_it;
    for (bfs::directory_iterator it(dir); it != end_it; ++it) {
        std::string filename = it->path().string();
        std::string const file_contents = read_file(filename);

        array_t new_parse;
        array_t old_parse;

        eve_callback_suite_t old_parse_callbacks;
        old_parse_callbacks.add_view_proc_m = store_add_view_params_t(old_parse, file_contents);
        old_parse_callbacks.add_cell_proc_m = store_add_cell_params_t(old_parse, file_contents);
        old_parse_callbacks.add_relation_proc_m = store_add_relation_params_t(old_parse, file_contents);
        old_parse_callbacks.finalize_sheet_proc_m = store_finalize_sheet_called_t(old_parse);
        old_parse_callbacks.add_interface_proc_m = store_add_interface_params_t(old_parse, file_contents);

        eve_callback_suite_t new_parse_callbacks;
        new_parse_callbacks.add_view_proc_m = store_add_view_params_t(new_parse, file_contents);
        new_parse_callbacks.add_cell_proc_m = store_add_cell_params_t(new_parse, file_contents);
        new_parse_callbacks.add_relation_proc_m = store_add_relation_params_t(new_parse, file_contents);
        new_parse_callbacks.finalize_sheet_proc_m = store_finalize_sheet_called_t(new_parse);
        new_parse_callbacks.add_interface_proc_m = store_add_interface_params_t(new_parse, file_contents);

        std::cout << "layout:\"\n" << file_contents << "\n\"\n"
                  << "filename: " << filename << '\n';
        bool original_parse_failed = false;
        try {
            std::stringstream ss(file_contents);
            parse(ss, line_position_t(filename.c_str()), boost::any(), old_parse_callbacks);
        } catch (const stream_error_t& e) {
            original_parse_failed = true;
        }
        if (original_parse_failed)
            std::cout << "original: <parse failure>\n";
        else
            std::cout << "original: " << old_parse << "\n";
        bool new_parse_failed = !spirit2::parse(file_contents, filename, boost::any(), new_parse_callbacks);
        if (new_parse_failed)
            std::cout << "new:      <parse failure>\n";
        else
            std::cout << "new:      " << new_parse << "\n";
        bool pass =
            original_parse_failed && new_parse_failed ||
            new_parse == old_parse;
        std::cout << (pass ? "PASS" : "FAIL") << "\n";

        if (!pass) {
            std::cout << "original (verbose):\n";
            verbose_dump(old_parse);
            std::cout << "new (verbose):\n";
            verbose_dump(new_parse);
        }

        std::cout << "\n";

        BOOST_CHECK(pass);
    }
}