/*
    Copyright 2013 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
/**************************************************************************************************/

#include <adobe/typeinfo.hpp>

using namespace std;

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

bad_cast::bad_cast() : what_m("bad_cast") {}

bad_cast::bad_cast(const bad_cast& error) : what_m(error.what_m) {}

/*
REVISIT (sparent) : This is only for debugging, but a reliable way to map a type to a name would
be a useful addition.
*/

bad_cast::bad_cast(const boost::typeindex::type_index& from, const boost::typeindex::type_index& to)
    : what_m(string() + "bad_cast: " + from.pretty_name() + " -> " + to.pretty_name()) {}

bad_cast& bad_cast::operator=(const bad_cast& error) {
    what_m = error.what_m;
    return *this;
}

bad_cast::~bad_cast() throw() {}

const char* bad_cast::what() const throw() { return what_m.c_str(); }

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/
