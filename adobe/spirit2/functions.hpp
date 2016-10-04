#ifndef ADOBE_SPIRIT2_FUNCTIONS_HPP
#define ADOBE_SPIRIT2_FUNCTIONS_HPP

#include <adobe/adam_function.hpp>

#include <boost/function.hpp>


namespace adobe { namespace spirit2 {

any_regular_t append(array_t const & parameters);
any_regular_t prepend(array_t const & parameters);
any_regular_t insert(array_t const & parameters);
any_regular_t erase(array_t const & parameters);
#if 0 // eval() is currently broken.
any_regular_t parse_(array_t const & parameters);
any_regular_t eval(sheet_t& sheet, array_t const & arguments);
#endif
any_regular_t size(array_t const & parameters);
any_regular_t join(array_t const & parameters);
any_regular_t split(array_t const & parameters);
any_regular_t to_string(array_t const & parameters);
any_regular_t to_name(array_t const & parameters);
any_regular_t print(array_t const & parameters);
any_regular_t assert_(array_t const & parameters);

typedef boost::function<any_regular_t (array_t const &)> array_function_t;
typedef boost::function<any_regular_t (dictionary_t const &)> dictionary_function_t;

typedef closed_hash_map<name_t, array_function_t > array_function_map_t;
typedef closed_hash_map<name_t, dictionary_function_t > dictionary_function_map_t;

// Higher order functions taking lookups or a sheet_t.

any_regular_t transform(
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    array_t const & arguments
);
any_regular_t fold(
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    array_t const & arguments
);
any_regular_t foldr(
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    array_t const & arguments
);

/** Adds all the other functions in this header to \a array_function_map;
    bakes in references to the other parameters. */
void add_predefined_functions(
    array_function_map_t & array_function_map,
    virtual_machine_t::array_function_lookup_t const & array_lookup,
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    sheet_t& sheet
);

} }

#endif
