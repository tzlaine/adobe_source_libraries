#include <adobe/spirit2/builtin_functions.hpp>

#include <adobe/array.hpp>
#include <adobe/dictionary.hpp>
#include <adobe/spirit2/adam_parser.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>


namespace adobe { namespace spirit2 {

namespace {

static_name_t key_k = "key"_name;
static_name_t value_k = "value"_name;
static_name_t state_k = "state"_name;

std::string to_string_impl(const any_regular_t& value, bool round_trippable)
{
    std::stringstream result;
    if (value == any_regular_t()) {
        result << "empty";
    } else {
        auto const type(value.type_info());
        if (type == boost::typeindex::type_id<double>()) {
            result << value;
        } else if (type == boost::typeindex::type_id<bool>()) {
            result << (value.cast<bool>() ? "true" : "false");
        } else if (type == boost::typeindex::type_id<name_t>()) {
            result << (round_trippable ? "@" : "") << value;
        } else if (type == boost::typeindex::type_id<array_t>()) {
            result << '[';
            array_t const & array = value.cast<array_t>();
            for (auto it = array.begin(), end_it = array.end();
                 it != end_it;
                 ++it) {
                result << to_string_impl(*it, round_trippable);
                if (boost::next(it) != end_it)
                    result << ", ";
            }
            result << ']';
        } else if (type == boost::typeindex::type_id<dictionary_t>()) {
            result << '{';
            dictionary_t const & dictionary = value.cast<dictionary_t>();
            for (auto it = dictionary.begin(), end_it = dictionary.end();
                 it != end_it;
                 ++it) {
                result << it->first << ": " << to_string_impl(it->second, round_trippable);
                if (boost::next(it) != end_it)
                    result << ", ";
            }
            result << '}';
        } else {
            std::string str = value.cast<std::string>();
            if (round_trippable) {
                char const quote = str.find('\'') != std::string::npos ? '"' : '\'';
                result << quote << str << quote;
            } else {
                result << str;
            }
        }
    }
    return result.str();
}

}

any_regular_t append(array_t const & parameters)
{
    if (parameters.size() < 1u)
        throw std::runtime_error("append() requires at least 1 parameter");
    array_t array = parameters[0].cast<array_t>();
    array.insert(array.end(),
                 boost::next(parameters.begin()),
                 parameters.end());
    return any_regular_t(array);
}

any_regular_t prepend(array_t const & parameters)
{
    if (parameters.size() < 1u)
        throw std::runtime_error("prepend() requires at least 1 parameter");
    array_t array(boost::next(parameters.begin()), parameters.end());
    array.insert(array.end(),
                 parameters[0].cast<array_t>().begin(),
                 parameters[0].cast<array_t>().end());
    return any_regular_t(array);
}

any_regular_t insert(array_t const & parameters)
{
    if (parameters.size() < 1u)
        throw std::runtime_error("insert() requires at least 1 parameter");
    if (parameters[0].type_info() == boost::typeindex::type_id<array_t>()) {
        if (parameters.size() < 2u)
            throw std::runtime_error("insert(array_t, i, ...) requires at least 2 parameters");
        array_t array = parameters[0].cast<array_t>();
        int index;
        if (!parameters[1].cast(index))
            throw std::runtime_error("insert(array_t, i, ...) requires a number as its second parameter");
        else if (array.size() < index)
            throw std::runtime_error("index i passed to insert(array_t, i, ...) is out of bounds");
        array.insert(array.begin() + index,
                     boost::next(parameters.begin(), 2),
                     parameters.end());
        return any_regular_t(array);
    } else if (parameters[0].type_info() == boost::typeindex::type_id<dictionary_t>()) {
        if (parameters.size() < 2u)
            throw std::runtime_error("insert(dictionary_t, ...) requires at least 2 parameters");
        dictionary_t dictionary = parameters[0].cast<dictionary_t>();
        name_t key;
        if (parameters[1].cast(key)) {
            if (parameters.size() < 3u)
                throw std::runtime_error("insert(dictionary_t, key, value) requires 3 parameters");
            dictionary.insert(std::make_pair(key, parameters[2]));
        } else {
            if (parameters[1].type_info() != boost::typeindex::type_id<dictionary_t>())
                throw std::runtime_error("insert(dictionary_t, {...}) requires a dictionary at its second parameter");
            dictionary.insert(parameters[1].cast<dictionary_t>().begin(),
                              parameters[1].cast<dictionary_t>().end());
        }
        return any_regular_t(dictionary);
    } else {
        throw std::runtime_error("insert() requires an array or dictionary as its first parameter");
    }
}

any_regular_t erase(array_t const & parameters)
{
    if (parameters.size() < 1u)
        throw std::runtime_error("erase() requires at least 1 parameter");
    if (parameters[0].type_info() == boost::typeindex::type_id<array_t>()) {
        if (parameters.size() < 2u)
            throw std::runtime_error("erase(array_t, i) requires at least 2 parameters");
        array_t array = parameters[0].cast<array_t>();
        int index;
        if (!parameters[1].cast(index))
            throw std::runtime_error("erase(array_t, i) requires a number as its second parameter");
        else if (array.size() <= index)
            throw std::runtime_error("index i passed to erase(array_t, i) is out of bounds");
        array.erase(array.begin() + index);
        return any_regular_t(array);
    } else if (parameters[0].type_info() == boost::typeindex::type_id<dictionary_t>()) {
        if (parameters.size() < 2u)
            throw std::runtime_error("erase(dictionary_t, ...) requires at least 2 parameters");
        dictionary_t dictionary = parameters[0].cast<dictionary_t>();
        for (std::size_t i = 1; i < parameters.size(); ++i) {
            name_t key;
            if (!parameters[i].cast(key))
                throw std::runtime_error("erase(dictionary_t, ...) requires names for all its parameters after the first");
            dictionary.erase(key);
        }
        return any_regular_t(dictionary);
    } else {
        throw std::runtime_error("erase() requires an array or dictionary as its first parameter");
    }
}

#if 0 // TODO eval() is currently broken.
any_regular_t parse_(array_t const & parameters)
{
    if (parameters.size() != 1u)
        throw std::runtime_error("parse() requires 1 parameter");
    std::string string;
    if (!parameters[0].cast(string))
        throw std::runtime_error("parse() requires a string as its first parameter");
    try {
        return any_regular_t(parse_adam_expression(string));
    } catch (...) {
        return any_regular_t(array_t());
    }
}

any_regular_t eval(sheet_t& sheet, array_t const & arguments)
{
    any_regular_t retval;

    if (arguments.size() != 1u)
        throw std::runtime_error("eval() takes exactly 1 argument; " + boost::lexical_cast<std::string>(arguments.size()) + " given.");

    if (arguments[0].type_info() != boost::typeindex::type_id<array_t>())
        throw std::runtime_error("The argument to eval() must be an array, such as is returned by parse().");

    try {
        retval = sheet.inspect(arguments[0].cast<array_t>());
    } catch (...) {}

    return retval;
}
#endif

any_regular_t size(array_t const & parameters)
{
    if (parameters.size() != 1u)
        throw std::runtime_error("size() requires 1 parameter");
    if (parameters[0].type_info() == boost::typeindex::type_id<array_t>())
        return any_regular_t(double(parameters[0].cast<array_t>().size()));
    else if (parameters[0].type_info() == boost::typeindex::type_id<dictionary_t>())
        return any_regular_t(double(parameters[0].cast<dictionary_t>().size()));
    else
        throw std::runtime_error("size() requires an array or dictionary as its first parameter");
}

any_regular_t join(array_t const & parameters)
{
    if (parameters.empty())
        throw std::runtime_error("join() requires at least 1 parameter");
    if (parameters[0].type_info() != boost::typeindex::type_id<array_t>())
        throw std::runtime_error("join() requires an array of strings as its first parameter");

    std::string retval;
    std::string join_str;
    if (2u <= parameters.size()) {
        if (parameters[1].type_info() != boost::typeindex::type_id<std::string>())
            throw std::runtime_error("The second parameter to join() must be a string, if provided");
        join_str = parameters[1].cast<std::string>();
    }

    array_t const & array = parameters[0].cast<array_t>();
    for (std::size_t i = 0; i < array.size(); ++i) {
        if (array[i].type_info() != boost::typeindex::type_id<std::string>())
            throw std::runtime_error("join() requires an array of strings as its first parameter");
        retval += array[i].cast<std::string>();
        if (i < array.size() - 1)
            retval += join_str;
    }

    return any_regular_t(retval);
}

any_regular_t split(array_t const & parameters)
{
    if (parameters.size() != 2u)
        throw std::runtime_error("split() requires 2 parameters");
    if (parameters[0].type_info() != boost::typeindex::type_id<std::string>())
        throw std::runtime_error("split() requires a string as its first parameter");
    if (parameters[1].type_info() != boost::typeindex::type_id<std::string>())
        throw std::runtime_error("split() requires a string as its second parameter");

    std::string str = parameters[0].cast<std::string>();
    std::string split_str = parameters[1].cast<std::string>();

    if (split_str.empty())
        throw std::runtime_error("split() requires a nonempty string as its second parameter");

    std::vector<std::string> split_vec;
    boost::algorithm::split(split_vec, str, boost::algorithm::is_any_of(split_str));

    array_t retval;
    for (std::size_t i = 0; i < split_vec.size(); ++i) {
        push_back(retval, split_vec[i]);
    }

    return any_regular_t(retval);
}

any_regular_t to_string(array_t const & parameters)
{
    if (parameters.size() < 1u || 2u < parameters.size())
        throw std::runtime_error("to_string() requires 1 or 2 parameters");

    bool round_trippable = false;
    if (1u < parameters.size() && !parameters[1].cast(round_trippable))
        throw std::runtime_error("to_string() requires a bool as its second parameter");

    return any_regular_t(to_string_impl(parameters[0], round_trippable));
}

any_regular_t to_name(array_t const & parameters)
{
    if (parameters.size() != 1u)
        throw std::runtime_error("to_name() requires 1 parameter");

    return any_regular_t(name_t(to_string_impl(parameters[0], false).c_str()));
}

any_regular_t print(array_t const & parameters)
{
    for (std::size_t i = 0; i < parameters.size(); ++i) {
        if (i)
            std::cout << ' ';
        std::cout << to_string_impl(parameters[i], true);
    }
    std::cout << std::endl;
    return any_regular_t();
}

any_regular_t assert_(array_t const & parameters)
{
    if (parameters.size() != 2u)
        throw std::runtime_error("assert() requires 2 parameters");
    if (parameters[0].type_info() != boost::typeindex::type_id<bool>())
        throw std::runtime_error("assert() requires a bool as its first parameter");
    if (parameters[1].type_info() != boost::typeindex::type_id<std::string>())
        throw std::runtime_error("assert() requires a string as its second parameter");
    if (!parameters[0].cast<bool>())
        throw std::runtime_error("Failed assertion! \"" + parameters[1].cast<std::string>() + "\"");
    return any_regular_t();
}

any_regular_t transform(
    virtual_machine_t::array_function_lookup_t const & array_lookup,
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    array_t const & arguments
) {
    any_regular_t retval;

    if (arguments.size() != 2u)
        throw std::runtime_error("transform() takes exactly 2 arguments; " + boost::lexical_cast<std::string>(arguments.size()) + " given.");

    name_t f;
    if (!arguments[1].cast(f))
        throw std::runtime_error("The second argument to transform() must be the name of a function.");

    dictionary_t f_arguments;

    if (arguments[0].type_info() == boost::typeindex::type_id<dictionary_t>()) {
        dictionary_t const & sequence = arguments[0].cast<dictionary_t>();
        retval = any_regular_t(dictionary_t());
        dictionary_t& result_elements = retval.cast<dictionary_t>();
        for (auto it = sequence.begin(), end_it = sequence.end();
             it != end_it;
             ++it) {
            f_arguments[key_k] = any_regular_t(it->first);
            f_arguments[value_k] = it->second;
            auto const adam_function = adam_lookup(f);
            if (adam_function)
                result_elements[it->first] = (*adam_function)(array_lookup, dictionary_lookup, adam_lookup, f_arguments);
            else
                result_elements[it->first] = dictionary_lookup(f, f_arguments);
        }
    } else if (arguments[0].type_info() == boost::typeindex::type_id<array_t>()) {
        array_t const & sequence = arguments[0].cast<array_t>();
        retval = any_regular_t(array_t());
        array_t& result_elements = retval.cast<array_t>();
        result_elements.reserve(sequence.size());
        for (auto it = sequence.begin(), end_it = sequence.end();
             it != end_it;
             ++it) {
            f_arguments[value_k] = *it;
            any_regular_t result;
            auto const adam_function = adam_lookup(f);
            if (adam_function)
                result = (*adam_function)(array_lookup, dictionary_lookup, adam_lookup, f_arguments);
            else
                result = dictionary_lookup(f, f_arguments);
            result_elements.push_back(result);
        }
    } else {
        f_arguments[value_k] = arguments[0];
        auto const adam_function = adam_lookup(f);
        if (adam_function)
            retval = (*adam_function)(array_lookup, dictionary_lookup, adam_lookup, f_arguments);
        else
            retval = dictionary_lookup(f, f_arguments);
    }

    return retval;
}

namespace {

template <typename Iter>
void fold_dictionary_impl(
    virtual_machine_t::array_function_lookup_t const & array_lookup,
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    name_t f,
    Iter it,
    Iter end_it,
    any_regular_t& retval
) {
    dictionary_t f_arguments;
    for (; it != end_it; ++it) {
        f_arguments[state_k] = retval;
        f_arguments[key_k] = any_regular_t(it->first);
        f_arguments[value_k] = it->second;
        auto const adam_function = adam_lookup(f);
        if (adam_function)
            retval = (*adam_function)(array_lookup, dictionary_lookup, adam_lookup, f_arguments);
        else
            retval = dictionary_lookup(f, f_arguments);
    }
}

template <typename Iter>
void fold_array_impl(
    virtual_machine_t::array_function_lookup_t const & array_lookup,
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    name_t f,
    Iter it,
    Iter end_it,
    any_regular_t& retval
) {
    dictionary_t f_arguments;
    for (; it != end_it; ++it) {
        f_arguments[state_k] = retval;
        f_arguments[value_k] = *it;
        auto const adam_function = adam_lookup(f);
        if (adam_function)
            retval = (*adam_function)(array_lookup, dictionary_lookup, adam_lookup, f_arguments);
        else
            retval = dictionary_lookup(f, f_arguments);
    }
}

}

any_regular_t fold(
    virtual_machine_t::array_function_lookup_t const & array_lookup,
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    array_t const & arguments
) {
    any_regular_t retval;

    if (arguments.size() != 3u)
        throw std::runtime_error("fold() takes exactly 3 arguments; " + boost::lexical_cast<std::string>(arguments.size()) + " given.");

    name_t f;
    if (!arguments[2].cast(f))
        throw std::runtime_error("The third argument to fold() must be the name of a function.");

    retval = arguments[1];

    if (arguments[0].type_info() == boost::typeindex::type_id<dictionary_t>()) {
        dictionary_t const & sequence = arguments[0].cast<dictionary_t>();
        fold_dictionary_impl(array_lookup, dictionary_lookup, adam_lookup, f, sequence.begin(), sequence.end(), retval);
    } else if (arguments[0].type_info() == boost::typeindex::type_id<array_t>()) {
        array_t const & sequence = arguments[0].cast<array_t>();
        fold_array_impl(array_lookup, dictionary_lookup, adam_lookup, f, sequence.begin(), sequence.end(), retval);
    } else {
        dictionary_t f_arguments;
        f_arguments[state_k] = retval;
        f_arguments[value_k] = arguments[0];
        auto const adam_function = adam_lookup(f);
        if (adam_function)
            retval = (*adam_function)(array_lookup, dictionary_lookup, adam_lookup, f_arguments);
        else
            retval = dictionary_lookup(f, f_arguments);
    }

    return retval;
}

any_regular_t foldr(
    virtual_machine_t::array_function_lookup_t const & array_lookup,
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    array_t const & arguments
) {
    any_regular_t retval;

    if (arguments.size() != 3u)
        throw std::runtime_error("foldr() takes exactly 3 arguments; " + boost::lexical_cast<std::string>(arguments.size()) + " given.");

    name_t f;
    if (!arguments[2].cast(f))
        throw std::runtime_error("The third argument to foldr() must be the name of a function.");

    retval = arguments[1];

    if (arguments[0].type_info() == boost::typeindex::type_id<dictionary_t>()) {
        dictionary_t const & sequence = arguments[0].cast<dictionary_t>();
        fold_dictionary_impl(array_lookup, dictionary_lookup, adam_lookup, f, sequence.rbegin(), sequence.rend(), retval);
    } else if (arguments[0].type_info() == boost::typeindex::type_id<array_t>()) {
        array_t const & sequence = arguments[0].cast<array_t>();
        fold_array_impl(array_lookup, dictionary_lookup, adam_lookup, f, sequence.rbegin(), sequence.rend(), retval);
    } else {
        dictionary_t f_arguments;
        f_arguments[state_k] = retval;
        f_arguments[value_k] = arguments[0];
        auto const adam_function = adam_lookup(f);
        if (adam_function)
            retval = (*adam_function)(array_lookup, dictionary_lookup, adam_lookup, f_arguments);
        else
            retval = dictionary_lookup(f, f_arguments);
    }

    return retval;
}


void add_predefined_functions(
    array_function_map_t & array_function_map,
    virtual_machine_t::array_function_lookup_t const & array_lookup,
    virtual_machine_t::dictionary_function_lookup_t const & dictionary_lookup,
    virtual_machine_t::adam_function_lookup_t const & adam_lookup,
    sheet_t& sheet
) {
    array_function_map["append"_name] = &append;
    array_function_map["prepend"_name] = &prepend;
    array_function_map["insert"_name] = &insert;
    array_function_map["erase"_name] = &erase;
#if 0 // TODO eval() is currently broken.
    array_function_map["parse"_name] = &parse_;
#endif
    array_function_map["size"_name] = &size;
    array_function_map["join"_name] = &join;
    array_function_map["split"_name] = &split;
    array_function_map["to_string"_name] = &to_string;
    array_function_map["to_name"_name] = &to_name;
    array_function_map["print"_name] = &print;
    array_function_map["assert"_name] = &assert_;

    array_function_map["transform"_name] = [&](array_t const & arguments) {
        return transform(array_lookup, dictionary_lookup, adam_lookup, arguments);
    };
    array_function_map["fold"_name] = [&](array_t const & arguments) {
        return fold(array_lookup, dictionary_lookup, adam_lookup, arguments);
    };
    array_function_map["foldr"_name] = [&](array_t const & arguments) {
        return foldr(array_lookup, dictionary_lookup, adam_lookup, arguments);
    };

#if 0 // TODO eval() is currently broken.
    array_function_map["eval"_name] = [&](array_t const & arguments) {
        return eval(sheet, arguments);
    };
#endif
}

} }
