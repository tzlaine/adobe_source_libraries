/*
    Copyright 2013 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
/*************************************************************************************************/

#ifndef ADOBE_TOKEN_HPP
#define ADOBE_TOKEN_HPP

/*************************************************************************************************/

#include <adobe/config.hpp>

#include <adobe/name.hpp>

/*************************************************************************************************/

/*
REVISIT (sparent) : This code is an implementation detail and should be moved into an implementation
namespace but at this moment that would require too many changes.
*/

namespace adobe {

/*************************************************************************************************/

extern static_name_t ifelse_k;

extern static_name_t number_k;
extern static_name_t identifier_k;
extern static_name_t string_k;
extern static_name_t lead_comment_k;
extern static_name_t trail_comment_k;

extern static_name_t semicolon_k;
extern static_name_t comma_k;
extern static_name_t assign_k;
extern static_name_t question_k;
extern static_name_t colon_k;
extern static_name_t open_brace_k;
extern static_name_t close_brace_k;
extern static_name_t open_parenthesis_k;
extern static_name_t close_parenthesis_k;
extern static_name_t dot_k;
extern static_name_t open_bracket_k;
extern static_name_t close_bracket_k;
extern static_name_t at_k;
extern static_name_t is_k;
extern static_name_t to_k;

extern static_name_t add_k;
extern static_name_t subtract_k;
extern static_name_t multiply_k;
extern static_name_t divide_k;
extern static_name_t modulus_k;

extern static_name_t not_k;
extern static_name_t unary_negate_k;

extern static_name_t less_k;
extern static_name_t greater_k;

extern static_name_t and_k;
extern static_name_t or_k;
extern static_name_t less_equal_k;
extern static_name_t greater_equal_k;
extern static_name_t not_equal_k;
extern static_name_t equal_k;

extern static_name_t keyword_k;
extern static_name_t empty_k;
extern static_name_t true_k;
extern static_name_t false_k;

extern static_name_t function_k;
extern static_name_t variable_k;
extern static_name_t index_k;
extern static_name_t array_k;
extern static_name_t dictionary_k;

extern static_name_t bitwise_and_k;
extern static_name_t bitwise_xor_k;
extern static_name_t bitwise_or_k;
extern static_name_t bitwise_rshift_k;
extern static_name_t bitwise_lshift_k;
extern static_name_t bitwise_negate_k;

extern static_name_t const_decl_k;
extern static_name_t decl_k;
extern static_name_t lvalue_k;
extern static_name_t return_k;
extern static_name_t stmt_ifelse_k;
extern static_name_t simple_for_k;
extern static_name_t for_decl_k;
extern static_name_t complex_for_k;
extern static_name_t continue_k;
extern static_name_t break_k;

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
