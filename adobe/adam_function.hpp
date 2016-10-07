#ifndef ADOBE_ADAM_FUNCTION_HPP
#define ADOBE_ADAM_FUNCTION_HPP

#include <adobe/adam.hpp>


namespace adobe {

class adam_function_t
{
public:
    typedef virtual_machine_t::dictionary_function_t dictionary_function_t;
    typedef virtual_machine_t::array_function_t array_function_t;
    typedef virtual_machine_t::dictionary_function_lookup_t dictionary_function_lookup_t;
    typedef virtual_machine_t::array_function_lookup_t array_function_lookup_t;
    typedef virtual_machine_t::adam_function_lookup_t adam_function_lookup_t;

    adam_function_t();
    adam_function_t(name_t name,
                    std::vector<name_t> const & parameter_names,
                    std::vector<array_t> const & statements);
    adam_function_t(name_t name,
                    dictionary_function_t const & dictionary_function,
                    array_function_t const & array_function);

    explicit operator bool() const;

    name_t name() const;
    std::vector<name_t> const & parameter_names() const;
    std::vector<array_t> const & statements() const;

    any_regular_t operator()(array_function_lookup_t const & array_function_lookup,
                             dictionary_function_lookup_t const & dictionary_function_lookup,
                             adam_function_lookup_t const & adam_function_lookup,
                             array_t const & parameters) const;
    any_regular_t operator()(array_function_lookup_t const & array_function_lookup,
                             dictionary_function_lookup_t const & dictionary_function_lookup,
                             adam_function_lookup_t const & adam_function_lookup,
                             dictionary_t const & parameters) const;

    inline friend bool operator==(adam_function_t const & lhs, adam_function_t const & rhs)
    {
        return
            lhs.name() == rhs.name() &&
            lhs.parameter_names() == rhs.parameter_names() &&
            lhs.statements() == rhs.statements() &&
            !lhs.dictionary_function_m == !rhs.dictionary_function_m &&
            !lhs.array_function_m == !rhs.array_function_m;
    }

private:
    name_t function_name_m;
    std::vector<name_t> parameter_names_m;
    std::vector<array_t> statements_m;
    dictionary_function_t dictionary_function_m;
    array_function_t array_function_m;
    bool use_std_functions_m;
};

inline bool operator!=(adam_function_t const & lhs, adam_function_t const & rhs)
{ return !(lhs == rhs); }

/** The map of functions used to evaluate user-defined Adam functions in Adam
    and Eve expressions. */
typedef closed_hash_map<name_t, adam_function_t> adam_function_map_t;

}

#endif
